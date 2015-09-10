#pragma once

#include "foo_subsonic.h"
#include "CoreEntity.h"

struct TrackData {
	int duration;
	int parentId;	
	int bitrate;
	int size;
	pfc::string8 artistId;
	pfc::string8 title;
	pfc::string8 artist;
	pfc::string8 type;
	pfc::string8 year;
	pfc::string8 contentType;
	pfc::string8 genre;
	pfc::string8 suffix;
	pfc::string8 album;
	pfc::string8 coverArt;
};

class Track : public CoreEntity {
	MEMBER(int, duration)
		MEMBER(int, tracknumber)
		STRING_MEMBER(title)
		STRING_MEMBER(streamUrl)
		MEMBER(int, parentId)
		MEMBER(int, artistId)
		STRING_MEMBER(artist)
		STRING_MEMBER(type)
		STRING_MEMBER(year)
		MEMBER(int, bitrate)
		STRING_MEMBER(contentType)
		MEMBER(int, size)
		STRING_MEMBER(genre)
		STRING_MEMBER(suffix)
		STRING_MEMBER(album)
		STRING_MEMBER(coverArt)

public:
	Track()
		: CoreEntity(),
		duration(0),
		bitrate(0),
		size(0),
		parentId(0),
		artistId(0),
		tracknumber(0) {}

	~Track() {

	}

};
