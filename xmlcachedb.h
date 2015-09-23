#pragma once

#include "foo_subsonic.h"
#include "album.h"
#include "playlist.h"
#include <list>
#include <map>

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
	Album searchResults;

	// map between the stream URL and the track object (as pointer). This allows faster access to track information for playlist view
	std::map<std::string, Track*> urlToTrackMap; 

	static XmlCacheDb* instance;

	void getAllAlbumsFromCache();
	void getAllPlaylistsFromCache();

	void addToUrlMap(Track* t);

public:
	static XmlCacheDb* getInstance() {
		if (instance == NULL) {
			instance = new XmlCacheDb();
		}
		return instance;
	}
	
	std::list<Album>* getAllAlbums();
	Album* getAllSearchResults();
	std::list<Playlist>* getAllPlaylists();

	void saveAlbums();
	void savePlaylists();

	void addSearchResult(Track* t);
	void addAlbum(Album a);
	void addPlaylist(Playlist p);

	bool getTrackDetailsByUrl(const char* url, Track &t);
};
