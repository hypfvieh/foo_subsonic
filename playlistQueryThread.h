#pragma once
#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"


namespace foo_subsonic {
	class PlaylistQueryThread : public threaded_process_callback
	{
	public:
		PlaylistQueryThread(SubsonicLibraryScanner *scanner, HWND window);
		void run(threaded_process_status &p_status, abort_callback &p_abort);

	private:
		HWND window;
		SubsonicLibraryScanner* scanner;
	};
}
