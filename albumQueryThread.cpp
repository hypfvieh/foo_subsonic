#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"
#include "albumQueryThread.h"

using namespace foo_subsonic;

AlbumQueryThread::AlbumQueryThread(SubsonicLibraryScanner *scanner, HWND window)
	: scanner(scanner),
	window(window) {}


void AlbumQueryThread::run(threaded_process_status &p_status, abort_callback &p_abort) {
	scanner->retrieveAllAlbums(window, p_status, p_abort);
}