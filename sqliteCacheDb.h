#pragma once

#include "foo_subsonic.h"
#include "album.h"
#include "playlist.h"
#include <map>

#define SQL_TABLE_CREATE_SIZE 15
#define SQL_TABLE_VERSION 2

const std::string SQL_TABLE_VERSION_KEY = "version";
const std::string SQL_TABLE_VERSION_STR = std::to_string(SQL_TABLE_VERSION);

const std::string SQL_VERSION_INFO = "INSERT OR REPLACE INTO metainfo (infokey, infovalue) VALUES ('" + SQL_TABLE_VERSION_KEY + "','" + SQL_TABLE_VERSION_STR + "')";

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

	const char* sql_table_create[SQL_TABLE_CREATE_SIZE] = {
		"CREATE TABLE IF NOT EXISTS metainfo (infokey TEXT PRIMARY KEY, infovalue TEXT)",
		SQL_VERSION_INFO.c_str(),
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
	
	bool isInteger(const std::string & s)
	{
		if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

		char * p;
		strtol(s.c_str(), &p, 10);

		return (*p == 0);
	}

	std::wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

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
	void getAlbumById(const char* id, Album &a);

	void clearCoverArtCache();
	void clearCache();

	void reloadCache();

	void checkMetaInfo();
};
