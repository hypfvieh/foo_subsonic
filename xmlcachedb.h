#pragma once

#include "foo_subsonic.h"
#include "album.h"
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
	std::list<Album> albums;

	static XmlCacheDb* instance;
public:
	static XmlCacheDb* getInstance() {
		if (instance == NULL) {
			instance = new XmlCacheDb();
		}
		return instance;
	}
	
	void addAlbumsToSave(std::list<Album>* albumList);
	std::list<Album>* getAllAlbums();
};