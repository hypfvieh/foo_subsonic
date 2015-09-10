#pragma once
#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"


namespace foo_subsonic {
	class AlbumQueryThread : public threaded_process_callback
	{
	public:
		AlbumQueryThread(SubsonicLibraryScanner *scanner, HWND window);
		void run(threaded_process_status &p_status, abort_callback &p_abort);

	private:
		HWND window;
		SubsonicLibraryScanner* scanner;
		pfc::list_t<metadb_handle_ptr> *tracks;
	};
}
