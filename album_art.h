#pragma once

#include "foo_subsonic.h"
#include <regex>
#include "simplehttpclient.h"
#include "SimpleHttpClientConfigurator.h"
#include "AlbumArtExtractorInstanceHttp.h"
#include "FakeAlbumArtExtractor.h"
#include <fstream>


class HttpAlbumArtFallback : public album_art_fallback {
public:

	album_art_extractor_instance_v2::ptr open(metadb_handle_list_cref items, pfc::list_base_const_t<GUID> const & ids, abort_callback & abort) {

		if (!Preferences::coverart_download) { // user disabled cover download
			throw exception_album_art_not_found();
		}

		for (unsigned int i = 0; i < items.get_size(); i++) {
			service_impl_single_t<FakeAlbumArtExtractor> *faae = new service_impl_single_t<FakeAlbumArtExtractor>();
			if (faae->is_our_path(items[i]->get_path(), "")) {
				album_art_extractor_instance_v2::ptr p2 = faae->open_v2(NULL, items[i]->get_path(), abort);
				return p2;
			}			
		}
		throw exception_album_art_not_found();
	}
};

static service_factory_t<HttpAlbumArtFallback> g_http_album_art_fallback_factory;

