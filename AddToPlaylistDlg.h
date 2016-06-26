#pragma once

#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"
#include "playlistQueryThread.h"
#include "playlistupdater.h"

#include "SimpleHttpClientConfigurator.h"
#include "sqliteCacheDb.h"

#include "consts.h"


class AddToPlaylistDlg : public CDialogImpl<AddToPlaylistDlg> {
private:
	CButton refresh_button;
	CEdit new_playlist_name;

	CComboBox cbAvailablePlaylists;

	HACCEL    m_haccelerator;
	foo_subsonic::SubsonicLibraryScanner scanner;

	std::list<Track*>* trackList;
public:
	enum { IDD = IDD_ADDTOPLAYLIST };

	BEGIN_MSG_MAP(AddToPlaylistDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(IDC_BTN_REFRESHLIST, OnRefreshList)
		MESSAGE_HANDLER(ID_CONTEXT_UPDATEPLAYLIST_DONE, OnContextPlaylistUpdateDone);
		//MESSAGE_HANDLER(ID_SEARCH_DONE, OnSearchDone)
	END_MSG_MAP()

	
	AddToPlaylistDlg(std::list<Track*>* lTrackList) {
		m_haccelerator = NULL;
		trackList = lTrackList;
		Create(core_api::get_main_window());
	}


	bool OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
		static_api_ptr_t<modeless_dialog_manager>()->add(m_hWnd);

		new_playlist_name.Attach(GetDlgItem(IDC_TXT_NEW_PL));
		refresh_button = GetDlgItem(IDC_BTN_REFRESHLIST);

		cbAvailablePlaylists = GetDlgItem(IDC_CB_AVAIL_PL);

		// populate combobox with current playlists
		updateComboBox();

		return true;
	}

	void updateComboBox() {

		cbAvailablePlaylists.Clear();

		std::list<Playlist>* playlists = SqliteCacheDb::getInstance()->getAllPlaylists();
		std::list<Playlist>::iterator it;
		for (it = playlists->begin(); it != playlists->end(); it++) {
			pfc::string8 tmp = it->get_id();
			tmp << " - " << it->get_name();
			pfc::stringcvt::string_wide_from_utf8 playlistName(tmp);
			cbAvailablePlaylists.AddString(playlistName);
		}
	}

	void OnClose() {
		DestroyWindow();
	}

	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl) {
		DestroyWindow();
	}


	void OnOk(UINT uNotifyCode, int nID, CWindow wndCtl) {
		if (nID == IDOK) {
			// SubsonicLibraryScanner *scanner, HWND window, boolean update, Playlist* playlist
			if (IsDlgButtonChecked(IDC_RADIO_NEWPL) == BST_CHECKED) {
				Playlist playlist = Playlist();
				pfc::string8 tmp;
				uGetWindowText(GetDlgItem(IDC_TXT_NEW_PL), tmp);
				playlist.set_name = tmp;
				threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::PlaylistUpdateThread>(&scanner, m_hWnd, false, &playlist),
					threaded_process::flag_show_progress_dual | threaded_process::flag_show_item | threaded_process::flag_show_abort, m_hWnd, "Creating playlist on Subsonic Server");
			}
			else {
				pfc::string8 tmp = string_utf8_from_window(m_hWnd, IDC_CB_AVAIL_PL);
				size_t pos = tmp.find_first("-");
				tmp.truncate(pos);
				Playlist playlist = Playlist();
				playlist.set_id(tmp);
				
				threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::PlaylistUpdateThread>(&scanner, m_hWnd, true, &playlist, trackList),
					threaded_process::flag_show_progress_dual | threaded_process::flag_show_item | threaded_process::flag_show_abort, m_hWnd, "Updating playlist on Subsonic Server");


			}
		}
	}

	void OnRefreshList(UINT uNotifyCode, int nID, CWindow wndCtl) {
		threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::PlaylistQueryThread>(&scanner, m_hWnd),
			threaded_process::flag_show_progress_dual | threaded_process::flag_show_item | threaded_process::flag_show_abort, m_hWnd, "Querying playlist data from Subsonic Server");
	}
	
	LRESULT OnContextPlaylistUpdateDone(UINT, WPARAM, LPARAM, BOOL &) {
		updateComboBox();

		return 0;
	}

};