#pragma once

#include "foo_subsonic.h"

class playlistUpdater : public process_locations_notify {
public:
	void on_completion(const pfc::list_base_const_t<metadb_handle_ptr> & p_items)
	{
		for (unsigned int i = 0; i < p_items.get_count(); i++) {
			metadb_handle_ptr item = p_items.get_item(i);

			static_api_ptr_t<metadb_io_v3> meta_db_io_api;
			file_info_impl f_info;
			Track t;

			if (SqliteCacheDb::getInstance()->getTrackDetailsByUrl(item.get_ptr()->get_path(), t)) {

				pfc::string8 codec = t.get_contentType();
				codec << " (" << t.get_suffix() << ")";

				int duration = atoi(t.get_duration());
				int bitrate = atoi(t.get_bitrate());

				uDebugLog() << "Artist: " << t.get_artist() << " - Album: " << t.get_album() << " - Title: " << t.get_title();

				f_info.meta_set("Artist", t.get_artist());
				f_info.meta_set("Album", t.get_album());
				f_info.meta_set("Title", t.get_title());
				f_info.meta_set("Year", t.get_year());
				f_info.meta_set("Tracknumber", t.get_tracknumber());
				f_info.set_length(duration); // seconds
				f_info.info_set_bitrate(bitrate);
				f_info.meta_set("Genre", t.get_genre());
				f_info.info_set("Codec", codec);

				t_filestats stats = item.get_ptr()->get_filestats();
				stats.m_size = atoi(t.get_size());

				meta_db_io_api->hint_async(item.get_ptr(), f_info, stats, true);
			}

		}

		static_api_ptr_t<playlist_manager> plm;
		plm->activeplaylist_add_items(p_items, bit_array_true());

	};

	void on_aborted() { };
};

service_ptr_t<playlistUpdater> p_notify = new service_impl_t<playlistUpdater>();