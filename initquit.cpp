#include "foo_subsonic.h"
#include "album_art.h"
#include "sqliteCacheDb.h"

class myinitquit : public initquit {
public:
	void on_init() {	
		console::print("Subsonic component: on_init()");		
		SqliteCacheDb::getInstance();
	}
	void on_quit() {
		console::print("Subsonic component: on_quit()");
	}
};


static initquit_factory_t<myinitquit> g_myinitquit_factory;
