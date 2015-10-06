#pragma once

#include "foo_subsonic.h"
#include "album.h"
#include "playlist.h"
#include <map>

#define SQL_TABLE_CREATE_SIZE 13

class SqliteCacheDb {

private:
	SQLite::Database *db = NULL;

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

	void SqliteCacheDb::parseTrackInfo(Track *t, SQLite::Statement *query_track);

	char* sql_table_create[SQL_TABLE_CREATE_SIZE] = {
		"CREATE TABLE IF NOT EXISTS artists (id TEXT PRIMARY KEY, artist TEXT)",
		"CREATE TABLE IF NOT EXISTS albums (id TEXT PRIMARY KEY, artistId TEXT, title TEXT, genre TEXT, year TEXT, coverArt TEXT, duration INT, songCount INT)",
		"CREATE TABLE IF NOT EXISTS tracks (id TEXT PRIMARY KEY, albumId TEXT, title TEXT, duration INT, bitrate INT, contentType TEXT, genre TEXT, suffix TEXT, track INT, year TEXT, size INT, coverArt TEXT, artistId TEXT)",
		"CREATE TABLE IF NOT EXISTS coverart (id TEXT PRIMARY KEY, coverartData BLOB)",
		"CREATE TABLE IF NOT EXISTS playlists (id TEXT PRIMARY KEY, comment TEXT, duration INT, coverArt TEXT, public INT, name TEXT, owner TEXT, songCount INT)",
		"CREATE TABLE IF NOT EXISTS playlist_tracks (playlist_id TEXT, track_id TEXT, PRIMARY KEY (playlist_id, track_id))",
		"CREATE INDEX IF NOT EXISTS artist_id_index ON artists(id)",
		"CREATE INDEX IF NOT EXISTS album_id_index ON albums(id)",
		"CREATE INDEX IF NOT EXISTS tracks_id_index ON tracks(id)",
		"CREATE INDEX IF NOT EXISTS tracks_album_id_index ON tracks(albumId)",
		"CREATE INDEX IF NOT EXISTS tracks_artist_id_index ON tracks(artistId)",
		"CREATE INDEX IF NOT EXISTS coverart_id_index ON coverart(id)",
		"CREATE INDEX IF NOT EXISTS playlist_id_index ON playlists(id)"
	};
	
	void loadOrCreateDb();
	void createTableStructure();
	
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

	void saveAlbums(threaded_process_status &p_status, abort_callback &p_abort);
	void savePlaylists(threaded_process_status &p_status, abort_callback &p_abort);

	void addSearchResult(Track* t);
	void addAlbum(Album a);
	void addPlaylist(Playlist p);

	void addCoverArtToCache(const char* coverArtId, const void * coverArtData, unsigned int dataLength);
	void getCoverArtById(const char* coverArtId, char* &coverArtData, unsigned int &dataLength);
	void getCoverArtByTrackId(const char* trackId, std::string &out_coverId, char* &coverArtData, unsigned int &dataLength);

	bool getTrackDetailsByUrl(const char* url, Track &t);

	void clearCoverArtCache();
	void clearCache();

	void reloadCache();
};
