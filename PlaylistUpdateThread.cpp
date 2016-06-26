#pragma once

#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"
#include "PlaylistUpdateThread.h"

using namespace foo_subsonic;

PlaylistUpdateThread::PlaylistUpdateThread(SubsonicLibraryScanner *scanner, HWND window, boolean update, Playlist* playlist, std::list<Track*>* lTracks) {
	tracks = lTracks;
}

void PlaylistUpdateThread::run(threaded_process_status &p_status, abort_callback &p_abort) {
	if (update) {

		scanner->getPlaylistEntries(&playlist, p_abort);
		scanner->updatePlaylistProps(&playlist, p_abort);
		scanner->addToPlaylist(playlist.get_id(), tracks, p_abort);
	} else {
		scanner->createNewPlayList(&playlist, p_abort);
	}
	
}