#pragma once

#include "foo_subsonic.h"
#include <regex>



class HttpAlbumArtFallback : public album_art_fallback {

public:

	HttpAlbumArtFallback() {
		uDebugLog() << "AlbumArtInjector Loaded";
	}

	album_art_extractor_instance_v2::ptr open(metadb_handle_list_cref items, pfc::list_base_const_t<GUID> const & ids, abort_callback & abort) {
		uDebugLog() << "AlbumArtInjector open() called: ";

		if (items.get_size() > 0) {

			pfc::string url = "^";
			url += Preferences::connect_url_data;

			std::regex rePort(".*(:[[:digit:]]+).*");
			std::smatch portmatch;
			std::string urlwithport = url.c_str();

			if (!std::regex_search(urlwithport, portmatch, rePort)) { // no port given, append the proper default
				if (url.startsWith("^https://")) {
					url += ":443";
				}
				else {
					url += ":80";
				}
			}

			if (url.lastChar() != '/') {
				url += "/";
			}
			// escape certain URL parts to not confuse regex matcher
			url = url.replace("\\", "\\\\");
			url = url.replace("/", "\\/");
			url = url.replace(".", "\\.");
			url = url.replace(":", "\\:");
			url += "rest/stream\\.view\\?";
			url += "(v=.*)&(c=.*)&(u=.*)&(p=.*)&id=(.*)$";

			std::regex re(url.c_str());
			std::smatch match;
			 
			for (unsigned int i = 0; i < items.get_size(); i++) {
				std::string itempath = items.get_item(i).get_ptr()->get_path();
				
				std::smatch match;
				if (std::regex_search(itempath, match, re)) { // matches
					//album_art_extractor bla = album_art_extractor::g
					uDebugLog() << "Found matching URL: " << itempath.c_str();

					return nullptr;
				}
				else { // does not match
					return nullptr;
				}
			}			
		}
		
		return nullptr;
	}
	
};

static service_factory_t<HttpAlbumArtFallback> g_http_album_art_fallback_factory;
