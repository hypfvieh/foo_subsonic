#pragma once

#include "foo_subsonic.h"
#include "CoreEntity.h"

class Playlist : public CoreEntity {
	MEMBER(int, playlistId)
	MEMBER(int, duration)
	STRING_MEMBER(name)
	STRING_MEMBER(comment)
	STRING_MEMBER(owner)
	MEMBER(bool, isPublic)
	STRING_MEMBER(coverArt)

public:
	Playlist()
		: CoreEntity(),
		duration(0),
		playlistId(0),
		isPublic(FALSE)
		{}

	~Playlist() {

	}

};
