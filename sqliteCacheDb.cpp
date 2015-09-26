#include "foo_subsonic.h"
#include "sqliteCacheDb.h"
#include <regex>


SqliteCacheDb::SqliteCacheDb() {
	char *errMsg = 0;

	std::string userDir = core_api::get_profile_path(); // save cache to user profile, if enabled
	userDir += "\\foo_subsonic_cache.db";

	int rc = sqlite3_open_v2(userDir.c_str(), &db, SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (rc) {
		uDebugLog() << "Unable to open database: " << sqlite3_errmsg(db);
		sqlite3_close(db);
		db = NULL;
	}
	else {
		// create tables if missing
		for (unsigned int i = 0; i < SQL_TABLE_CREATE_SIZE; i++) {
			rc = sqlite3_exec(db, sql_table_create[i], db_callback, 0, &errMsg);
			if (rc != SQLITE_OK) {
				uDebugLog() << "Unable to create table: " << errMsg;
			}
		}
	}
}

SqliteCacheDb::~SqliteCacheDb() {
	if (db != NULL) {
		sqlite3_close(db);
	}
}

std::list<Album>* SqliteCacheDb::getAllAlbums() {
	return &albumlist;
}

Album* SqliteCacheDb::getAllSearchResults() {
	return &searchResults;
}

void SqliteCacheDb::addToUrlMap(Track* t) {
	if (urlToTrackMap.count(t->get_id().c_str()) == 0) {
		/*
		Track* trk = new Track();
		trk->set_title(t->get_title());
		trk->set_album(t->get_album());
		trk->set_tracknumber(t->get_tracknumber());
		trk->set_id(t->get_id());
		*/
		urlToTrackMap[t->get_id().c_str()] = t;
	}
}

void SqliteCacheDb::addSearchResult(Track* t) {
	searchResults.addTrack(t);
	addToUrlMap(t);
}

void SqliteCacheDb::addAlbum(Album a) {
	std::list<Track*>* trackList = a.getTracks();
	std::list<Track*>::iterator trackIterator;

	for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
		addToUrlMap(*trackIterator);
	}

	albumlist.push_back(a);
}

void SqliteCacheDb::addPlaylist(Playlist p) {
	std::list<Track*>* trackList = p.getTracks();
	std::list<Track*>::iterator trackIterator;

	for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
		addToUrlMap(*trackIterator);
	}

	playlists.push_back(p);
}

std::list<Playlist>* SqliteCacheDb::getAllPlaylists() {
	return &playlists;
}

bool SqliteCacheDb::getTrackDetailsByUrl(const char* url, Track &t) {

	std::string strUrl = url;
	std::string result;

	std::regex re(".*id=([^&]+).*");
	std::smatch match;
	if (std::regex_search(strUrl, match, re) && match.size() > 1) { // found ID
		result = match.str(1);
	}
	else { // no ID, cannot continue
		return FALSE;
	}

	if (urlToTrackMap.count(result) > 0) { // fast way
		std::map<std::string, Track*>::iterator i = urlToTrackMap.find(result);
		t = *i->second;
		return TRUE;
	}
	else { // no luck on the fast lane, take the long and slow road ...

		std::list<Album>::iterator it;
		for (it = albumlist.begin(); it != albumlist.end(); it++) {
			std::list<Track*>* trackList = it->getTracks();
			std::list<Track*>::iterator trackIterator;
			for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {

				uDebugLog() << "Comparing: T->ID: '" << (*trackIterator)->get_id() << "' --- Given ID: '" << result.c_str() << "'";

				if (strcmp((*trackIterator)->get_id().c_str(), result.c_str()) == 0) {
					t = **trackIterator;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void SqliteCacheDb::savePlaylists() {
	std::list<Playlist>::iterator it;

	int rc;

	for (it = playlists.begin(); it != playlists.end(); it++) {
		sqlite3_stmt* t_stmt = NULL;
		sqlite3_prepare_v2(db, "INSERT INTO playlists (id, comment, coverArt, duration, public, name, owner, songCount) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8)", -1, &t_stmt, NULL);

		sqlite3_bind_text(t_stmt, 1, it->get_id(), -1, SQLITE_STATIC);
		sqlite3_bind_text(t_stmt, 2, it->get_comment(), -1, SQLITE_STATIC);
		sqlite3_bind_text(t_stmt, 3, it->get_coverArt(), -1, SQLITE_STATIC);
		sqlite3_bind_text(t_stmt, 6, it->get_name(), -1, SQLITE_STATIC);
		sqlite3_bind_text(t_stmt, 7, it->get_owner(), -1, SQLITE_STATIC);
		
		sqlite3_bind_int(t_stmt, 4, it->get_duration());
		sqlite3_bind_int(t_stmt, 5, it->get_isPublic());
		sqlite3_bind_int(t_stmt, 8, it->get_songcount());

		if (sqlite3_step(t_stmt) == SQLITE_DONE) {

			std::list<Track*>* trackList = it->getTracks();
			std::list<Track*>::iterator trackIterator;
			for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
				TiXmlElement* track = new TiXmlElement("Track");
				Track* t = *trackIterator;

				sqlite3_stmt* t_stmt = NULL;
				sqlite3_prepare_v2(db, "INSERT INTO playlist_trackss (playlist_id, track_id) VALUES (?1, ?2)", -1, &t_stmt, NULL);

				sqlite3_bind_text(t_stmt, 1, t->get_id(), -1, SQLITE_STATIC);
				sqlite3_bind_text(t_stmt, 2, it->get_id(), -1, SQLITE_STATIC);

				if (sqlite3_step(t_stmt) != SQLITE_DONE) {
					uDebugLog() << "Error while inserting track";
				}

			}
		}
	}
}

void SqliteCacheDb::saveAlbums() {
	std::list<Album>::iterator it;
	int rc;
	
	for (it = albumlist.begin(); it != albumlist.end(); it++) {


		sqlite3_stmt* t_stmt = NULL;
		sqlite3_prepare_v2(db, "INSERT INTO albums (id, artist, title, genre, year, coverArt, duration, songCount) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8)", -1, &t_stmt, NULL);

		sqlite3_bind_text(t_stmt, 1, it->get_id(), -1, SQLITE_STATIC);
		sqlite3_bind_text(t_stmt, 2, it->get_artist(), -1, SQLITE_STATIC);
		sqlite3_bind_text(t_stmt, 3, it->get_title(), -1, SQLITE_STATIC);
		sqlite3_bind_text(t_stmt, 4, it->get_genre(), -1, SQLITE_STATIC);
		sqlite3_bind_text(t_stmt, 5, it->get_year(), -1, SQLITE_STATIC);
		sqlite3_bind_text(t_stmt, 6, it->get_coverArt(), -1, SQLITE_STATIC);

		sqlite3_bind_int(t_stmt, 7, it->get_duration());
		sqlite3_bind_int(t_stmt, 8, it->get_songCount());

		if (sqlite3_step(t_stmt) == SQLITE_DONE) {
			std::list<Track*>* trackList = it->getTracks();
			std::list<Track*>::iterator trackIterator;

			for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
				TiXmlElement* track = new TiXmlElement("Track");
				Track* t = *trackIterator;

				sqlite3_stmt* t_stmt = NULL;
				sqlite3_prepare_v2(db, "INSERT INTO albums (id, title, duration, bitRate, contentType, coverArt, genre, suffix, track, year, size) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11)", -1, &t_stmt, NULL);

				sqlite3_bind_text(t_stmt, 1, t->get_id(), -1, SQLITE_STATIC);
				sqlite3_bind_text(t_stmt, 2, t->get_title(), -1, SQLITE_STATIC);
				sqlite3_bind_int(t_stmt, 3, t->get_duration());
				sqlite3_bind_int(t_stmt, 4, t->get_bitrate());
				sqlite3_bind_text(t_stmt, 5, t->get_contentType(), -1, SQLITE_STATIC);
				sqlite3_bind_text(t_stmt, 6, t->get_coverArt(), -1, SQLITE_STATIC);
				sqlite3_bind_text(t_stmt, 7, t->get_genre(), -1, SQLITE_STATIC);
				sqlite3_bind_text(t_stmt, 8, t->get_suffix(), -1, SQLITE_STATIC);
				sqlite3_bind_int(t_stmt, 9, t->get_tracknumber());
				sqlite3_bind_text(t_stmt, 10, t->get_year(), -1, SQLITE_STATIC);
				sqlite3_bind_int(t_stmt, 11, t->get_size());

				if (sqlite3_step(t_stmt) != SQLITE_DONE) {
					uDebugLog() << "Error while inserting track";
				}
			}
		}
	}
}

char* unsigned_to_signed_char(const unsigned char* bar) {
	char buf[250];
	memset(buf, '\0', sizeof(buf));
	snprintf(buf, sizeof(buf), "%s", bar);

	return buf;
}

std::list<Track> SqliteCacheDb::getTrackInfos(const char* id, const char* artist) {
	sqlite3_stmt *trk_stmt;
	int trk_rc;
	sqlite3_prepare_v2(db, "SELECT id, albumId, title, duration, bitrate, contentType, genre, suffix, track, year, size, coverArt FROM tracks WHERE albumId = ?1", -1, &trk_stmt, NULL);
	sqlite3_bind_text(trk_stmt, 1, id, -1, SQLITE_STATIC);

	std::list<Track> list;

	while ((trk_rc = sqlite3_step(trk_stmt)) == SQLITE_ROW) {
		Track t;
		t.set_id(unsigned_to_signed_char(sqlite3_column_text(trk_stmt, 0)));
		t.set_parentId(unsigned_to_signed_char(sqlite3_column_text(trk_stmt, 1)));

		t.set_title(unsigned_to_signed_char(sqlite3_column_text(trk_stmt, 2)));
		t.set_duration(sqlite3_column_int(trk_stmt, 3));
		t.set_bitrate(sqlite3_column_int(trk_stmt, 4));
		t.set_contentType(unsigned_to_signed_char(sqlite3_column_text(trk_stmt, 5)));
		t.set_genre(unsigned_to_signed_char(sqlite3_column_text(trk_stmt, 6)));
		t.set_suffix(unsigned_to_signed_char(sqlite3_column_text(trk_stmt, 7)));
		t.set_tracknumber(sqlite3_column_int(trk_stmt, 8));
		t.set_year(unsigned_to_signed_char(sqlite3_column_text(trk_stmt, 9)));
		t.set_size(sqlite3_column_int(trk_stmt, 10));

		t.set_coverArt(unsigned_to_signed_char(sqlite3_column_text(trk_stmt, 11)));
		t.set_artist(artist);

		list.push_back(t);
	}
	return list;
}

void SqliteCacheDb::getAllAlbumsFromCache() {
	sqlite3_stmt *stmt;
	int rc;
	sqlite3_prepare_v2(db, "SELECT id, artist, title, genre, year, coverArt, duration, songCount FROM albums", -1, &stmt, NULL);

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		Album a;

		a.set_id(unsigned_to_signed_char(sqlite3_column_text(stmt, 0)));
		a.set_artist(unsigned_to_signed_char(sqlite3_column_text(stmt, 1)));
		a.set_title(unsigned_to_signed_char(sqlite3_column_text(stmt, 2)));
		a.set_genre(unsigned_to_signed_char(sqlite3_column_text(stmt, 3)));
		a.set_year(unsigned_to_signed_char(sqlite3_column_text(stmt, 4)));
		a.set_coverArt(unsigned_to_signed_char(sqlite3_column_text(stmt, 5)));
		a.set_duration(sqlite3_column_int(stmt, 6));
		a.set_songCount(sqlite3_column_int(stmt, 7));

		std::list<Track> list = getTrackInfos(a.get_id(), a.get_artist());
		
		a.addTracks(list);
		albumlist.push_back(a);
	}	
}

void SqliteCacheDb::getAllPlaylistsFromCache() {
	sqlite3_stmt *stmt;
	int rc;
	sqlite3_prepare_v2(db, "SELECT id, comment, duration, coverArt, public, name, owner, songCount FROM playlists", -1, &stmt, NULL);

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		Playlist p;

		p.set_id(unsigned_to_signed_char(sqlite3_column_text(stmt, 0)));
		p.set_comment(unsigned_to_signed_char(sqlite3_column_text(stmt, 1)));
		p.set_duration(sqlite3_column_int(stmt, 2));
		p.set_coverArt(unsigned_to_signed_char(sqlite3_column_text(stmt, 3)));
		p.set_isPublic(sqlite3_column_int(stmt, 4));
		p.set_name(unsigned_to_signed_char(sqlite3_column_text(stmt, 5)));		
		p.set_owner(unsigned_to_signed_char(sqlite3_column_text(stmt, 6)));
		p.set_songcount(sqlite3_column_int(stmt, 7));

		sqlite3_stmt *trk_pl_stmt;
		int trk_pl_rc;
		sqlite3_prepare_v2(db, "SELECT track_id FROM playlist_tracks WHERE playlist_id = ?1", -1, &trk_pl_stmt, NULL);
		sqlite3_bind_text(trk_pl_stmt, 1, p.get_id(), -1, SQLITE_STATIC);
		while ((trk_pl_rc = sqlite3_step(trk_pl_stmt)) == SQLITE_ROW) {
			std::list<Track> list = getTrackInfos(p.get_id(), p.get_name()); // TODO: get artist name ... funktioniert nicht mit playlists
			p.addTracks(list);
			playlists.push_back(p);
		}
	}
}
