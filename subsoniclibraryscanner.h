#pragma once

#include "foo_subsonic.h"
#include "album.h"
#include "playlist.h"
#include <list>

#define SUBSONIC_MAX_ALBUMLIST_SIZE		500;

namespace foo_subsonic {
	PFC_DECLARE_EXCEPTION(ConnectionError, pfc::exception, "Error connecting to subsonic server");
	class SubsonicLibraryScanner {
		friend class AlbumQueryThread;
		friend class PlaylistQueryThread;
	public:
	protected:
		void retrieveAllAlbums(HWND window, threaded_process_status &p_status);
		void retrieveAllPlaylists(HWND window, threaded_process_status &p_status);

	private:
		BOOL connectAndGet(TiXmlDocument* doc, const char* restMethod, const char* urlparams);
		
		void getAlbumList(threaded_process_status &p_status, std::list<Album>* albumList, int size, int offset);
		void getAlbumTracks(Album *album);
		
		void getPlaylists(threaded_process_status &p_status, std::list<Playlist>* playlists);
		void getPlaylistEntries(Playlist *playlist);

		void parsingError(const char* message, const char* errCode);
		bool checkForError(TiXmlDocument* xml);
	};
}