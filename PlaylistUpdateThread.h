#pragma once
#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"


namespace foo_subsonic {
	class PlaylistUpdateThread : public threaded_process_callback
	{
	public:
		PlaylistUpdateThread(SubsonicLibraryScanner *scanner, HWND window, boolean update, Playlist* playlist, std::list<Track*>* lTracks);
		void run(threaded_process_status &p_status, abort_callback &p_abort);

	private:
		HWND window;
		SubsonicLibraryScanner* scanner;
		boolean update;
		Playlist playlist;
		std::list<Track*>* tracks;
	};
}
