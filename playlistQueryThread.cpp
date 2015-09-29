#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"
#include "playlistQueryThread.h"

using namespace foo_subsonic;

PlaylistQueryThread::PlaylistQueryThread(SubsonicLibraryScanner *scanner, HWND window)
	: scanner(scanner),
	window(window) {}


void PlaylistQueryThread::run(threaded_process_status &p_status, abort_callback &p_abort) {
	scanner->retrieveAllPlaylists(window, p_status, p_abort);
}