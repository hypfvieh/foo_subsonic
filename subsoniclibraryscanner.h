#pragma once

#include "foo_subsonic.h"
#include "album.h"
#include <list>


namespace foo_subsonic {
	PFC_DECLARE_EXCEPTION(ConnectionError, pfc::exception, "Error connecting to subsonic server");
	class SubsonicLibraryScanner {

	public:
		std::list<Album>* getFetchedAlbumList();
		void scan(HWND window);
	private:
		std::list<Album> albList;

		BOOL connectAndGet(TiXmlDocument* doc, const char* restMethod, const char* urlparams);
		
		void getAlbumList(std::list<Album>* albumList, int size, int offset);
		void getAlbumTracks(Album *album);
		void parsingError(const char* message, const char* errCode);
		bool checkForError(TiXmlDocument* xml);
	};
}