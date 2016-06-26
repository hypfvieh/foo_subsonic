#include "foo_subsonic.h"
#include "track.h"
#include "AddToPlaylistDlg.h"
#include <regex>
#include <list>


namespace foo_subsonic {
	static const GUID guid_subsonic_contextmenu = { 0xee2c354, 0x7118, 0x4640,{ 0x84, 0xab, 0xf0, 0x7f, 0x6f, 0xb3, 0x50, 0x1d } };

	// Switch to contextmenu_group_factory to embed your commands in the root menu but separated from other commands.

	//static contextmenu_group_factory g_mygroup(guid_mygroup, contextmenu_groups::root, 0);
	static contextmenu_group_popup_factory g_mygroup(guid_subsonic_contextmenu, contextmenu_groups::root, "Subsonic", 0);

	class ContextMenu : public contextmenu_item_simple {
	public:

		GUID get_parent() { return guid_subsonic_contextmenu; }

		unsigned get_num_items() {
			return 1;
		}

		void get_item_name(unsigned p_index, pfc::string_base & p_out) {
			static const char *item_name[] = {
				"Add to Remote Playlist"
			};
			p_out = item_name[p_index];
		}

		const char* extractTrackIdFromUrl(pfc::string8 path) {
			std::regex reTrackid("(?:.*)id=([[:digit:]]+).*");
			std::smatch trackmatch;
			std::string fullpath = path.c_str();
			if (std::regex_search(fullpath, trackmatch, reTrackid)) { // found track id
				for (auto x : trackmatch) {
					return x.str().c_str();
				}				
			}

		}

		void context_command(unsigned p_index, metadb_handle_list_cref p_data, const GUID& p_caller) {
			unsigned int count = p_data.get_count();
		
			switch (p_index) {
				case 0: {
					std::list<Track*>* trackList = new std::list<Track*>();
					for (t_size i = 0; i < count; i++) {
						auto item = p_data.get_item(i);
						pfc::string tmp = item->get_path();

						// only add urls which belongs to our subsonic server
						if (tmp.startsWith("http://") || tmp.startsWith("https://") && tmp.contains("stream.view")) {
							Track* t = new Track();
						
							t->set_id(extractTrackIdFromUrl(item->get_path()));
							trackList->push_back(t);
						}
					}
					new AddToPlaylistDlg(trackList);
					break;
				}
			}
		}

		GUID get_item_guid(unsigned p_index) {
			static const GUID guid_foo_mb_menu[] = {
				{ 0xf0a4fecd, 0x5204, 0x495b,{ 0x95, 0xf7, 0xcb, 0xd4, 0x90, 0xba, 0xe1, 0x24 } }
			};
			return guid_foo_mb_menu[p_index];
		}

		bool get_item_description(unsigned p_index, pfc::string_base & p_out) {
			static const char *item_description[] = {
				"Add selected tracks to an existing or new (remote) Subsonic playlist.",
			};
			p_out = item_description[p_index];
			return true;
		}


	};

	contextmenu_item_factory_t<ContextMenu> _;
}