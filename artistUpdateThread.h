#pragma once
#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"


namespace foo_subsonic {
	class ArtistUpdateThread : public threaded_process_callback
	{
	public:
		ArtistUpdateThread(SubsonicLibraryScanner *scanner, HWND window, pfc::string8 _artistId);
		void run(threaded_process_status &p_status, abort_callback &p_abort);

	private:
		pfc::string8 artistId;
		HWND window;
		SubsonicLibraryScanner* scanner;
		pfc::list_t<metadb_handle_ptr> *tracks;
	};
}
