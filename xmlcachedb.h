#pragma once

#include "foo_subsonic.h"
#include "album.h"
#include "playlist.h"
#include <list>

/*
	Singleton class which contains all cached subsonic results.
*/
class XmlCacheDb {
private:
	XmlCacheDb();

	~XmlCacheDb() {
		//internalDoc = NULL
	}

	TiXmlDocument internalDoc;

	std::list<Album> albumlist;
	std::list<Playlist> playlists;

	static XmlCacheDb* instance;

	void getAllAlbumsFromCache();
	void getAllPlaylistsFromCache();

	void saveAlbums();
	void savePlaylists();

public:
	static XmlCacheDb* getInstance() {
		if (instance == NULL) {
			instance = new XmlCacheDb();
		}
		return instance;
	}
	
	std::list<Album>* getAllAlbums();
	std::list<Playlist>* getAllPlaylists();

	bool getTrackDetailsByUrl(const char* url, Track* t);
};
