#pragma once

#include "foo_subsonic.h"

class playlistUpdater : public process_locations_notify {
public:
	void on_completion(const pfc::list_base_const_t<metadb_handle_ptr> & p_items)
	{

		file_info_impl finfo;

		if (p_items.get_count() > 0) {
			p_items.get_item(0).get_ptr()->get_info(finfo);
			finfo.info_set_ex("TITLE", sizeof("TITLE"), "Tralala", sizeof("Tralala"));
		}
		static_api_ptr_t<playlist_manager> plm;
		plm->activeplaylist_add_items(p_items, bit_array_true());

	};

	void on_aborted() { };
};

service_ptr_t<playlistUpdater> p_notify = new service_impl_t<playlistUpdater>();