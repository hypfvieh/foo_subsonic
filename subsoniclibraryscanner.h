#pragma once

#include "foo_subsonic.h"
#include "album.h"
#include "playlist.h"
#include <list>

#define SUBSONIC_MAX_ALBUMLIST_SIZE		500
//#define SUBSONIC_MAX_ALBUMLIST_SIZE		20;

namespace foo_subsonic {
	PFC_DECLARE_EXCEPTION(ConnectionError, pfc::exception, "Error connecting to subsonic server");
	class SubsonicLibraryScanner {
		friend class AlbumQueryThread;
		friend class PlaylistQueryThread;
		friend class SearchQueryThread;
		friend class ArtistUpdateThread;
	public:
	protected:
		void retrieveAllAlbums(HWND window, threaded_process_status &p_status, abort_callback &p_abort);
		void retrieveAllPlaylists(HWND window, threaded_process_status &p_status, abort_callback &p_abort);
		void retrieveAllSearchResults(HWND window, threaded_process_status &p_status, const char* url);

		void retrieveArtistUpdate(HWND window, threaded_process_status &p_status, abort_callback &p_abort, const char* artistId);

	private:
		BOOL connectAndGet(TiXmlDocument* doc, const char* restMethod, const char* urlparams);
		
		void getAlbumList(threaded_process_status &p_status, int size, int offset, abort_callback &p_abort);
		void getAlbumTracks(Album *album, abort_callback &p_abort);
		
		void getPlaylists(threaded_process_status &p_status, abort_callback &p_abort);
		void getPlaylistEntries(Playlist *playlist, abort_callback &p_abort);

		void getAlbumAndTracksByArtistId(const char *artist_id, abort_callback &p_abort);
		
		void getSearchResults(const char* urlParams);

		void parsingError(const char* message, const char* errCode);
		bool checkForError(TiXmlDocument* xml);
	};
}