#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"

#include "album.h"
#include "ui.h"
#include "simplehttpclient.h"

#include <winhttp.h>

using namespace foo_subsonic; 

/*
	Retrieve a int value of a XML Element, or return a default value if element could not be found/read.
*/
int XmlIntOrDefault(TiXmlElement* element, const char* attribute, unsigned int default) {
	auto temp = default;
	if (element->QueryUnsignedAttribute(attribute, &temp) == TIXML_SUCCESS) {
		return temp;
	}
	else {
		return default;
	}
}

/*
	Retrieve a String value of a XML Element, or return a default value if element could not be found/read.
*/
pfc::string8 XmlStrOrDefault(TiXmlElement* element, const char* attribute, const char* default) {
	auto tmp = element->Attribute(attribute);
	return tmp == nullptr ? "" : tmp;
}

/*
	Turn char to Hex-representation.
*/
char to_hex(char c) {
	return c < 0xa ? '0' + c : 'a' - 0xa + c;
}

/*
	Encode a URL (which means mask all none ASCII characters).
*/
pfc::string8 url_encode(const char *in) {
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
			out.add_char(to_hex(c >> 4));
			out.add_char(to_hex(c % 16));
		}
	}
	return out;
}

/*
	Build the request URL required for subsonic.
	This will build the URL using the configured server and add the required parameters like client (c), user (u) and password (p).
*/
pfc::string8 buildRequestUrl(const char* restMethod, pfc::string8 urlparams) {
	pfc::string8 url;
	url << Preferences::connect_url_data;
	url << "/rest/";
	url << restMethod << ".view";
	url << "?v=1.8.0";
	url << "&c=" << COMPONENT_SHORT_NAME;
	url << "&u=" << Preferences::username_data;
	url << "&p=" << url_encode(Preferences::password_data);

	if (sizeof(urlparams) > 0) {
		url << "&" << urlparams;
	}

	return url;
}

/*
  Checks if all required preferences were configured before, and if they are "correct".
*/
BOOL check_preferences() {
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
	Setup everything and connect to server, calling given @restMethod.
*/
BOOL SubsonicLibraryScanner::connectAndGet(TiXmlDocument* doc, const char* restMethod, const char* urlparams) {

	if (!check_preferences()) {
		return FALSE;
	}
	pfc::string8 url = buildRequestUrl(restMethod, urlparams);
	
	BOOL isHttps = FALSE;

	SimpleHttpClientConfig cliConfig;

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

	SimpleHttpClient cli = SimpleHttpClient(cliConfig);

	pfc::string8 buffer;

	cli.open(url.c_str());
	cli.send_request(buffer);	
	
	// Parse	
	doc->Parse(buffer, 0, TIXML_ENCODING_UTF8);
	return checkForError(doc);
}

/*
	Retrieve @size albums from server starting at @offset.
*/
void SubsonicLibraryScanner::getAlbumList(std::list<Album>* albumList, int size, int offset) {
	pfc::string8 urlparms;
	urlparms = "type=alphabeticalByName&size=";
	urlparms << size << "&offset=" << offset;

	TiXmlDocument doc;
	
	if (!connectAndGet(&doc, "getAlbumList2", urlparms)) { // error occoured, we are done
		return;
	}

	if (albumList == nullptr) {
		console::error("Albumlist should not be null!");
		return;
	}

	TiXmlElement* rootNode = doc.FirstChildElement("subsonic-response");
	if (rootNode) {
		TiXmlElement* firstChild = rootNode->FirstChildElement("albumList2");
		if (firstChild) {
			if (!rootNode->FirstChildElement("albumList2")->NoChildren()) { // list is not empty
				for (TiXmlElement* e = firstChild->FirstChildElement("album"); e != NULL; e = e->NextSiblingElement("album")) {
					Album a;

					a.set_artist(XmlStrOrDefault(e, "artist", ""));
					a.set_title(XmlStrOrDefault(e, "title", "")); // use title instead of album, otherwise multi disc albums were mixed

					a.set_genre(XmlStrOrDefault(e, "genre", ""));
					a.set_year(XmlStrOrDefault(e, "year", ""));

					a.set_coverArt(XmlStrOrDefault(e, "coverArt", ""));

					a.set_artistid(XmlIntOrDefault(e, "artistId", 0));
					a.set_id(XmlIntOrDefault(e, "id", 0));
					a.set_parentid(XmlIntOrDefault(e, "parent", 0));
					
					a.set_duration(XmlIntOrDefault(e, "duration", 0));

					pfc::string8 tmp = "Found artist=";
					tmp << a.get_artist() << ", album=" << a.get_title() << ", coverArt=" << a.get_coverArt();

					console::print(tmp);

					// TODO: Fetch titles
					getAlbumTracks(&a);

					//albumList->add(&a);
					albumList->push_back(a);
				}
				// recurse until empty list is returned
				offset += 500;
				// TODO: Later do recursing!    
				// getAlbumList(p_abort, albumList, 500, offset);

			}
		}
	}
}

/*
	Get all tracks for the album specified in @album.
*/
void SubsonicLibraryScanner::getAlbumTracks(Album *album) {
	pfc::string8 urlparms;
	urlparms = "id=";
	urlparms << album->get_id();

	TiXmlDocument doc;
		
	if (!connectAndGet(&doc, "getAlbum", urlparms)) { // error occoured, we are done
		return;
	}

	TiXmlElement* rootNode = doc.FirstChildElement("subsonic-response");
	if (rootNode) {
		TiXmlElement* firstChild = rootNode->FirstChildElement("album");
		if (firstChild) {
			if (!rootNode->FirstChildElement("album")->NoChildren()) { // list is not empty
				
				for (TiXmlElement* e = firstChild->FirstChildElement("song"); e != NULL; e = e->NextSiblingElement("song")) {
					Track t = Track();
					
					t.set_duration(XmlIntOrDefault(e, "duration", 0));
					t.set_tracknumber(XmlIntOrDefault(e, "track", 0));
					t.set_parentId(XmlIntOrDefault(e, "parent", 0));
					t.set_bitrate(XmlIntOrDefault(e, "bitrate", 0));
					t.set_size(XmlIntOrDefault(e, "size", 0));

					t.set_id(XmlStrOrDefault(e, "id", ""));
					t.set_artist(XmlStrOrDefault(e, "artist", ""));
					t.set_album(XmlStrOrDefault(e, "album", ""));

					t.set_genre(XmlStrOrDefault(e, "genre", ""));
					t.set_contentType(XmlStrOrDefault(e, "contentType", ""));
					t.set_coverArt(XmlStrOrDefault(e, "coverArt", "0"));
					t.set_title(XmlStrOrDefault(e, "title", ""));
					t.set_suffix(XmlStrOrDefault(e, "suffix", ""));

					// build and add url for streaming
					pfc::string8 idparam = "id=";
					idparam << t.get_id();

					pfc::string8 streamUrl = buildRequestUrl("stream", idparam);
					t.set_streamUrl(streamUrl);

					album->addTrack(t);					
					
				}
			}
		}
	}
}


/*
  Checks if returned Response of Subsonic is an error message.
  Returns FALSE if an error occured, TRUE otherwise
*/
bool SubsonicLibraryScanner::checkForError(TiXmlDocument* xml) {
	TiXmlElement* rootNode = xml->FirstChildElement("subsonic-response");
	if (rootNode) {
		const char* status = rootNode->Attribute("status");
		if (status == "failed") { // this is failed status
			const char* errorCode = rootNode->FirstChildElement("error")->Attribute("code");
			const char* errorMsg = rootNode->FirstChildElement("error")->Attribute("message");
			parsingError(errorCode, errorMsg);
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

/*
	If subsonic returned some kind of error, log it to foobar console.
*/
void SubsonicLibraryScanner::parsingError(const char* message, const char* errCode) {
	console::printf("SubSonic Remote Error (ErrCode=%s): %s", errCode, message);
}

/*
  Tries to query all albums from subsonic server.
  The result will be stored in the given albumList reference.
*/
void SubsonicLibraryScanner::scan(HWND window) {
	
	// TODO: increase after debug (max is 500)
	int size = 20;
	int offset = 0;

	getAlbumList(&albList, size, offset);

	SetLastError(ERROR_SUCCESS); // reset GLE before SendMessage call

	// signal the main window that the thread has done fetching
	SendMessage(window, ID_CONTEXT_UPDATEDONE, HIWORD(0), LOWORD(0));
	DWORD lastError = GetLastError();
	if (lastError != ERROR_SUCCESS) {
		console::printf("Got error while sending message to window: %i", lastError);
	}
	
}

/*
	Return pointer to the list of fetched albums.
*/
std::list<Album>* SubsonicLibraryScanner::getFetchedAlbumList() {
	return &albList;
}