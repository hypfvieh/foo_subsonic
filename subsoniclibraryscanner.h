#pragma once

#include "foo_subsonic.h"
#include "album.h"
#include "playlist.h"
#include <list>


namespace foo_subsonic {
	PFC_DECLARE_EXCEPTION(ConnectionError, pfc::exception, "Error connecting to subsonic server");
	class SubsonicLibraryScanner {

	public:
		std::list<Album>* getFetchedAlbumList();
		void retrieveAllAlbums(HWND window, threaded_process_status &p_status);
		void retrieveAllPlaylists(HWND window);
	private:
		std::list<Album> albList;

		BOOL connectAndGet(TiXmlDocument* doc, const char* restMethod, const char* urlparams);
		
		void getAlbumList(threaded_process_status &p_status, std::list<Album>* albumList, int size, int offset);
		void getAlbumTracks(Album *album);

		void getPlaylists(threaded_process_status &p_status, std::list<Playlist>* playlists);

		void parsingError(const char* message, const char* errCode);
		bool checkForError(TiXmlDocument* xml);
	};
}