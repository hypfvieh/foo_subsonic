#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"
#include "artistUpdateThread.h"

using namespace foo_subsonic;

ArtistUpdateThread::ArtistUpdateThread(SubsonicLibraryScanner *scanner, HWND window, pfc::string8 _artistId)
	: scanner(scanner),
	window(window),
	artistId(_artistId) {}


void ArtistUpdateThread::run(threaded_process_status &p_status, abort_callback &p_abort) {
	scanner->retrieveArtistUpdate(window, p_status, p_abort, artistId.c_str());
}