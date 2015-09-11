#include "foo_subsonic.h"
#include "xmlcachedb.h"
#include "xmlhelper.h"

using namespace XmlHelper;

XmlCacheDb* XmlCacheDb::instance = NULL;

XmlCacheDb::XmlCacheDb() {
	internalDoc = TiXmlDocument("foo_subsonic_cache.xml");
	if (!internalDoc.LoadFile()) {
		console::print("Offline cache no found, creating new");
	}
	else {
		getAllAlbumsFromCache();
		getAllPlaylistsFromCache();
	}
}

void setTrackInfo(TiXmlElement* e, Track* track) {

	if (track == nullptr || e == nullptr) {
		return;
	}

	e->SetAttribute("id", track->get_id().c_str());
	e->SetAttribute("title", track->get_title().c_str());
	e->SetAttribute("duration", track->get_duration());
	e->SetAttribute("bitRate", track->get_bitrate());
	e->SetAttribute("contentType", track->get_contentType().c_str());
	e->SetAttribute("coverArt", track->get_coverArt().c_str());
	e->SetAttribute("genre", track->get_genre().c_str());
	e->SetAttribute("parent", track->get_parentId());
	e->SetAttribute("suffix", track->get_suffix().c_str());
	e->SetAttribute("track", track->get_tracknumber());
	e->SetAttribute("year", track->get_year().c_str());
	e->SetAttribute("size", track->get_size());
}

void XmlCacheDb::saveAlbums() {
	std::list<Album>::iterator it;
		
	TiXmlElement* albumRootElement = internalDoc.FirstChildElement("Albums"); 

	if (albumRootElement != NULL) {
		// remove existing album entries, we got new entries
		internalDoc.RemoveChild(internalDoc.FirstChildElement("Albums"));
		
		internalDoc.SaveFile(); // dirty trick to remove all cached entries from TiXmlDocument
		internalDoc.LoadFile();
	}
	else {
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "1");
		internalDoc.LinkEndChild(decl);
	}
	albumRootElement = new TiXmlElement("Albums");
	internalDoc.LinkEndChild(albumRootElement);

	for (it = albumlist.begin(); it != albumlist.end(); it++) {
		TiXmlElement* album = new TiXmlElement("Album");

		pfc::stringcvt::string_wide_from_utf8 albumName(it->get_title());

		album->SetAttribute("id", it->get_id().c_str());
		album->SetAttribute("artist", it->get_artist().c_str());
		album->SetAttribute("title", it->get_title().c_str());
		album->SetAttribute("genre", it->get_genre().c_str());
		album->SetAttribute("year", it->get_year().c_str());
		album->SetAttribute("coverArt", it->get_coverArt().c_str());
		album->SetAttribute("artistId", it->get_artistid());
		album->SetAttribute("parent", it->get_parentid());
		album->SetAttribute("duration", it->get_duration());
		album->SetAttribute("songCount", it->get_songCount());
		

		std::list<Track>* trackList = it->getTracks();
		std::list<Track>::iterator trackIterator;
		for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
			TiXmlElement* track = new TiXmlElement("Track");
			/*
			track->SetAttribute("id", trackIterator->get_id().c_str());
			track->SetAttribute("title", trackIterator->get_title().c_str());
			track->SetAttribute("duration", trackIterator->get_duration());
			track->SetAttribute("bitRate", trackIterator->get_bitrate());
			track->SetAttribute("contentType", trackIterator->get_contentType().c_str());
			track->SetAttribute("coverArt", trackIterator->get_coverArt().c_str());
			track->SetAttribute("genre", trackIterator->get_genre().c_str());
			track->SetAttribute("parent", trackIterator->get_parentId());
			track->SetAttribute("suffix", trackIterator->get_suffix().c_str());
			track->SetAttribute("track", trackIterator->get_tracknumber());
			track->SetAttribute("year", trackIterator->get_year().c_str());
			track->SetAttribute("size", trackIterator->get_size());
			*/
			Track* t = &*trackIterator;

			setTrackInfo(track, t);
			// do not save stream url, as it contains username and password!

			album->LinkEndChild(track);
		}
		albumRootElement->LinkEndChild(album);
	}

	internalDoc.SaveFile();
}


void XmlCacheDb::getAllAlbumsFromCache() {
	
	TiXmlElement* rootNode = internalDoc.FirstChildElement("Albums");
	if (rootNode) {
		for (TiXmlElement* ae = rootNode->FirstChildElement("Album"); ae != NULL; ae = ae->NextSiblingElement("Album")) {
			Album a;

			parseAlbumInfo(ae, &a);

			if (!rootNode->FirstChildElement("Album")->NoChildren()) { // album has tracks

				for (TiXmlElement* e = ae->FirstChildElement("Track"); e != NULL; e = e->NextSiblingElement("Track")) {
					Track t = Track();

					parseTrackInfo(e, &t);
					// we dont save artist and album title in each record to avoid unnecessary information duplication
					t.set_artist(XmlStrOrDefault(ae, "artist", "")); // add artist info from album record
					t.set_album(XmlStrOrDefault(ae, "title", "")); // add album title from album record

					a.addTrack(t);
				}
			}
			albumlist.push_back(a);
		}
	}
}

std::list<Album>* XmlCacheDb::getAllAlbums() {
	saveAlbums();
	return &albumlist;
}

void XmlCacheDb::savePlaylists() {
	std::list<Playlist>::iterator it;

	TiXmlElement* playlistRootElement = internalDoc.FirstChildElement("Playlists");

	if (playlistRootElement != NULL) {
		// remove existing album entries, we got new entries
		internalDoc.RemoveChild(internalDoc.FirstChildElement("Playlists"));

		internalDoc.SaveFile(); // dirty trick to remove all cached entries from TiXmlDocument
		internalDoc.LoadFile();
	}
	else {
		TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "1");
		internalDoc.LinkEndChild(decl);
	}
	playlistRootElement = new TiXmlElement("Playlists");
	internalDoc.LinkEndChild(playlistRootElement);

	for (it = playlists.begin(); it != playlists.end(); it++) {
		TiXmlElement* playlist = new TiXmlElement("Playlist");

		playlist->SetAttribute("id", it->get_id());
		playlist->SetAttribute("comment", it->get_comment());
		playlist->SetAttribute("coverArt", it->get_coverArt());
		playlist->SetAttribute("duration", it->get_duration());
		playlist->SetAttribute("public", it->get_isPublic());
		playlist->SetAttribute("name", it->get_name());
		playlist->SetAttribute("owner", it->get_owner());
		playlist->SetAttribute("songCount", it->get_songcount());

		std::list<Track>* trackList = it->getTracks();
		std::list<Track>::iterator trackIterator;
		for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
			TiXmlElement* track = new TiXmlElement("Track");
			Track* t = &*trackIterator;

			setTrackInfo(track, t);
			// do not save stream url, as it contains username and password!

			playlist->LinkEndChild(track);
		}
	}
	internalDoc.SaveFile();
}

void XmlCacheDb::getAllPlaylistsFromCache() {
	TiXmlElement* rootNode = internalDoc.FirstChildElement("Playlists");
	if (rootNode) {
		for (TiXmlElement* ae = rootNode->FirstChildElement("Playlist"); ae != NULL; ae = ae->NextSiblingElement("Playlist")) {
			Playlist a;

			parsePlaylistInfo(ae, &a);
			if (!rootNode->FirstChildElement("Playlist")->NoChildren()) { // album has tracks

				for (TiXmlElement* e = ae->FirstChildElement("Entry"); e != NULL; e = e->NextSiblingElement("Entry")) {
					Track t = Track();

					parseTrackInfo(e, &t);
					// we dont save artist and album title in each record to avoid unnecessary information duplication
					t.set_artist(XmlStrOrDefault(ae, "artist", "")); // add artist info from album record
					t.set_album(XmlStrOrDefault(ae, "title", "")); // add album title from album record

					a.addTrack(t);
				}
			}
			playlists.push_back(a);
		}
	}
}

std::list<Playlist>* XmlCacheDb::getAllPlaylists() {
	savePlaylists();
	return &playlists;
}
