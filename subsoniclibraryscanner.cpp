#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"

#include "album.h"
#include "ui.h"
#include "simplehttpclient.h"
#include "xmlhelper.h"

#include <winhttp.h>

using namespace foo_subsonic; 
using namespace XmlHelper;

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
void SubsonicLibraryScanner::getAlbumList(threaded_process_status &p_status, std::list<Album>* albumList, int size, int offset) {
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
				unsigned int counter = 0;
				for (TiXmlElement* e = firstChild->FirstChildElement("album"); e != NULL; e = e->NextSiblingElement("album")) {
					Album a;

					a.set_artist(XmlStrOrDefault(e, "artist", ""));
					a.set_title(XmlStrOrDefault(e, "title", "")); // use title instead of album, otherwise multi disc albums were mixed

					a.set_genre(XmlStrOrDefault(e, "genre", ""));
					a.set_year(XmlStrOrDefault(e, "year", ""));

					a.set_coverArt(XmlStrOrDefault(e, "coverArt", ""));

					a.set_artistid(XmlStrOrDefault(e, "artistId", "0"));
					a.set_id(XmlStrOrDefault(e, "id", "0"));
					a.set_parentid(XmlStrOrDefault(e, "parent", "0"));
					
					a.set_duration(XmlIntOrDefault(e, "duration", 0));

					pfc::string8 tmp = "Found artist=";
					tmp << a.get_artist() << ", album=" << a.get_title() << ", coverArt=" << a.get_coverArt();

					console::print(tmp);

					getAlbumTracks(&a);

					albumList->push_back(a);

					counter++;
					p_status.set_progress(counter, offset + 1000);
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
					t.set_parentId(XmlStrOrDefault(e, "parent", "0"));
					t.set_bitrate(XmlIntOrDefault(e, "bitrate", 0));
					t.set_size(XmlIntOrDefault(e, "size", 0));
					t.set_year(XmlStrOrDefault(e, "year", "0"));

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

void SubsonicLibraryScanner::getPlaylists(threaded_process_status &p_status, std::list<Playlist>* playlists) {
	//TODO: Retrieve playlist
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
void SubsonicLibraryScanner::retrieveAllAlbums(HWND window, threaded_process_status &p_status) {
	
	// TODO: increase after debug (max is 500)
	int size = 20;
	int offset = 0;

	getAlbumList(p_status, &albList, size, offset);

	SetLastError(ERROR_SUCCESS); // reset GLE before SendMessage call

	// signal the main window that the thread has done fetching
	SendMessage(window, ID_CONTEXT_UPDATECATALOG_DONE, HIWORD(0), LOWORD(0));
	DWORD lastError = GetLastError();
	if (lastError != ERROR_SUCCESS) {
		console::printf("Got error while sending message to window: %i", lastError);
	}
	
}

/*
	Tries to retrieve all playlists stored for the current user (or being public).
*/
void SubsonicLibraryScanner::retrieveAllPlaylists(HWND window) {
	// TODO: Get playlists
}

/*
	Return pointer to the list of fetched albums.
*/
std::list<Album>* SubsonicLibraryScanner::getFetchedAlbumList() {
	return &albList;
}