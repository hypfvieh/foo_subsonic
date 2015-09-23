#pragma once
#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"


namespace foo_subsonic {
	class SearchQueryThread : public threaded_process_callback
	{
	public:
		SearchQueryThread(SubsonicLibraryScanner *scanner, HWND window, const char* queryString);
		void run(threaded_process_status &p_status, abort_callback &p_abort);

	private:
		HWND window;		
		SubsonicLibraryScanner* scanner;
		const char* queryString;
		Album* results;
		pfc::list_t<metadb_handle_ptr> *tracks;
	};
}
