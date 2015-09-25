#pragma once

#include "foo_subsonic.h"
#include <regex>
#include "simplehttpclient.h"
#include "SimpleHttpClientConfigurator.h"
#include "AlbumArtExtractorInstanceHttp.h"
#include <fstream>

class FakeAlbumArtExtractor : public service_base {

private:
	GUID dummyGuid = { 0x4dc43764, 0x8e3f, 0x42a5,{ 0x9f, 0x94, 0x99, 0x1c, 0xba, 0x90, 0xd5, 0x69 } };
	void dbg(const void * buffer, size_t buffersize) {
		FILE *file = fopen("C:\\testfile_charbuf.jpg", "wb");
		fwrite(buffer, sizeof(u_char), buffersize, file);
		fclose(file);
	}

public:
	bool is_our_path(const char * p_path, const char * p_extension);
	album_art_extractor_instance_v2::ptr open_v2(file_ptr p_filehint, const char *p_path, abort_callback &p_abort);
	pfc::string8 extractPathId(const char* p_path);

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(FakeAlbumArtExtractor);
};