#pragma once

#include "foo_subsonic.h"
#include "CoreEntity.h"
#include "track.h"
#include <list>

class Album : public CoreEntity {
	STRING_MEMBER(id)
	STRING_MEMBER(title)
	STRING_MEMBER(coverArt)
	MEMBER(int, songCount)
	MEMBER(long, duration)
	STRING_MEMBER(artistid)
	STRING_MEMBER(parentid)

	STRING_MEMBER(artist)
	STRING_MEMBER(genre)
	STRING_MEMBER(year)

public:

	void addTrack(Track* t) {
		albumTracks.push_back(t);
	}

	void addTracks(std::list<Track> list) {
		std::list<Track>::iterator trackIterator;
		for (trackIterator = list.begin(); trackIterator != list.end(); trackIterator++) {
			Track t = *trackIterator;
			albumTracks.push_back(&t);
		}
	}

	std::list<Track*>* getTracks() {
		return &albumTracks;
	}


	Album() :
		CoreEntity(ENTRY_TYPE_ALBUM),
		songCount(0),
		duration(0)		
		{};

	~Album() {
	}
private:
	std::list<Track*> albumTracks;

};

