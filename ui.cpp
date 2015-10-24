#include "foo_subsonic.h"
#include "ui.h"
#include "subsoniclibraryscanner.h"
#include "albumQueryThread.h"
#include "playlistQueryThread.h"
#include "artistUpdateThread.h"
#include "sqliteCacheDb.h"
#include "playlistupdater.h"
#include "search.h"
#include "artist.h"


using namespace foo_subsonic;

CSubsonicUi::CSubsonicUi(ui_element_config::ptr config, ui_element_instance_callback_ptr p_callback) : m_callback(p_callback), m_config(config) {
	AtlInitCommonControls(ICC_TREEVIEW_CLASSES);
}

LRESULT CSubsonicUi::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

	CTreeViewCtrlEx::SetDlgCtrlID(IDC_TREEVIEWCTRL);

	CTreeViewCtrlEx::SetTextColor(m_callback->query_std_color(ui_color_text));
	CTreeViewCtrlEx::SetBkColor(m_callback->query_std_color(ui_color_background));

	// show +/- and the connecting lines
	CTreeViewCtrlEx::ModifyStyle(1, TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);
	//CTreeViewCtrlEx::SetExtendedStyle()

	if (Preferences::load_album_cache_on_startup) {
		// Read cached album catalog
		SendMessage(m_hWnd, ID_CONTEXT_UPDATECATALOG_DONE, HIWORD(0), LOWORD(0));
	}
	if (Preferences::load_playlist_cache_on_startup) {
		// Read cached playlist
		SendMessage(m_hWnd, ID_CONTEXT_UPDATEPLAYLIST_DONE, HIWORD(0), LOWORD(0));
	}

	return lRet;
}

LRESULT CSubsonicUi::OnReloadCache(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	SqliteCacheDb::getInstance()->reloadCache();
	// Read cached album catalog
	SendMessage(m_hWnd, ID_CONTEXT_UPDATECATALOG_DONE, HIWORD(0), LOWORD(0));
	// Read cached playlist
	SendMessage(m_hWnd, ID_CONTEXT_UPDATEPLAYLIST_DONE, HIWORD(0), LOWORD(0));

	return 0;
}

LRESULT CSubsonicUi::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
	
	return lRet;
}

LRESULT CSubsonicUi::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&) {
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
	HMENU hMenu = ::CreatePopupMenu();

	if (NULL != hMenu) {

		HTREEITEM selected = GetSelectedItem();

		if ((selected != NULL)) {
			DWORD_PTR ptr = GetItemData(selected);
			if (ptr != NULL) {
				CoreEntity* coreType = reinterpret_cast<CoreEntity*>(ptr);
				if (coreType->get_type() == ENTRY_TYPE_ARTIST) { // Artist
					::AppendMenu(hMenu, MF_STRING, ID_CONTEXT_UPDATEARTIST, _T("Update selected Artist"));
					::AppendMenu(hMenu, MF_SEPARATOR, ID_CONTEXT_NOTHING, _T(""));
				}
			}
		}

		::AppendMenu(hMenu, MF_STRING, ID_CONTEXT_SEARCHDIALOG, _T("Search"));
		::AppendMenu(hMenu, MF_SEPARATOR, ID_CONTEXT_NOTHING, _T(""));
		::AppendMenu(hMenu, MF_STRING, ID_CONTEXT_UPDATECATALOG, _T("Retrieve/Update Subsonic Catalog"));
		::AppendMenu(hMenu, MF_STRING, ID_CONTEXT_UPDATEPLAYLIST, _T("Retrieve/Update Subsonic Playlists"));
		::AppendMenu(hMenu, MF_SEPARATOR, ID_CONTEXT_NOTHING, _T(""));
		::AppendMenu(hMenu, MF_STRING, ID_CONTEXT_RELOADCACHE, _T("Reload Cache"));
		
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		CPoint p = CPoint(xPos, yPos);
		ClientToScreen(&p);

		int sel = ::TrackPopupMenuEx(hMenu,
			TPM_CENTERALIGN | TPM_RIGHTBUTTON,
			p.x,
			p.y,
			m_hWnd,
			NULL);
		::DestroyMenu(hMenu);

	}
	return lRet;
}

LRESULT CSubsonicUi::OnLButtonDblClick(UINT, WPARAM, LPARAM, BOOL&) {
	HTREEITEM selected = GetSelectedItem();

	if ((selected != NULL)) {
		DWORD_PTR ptr = GetItemData(selected);
		if (ptr == NULL) {
			return 0;
		}
		CoreEntity* coreType = reinterpret_cast<CoreEntity*>(ptr);

		if (coreType->get_type() == ENTRY_TYPE_TRACK) { // Track
			

			if (ptr != NULL) {
				Track* track = reinterpret_cast<Track*>(ptr);

				uDebugLog() << "Got Track=" << track->get_title() << ", Artist=" << track->get_artist();

				const char* url = track->get_streamUrl().c_str();
				console::printf("Adding URL: %s", track->get_streamUrl());
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
		else { // Album or Playlist
			if (ptr != NULL) {
				std::list<Track*>* trackList;
				if (coreType->get_type() == ENTRY_TYPE_ALBUM) {
					Album* album = reinterpret_cast<Album*>(ptr);

					console::formatter() << "Got Album=" << album->get_title() << ", Artist=" << album->get_artist();

					trackList = album->getTracks();

				}
				else if (coreType->get_type() == ENTRY_TYPE_PLAYLIST) {
					Playlist* playlist = reinterpret_cast<Playlist*>(ptr);
					console::formatter() << "Got Playlist=" << playlist->get_name() << ", Entries=" << playlist->getTracks()->size();
					trackList = playlist->getTracks();
				}
				else {
					return 0;
				}

				if (trackList->size() > 0) {
					std::list<Track*>::iterator trackIterator;
					pfc::list_t<const char*> data;
					for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {						
						data.add_item((*trackIterator)->get_streamUrl());
					}

					static_api_ptr_t<playlist_incoming_item_filter_v2>()->process_locations_async(
						data,
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

LRESULT CSubsonicUi::OnContextCatalogUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const int result = MessageBox(L"Are you sure you want to update your local subsonic album list?", L"Update album list", MB_YESNO | MB_ICONQUESTION);

	if (result == IDYES) {
		threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::AlbumQueryThread>(&scanner, m_hWnd),
			threaded_process::flag_show_progress_dual | threaded_process::flag_show_item | threaded_process::flag_show_abort, m_hWnd, "Querying album catalog from Subsonic Server");
	}
	return 0;
}

LRESULT CSubsonicUi::OnContextPlaylistUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	const int result = MessageBox(L"Are you sure you want to update your local subsonic playlists?", L"Update playlists", MB_YESNO | MB_ICONQUESTION);

	if (result == IDYES) {
		threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::PlaylistQueryThread>(&scanner, m_hWnd),
			threaded_process::flag_show_progress_dual | threaded_process::flag_show_item | threaded_process::flag_show_abort, m_hWnd, "Querying playlist data from Subsonic Server");
	}

	return 0;
}

LRESULT CSubsonicUi::OnContextArtistUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	const int result = MessageBox(L"Are you sure you want to update the selected artist and all albums/tracks?", L"Update artist", MB_YESNO | MB_ICONQUESTION);

	if (result == IDYES) {
		HTREEITEM selected = GetSelectedItem();

		if ((selected != NULL)) {
			DWORD_PTR ptr = GetItemData(selected);
			if (ptr == NULL) {
				return 0;
			}
			CoreEntity* coreType = reinterpret_cast<CoreEntity*>(ptr);
			if (coreType->get_type() == ENTRY_TYPE_ARTIST) { // Artist
				
				if (!coreType->get_id().is_empty()) {
					threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::ArtistUpdateThread>(&scanner, m_hWnd, coreType->get_id()),
						threaded_process::flag_show_progress_dual | threaded_process::flag_show_item | threaded_process::flag_show_abort, m_hWnd, "Querying artist data from Subsonic Server");
				}
			}
		}
	}

	return 0;
}

LRESULT CSubsonicUi::OnSearchDialogShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	new SearchDialog(m_callback->query_std_color(ui_color_text), m_callback->query_std_color(ui_color_background));

	return 0;
}

HTREEITEM CSubsonicUi::getRootTreeNodeForArtist(wchar_t bgnLetter) {
	std::wstring alpha = _T("#ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	for (unsigned int i = 0; i < alpha.length(); i++) {
		wchar_t c = alpha[i];
		if (c == bgnLetter) {
			return catalogRootNodes[i];
		}
	}
	return catalogRootNodes[0]; // should match for everything which is not beginning with a letter
}

void CSubsonicUi::addTracksToTreeNode(std::list<Track*>* trackList, HTREEITEM albumNode, bool withTrackNumber, bool withArtistName) {
	
	std::list<Track*>::iterator trackIterator;
	for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
		pfc::stringcvt::string_wide_from_utf8 trackName((*trackIterator)->get_title());
		pfc::stringcvt::string_wide_from_utf8 artistName((*trackIterator)->get_artist());
		pfc::stringcvt::string_wide_from_utf8 trackNumber((*trackIterator)->get_tracknumber());
		
		wchar_t track[250];
		if (withTrackNumber && withArtistName) {
			swprintf(track, sizeof(track), L"%s) %s - %s", trackNumber.get_ptr(), artistName.get_ptr(), trackName.get_ptr());
		}
		else if (withTrackNumber) {
			swprintf(track, sizeof(track), L"%s) %s", trackNumber.get_ptr(), trackName.get_ptr());
		}
		else if (withArtistName) {
			swprintf(track, sizeof(track), L"%s - %s", artistName.get_ptr(), trackName.get_ptr());
		}
		else {
			swprintf(track, sizeof(track), L"%s", trackName.get_ptr());
		}
		Track* store = *trackIterator;
		HTREEITEM titleNode = InsertItem(track, albumNode, TVI_LAST);

		SetItemData(titleNode, (DWORD_PTR)store); // attach track meta data to node, so we can use this as shortcut for adding tracks to playlist
	}
}

void CSubsonicUi::populateTreeWithAlbums(std::list<Album>* albumList) {
	std::list<Album>::iterator it;

	if (rootNodes[TREE_ROOT_CATALOG] != NULL) {
		CTreeViewCtrlEx::DeleteItem(rootNodes[TREE_ROOT_CATALOG]);  // remove old catalog node and all childs
	}
	rootNodes[TREE_ROOT_CATALOG] = CTreeViewCtrlEx::InsertItem(L"Remote Catalog", NULL, TVI_ROOT); // create new catalog node
	std::wstring alpha = _T("#ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	for (unsigned int i = 0; i < alpha.length(); i++) {
		wchar_t name[2] = { alpha[i] ,'\0' };
		catalogRootNodes[i] = CTreeViewCtrlEx::InsertItem(name, rootNodes[TREE_ROOT_CATALOG], TVI_LAST);
	}

	for (it = albumList->begin(); it != albumList->end(); it++) {

		pfc::stringcvt::string_wide_from_utf8 artistName(it->get_artist());
		pfc::stringcvt::string_wide_from_utf8 albumName(it->get_title());

		HTREEITEM rootNode = getRootTreeNodeForArtist(artistName[0]);

		bool needNewNode = true;
		HTREEITEM artistNode = nullptr;
		if (ItemHasChildren(rootNode)) { // there are already some artists
			for (HTREEITEM j = GetNextItem(rootNode, TVGN_CHILD); j; j = GetNextItem(j, TVGN_NEXT)) { // iterate all artists for grouping same names
				CString str;

				if (GetItemText(j, str)) {
					if (str == artistName) {
						needNewNode = false;
						artistNode = j;
						break;
					}
				}
			}
			if (artistNode != nullptr) {
				HTREEITEM albumNode = InsertItem(albumName, artistNode, TVI_LAST);

				std::list<Track*>* trackList = it->getTracks();
				addTracksToTreeNode(trackList, albumNode, true, false);

				Album* store = &*it;
				SetItemData(albumNode, (DWORD_PTR)store); // attach album details
			}
		}

		if (needNewNode) {
			HTREEITEM artistRoot = InsertItem(artistName, rootNode, TVI_LAST); // add artist as new entry
			HTREEITEM albumNode = InsertItem(albumName, artistRoot, TVI_LAST); // add the current album as entry to artist
			
			Artist *artist = new Artist(it->get_artistid());
			SetItemData(artistRoot, (DWORD_PTR)artist); // attach artist details

			std::list<Track*>* trackList = it->getTracks();
			addTracksToTreeNode(trackList, albumNode, true, false);

			Album* store = &*it;
			SetItemData(albumNode, (DWORD_PTR)store); // attach album details
		}
	}
	CTreeViewCtrlEx::Expand(rootNodes[TREE_ROOT_CATALOG]);
}

void CSubsonicUi::populateTreeWithPlaylists(std::list<Playlist>* playlists) {
	std::list<Playlist>::iterator it;

	if (rootNodes[TREE_ROOT_PLAYLISTS] != NULL) {
		CTreeViewCtrlEx::DeleteItem(rootNodes[TREE_ROOT_PLAYLISTS]);  // remove old playlists node and all childs
	}
	rootNodes[TREE_ROOT_PLAYLISTS] = CTreeViewCtrlEx::InsertItem(L"Remote Playlists", NULL, TVI_ROOT); // create playlist node

	for (it = playlists->begin(); it != playlists->end(); it++) {

		char playtime[20];

		int hours = it->get_duration() / 3600;
		int remaining = it->get_duration() % 3600;
		int minutes = remaining / 60;		
		int seconds = remaining % 60;

		snprintf(playtime, 20, "%02d:%02d:%02d", hours, minutes, seconds);

		pfc::string8 tmp = it->get_name();
		tmp << " <Len: " << playtime << ">";

		pfc::stringcvt::string_wide_from_utf8 playlistName(tmp);

		HTREEITEM rootNode = rootNodes[TREE_ROOT_PLAYLISTS];
		
		HTREEITEM playlistNode = InsertItem(playlistName, rootNode, TVI_LAST); // add the current playlist as entry to artist

		Playlist* store = &*it;
		SetItemData(playlistNode, (DWORD_PTR)store); // attach album details

		std::list<Track*>* tracks = it->getTracks();
		addTracksToTreeNode(tracks, playlistNode, false, true);
	}

}

LRESULT CSubsonicUi::OnContextCatalogUpdateDone(UINT, WPARAM, LPARAM, BOOL&) {
	populateTreeWithAlbums(SqliteCacheDb::getInstance()->getAllAlbums());

	return 0;
}

LRESULT CSubsonicUi::OnContextPlaylistUpdateDone(UINT, WPARAM, LPARAM, BOOL &) {	
	populateTreeWithPlaylists(SqliteCacheDb::getInstance()->getAllPlaylists());

	return 0;
}

LRESULT CSubsonicUi::OnContextArtistUpdateDone(UINT, WPARAM, LPARAM, BOOL &) {
	populateTreeWithAlbums(SqliteCacheDb::getInstance()->getAllAlbums());

	return 0;
}

LRESULT CSubsonicUi::OnDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	HTREEITEM selected = GetSelectedItem();
	if (selected) {
//		CIDropSource* pdsrc = new CIDropSource;
//		CIDataObject* pdobj = new CIDataObject(pdsrc);
		// Init the supported format
		FORMATETC fmtetc = { 0 };
		fmtetc.cfFormat = CF_TEXT;
		fmtetc.dwAspect = DVASPECT_CONTENT;
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;
		// Init the medium used
		STGMEDIUM medium = { 0 };
		medium.tymed = TYMED_HGLOBAL;
		// medium.hGlobal = init to something
		// Add it to DataObject
//		pdobj->SetData(&fmtetc, &medium, TRUE); // Release the medium for me
												// add more formats and medium if needed
												// Initiate the Drag & Drop
//		::DoDragDrop(pdobj, pdsrc, DROPEFFECT_COPY, &dwEffect);
		
	}
	return 0;
}

LRESULT CSubsonicUi::OnBeginDrag(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	if (uMsg == TVN_BEGINDRAG) {
		HTREEITEM selected = GetSelectedItem();
		if (selected) {
			m_dragging = true;

			

			POINT pt = { 0 };
			m_dragImage = &CTreeViewCtrlEx::CreateDragImage(selected);
			m_dragImage->BeginDrag(0, 0, 0);

			::ClientToScreen(m_hWnd, &pt);
			m_dragImage->DragEnter(NULL, pt);

			SetCapture();
			return 0;
		}

	}


	return 0;
}

void CSubsonicUi::OnMouseMove(UINT nFlags, CPoint point) {
	if (m_dragging) {
		POINT pt = point;
		ClientToScreen(&pt);
		CImageList::DragMove(pt);
	}
}

void CSubsonicUi::OnLButtonUp(UINT nFlags, CPoint point) {
	if (m_dragging) {
		m_dragging = false;
		CImageList::DragLeave(*this);
		CImageList::EndDrag();
		ReleaseCapture();
		// move the dragged item
	}
}

void CSubsonicUi::notify(const GUID & p_what, t_size p_param1, const void * p_param2, t_size p_param2size) {
	if (p_what == ui_element_notify_colors_changed || p_what == ui_element_notify_font_changed) {
		CTreeViewCtrlEx::SetTextColor(m_callback->query_std_color(ui_color_text));
		CTreeViewCtrlEx::SetBkColor(m_callback->query_std_color(ui_color_background));

		Invalidate();
	}
}