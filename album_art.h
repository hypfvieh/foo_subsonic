#pragma once

#include "foo_subsonic.h"
#include <regex>
#include "simplehttpclient.h"
#include "SimpleHttpClientConfigurator.h"
#include "AlbumArtExtractorInstanceHttp.h"
#include "FakeAlbumArtExtractor.h"
#include <fstream>

GUID dummyGuid = { 0x4dc43764, 0x8e3f, 0x42a5,{ 0x9f, 0x94, 0x99, 0x1c, 0xba, 0x90, 0xd5, 0x69 } };

class HttpAlbumArtExtractor : public album_art_extractor {
private:
	pfc::string8 id;

	// {4DC43764-8E3F-42A5-9F94-991CBA90D569}
	


	void dbg(const void * buffer, size_t buffersize) {
		FILE *file = fopen("C:\\testfile_charbuf.jpg", "wb");
		fwrite(buffer, sizeof(u_char), buffersize, file);
		fclose(file);
	}	

public:
	//const GUID class_guid = { 0x290ac90, 0x4d2b, 0x4856,{ 0xbf, 0xd9, 0x8e, 0xf9, 0x86, 0xde, 0x37, 0x8b } };
	/*



	HttpAlbumArtExtractor(std::string trackId) {
		id = trackId.c_str();
	}
	*/
	pfc::string8 extractPathId(const char* p_path) {

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

	album_art_extractor_instance_v2::ptr open2(file_ptr p_filehint, const char *p_path, abort_callback &p_abort) {
		SimpleHttpClientConfig config;
		SimpleHttpClientConfigurator::createSimpleHttpClientConfigFromPreferences(config);
		SimpleHttpClient client = SimpleHttpClient(config);

		char* buffer = NULL;
		size_t buffSize = 0;

		pfc::string8 id = extractPathId(p_path);

		pfc::string8 artId = "id=";
		artId << SimpleHttpClientConfigurator::url_encode(id);

		pfc::string8 url = SimpleHttpClientConfigurator::buildRequestUrl("getCoverArt", artId);

		client.open(url.c_str());
		client.send_request(buffer, buffSize);

		dbg(buffer, buffSize);

		if (buffSize <= 0) {
			throw exception_album_art_not_found();
		}

		auto inst = new service_impl_t<HttpAlbumArtExtractorInstance>();
		
		//service_ptr_t<HttpAlbumArtExtractorInstance> inst; // always NULL?!?!
		
		album_art_data_ptr data = album_art_data_impl::g_create(buffer, buffSize);

		inst->set(album_art_ids::cover_front, data);

		return inst;
	}

	album_art_extractor_instance_ptr open(file_ptr p_filehint, const char *p_path, abort_callback &p_abort) {

		SimpleHttpClientConfig config;
		SimpleHttpClientConfigurator::createSimpleHttpClientConfigFromPreferences(config);
		SimpleHttpClient client = SimpleHttpClient(config);

		char* buffer = NULL;
		size_t buffSize = 0;

		pfc::string8 id = extractPathId(p_path);

		pfc::string8 artId = "id=";
		artId << SimpleHttpClientConfigurator::url_encode(id);

		pfc::string8 url = SimpleHttpClientConfigurator::buildRequestUrl("getCoverArt", artId);
		
		client.open(url.c_str());
		client.send_request(buffer, buffSize);

		dbg(buffer, buffSize);

		if (buffSize <= 0) {
			throw exception_album_art_not_found();
		}

		auto inst = new service_impl_t<HttpAlbumArtExtractorInstance>();

		//service_ptr_t<HttpAlbumArtExtractorInstance> inst; // always NULL?!?!

	    album_art_data_ptr data = album_art_data_impl::g_create(buffer, buffSize);

		inst->set(dummyGuid, data);

		

		return inst;
	}


	bool is_our_path(const char * p_path, const char * p_extension) {
		return (extractPathId(p_path) != NULL);
	}


};

class HttpAlbumArtManager : public album_art_manager_v2 {
	
public:
	// {5377C5C2-0080-4FFD-8734-12C445D8B77F}
	const GUID class_guid = { 0x5377c5c2, 0x80, 0x4ffd,{ 0x87, 0x34, 0x12, 0xc4, 0x45, 0xd8, 0xb7, 0x7f } };
	

	album_art_extractor_instance_v2::ptr open(metadb_handle_list_cref items, pfc::list_base_const_t<GUID> const & ids, abort_callback & abort) {
		return NULL;
	}

	
	album_art_extractor_instance_v2::ptr open_stub(abort_callback & abort) {
		return NULL;
	}

};

class HttpAlbumArtFallback : public album_art_fallback {

public:

	HttpAlbumArtFallback() {
		uDebugLog() << "HttpAlbumArtFallback Loaded";
	}

	album_art_extractor_instance_v2::ptr open(metadb_handle_list_cref items, pfc::list_base_const_t<GUID> const & ids, abort_callback & abort) {
		uDebugLog() << "HttpAlbumArtFallback open() called: ";

		for (unsigned int i = 0; i < items.get_size(); i++) {

		//	service_enum_t<album_art_extractor> e;
		//	service_ptr_t<album_art_extractor> ptr;

		//	service_ptr_t<HttpAlbumArtExtractor> foo = new service_impl_t<HttpAlbumArtExtractor>();
		//	album_art_extractor_instance_v2::ptr p2 = foo->open2(NULL, items[i]->get_path(), abort);
		//	return p2;

			service_impl_single_t<FakeAlbumArtExtractor> *foo = new service_impl_single_t<FakeAlbumArtExtractor>();
			if (foo->is_our_path(items[i]->get_path(), "")) {
				album_art_extractor_instance_v2::ptr p2 = foo->open_v2(NULL, items[i]->get_path(), abort);
				return p2.get_ptr();
			}
			/*
			while (e.next(ptr))
			{
				if (ptr->is_our_path(items[i]->get_path(), ""))
				{
					//album_art_extractor_instance_ptr aaep;
					album_art_extractor_instance_v2::ptr aaep;

					//aaep = ptr->open(NULL, items[i]->get_path(), abort);

					
					album_art_data_ptr data = aaep->query(dummyGuid, abort);
					
					auto inst = new service_impl_t<HttpAlbumArtExtractorInstance>();

					//service_ptr_t<HttpAlbumArtExtractorInstance> inst; // always NULL?!?!
					inst->set(dummyGuid, data);

					//aaeiv2_stub->
					//album_art_extractor_instance_v2::ptr p2 = (http_album_art_extractor_instance_ptr) aaep;
					
					
				}
			}
				*/
			throw exception_album_art_not_found();

			/*
			static_api_ptr_t<album_art_manager_v2> aamv2;


			album_art_extractor_instance_v2::ptr aaeiv2_stub;
			aaeiv2_stub = aamv2->open_stub(abort);

			aaeiv2_stub->

			album_art_extractor_instance_v2::ptr aaeiv2;
			album_art_extractor_instance::ptr aaeiv;
			service_ptr_t<HttpAlbumArtExtractor> out;

			aaeiv = out->open(nullptr, items[i]->get_path(), abort);
		//	static_api_ptr_t<HttpAlbumArtExtractor> ext;
			
			
			aaeiv2 = aaeiv;
			//aaeiv2 = aamv2->open(items, ids, abort);

			//return  aaeiv2.get_ptr(); // new service_impl_t<HttpAlbumArtExtractor>(match.str(5));
			*/
		}
		throw exception_album_art_not_found();
		return NULL;	
	}
	
};

static service_factory_t<HttpAlbumArtExtractor> g_http_album_art_extractor_factory;
static service_factory_t<HttpAlbumArtFallback> g_http_album_art_fallback_factory;
//static service_factory_t<HttpAlbumArtExtractorInstance> g_http_album_art_extractor_instance_factory;
//static service_factory_t<HttpAlbumArtManager> g_http_album_art_manager_factory;

