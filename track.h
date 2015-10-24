#pragma once

#include "foo_subsonic.h"
#include "CoreEntity.h"

class Track : public CoreEntity {
	STRING_MEMBER(duration)
	STRING_MEMBER(tracknumber)
	STRING_MEMBER(title)
	STRING_MEMBER(streamUrl)
	STRING_MEMBER(parentId)
	STRING_MEMBER(artistId)
	STRING_MEMBER(artist)
	STRING_MEMBER(year)
	STRING_MEMBER(bitrate)
	STRING_MEMBER(contentType)
	STRING_MEMBER(size)
	STRING_MEMBER(genre)
	STRING_MEMBER(suffix)
	STRING_MEMBER(album)
	STRING_MEMBER(coverArt)

public:
	Track()
		: CoreEntity(ENTRY_TYPE_TRACK),
		duration("0"),
		bitrate("0"),
		size("0"),
		tracknumber("0") {}

	~Track() {

	}

};
