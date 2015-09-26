#pragma once

#include "foo_subsonic.h"
#include "album.h"
#include "playlist.h"
#include <map>

#define SQL_TABLE_CREATE_SIZE 9
class SqliteCacheDb {

private:
	sqlite3 *db = NULL;

	std::list<Album> albumlist;
	std::list<Playlist> playlists;
	Album searchResults;

	static SqliteCacheDb* instance;

	// map between the stream URL and the track object (as pointer). This allows faster access to track information for playlist view
	std::map<std::string, Track*> urlToTrackMap;

	SqliteCacheDb();
	~SqliteCacheDb();

	void getAllAlbumsFromCache();
	void getAllPlaylistsFromCache();

	void addToUrlMap(Track* t);
	std::list<Track> SqliteCacheDb::getTrackInfos(const char* id, const char* artist);

	char* sql_table_create[SQL_TABLE_CREATE_SIZE] = {
		"CREATE TABLE IF NOT EXISTS albums (id TEXT PRIMARY KEY, artist TEXT, title TEXT, genre TEXT, year TEXT, coverArt TEXT, duration INT, songCount INT)",
		"CREATE TABLE IF NOT EXISTS tracks (id TEXT PRIMARY KEY, albumId TEXT, title TEXT, duration INT, bitrate INT, contentType TEXT, genre TEXT, suffix TEXT, track INT, year TEXT, size INT, coverArt TEXT, songCount INT)",
		"CREATE TABLE IF NOT EXISTS coverart (id TEXT PRIMARY KEY, coverartData BLOB)",
		"CREATE TABLE IF NOT EXISTS playlists (id TEXT PRIMARY KEY, comment TEXT, duration INT, coverArt TEXT, public INT, name TEXT, owner TEXT, songCount INT)",
		"CREATE TABLE IF NOT EXISTS playlist_tracks (playlist_id TEXT, track_id TEXT, PRIMARY KEY (playlist_id, track_id))",
		"CREATE INDEX IF NOT EXISTS album_id_index ON albums(id)",
		"CREATE INDEX IF NOT EXISTS tracks_id_index ON tracks(id)",
		"CREATE INDEX IF NOT EXISTS coverart_id_index ON coverart(id)",
		"CREATE INDEX IF NOT EXISTS playlist_id_index ON playlists(id)"
	};
	
	static int db_callback(void *NotUsed, int argc, char **argv, char **azColName) {
		    return 0;		
	}

public:

	static SqliteCacheDb* getInstance() {
		if (instance == NULL) {
			instance = new SqliteCacheDb();
		}
		return instance;
	}

	std::list<Album>* getAllAlbums();
	Album* getAllSearchResults();
	std::list<Playlist>* getAllPlaylists();

	void saveAlbums();
	void savePlaylists();

	void addSearchResult(Track* t);
	void addAlbum(Album a);
	void addPlaylist(Playlist p);

	bool getTrackDetailsByUrl(const char* url, Track &t);

};
