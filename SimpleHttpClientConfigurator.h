#pragma once

#include "foo_subsonic.h"
#include "simplehttpclient.h"
#include "logindlg.h"

class SimpleHttpClientConfigurator {

public:

	/*
	Checks if all required preferences were configured before, and if they are "correct".
	*/
	static BOOL check_preferences() {
		if (Preferences::connect_url_data.is_empty()) {
			console::error("URL cannot be empty!");
			return FALSE;
		}
		else { // validate url
			pfc::string url = Preferences::connect_url_data;
			url = url.toLower();
			if (!url.startsWith("http://") && !url.startsWith("https://")) {
				console::error("Url has to start with http:// or https://");
				return FALSE;
			}

			if (Preferences::proxy_settings_custom_data) {
				url = Preferences::proxy_url_data;
				url = url.toLower();
				if (!url.startsWith("http://") && !url.startsWith("https://") && !url.startsWith("socks://") && !url.startsWith("socks5://")) {
					console::error("Proxy-Address has to start with http:// or socks5://");
					return FALSE;
				}
			}

		}

		return TRUE;
	}

	/*
	Create SimpleHttpClientConfig using parameters found in foobar config.
	*/
	static BOOL createSimpleHttpClientConfigFromPreferences(SimpleHttpClientConfig &cliConfig) {
		if (!SimpleHttpClientConfigurator::check_preferences()) {
			return FALSE;
		}

		if (Preferences::proxy_settings_custom_data) {

			pfc::stringcvt::string_wide_from_utf8 proxyHost(Preferences::proxy_url_data);

			cliConfig.setProxyUrl(proxyHost);
			cliConfig.useProxy = PROXY_CUSTOM;
		}
		else if (Preferences::proxy_settings_system_data) {
			cliConfig.useProxy = PROXY_SYSTEM;
		}
		else {
			cliConfig.useProxy = PROXY_OFF;
		}

		cliConfig.timeoutInSec = Preferences::connect_timeout_data;
		cliConfig.disableCertVerify = Preferences::check_selfsignedcerts_data;

		return TRUE;
	}

	/*
	Turn char to Hex-representation.
	*/
	static char to_hex(char c) {
		return c < 0xa ? '0' + c : 'a' - 0xa + c;
	}

	/*
	Turn string to hex representation.
	*/
	static std::string string_to_hex(const std::string& input)
	{
		static const char* const lut = "0123456789ABCDEF";
		size_t len = input.length();

		std::string output;
		output.reserve(2 * len);
		for (size_t i = 0; i < len; ++i)
		{
			const unsigned char c = input[i];
			output.push_back(lut[c >> 4]);
			output.push_back(lut[c & 15]);
		}
		return output;
	}

	/*
	Encode a URL (which means mask all none ASCII characters).
	*/
	static pfc::string8 url_encode(const char *in) {
		pfc::string8 out;
		out.prealloc(strlen(in) * 3 + 1);

		for (register const char *tmp = in; *tmp != '\0'; tmp++) {
			auto c = static_cast<unsigned char>(*tmp);
			if (isalnum(c)) {
				out.add_char(c);
			}
			else if (isspace(c)) {
				out.add_char('+');
			}
			else {
				out.add_char('%');
				out.add_char(SimpleHttpClientConfigurator::to_hex(c >> 4));
				out.add_char(SimpleHttpClientConfigurator::to_hex(c % 16));
			}
		}
		return out;
	}

	/*
	Build the request URL required for subsonic.
	This will build the URL using the configured server and add the required parameters like client (c), user (u) and password (p).
	*/
	static pfc::string8 buildRequestUrl(const char* restMethod, pfc::string8 urlparams) {

		if (Preferences::password_data.is_empty() || Preferences::username_data.is_empty()) {
			LoginDialog *dlg = new LoginDialog();
			dlg->DoModal(core_api::get_main_window(), LPARAM(0));
		}

		std::string pass = Preferences::password_data.c_str();
		if (Preferences::check_pass_as_hex_data.get_value()) {
			pass = "enc:";
			pass += string_to_hex(Preferences::password_data.c_str());
		}

		pfc::string8 url;
		url << Preferences::connect_url_data;
		url << "/rest/";
		url << restMethod << ".view";
		url << "?v=1.8.0";
		url << "&c=" << COMPONENT_SHORT_NAME;
		url << "&u=" << url_encode(Preferences::username_data);
		url << "&p=" << pass.c_str();

		if (strlen(urlparams) > 0) {
			url << "&" << urlparams;
		}

		return url;
	}

};