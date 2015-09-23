#include "foo_subsonic.h"
#include "searchQueryThread.h"
#include "subsoniclibraryscanner.h"


using namespace foo_subsonic;

SearchQueryThread::SearchQueryThread(SubsonicLibraryScanner *scanner, HWND window, const char* queryString)
	: scanner(scanner),
	window(window),
	queryString(queryString)
	{}


void SearchQueryThread::run(threaded_process_status &p_status, abort_callback &p_abort) {
	scanner->retrieveAllSearchResults(window, p_status, queryString);
}