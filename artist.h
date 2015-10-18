#pragma once

#include "foo_subsonic.h"
#include "CoreEntity.h"

class Artist : public CoreEntity {

public:

	Artist(pfc::string8 _id) : CoreEntity(ENTRY_TYPE_ARTIST) {
		set_id(_id);
	};

	~Artist() {
	}

};

