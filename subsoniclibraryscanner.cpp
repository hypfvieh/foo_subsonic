#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"

#include "album.h"
#include "ui.h"
#include "simplehttpclient.h"
#include "xmlhelper.h"
#include "sqliteCacheDb.h"
#include "SimpleHttpClientConfigurator.h"

#include <winhttp.h>

using namespace foo_subsonic; 
using namespace XmlHelper;

/*
	Setup everything and connect to server, calling given @restMethod.
*/
BOOL SubsonicLibraryScanner::connectAndGet(TiXmlDocument* doc, const char* restMethod, const char* urlparams) {

	if (!SimpleHttpClientConfigurator::check_preferences()) {
		return FALSE;
	}
	pfc::string8 url = SimpleHttpClientConfigurator::buildRequestUrl(restMethod, urlparams);
	uDebugLog() << "connectAndGet->URL: " << url;
	SimpleHttpClientConfig cliConfig;

	if (SimpleHttpClientConfigurator::createSimpleHttpClientConfigFromPreferences(cliConfig)) {

		SimpleHttpClient cli = SimpleHttpClient(cliConfig);

		char* buffer = NULL;
		size_t buffSize = 0;

		cli.open(url.c_str());
		cli.send_request(buffer, buffSize);	
			
		// Parse	
		doc->Parse(buffer, 0, TIXML_ENCODING_UTF8);
		return checkForError(doc);
	}
	else {
		console::error("Error while configuring HTTP(s) connection");
	}
	return FALSE;
}

/*
	Retrieve @size albums from server starting at @offset.
*/
void SubsonicLibraryScanner::getAlbumList(threaded_process_status &p_status, int size, int offset, abort_callback &p_abort) {
	pfc::string8 urlparms;
	urlparms = "type=alphabeticalByName&size=";
	urlparms << size << "&offset=" << offset;

	pfc::string8 progressText = "Retrieving Album List...";

	TiXmlDocument doc;
	

	if (p_abort.is_aborting()) {
		console::print("Albumlist retrieval aborted: Stop");
		progressText = "Canceling";
		p_status.set_item(progressText);
		return;
	}

	if (!connectAndGet(&doc, "getAlbumList", urlparms)) { // error occoured, we are done
		return;
	}

	TiXmlElement* rootNode = doc.FirstChildElement("subsonic-response");
	if (rootNode) {
		TiXmlElement* firstChild = rootNode->FirstChildElement("albumList");
		if (firstChild) {
			if (!firstChild->NoChildren()) { // list is not empty
				unsigned int counter = 0;
				for (TiXmlElement* e = firstChild->FirstChildElement("album"); e != NULL; e = e->NextSiblingElement("album")) {
					Album a;

					parseAlbumInfo(e, &a);

					pfc::string8 tmp = "Found artist=";
					tmp << a.get_artist() << ", album=" << a.get_title() << ", coverArt=" << a.get_coverArt();

					progressText = "Retrieving Album List... || ";
					progressText << a.get_artist() << " => " << a.get_title();
					p_status.set_item(progressText);

					console::print(tmp);

					getAlbumTracks(&a, p_abort);
					if (p_abort.is_aborting()) {
						console::print("Albumlist retrieval aborted: Stop loop");
						progressText = "Canceling";
						p_status.set_item(progressText);

						break;
					}
					SqliteCacheDb::getInstance()->addAlbum(a);

					counter++;

					if (offset == 0) {
						p_status.set_progress(counter, SUBSONIC_MAX_ALBUMLIST_SIZE);
					}
					else if (offset > SUBSONIC_MAX_ALBUMLIST_SIZE) {
						int p = offset / SUBSONIC_MAX_ALBUMLIST_SIZE;
						p_status.set_progress(counter*p, offset);
					}					
					else {
						p_status.set_progress(counter, offset);
					}
					
				}
				// recurse until empty list is returned
				offset += SUBSONIC_MAX_ALBUMLIST_SIZE;
				p_status.set_progress(1, 1);

				getAlbumList(p_status, size, offset, p_abort);
			}
		}
	}
}

/*
	Get all tracks for the album specified in @album.
*/
void SubsonicLibraryScanner::getAlbumTracks(Album *album, abort_callback &p_abort) {
	pfc::string8 urlparms;
	urlparms = "id=";
	urlparms << album->get_id();

	if (p_abort.is_aborting()) {
		console::print("Album Track retrieval aborted: Stop");
		return;
	}

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
					Track* t = new Track();					
					parseTrackInfo(e, t);
					if (p_abort.is_aborting()) {
						console::print("Album Track retrieval aborted: Stop loop");
						break;
					}
					album->addTrack(t);										
				}
			}
		}
	}
}

void SubsonicLibraryScanner::getAlbumAndTracksByArtistId(const char *artist_id, abort_callback &p_abort) {
	pfc::string8 urlparms;
	urlparms = "id=";
	urlparms << artist_id;

	if (p_abort.is_aborting()) {
		console::print("Artist retrieval aborted: Stop");
		return;
	}

	TiXmlDocument doc;

	if (!connectAndGet(&doc, "getArtist", urlparms)) { // error occoured, we are done
		return;
	}

	TiXmlElement* rootNode = doc.FirstChildElement("subsonic-response");
	if (rootNode) {
		TiXmlElement* firstChild = rootNode->FirstChildElement("artist");
		if (firstChild) {
			if (!firstChild->NoChildren()) { // list is not empty

				for (TiXmlElement* e = firstChild->FirstChildElement("album"); e != NULL; e = e->NextSiblingElement("album")) {
					
					Album a;
					SqliteCacheDb::getInstance()->getAlbumById(XmlStrOrDefault(e, "id", "unknown_error"), a);
					a.getTracks()->clear(); // remove previously stored tracks
					uDebugLog() << "Updating Album: " << a.get_title();

					getAlbumTracks(&a, p_abort); // add the new tracks
				}
			}
		}
	}
}

/*
	Retrieve all playlists from subsonic server.
*/
void SubsonicLibraryScanner::getPlaylists(threaded_process_status &p_status, abort_callback &p_abort) {

	TiXmlDocument doc;

	pfc::string8 progressText;

	if (p_abort.is_aborting()) {
		progressText = "Canceling";
		p_status.set_item(progressText);
		console::print("Playlist retrieval aborted: Stop");
		return;
	}

	if (!connectAndGet(&doc, "getPlaylists", "")) { // error occoured, we are done
		return;
	}

	TiXmlElement* rootNode = doc.FirstChildElement("subsonic-response");
	if (rootNode) {
		TiXmlElement* firstChild = rootNode->FirstChildElement("playlists");
		if (firstChild) {
			if (!firstChild->NoChildren()) { // list is not empty
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

					progressText << "Retrieving playlists... | Name: ";
					progressText << p.get_name();
					p_status.set_item(progressText);

					// retrieve playlist entries
					getPlaylistEntries(&p, p_abort);
					if (p_abort.is_aborting()) {
						progressText = "Canceling";
						p_status.set_item(progressText);
						console::print("Playlist retrieval aborted: Stop loop");
						break;
					}
					SqliteCacheDb::getInstance()->addPlaylist(p);
				}
			}
		}
	}
}

/*
	Get all entries (tracks) for the playlist specified in @playlist.
*/
void SubsonicLibraryScanner::getPlaylistEntries(Playlist *playlist, abort_callback &p_abort) {
	pfc::string8 urlparms;
	urlparms = "id=";
	urlparms << playlist->get_id();

	if (p_abort.is_aborting()) {
		console::print("Playlist Track retrieval aborted: stop");
		return;
	}

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
					Track* t = new Track();
					parseTrackInfo(e, t);
					playlist->addTrack(t);
					if (p_abort.is_aborting()) {
						console::print("Playlist Track retrieval aborted: stop loop");
						break;
					}
				}
			}
		}
	}
}


void SubsonicLibraryScanner::getSearchResults(const char* urlParams) {

	pfc::string8 params = "query=";
	params << urlParams;

	TiXmlDocument doc;

	if (!connectAndGet(&doc, "search2", params)) { // error occoured, we are done
		return;
	}

	TiXmlElement* rootNode = doc.FirstChildElement("subsonic-response");
	if (rootNode) {
		TiXmlElement* firstChild = rootNode->FirstChildElement("searchResult2");
		if (firstChild) {
			if (!firstChild->NoChildren()) { // list is not empty

				for (TiXmlElement* e = firstChild->FirstChildElement("song"); e != NULL; e = e->NextSiblingElement("song")) {
					Track* t = new Track();
					parseTrackInfo(e, t);
					SqliteCacheDb::getInstance()->addSearchResult(t);
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
		if (strcmp(status, "failed") == 0) { // this is failed status		 
			const char* errorCode = rootNode->FirstChildElement("error")->Attribute("code");
			const char* errorMsg = rootNode->FirstChildElement("error")->Attribute("message");
			parsingError(errorCode, errorMsg);

			if (strcmp(errorCode, "40") == 0 || strcmp(errorCode, "50") == 0) { // errorCode 40 = wrong user/pass; 50 = user has no permission
				// if no permission or invalid login data, reset the data to "none" so the user can enter new data the next time
				Preferences::username_data = "";
				Preferences::password_data = "";
			}

			std::string tmp = "Error while querying subsonic server, response was:\r\n\r\n";
			tmp += "ErrorCode: ";
			tmp += errorCode;
			tmp += "\r\nMessage:";
			tmp += errorMsg;

			size_t strSize = tmp.size() + 1;
			wchar_t* wChar = new wchar_t[strSize];

			size_t outSize;
			mbstowcs_s(&outSize, wChar, strSize, tmp.c_str(), strSize - 1);

			MessageBox(core_api::get_main_window(), wChar, L"Query Error", MB_OK | MB_ICONERROR);
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

/*
	If subsonic returned some kind of error, log it to foobar console.
*/
void SubsonicLibraryScanner::parsingError(const char* errCode, const char* message) {
	console::printf("SubSonic Remote Error (ErrCode=%s): %s", errCode, message);
}

/*
  Tries to query all albums from subsonic server.
  The result will be stored in the given albumList reference.
*/
void SubsonicLibraryScanner::retrieveAllAlbums(HWND window, threaded_process_status &p_status, abort_callback &p_abort) {
	
	int size = SUBSONIC_MAX_ALBUMLIST_SIZE;
	int offset = 0;

	SqliteCacheDb::getInstance()->getAllAlbums()->clear(); // remove old entries first

	pfc::string8 progressText;
	progressText = "Retrieving catalog";

	p_status.set_progress_secondary(1, 3);
	p_status.set_item(progressText);
	getAlbumList(p_status, size, offset, p_abort);

	// save our new results
	p_status.set_title("Saving catalog to cache database (this can take some time)");
	progressText = "Updating cache (this can take some time)";
	p_status.set_item(progressText);
	SqliteCacheDb::getInstance()->saveAlbums(p_status, p_abort);

	// signal the main window that the thread has done fetching
	SendMessage(window, ID_CONTEXT_UPDATECATALOG_DONE, HIWORD(0), LOWORD(0));
}

/*
	Tries to retrieve all playlists stored for the current user (or being public).
*/
void SubsonicLibraryScanner::retrieveAllPlaylists(HWND window, threaded_process_status &p_status, abort_callback &p_abort) {
	SqliteCacheDb::getInstance()->getAllPlaylists()->clear(); // remove old entries

	pfc::string8 progressText;
	progressText = "Retrieving playlists";

	p_status.set_progress_secondary(1, 3);
	p_status.set_item(progressText);
	
	getPlaylists(p_status, p_abort);

	SetLastError(ERROR_SUCCESS); // reset GLE before SendMessage call

	// save our new results
	p_status.set_title("Saving playlists to cache database (this can take some time)");
	progressText = "Updating Cache (this can take some time)";
	p_status.set_item(progressText);
	SqliteCacheDb::getInstance()->savePlaylists(p_status, p_abort);

	// signal the main window that the thread has done fetching
	SendMessage(window, ID_CONTEXT_UPDATEPLAYLIST_DONE, HIWORD(0), LOWORD(0));
}

void SubsonicLibraryScanner::retrieveAllSearchResults(HWND window, threaded_process_status &p_status, const char* url) {	
	getSearchResults(url);

	SendMessage(window, ID_SEARCH_DONE, HIWORD(0), LOWORD(0));
}

void SubsonicLibraryScanner::retrieveArtistUpdate(HWND window, threaded_process_status &p_status, abort_callback &p_abort, const char* artistId) {

	pfc::string8 progressText;
	progressText = "Updating Artist record";

	p_status.set_progress_secondary(1, 3);
	p_status.set_item(progressText);

	getAlbumAndTracksByArtistId(artistId, p_abort);

	p_status.set_title("Updating artist record in cache database (this can take some time)");
	progressText = "Updating Cache (this can take some time)";
	p_status.set_item(progressText);
	SqliteCacheDb::getInstance()->saveAlbums(p_status, p_abort);

	SendMessage(window, ID_CONTEXT_UPDATEARTIST_DONE, HIWORD(0), LOWORD(0));
}