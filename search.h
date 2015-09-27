#pragma once

#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"
#include "searchQueryThread.h"
#include "ListviewHelper.h"
#include "SimpleHttpClientConfigurator.h"


class SearchDialog : public CDialogImpl<SearchDialog> {
private:
	CButton find_button;
	CEdit search_term;
	CListViewCtrl results;
	HACCEL    m_haccelerator;

	foo_subsonic::SubsonicLibraryScanner scanner;

	COLORREF bgColor;
	COLORREF fgColor;

public:
	enum { IDD = IDD_SEARCH };

	BEGIN_MSG_MAP(SearchDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		MESSAGE_HANDLER(ID_SEARCH_DONE, OnSearchDone)
		NOTIFY_HANDLER(IDC_RESULTLIST, NM_DBLCLK, OnLButtonDblClick)
	END_MSG_MAP()

	enum columns {
		artist_column,
		track_column,
		album_column,		
		duration_column
	};

	SearchDialog(COLORREF fgcolor, COLORREF bgcolor) {
		m_haccelerator = NULL;
		fgColor = fgcolor;
		bgColor = bgcolor;
		Create(core_api::get_main_window());
	}

	
	bool OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
		static_api_ptr_t<modeless_dialog_manager>()->add(m_hWnd);

		search_term.Attach(GetDlgItem(IDC_SEARCHTERM));
		find_button = GetDlgItem(IDOK);
		results.Attach(GetDlgItem(IDC_RESULTLIST));

		auto styles = LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP;
		results.SetExtendedListViewStyle(styles, styles);

		// Adding release list columns
		listviewHelper::insert_column(results, artist_column, "Artist", 104);
		listviewHelper::insert_column(results, track_column, "Track", 110);
		listviewHelper::insert_column(results, album_column, "Album", 110);
		listviewHelper::insert_column(results, duration_column, "Duration", 50);

		results.SetBkColor(bgColor);
		results.SetTextColor(fgColor);
		results.SetTextBkColor(bgColor);
		
		search_term.SetFocus();
		
		return true;
	}

	void OnClose() {
		DestroyWindow();
	}

	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl) {
		uDebugLog() << "OnCancel!";
		DestroyWindow();
	}

	
	void OnOk(UINT uNotifyCode, int nID, CWindow wndCtl) {
		if (nID == IDOK) {
			uDebugLog() << "search enter";

			pfc::string8 params = SimpleHttpClientConfigurator::url_encode(string_utf8_from_window(m_hWnd, IDC_SEARCHTERM));

			threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::SearchQueryThread>(&scanner, m_hWnd, params),
				threaded_process::flag_show_progress | threaded_process::flag_show_abort, m_hWnd, "Searching Subsonic Server");
		}
	}	
	
	LRESULT OnSearchDone(UINT, WPARAM, LPARAM, BOOL&) {
		std::list<Track*>* trackList = SqliteCacheDb::getInstance()->getAllSearchResults()->getTracks();

		std::list<Track*>::iterator trackIterator;
		for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
			
			char durationStr[20];
			snprintf(durationStr, sizeof(durationStr), "%02d:%02d", (*trackIterator)->get_duration() / 60, (*trackIterator)->get_duration() % 60);

			Track* store = *trackIterator;

			listviewHelper::insert_item4(results, 0, (*trackIterator)->get_artist(), (*trackIterator)->get_title(), (*trackIterator)->get_album(), durationStr, (DWORD_PTR)store);
		}

		return 0;
	}

	LRESULT OnLButtonDblClick(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/) {

		for (INT nItem = results.GetNextItem(-1, LVNI_SELECTED); nItem >= 0; nItem = results.GetNextItem(nItem, LVNI_SELECTED))
		{
			if (nItem > -1) {
				DWORD_PTR ptr = results.GetItemData(nItem);
				CoreEntity* coreType = reinterpret_cast<CoreEntity*>(ptr);
				if (coreType->get_type() == ENTRY_TYPE_TRACK) { // Track
					if (ptr != NULL) {
						Track* track = reinterpret_cast<Track*>(ptr);

						uDebugLog() << "Got Track=" << track->get_title() << ", Artist=" << track->get_artist();

						const char* url = track->get_streamUrl().c_str();

						static_api_ptr_t<playlist_incoming_item_filter_v2>()->process_locations_async(
							pfc::list_single_ref_t<const char*>(url),
							playlist_incoming_item_filter_v2::op_flag_background,
							NULL,
							NULL,
							m_hWnd,
							p_notify
							);
					}
				}
			}
		}
		return 0;
	}
	
};