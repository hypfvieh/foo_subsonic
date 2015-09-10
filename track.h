#pragma once

#include "foo_subsonic.h"
#include "CoreEntity.h"

class Track : public CoreEntity {
	MEMBER(int, duration)
	MEMBER(int, tracknumber)
	STRING_MEMBER(title)
	STRING_MEMBER(streamUrl)
	STRING_MEMBER(parentId)
	STRING_MEMBER(artistId)
	STRING_MEMBER(artist)
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
		tracknumber(0) {}

	~Track() {

	}

};
