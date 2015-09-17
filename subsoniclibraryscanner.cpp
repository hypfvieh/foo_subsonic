#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"

#include "album.h"
#include "ui.h"
#include "simplehttpclient.h"
#include "xmlhelper.h"
#include "xmlcachedb.h"

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
			if (!firstChild->NoChildren()) { // list is not empty
				unsigned int counter = 0;
				for (TiXmlElement* e = firstChild->FirstChildElement("album"); e != NULL; e = e->NextSiblingElement("album")) {
					Album a;

					parseAlbumInfo(e, &a);

					pfc::string8 tmp = "Found artist=";
					tmp << a.get_artist() << ", album=" << a.get_title() << ", coverArt=" << a.get_coverArt();

					console::print(tmp);

					getAlbumTracks(&a);

					albumList->push_back(a);

					counter++;
					p_status.set_progress(counter, offset + 1000);
				}
				// recurse until empty list is returned
				offset += SUBSONIC_MAX_ALBUMLIST_SIZE;
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
			if (!firstChild->NoChildren()) { // list is not empty
				
				for (TiXmlElement* e = firstChild->FirstChildElement("song"); e != NULL; e = e->NextSiblingElement("song")) {
					Track t = Track();					
					parseTrackInfo(e, &t);
					album->addTrack(t);										
				}
			}
		}
	}
}

/*
	Retrieve all playlists from subsonic server.
*/
void SubsonicLibraryScanner::getPlaylists(threaded_process_status &p_status, std::list<Playlist>* playlists) {
	//TODO: Retrieve playlist

	TiXmlDocument doc;

	if (!connectAndGet(&doc, "getPlaylists", "")) { // error occoured, we are done
		return;
	}

	if (playlists == nullptr) {
		console::error("Playlists should not be null!");
		return;
	}

	TiXmlElement* rootNode = doc.FirstChildElement("subsonic-response");
	if (rootNode) {
		TiXmlElement* firstChild = rootNode->FirstChildElement("playlists");
		if (firstChild) {
			if (firstChild->NoChildren()) { // list is not empty
				for (TiXmlElement* e = firstChild->FirstChildElement("playlist"); e != NULL; e = e->NextSiblingElement("playlist")) {
					Playlist p;

					p.set_comment(XmlStrOrDefault(e, "comment", ""));
					p.set_coverArt(XmlStrOrDefault(e, "covertArt", ""));
					p.set_duration(XmlIntOrDefault(e, "duration", 0));
					p.set_id(XmlStrOrDefault(e, "id", "0"));
					p.set_isPublic(XmlBoolOrDefault(e, "public", false));
					p.set_name(XmlStrOrDefault(e, "name", "No Name given"));
					p.set_owner(XmlStrOrDefault(e, "owner", ""));
					p.set_songcount(XmlIntOrDefault(e, "songCount", 0));

					// retrieve playlist entries
					getPlaylistEntries(&p);
					playlists->push_back(p);
				}
			}
		}
	}
}

/*
	Get all entries (tracks) for the playlist specified in @playlist.
*/
void SubsonicLibraryScanner::getPlaylistEntries(Playlist *playlist) {
	pfc::string8 urlparms;
	urlparms = "id=";
	urlparms << playlist->get_id();

	TiXmlDocument doc;

	if (!connectAndGet(&doc, "getPlaylist", urlparms)) { // error occoured, we are done
		return;
	}

	TiXmlElement* rootNode = doc.FirstChildElement("subsonic-response");
	if (rootNode) {
		TiXmlElement* firstChild = rootNode->FirstChildElement("playlist");
		if (firstChild) {
			if (!firstChild->NoChildren()) { // list is not empty

				for (TiXmlElement* e = firstChild->FirstChildElement("entry"); e != NULL; e = e->NextSiblingElement("entry")) {
					Track t = Track();
					parseTrackInfo(e, &t);
					playlist->addTrack(t);
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
void SubsonicLibraryScanner::retrieveAllAlbums(HWND window, threaded_process_status &p_status) {
	
	// TODO: increase after debug (max is 500)
	int size = 2;//SUBSONIC_MAX_ALBUMLIST_SIZE;
	int offset = 0;

	XmlCacheDb::getInstance()->getAllAlbums()->clear(); // remove old entries first

	getAlbumList(p_status, XmlCacheDb::getInstance()->getAllAlbums(), size, offset);

	SetLastError(ERROR_SUCCESS); // reset GLE before SendMessage call

	// save our new results
	XmlCacheDb::getInstance()->saveAlbums();

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
void SubsonicLibraryScanner::retrieveAllPlaylists(HWND window, threaded_process_status &p_status) {
	XmlCacheDb::getInstance()->getAllPlaylists()->clear(); // remove old entries
	getPlaylists(p_status, XmlCacheDb::getInstance()->getAllPlaylists());

	SetLastError(ERROR_SUCCESS); // reset GLE before SendMessage call

	// save our new results
	XmlCacheDb::getInstance()->savePlaylists();

	// signal the main window that the thread has done fetching
	SendMessage(window, ID_CONTEXT_UPDATEPLAYLIST_DONE, HIWORD(0), LOWORD(0));
	DWORD lastError = GetLastError();
	if (lastError != ERROR_SUCCESS) {
		console::printf("Got error while sending message to window: %i", lastError);
	}
}