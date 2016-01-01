#include "foo_subsonic.h"
#include "sqliteCacheDb.h"
#include "SimpleHttpClientConfigurator.h"
#include <regex>
#include <sstream>

SqliteCacheDb* SqliteCacheDb::instance = NULL;


SqliteCacheDb::SqliteCacheDb() {
	loadOrCreateDb();	
}

void SqliteCacheDb::loadOrCreateDb() {
	pfc::string userDir = core_api::get_profile_path(); // save cache to user profile, if enabled
	userDir += "\\foo_subsonic_cache.db";

	userDir = userDir.replace("file://", "");
	
	db = new SQLite::Database(userDir.c_str(), SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	
	if (db != NULL) {
		createTableStructure();
		getAllAlbumsFromCache();
		getAllPlaylistsFromCache();

	}
}

void SqliteCacheDb::createTableStructure() {
	if (db == NULL) return;

	for (unsigned int i = 0; i < SQL_TABLE_CREATE_SIZE; i++) {
		try {
			db->exec(sql_table_create[i]);
		}
		catch (...) {
			uDebugLog() << "Unable to create table: " << sql_table_create[i];
		}
	}
}

SqliteCacheDb::~SqliteCacheDb() {
	if (db != NULL) {
		db->exec("VACUUM;"); // restructure cache, maybe we have to free some space
		db->~Database();
	}
}

void SqliteCacheDb::reloadCache() {
	if (db != NULL) {
		db->exec("VACUUM;"); // restructure cache, maybe we have to free some space
		db->~Database();
	}

	albumlist.clear();
	playlists.clear();
	urlToTrackMap.clear();

	loadOrCreateDb();
}

std::list<Album>* SqliteCacheDb::getAllAlbums() {
	return &albumlist;
}

Album* SqliteCacheDb::getAllSearchResults() {
	return &searchResults;
}

void SqliteCacheDb::addToUrlMap(Track* t) {
	if (urlToTrackMap.count(t->get_id().c_str()) == 0) {
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

void SqliteCacheDb::getAlbumById(const char* id, Album &a) {
	std::list<Album>::iterator it;
	for (it = albumlist.begin(); it != albumlist.end(); it++) {
		if (strcmp(it->get_id().c_str(), id) == 0) {
			a = *it;
		}
	}
}

void SqliteCacheDb::savePlaylists(threaded_process_status &p_status, abort_callback &p_abort) {
	if (db == NULL) return;
	std::list<Playlist>::iterator it;

	unsigned int prg = 0;
	p_status.set_progress(prg, playlists.size() + 1);
	p_status.set_progress_secondary(2, 3);

	SQLite::Transaction transaction(*db);
	SQLite::Statement query(*db, "INSERT OR REPLACE INTO playlists (id, comment, coverArt, duration, public, name, owner, songCount) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);");
	for (it = playlists.begin(); it != playlists.end(); it++) {
		
		if (p_abort.is_aborting()) {
			break;
		}

		
		query.reset();

		query.bind(1, it->get_id());
		query.bind(2, it->get_comment());
		query.bind(3, it->get_coverArt());
		query.bind(4, it->get_duration());
		query.bind(5, it->get_isPublic());
		query.bind(6, it->get_name());
		query.bind(7, it->get_owner());
		query.bind(8, it->get_songcount());

		if (query.exec() > 0) {
			std::list<Track*>* trackList = it->getTracks();
			std::list<Track*>::iterator trackIterator;
			for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
				Track* t = *trackIterator;

				if (p_abort.is_aborting()) {
					break;
				}

				// save assignment track <-> playlist
				SQLite::Statement query_track_playlist(*db, "INSERT OR REPLACE INTO playlist_tracks (playlist_id, track_id) VALUES (?1, ?2);");

				query_track_playlist.bind(1, it->get_id());
				query_track_playlist.bind(2, t->get_id());

				query_track_playlist.exec();

				// update artist
				SQLite::Statement artist_query(*db, "INSERT OR REPLACE INTO artists (id, artist) VALUES (?1, ?2);");
				artist_query.bind(1, t->get_artistId());
				artist_query.bind(2, t->get_artist());
				artist_query.exec();

				// save track information
				SQLite::Statement query_track(*db, "INSERT OR REPLACE INTO tracks (id, title, duration, bitRate, contentType, coverArt, genre, suffix, track, year, size, albumId, artistId) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13);");

				query_track.bind(1, t->get_id());
				query_track.bind(2, t->get_title());
				query_track.bind(3, t->get_duration());
				query_track.bind(4, t->get_bitrate());
				query_track.bind(5, t->get_contentType());
				query_track.bind(6, t->get_coverArt());
				query_track.bind(7, t->get_genre());
				query_track.bind(8, t->get_suffix());
				query_track.bind(9, t->get_tracknumber());
				query_track.bind(10, t->get_year());
				query_track.bind(11, t->get_size());
				query_track.bind(12, t->get_parentId());
				query_track.bind(13, t->get_artistId());

				if (query_track.exec() != 1) {
					uDebugLog() << "Error while inserting track";
				}

			}
		}
		p_status.set_progress(++prg, playlists.size() + 1);
	}
	transaction.commit();
	p_status.set_progress(++prg, playlists.size() + 1);
	p_status.set_progress_secondary(3, 3);
}

void SqliteCacheDb::saveAlbums(threaded_process_status &p_status, abort_callback &p_abort) {
	if (db == NULL) return;
	std::list<Album>::iterator it;

	unsigned int prg = 0;

	p_status.set_progress(prg, albumlist.size() + 1);
	p_status.set_progress_secondary(2, 3);

	SQLite::Statement query(*db, "INSERT OR REPLACE INTO albums (id, artistId, title, genre, year, coverArt, duration, songCount) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);");
	SQLite::Transaction transaction(*db);
	for (it = albumlist.begin(); it != albumlist.end(); it++) {

		if (p_abort.is_aborting()) {
			break;
		}

		SQLite::Statement artist_query(*db, "INSERT OR REPLACE INTO artists (id, artist) VALUES (?1, ?2);");
		artist_query.bind(1, it->get_artistid());
		artist_query.bind(2, it->get_artist());
		artist_query.exec();

		query.reset();

		query.bind(1, it->get_id());
		query.bind(2, it->get_artistid());
		query.bind(3, it->get_title());
		query.bind(4, it->get_genre());
		query.bind(5, it->get_year());
		query.bind(6, it->get_coverArt());
		query.bind(7, it->get_duration());
		query.bind(8, it->get_songCount());

		// adding the tracks requires some extra work to speed up the writing of sqlite.
		// doing all INSERTs seperately is incredibly slow, so we try some sort of batch processing here
		if (query.exec() >= 1) {
			std::list<Track*>* trackList = it->getTracks();
			std::list<Track*>::iterator trackIterator;

			// list of maps which contains querystring and a map with placeholder<->values
			std::list<std::map<std::string, std::map<int, const char*>>> allInone;

			unsigned int colcount = 0;
			bool first = true;

			std::ostringstream tmp;

			std::map<std::string, std::map<int, const char*>> listEntry;
			std::map<int, const char*> val;

			// generate query string and value map
			// we need to take care of the SQLITE_LIMIT_VARIABLE_NUMBER which is 999 by default
			// so we first create a list of maps containg query and the placeholder values
			// after that, we iterate over that list and execute each query
			for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
				Track* t = *trackIterator;

				if (p_abort.is_aborting()) {
					break;
				}

				// add the values required for the prepared statement				

				val[colcount +  1] = t->get_id();
				val[colcount +  2] = t->get_title();
				val[colcount +  3] = t->get_duration();
				val[colcount +  4] = t->get_bitrate();
				val[colcount +  5] = t->get_contentType();
				val[colcount +  6] = t->get_coverArt();
				val[colcount +  7] = t->get_genre();
				val[colcount +  8] = t->get_suffix();
				val[colcount +  9] = t->get_tracknumber();
				val[colcount + 10] = t->get_year();
				val[colcount + 11] = t->get_size();
				val[colcount + 12] = it->get_id();
				val[colcount + 13] = it->get_artistid();

				if (first) { // create a new query, the first part is done statically as the syntax is slightly different than the following UNION SELECTs
					first = false;
					tmp << "INSERT OR REPLACE INTO tracks (id, title, duration, bitRate, contentType, coverArt, genre, suffix, track, year, size, albumId, artistId) ";
					tmp << "SELECT ?1 as id, ?2 as title, ?3 as duration, ?4 as bitRate, ?5 as contentType, ?6 as coverArt, ?7 as genre, ?8 as suffix, ?9 as track, ?10 as year, ?11 as size, ?12 as albumId, ?13 as artistId ";
					colcount = 13;

				}
				else { // add a new value set
					tmp << "UNION SELECT ?"	<<  (colcount + 1) << ", ?" <<  (colcount + 2) << ", ?" << (colcount + 3)
						<< ", ?"			<<  (colcount + 4) << ", ?" <<  (colcount + 5) << ", ?" << (colcount + 6)
						<< ", ?"			<<  (colcount + 7) << ", ?" <<  (colcount + 8) << ", ?" << (colcount + 9)
						<< ", ?"			<< (colcount + 10) << ", ?" << (colcount + 11) << ", ?"	<< (colcount + 12)
						<< ", ?"			<< (colcount + 13) << " ";

					if (colcount >= SQLITE_LIMIT_VARIABLE_NUMBER || colcount + 13 >= SQLITE_LIMIT_VARIABLE_NUMBER) { // if we reach limit (999) create new query block
						listEntry[tmp.str()] = val;
						allInone.push_back(listEntry);

						tmp = std::ostringstream(); // create new query
						val = std::map<int, const char*>(); // create new entry map
						listEntry = std::map<std::string, std::map<int, const char*>>(); // create new list entry
						first = true; // start again from the beginning, adding the static part as well
						colcount = 0; // no columns are used yet
					}
					else {
						colcount += 13; // increase total column count
					}
				}
			}

			// if there is a query which was not added before (limit not reached), add it now
			if (strlen(tmp.str().c_str()) > 0) {
				listEntry[tmp.str()] = val;
				allInone.push_back(listEntry);
			}

			std::list<std::map<std::string, std::map<int, const char*>>>::iterator queryIterator;

			// iterate over the list of maps, create and run every statement
			for (queryIterator = allInone.begin(); queryIterator != allInone.end(); queryIterator++) {
				std::map<std::string, std::map<int, const char*>> sqlValueMap = *queryIterator;

				std::map<std::string, std::map<int, const char*>>::iterator sqlValueMapIterator;
				for (sqlValueMapIterator = sqlValueMap.begin(); sqlValueMapIterator != sqlValueMap.end(); sqlValueMapIterator++) {

					if (p_abort.is_aborting()) {
						break;
					}

					const char* sqlQueryString = sqlValueMapIterator->first.c_str();
					SQLite::Statement query_track(*db, sqlQueryString);

					std::map<int, const char*> valueMap = sqlValueMapIterator->second;
					std::map<int, const char*>::iterator valueMapIterator;

					for (valueMapIterator = valueMap.begin(); valueMapIterator != valueMap.end(); valueMapIterator++) {
						query_track.bind(valueMapIterator->first, valueMapIterator->second);
					}

					query_track.exec();
					if (query_track.getErrorCode() != SQLITE_DONE) {
						uDebugLog() << "Error while inserting track ErrCode: " << query_track.getErrorCode() << " -- ExtErrCode: " << query_track.getExtendedErrorCode();
					}
					query_track.reset();
				}
			}
		}
		p_status.set_progress(++prg, albumlist.size() + 1);
	}
	transaction.commit();
	p_status.set_progress(++prg, albumlist.size() + 1);
	p_status.set_progress_secondary(3, 3);
}


void SqliteCacheDb::parseTrackInfo(Track *t, SQLite::Statement *query_track) {

	t->set_id(query_track->getColumn(0));
	t->set_parentId(query_track->getColumn(1));
	t->set_title(query_track->getColumn(2));
	t->set_duration(query_track->getColumn(3));
	t->set_bitrate(query_track->getColumn(4));
	t->set_contentType(query_track->getColumn(5));
	t->set_genre(query_track->getColumn(6));
	t->set_suffix(query_track->getColumn(7));
	t->set_tracknumber(query_track->getColumn(8));
	t->set_year(query_track->getColumn(9));
	t->set_size(query_track->getColumn(10));
	t->set_coverArt(query_track->getColumn(11));
	t->set_artistId(query_track->getColumn(12));

	pfc::string8 idparam = "id=";
	idparam << t->get_id();
	pfc::string8 streamUrl = SimpleHttpClientConfigurator::buildRequestUrl("stream", idparam);
	t->set_streamUrl(streamUrl);

}

void SqliteCacheDb::getAllAlbumsFromCache() {
	if (db == NULL) return;
	// get all albums assigned to the artist and sorted by album name
	SQLite::Statement query(*db, "SELECT albums.id, artists.artist, title, genre, year, coverArt, duration, songCount, artistId FROM albums, artists WHERE albums.artistId = artists.id ORDER BY artists.artist ASC, title ASC");

	while (query.executeStep()) {
		Album a;
		a.set_id(query.getColumn(0));
		a.set_artist(query.getColumn(1));
		a.set_title(query.getColumn(2));
		a.set_genre(query.getColumn(3));
		a.set_year(query.getColumn(4));
		a.set_coverArt(query.getColumn(5));
		a.set_duration(query.getColumn(6));
		a.set_songCount(query.getColumn(7));
		a.set_artistid(query.getColumn(8));

		// get all tracks from the current album sorted by tracknumber
		SQLite::Statement query_track(*db, "SELECT id, albumId, title, duration, bitrate, contentType, genre, suffix, track, year, size, coverArt, artistId FROM tracks WHERE albumId = ?1 ORDER BY year ASC, track ASC");
		query_track.bind(1, a.get_id());

		while (query_track.executeStep()) {
			Track* t = new Track();
			parseTrackInfo(t, &query_track);
			t->set_artist(a.get_artist());
			t->set_album(a.get_title());

			a.addTrack(t);
			addToUrlMap(t);

		}
		albumlist.push_back(a);
	}

}

void SqliteCacheDb::getAllPlaylistsFromCache() {
	if (db == NULL) return;
	SQLite::Statement query(*db, "SELECT id, comment, duration, coverArt, public, name, owner, songCount FROM playlists");

	while (query.executeStep()) {
		Playlist p;

		int i_isPublic = query.getColumn(4);
		bool isPublic = i_isPublic > 0 ? TRUE : FALSE;

		p.set_id(query.getColumn(0));
		p.set_comment(query.getColumn(1));
		p.set_duration(query.getColumn(2));
		p.set_coverArt(query.getColumn(3));
		p.set_isPublic(isPublic);
		p.set_name(query.getColumn(5));
		p.set_owner(query.getColumn(6));
		p.set_songcount(query.getColumn(7));

		SQLite::Statement query_track(*db, "SELECT track_id FROM playlist_tracks WHERE playlist_id = ?1");
		query_track.bind(1, p.get_id());

		while (query_track.executeStep()) {
			const char* trackId = query_track.getColumn(0);
			SQLite::Statement query_track(*db, "SELECT id, albumId, title, duration, bitrate, contentType, genre, suffix, track, year, size, coverArt, artistId FROM tracks WHERE id = ?1");
			query_track.bind(1, trackId);

			while (query_track.executeStep()) {
				Track* t = new Track();
				parseTrackInfo(t, &query_track);

				SQLite::Statement query_artist(*db, "SELECT artist FROM artists WHERE id = ?1 LIMIT 1");
				query_artist.bind(1, t->get_artistId());

				if (query_artist.executeStep()) {
					t->set_artist(query_artist.getColumn(0));
				}

				p.addTrack(t);
			}						
		}
		playlists.push_back(p);
	}	
}

void SqliteCacheDb::addCoverArtToCache(const char* coverArtId, const void * coverArtData, unsigned int dataLength) {
	if (db == NULL) return;

	if (dataLength > 0 && coverArtId != NULL && strlen(coverArtId) > 0) {
		SQLite::Transaction transaction(*db);
		SQLite::Statement query_coverArt(*db, "INSERT OR REPLACE INTO coverart (id, coverArtData) VALUES (?1, ?2)");

		query_coverArt.bind(1, coverArtId);
		query_coverArt.bind(2, coverArtData, dataLength);

		if (query_coverArt.exec() < 1) {
			uDebugLog() << "Error inserting coverArtData";
		}
		transaction.commit();
	}
}

void SqliteCacheDb::getCoverArtById(const char* coverArtId, char* &coverArtData, unsigned int &dataLength) {
	if (db == NULL) return;

	SQLite::Statement query_coverArt(*db, "SELECT coverArtData FROM coverart WHERE id = ?1 LIMIT 1");
	dataLength = 0;
	query_coverArt.bind(1, coverArtId);

	if (query_coverArt.executeStep()) {

		dataLength = query_coverArt.getColumn(0).getBytes();

		char *tmpBuffer = new char[dataLength];
		memcpy(tmpBuffer, query_coverArt.getColumn(0).getBlob(), dataLength);

		delete[] coverArtData;
		coverArtData = tmpBuffer;
	}
}

void SqliteCacheDb::getCoverArtByTrackId(const char* trackId, std::string &out_coverId, char* &coverArtData, unsigned int &dataLength) {
	if (db == NULL) return;

	SQLite::Statement query_coverId(*db, "SELECT coverArt FROM tracks WHERE id = ?1 LIMIT 1");

	query_coverId.bind(1, trackId);

	if (query_coverId.executeStep()) {
		getCoverArtById(query_coverId.getColumn(0), coverArtData, dataLength);

		out_coverId.append(query_coverId.getColumn(0).getText());
	}
}


void SqliteCacheDb::clearCoverArtCache() {
	if (db == NULL) return;

	db->exec("DROP TABLE coverart;");
	db->exec("VACUUM;");
	createTableStructure();	
}

void SqliteCacheDb::clearCache() {
	if (db == NULL) return;

	SQLite::Transaction transaction(*db);
	db->exec("DROP TABLE coverart;");
	db->exec("DROP TABLE artists;");
	db->exec("DROP TABLE albums;");
	db->exec("DROP TABLE playlist_tracks;");
	db->exec("DROP TABLE tracks;");
	db->exec("DROP TABLE playlists;");
	
	transaction.commit();	

	db->exec("VACUUM;");
	createTableStructure();

	albumlist.clear();
	playlists.clear();
	urlToTrackMap.clear();
}

void SqliteCacheDb::checkMetaInfo() {
	SQLite::Statement query(*db, "SELECT * FROM metainfo");

	while (query.executeStep()) {
		std::string key = query.getColumn(0);
		std::string val = query.getColumn(1);
		if (strcmp(key.c_str(), SQL_TABLE_VERSION_KEY.c_str()) == 0) {
			if (isInteger(val)) {
				int valInt = atoi(val.c_str());
				if (valInt != SQL_TABLE_VERSION) {
					MessageBox(NULL, L"Your local subsonic cache database is not compatible with the current plugin version.\r\nThe old database will be removed and you have to re-query your catalog/playlists!", L"Cache outdated", MB_OK | MB_ICONINFORMATION);
					
					if (db != NULL) {
						std::wstring dbFile = s2ws(db->getFilename());						
						db->~Database(); // destory handle
						DeleteFile(dbFile.c_str());
						instance = new SqliteCacheDb();
					}
				}
			}
		}
	}
}


