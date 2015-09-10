#pragma once

#include "foo_subsonic.h"
#include "CoreEntity.h"
#include "track.h"
#include <list>

/*
<album id="200005519" album="$5.98 E.P. Garage Days Re-Revisited" title="$5.98 E.P. Garage Days Re-Revisited [1987]" name="$5.98 E.P. Garage Days Re-Revisited" isDir="true" coverArt="200005519" songCount="5" duration="1495" artistId="100001438" parent="100001438" artist="Metallica" year="1987" genre="Heavy Metal"/>
*/

class Album : public CoreEntity {
	MEMBER(int, id)
		STRING_MEMBER(title)
		STRING_MEMBER(coverArt)
		MEMBER(int, songCount)
		MEMBER(long, duration)
		MEMBER(int, artistid)
		MEMBER(int, parentid)

		STRING_MEMBER(artist)
		STRING_MEMBER(releasedate)
		STRING_MEMBER(genre)
		STRING_MEMBER(year)
public:

	void addTrack(Track t) {
		albumTracks.push_back(t);
	}

	std::list<Track>* getTracks() {
		return &albumTracks;
	}


	Album() :
		CoreEntity(),
		id(0),
		songCount(0),
		duration(0),
		artistid(0),
		parentid(0)
		
		{};

	~Album() {
	}

private:
	std::list<Track> albumTracks;
};

