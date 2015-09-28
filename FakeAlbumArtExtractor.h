#pragma once

#include "foo_subsonic.h"
#include <regex>
#include "simplehttpclient.h"
#include "SimpleHttpClientConfigurator.h"
#include "AlbumArtExtractorInstanceHttp.h"
#include <fstream>

class FakeAlbumArtExtractor : public service_base {

public:
	bool is_our_path(const char * p_path, const char * p_extension);
	album_art_extractor_instance_v2::ptr open_v2(file_ptr p_filehint, const char *p_path, abort_callback &p_abort);
	pfc::string8 extractPathId(const char* p_path);

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(FakeAlbumArtExtractor);
};

static service_factory_t<FakeAlbumArtExtractor> g_fake_album_art_fallback_factory;