#pragma once

#include "foo_subsonic.h"
#include <winhttp.h>


#define PROXY_OFF 0
#define PROXY_SYSTEM 1
#define PROXY_CUSTOM 2

/*
	Container class for configuration of SimpleHTTPClient
*/
class SimpleHttpClientConfig {
public:
	void setProxyUrl(LPCWSTR _url) {
		proxyUrl = _url;
	}

	LPCWSTR getProxyUrl() {
		return proxyUrl.c_str();
	}

	int useProxy = PROXY_OFF;
	unsigned int timeoutInSec = 0;
	bool guarantee_order = FALSE;
	bool disableCertVerify = FALSE;

private:
	std::wstring proxyUrl;
};

/*
	Container class for URL parts required for different calls to WINHTTP.
*/
class SimpleHttpClientUrl {
public:
	void setHttpHost(LPCWSTR host) {
		httpHost = host;		
	}
	void setPath(LPCWSTR _path) {
		path = _path;
	}
	void setfullURL(LPCWSTR _url) {
		fullURL = _url;
	}

	LPCWSTR getHttpHost() {
		return httpHost.c_str();
	}
	LPCWSTR getPath() {
		return path.c_str();
	}

	LPCWSTR getfullURL() {
		return fullURL.c_str();
	}

	bool isHttps = false;
	int httpPort = 80;

private:
	std::wstring httpHost;
	std::wstring path;
	std::wstring fullURL;
};


/*
	SimpleHttpClient class
*/
class SimpleHttpClient {

public:
	SimpleHttpClient(SimpleHttpClientConfig config) {
		m_client_config = config;
	}

	~SimpleHttpClient() {
		if (m_request_handle != NULL) WinHttpCloseHandle(m_request_handle);
		if (m_hSession != NULL) WinHttpCloseHandle(m_hSession);
		if (m_hConnection != NULL) WinHttpCloseHandle(m_hConnection);		
	};

	/*
		Open connection to server.
		Use paramUrl to add parameters to the previously defined URL.
	*/
	unsigned long SimpleHttpClient::open(pfc::string8 paramUrl)
	{
		DWORD access_type;
		LPCWSTR proxy_name;
		SimpleHttpClientUrl* url = new SimpleHttpClientUrl;
		strToSimpleHttpClientUrl(paramUrl, url);

		m_url = url;

		SimpleHttpClientConfig config = client_config();

		if (config.useProxy == PROXY_OFF)
		{
			access_type = WINHTTP_ACCESS_TYPE_NO_PROXY;
			proxy_name = WINHTTP_NO_PROXY_NAME;
		}
		else if (config.useProxy == PROXY_SYSTEM)
		{
			access_type = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
			proxy_name = WINHTTP_NO_PROXY_NAME;
		}
		else
		{
			access_type = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
			std::wstring proxyurl;
			proxyurl = config.getProxyUrl();			

			if (proxyurl.back() == L'/') { // if we have trailing slash, remove it (WINHTTP doesn't like it)
				proxyurl = proxyurl.substr(0, proxyurl.size() - 1); // remove trailing slash
			}

			proxy_name = proxyurl.c_str();
		}

		// Open session.
		m_hSession = WinHttpOpen(
			NULL,
			access_type,
			proxy_name,
			WINHTTP_NO_PROXY_BYPASS,
			WINHTTP_FLAG_ASYNC);
		if (!m_hSession)
		{
			return report_failure("Error opening session");
		}

		// Set timeouts.
		if (config.timeoutInSec > 0) {
			const int milliseconds = 1000 * config.timeoutInSec;
			if (!WinHttpSetTimeouts(m_hSession,
				milliseconds,
				milliseconds,
				milliseconds,
				milliseconds))
			{
				return report_failure("Error setting timeouts");
			}
		}

		if (config.guarantee_order)
		{
			// Set max connection to use per server to 1.
			DWORD maxConnections = 1;
			if (!WinHttpSetOption(m_hSession, WINHTTP_OPTION_MAX_CONNS_PER_SERVER, &maxConnections, sizeof(maxConnections)))
			{
				return report_failure("Error setting options");
			}
		}

		// Open connection.
		m_hConnection = WinHttpConnect(
			m_hSession,
			url->getHttpHost(),
			(INTERNET_PORT)url->httpPort,
			0);

		if (m_hConnection == nullptr)
		{
			return report_failure("Error opening connection");
		}

		return S_OK;
	}

	/*
		Converts a given URL to a SimpleHttpClientUrl by parsing different information from the original string.
	*/
	void SimpleHttpClient::strToSimpleHttpClientUrl(pfc::string8 strUrl, SimpleHttpClientUrl* simpleUrl) {
		pfc::string workstr = strUrl;

		pfc::string workstr_lower = workstr.toLower();

		simpleUrl->isHttps = workstr_lower.startsWith("https://");
		simpleUrl->httpPort = simpleUrl->isHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

		pfc::stringcvt::string_os_from_utf8 fUrl(strUrl);
		simpleUrl->setfullURL(fUrl);

		workstr_lower = workstr_lower.replace("http://", ""); // remove http://
		workstr_lower = workstr_lower.replace("https://", ""); // or remove https://

		int position = workstr_lower.indexOf('/'); // find first slash, which means host name part has ended

		if (position <= 0) { // no slash in URL
			position = workstr_lower.length();
		}

		if (position > 0) {
			pfc::string hostpartWithPort = workstr_lower.subString(0, position); // host and potentially the port
			pfc::string hostpart = hostpartWithPort; // default is no port in url

			if (hostpartWithPort.contains(':')) { // we have a port!
				int portpos = hostpartWithPort.indexOf(':'); // port begin behind the colon
				hostpart = hostpartWithPort.subString(0, portpos); // only the host
				pfc::string port = hostpartWithPort.subString(portpos + 1, hostpartWithPort.length()); // only the port (+1 to remove the colon)

				int iport = std::stol(port.c_str());
				if (port <= 65535 && port > 0) { // port is numeric and is in range (port_max = 65535, port_min = 1)
					simpleUrl->httpPort = iport;
				}
				else { // if port not in range, we ignore it
					console::formatter() << "The specified port '" << port.c_str() << "' is either not numeric or out of range (1-65535)";
				}
			}

			pfc::stringcvt::string_os_from_utf8 hostpart_wide(hostpart.c_str());

			simpleUrl->setHttpHost(hostpart_wide);

			// as we need to use the original part behind the hostname (pathes are case sensitive), we have to add the length of the http-part
			if (!simpleUrl->isHttps) {
				position += sizeof("http://");
			}
			else {
				position += sizeof("https://");
			}

			pfc::string remaining = workstr.subString(position, workstr.length()); // we take the original entered text instead of the lower case one
			pfc::stringcvt::string_os_from_utf8 path_wide(remaining.c_str());

			simpleUrl->setPath(path_wide);
		}
		else {
			pfc::stringcvt::string_os_from_utf8 host_wide(workstr_lower.c_str());
			pfc::stringcvt::string_os_from_utf8 hostfull_wide(workstr.c_str());
			simpleUrl->setHttpHost(host_wide);
			simpleUrl->setPath(L"");
			simpleUrl->setfullURL(hostfull_wide);
		}
	}

	/*
		Try to figure out the proxy configuration used in the system.
		I assume that the client is used in an desktop application, so I guess the Internet Explorer proxy settings 
		are the settings the user wants to have.

		There a multiple ways of defining a proxy. 
		The first one is to use PROXY_AUTODETECT, which uses WPAD protocol (which I've never seen in action).

		The second is a Proxy URL is configured. This URL points to proxy.pac file (which has to be ECMAScript) which then will return the proxy to use for this particular URL.

		The third option is using a manual configured proxy. This can either be "one for all" or different for each protocol (http, https, ftp ..).
		If the third option is configured, I take the string found from the IE proxy settings and use this unmodified, assuming that the syntax returned by
		WinHttpGetIEProxyConfigForCurrentUser is valid for WINHTTP as well (which seems to be the case in my testings).
	*/
	bool SimpleHttpClient::proxyRequired(HINTERNET session, WINHTTP_PROXY_INFO* info) {
		WINHTTP_AUTOPROXY_OPTIONS autoproxy_options;
		memset(&autoproxy_options, 0, sizeof(WINHTTP_AUTOPROXY_OPTIONS));

		autoproxy_options.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT; // default is to use WPAD proxy auto detection

		WINHTTP_CURRENT_USER_IE_PROXY_CONFIG pIEProxyConfig;
		memset(&pIEProxyConfig, 0, sizeof(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG));
		WinHttpGetIEProxyConfigForCurrentUser(&pIEProxyConfig);

		if (!pIEProxyConfig.fAutoDetect) { // autodetection is disabled, maybe another proxy setup
			if (pIEProxyConfig.lpszAutoConfigUrl != NULL && wcslen(pIEProxyConfig.lpszAutoConfigUrl) > 0) { // proxy config url (proxy pac) found
				autoproxy_options.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
				autoproxy_options.lpszAutoConfigUrl = pIEProxyConfig.lpszAutoConfigUrl;
			}
			else if (pIEProxyConfig.lpszProxy != NULL && wcslen(pIEProxyConfig.lpszProxy) > 0) { // manual proxy config
				info->lpszProxy = pIEProxyConfig.lpszProxy;
				info->dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
				// proxy is configured manually, so we dont need to query WinHttpGetProxyForUrl, we can just return the static configuration
				return true;
			}
			else { // no proxy setup at all
				return false;
			}
		}

		autoproxy_options.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
		autoproxy_options.fAutoLogonIfChallenged = TRUE;
		auto result = WinHttpGetProxyForUrl(
			m_hSession,
			m_url->getfullURL(),
			&autoproxy_options,
			info);

		return result ? true : false;
	}




	/*
		Send the request to the server and return the response in &responsebuffer.
	*/
	void SimpleHttpClient::send_request(char* &responsebuffer, size_t &buffSize) {

		WINHTTP_PROXY_INFO info;
		memset(&info, 0, sizeof(WINHTTP_PROXY_INFO));

		bool proxy_info_required = false;
		const auto& config = client_config();
		if (config.useProxy == PROXY_SYSTEM)
		{
				proxy_info_required = proxyRequired(m_hSession, &info);
		}

		// Need to form uri path, query, and fragment for this request.
		// Make sure to keep any path that was specified with the uri when the http_client was created.
		// Open the request.
		m_request_handle = WinHttpOpenRequest(
			m_hConnection,
			L"GET",
			m_url->getPath(),
			L"HTTP/1.1",
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_ESCAPE_DISABLE | (m_url->isHttps ? WINHTTP_FLAG_SECURE : 0));
		if (m_request_handle == nullptr)
		{
			report_failure("WinHttpOpenRequest Error");
			return;
		}

		if (proxy_info_required)
		{
			auto result = WinHttpSetOption(
				m_request_handle,
				WINHTTP_OPTION_PROXY,
				&info,
				sizeof(WINHTTP_PROXY_INFO));
			if (!result)
			{
				report_failure("Setting proxy options");
				return;
			}
		}

		// Check to turn off server certificate verification.
		if (config.disableCertVerify)
		{
			DWORD data = SECURITY_FLAG_IGNORE_UNKNOWN_CA
				| SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
				| SECURITY_FLAG_IGNORE_CERT_CN_INVALID
				| SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;

			auto result = WinHttpSetOption(
				m_request_handle,
				WINHTTP_OPTION_SECURITY_FLAGS,
				&data,
				sizeof(data));
			if (!result)
			{
				report_failure("Setting ignore server certificate verification");
				return;
			}
		}

		if (!WinHttpSendRequest(m_request_handle, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
			report_failure("Send Request");
			return;
		}
  
		unsigned long size;
		if (!WinHttpReceiveResponse(m_request_handle, NULL) || !WinHttpQueryDataAvailable(m_request_handle, &size)) {
			report_failure("Receiving Response");
			return;
		}
		
		while (size > 0) {
			char *tmp = new char[size];
			unsigned long downloaded;
			WinHttpReadData(m_request_handle, tmp, size, &downloaded);
			//responsebuffer.add_string(tmp, downloaded);

			if (responsebuffer == NULL) {
				responsebuffer = new char[downloaded];
				memcpy(responsebuffer, tmp, downloaded);
				buffSize = downloaded;
			}
			else {
				char *tmpBuffer = new char[downloaded + buffSize];
				memcpy(tmpBuffer, responsebuffer, buffSize);
				memcpy(&tmpBuffer[buffSize], tmp, downloaded);

				delete[] responsebuffer;
				responsebuffer = tmpBuffer;
				buffSize += downloaded;
			}

			delete[] tmp;
			WinHttpQueryDataAvailable(m_request_handle, &size);

		};
		return;
	}

	
protected:

private:

	SimpleHttpClientConfig m_client_config;

	HINTERNET m_hSession = NULL;
	HINTERNET m_hConnection = NULL;
	HINTERNET m_request_handle = NULL;

	SimpleHttpClientUrl* m_url;

	const SimpleHttpClientConfig& client_config() const
	{
		return m_client_config;
	}
	unsigned long report_failure(const char* errorMessage)
	{
		DWORD err = GetLastError();
		console::printf("SimpleHttpClient [ERR] (%i): %s", err, errorMessage);

		return err;
	}
};

