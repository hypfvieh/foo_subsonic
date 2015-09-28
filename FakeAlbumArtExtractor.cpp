#include "foo_subsonic.h"
#include "FakeAlbumArtExtractor.h"
#include "sqliteCacheDb.h"

const GUID FakeAlbumArtExtractor::class_guid = { 0xe1b99b8c, 0x518a, 0x4ea3,{ 0x89, 0x33, 0xf4, 0xd8, 0x7d, 0x3e, 0x68, 0x13 } };


pfc::string8 FakeAlbumArtExtractor::extractPathId(const char* p_path) {

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

	std::string itempath = p_path;
	if (std::regex_search(itempath, match, re)) { // matches
		uDebugLog() << "Found matching URL: " << itempath.c_str();
		if (match.size() >= 5) {
			uDebugLog() << "id: " << match.str(5).c_str();
			return match.str(5).c_str();
		}
	}
	return NULL;
}

album_art_extractor_instance_v2::ptr FakeAlbumArtExtractor::open_v2(file_ptr p_filehint, const char *p_path, abort_callback &p_abort) {
	
	char* buffer = NULL;
	size_t buffSize = 0;

	pfc::string8 trackId = extractPathId(p_path);
	
	std::string coverId;
	SqliteCacheDb::getInstance()->getCoverArtByTrackId(trackId.c_str(), coverId, buffer, buffSize);

	if (buffSize == 0) { // no cached image, retrive new
		SimpleHttpClientConfig config;
		SimpleHttpClientConfigurator::createSimpleHttpClientConfigFromPreferences(config);
		SimpleHttpClient client = SimpleHttpClient(config);


		pfc::string8 artId = "id=";
		artId << SimpleHttpClientConfigurator::url_encode(trackId);

		if (Preferences::coverart_resize) { // User wants to 'save bandwidth', add resize flag
			artId << "&size=" << Preferences::coverart_size_data;
		}

		pfc::string8 url = SimpleHttpClientConfigurator::buildRequestUrl("getCoverArt", artId);

		client.open(url.c_str());
		client.send_request(buffer, buffSize);
		if (buffSize > 0 && !coverId.empty()) { // store coverart to cachedb
			SqliteCacheDb::getInstance()->addCoverArtToCache(coverId.c_str(), buffer, buffSize);
		}
	}

	if (buffSize <= 0) {
		throw exception_album_art_not_found();
	}

	service_impl_t<HttpAlbumArtExtractorInstance>* inst = new service_impl_t<HttpAlbumArtExtractorInstance>();

	album_art_data_ptr data = album_art_data_impl::g_create(buffer, buffSize);

	inst->set(album_art_ids::cover_front, data);

	return inst;
}


bool FakeAlbumArtExtractor::is_our_path(const char * p_path, const char * p_extension) {
	return (extractPathId(p_path) != NULL);
}