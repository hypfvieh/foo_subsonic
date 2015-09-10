#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"
#include "requestThread.h"

using namespace foo_subsonic;

RequestThread::RequestThread(SubsonicLibraryScanner *scanner, HWND window)
	: scanner(scanner),
	window(window) {}


void RequestThread::run(threaded_process_status &p_status, abort_callback &p_abort) {
	scanner->scan(window);
}