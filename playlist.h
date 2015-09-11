#pragma once

#include "foo_subsonic.h"
#include "CoreEntity.h"

class Playlist : public CoreEntity {
	MEMBER(int, duration)
	MEMBER(int, songcount)
	STRING_MEMBER(name)
	STRING_MEMBER(comment)
	STRING_MEMBER(owner)
	MEMBER(bool, isPublic)
	STRING_MEMBER(coverArt)

public:
	void addTrack(Track t) {
		albumTracks.push_back(t);
	}

	std::list<Track>* getTracks() {
		return &albumTracks;
	}

	Playlist()
		: CoreEntity(),
		duration(0),
		songcount(0),
		isPublic(FALSE)
		{}

	~Playlist() {

	}
private:
	std::list<Track> albumTracks;

};
