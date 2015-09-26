#pragma once
#include "foo_subsonic.h"

#ifndef DUMMY_GUID

#define DUMMY_GUID
#endif


class HttpAlbumArtExtractorInstance : public album_art_extractor_instance_v2 {
public:
	void set(const GUID & p_what, album_art_data_ptr p_content) { 
		m_content.set(p_what, p_content); 
	}

	bool have_item(const GUID & p_what) { 
		return m_content.have_item(p_what); 
	}

	album_art_data_ptr query(const GUID & p_what, abort_callback & p_abort) {
		album_art_data_ptr temp;
		if (!m_content.query(p_what, temp)) throw exception_album_art_not_found();
		return temp;
	}

	bool is_empty() const { 
		return m_content.get_count() == 0; 
	}

	bool remove(const GUID & p_what) {
		return m_content.remove(p_what);
	}

	album_art_path_list::ptr query_paths(const GUID & p_what, abort_callback & p_abort) {
		// we don't need a path list, and just return a dummy here
		return new service_impl_t<album_art_path_list_dummy>();
	}
private:
	pfc::map_t<GUID, album_art_data_ptr> m_content;
};