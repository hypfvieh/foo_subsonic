#include "foo_subsonic.h"
#include "ui.h"
#include "subsoniclibraryscanner.h"
#include "albumQueryThread.h"
#include "xmlcachedb.h"
#include "playlistupdater.h"


using namespace foo_subsonic;

CSubsonicUi::CSubsonicUi(ui_element_config::ptr config, ui_element_instance_callback_ptr p_callback) : m_callback(p_callback), m_config(config) {
	AtlInitCommonControls(ICC_TREEVIEW_CLASSES);
}

LRESULT CSubsonicUi::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

	CTreeViewCtrlEx::SetDlgCtrlID(IDC_TREEVIEWCTRL);

	console::printf("Found text color: %x", m_callback->query_std_color(ui_color_text));
	console::printf("Found bg color: %x", m_callback->query_std_color(ui_color_background));

	CTreeViewCtrlEx::SetTextColor(m_callback->query_std_color(ui_color_text));
	CTreeViewCtrlEx::SetBkColor(m_callback->query_std_color(ui_color_background));

	// show +/- and the connecting lines
	CTreeViewCtrlEx::ModifyStyle(1, TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);
	//CTreeViewCtrlEx::SetExtendedStyle()

	createRootTree(true, true);

	return lRet;
}

LRESULT CSubsonicUi::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
	console::print("left mouse clicked");
	return lRet;
}

LRESULT CSubsonicUi::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
	console::print("right mouse clicked");
	
	return lRet;
}

LRESULT CSubsonicUi::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&) {
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
	HMENU hMenu = ::CreatePopupMenu();

	if (NULL != hMenu) {
		::AppendMenu(hMenu, MF_STRING, ID_CONTEXT_UPDATECATALOG, _T("Retrieve/Update Subsonic Catalog"));
		::AppendMenu(hMenu, MF_STRING, ID_CONTEXT_UPDATEPLAYLIST, _T("Retrieve/Update Subsonic Playlists"));
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
		CoreEntity* coreType = reinterpret_cast<CoreEntity*>(ptr);
		if (coreType->get_type() == ENTRY_TYPE_TRACK) {
			

			if (ptr != NULL) {
				Track* track = reinterpret_cast<Track*>(ptr);

				console::formatter() << "Got Track=" << track->get_title() << ", Artist=" << track->get_artist();

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
		else if (coreType->get_type() == ENTRY_TYPE_ALBUM) {

			if (ptr != NULL) {
				Album* album = reinterpret_cast<Album*>(ptr);

				console::formatter() << "Got Album=" << album->get_title() << ", Artist=" << album->get_artist();

				std::list<Track>* trackList = album->getTracks();

				std::list<Track>::iterator trackIterator;
				pfc::list_t<const char*> data;
				for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
					data.add_item(trackIterator->get_streamUrl());
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
	
	return 0;
}

LRESULT CSubsonicUi::OnContextCatalogUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	console::print("Catalog Menu Item clicked");

	threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::AlbumQueryThread>(&scanner, m_hWnd),
		threaded_process::flag_show_progress | threaded_process::flag_show_abort, m_hWnd, "Querying album catalog from Subsonic Server");
	
	return 0;
}

LRESULT CSubsonicUi::OnContextPlaylistUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	console::print("Playlist Menu Item clicked");

	threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::AlbumQueryThread>(&scanner, m_hWnd),
		threaded_process::flag_show_progress | threaded_process::flag_show_abort, m_hWnd, "Querying playlist data from Subsonic Server");

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

void CSubsonicUi::addTracksToAlbum(std::list<Track>* trackList, HTREEITEM albumNode, bool withTrackNumber) {
	//std::list<Track>* trackList = it->getTracks();
	std::list<Track>::iterator trackIterator;
	for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
		pfc::stringcvt::string_wide_from_utf8 trackName(trackIterator->get_title());
		wchar_t track[250];
		if (withTrackNumber) {
			swprintf(track, sizeof(track), L"%i) %s", trackIterator->get_tracknumber(), trackName.get_ptr());
		}
		else {
			swprintf(track, sizeof(track), L"%s", trackName.get_ptr());
		}
		Track* store = &*trackIterator;
		HTREEITEM titleNode = InsertItem(track, albumNode, TVI_LAST);

		SetItemData(titleNode, (DWORD_PTR)store); // attach track meta data to node, so we can use this as shortcut for adding tracks to playlist
	}

}

void CSubsonicUi::createRootTree(bool loadCachedAlbums, bool loadCachedPlaylists) {
	rootNodes[TREE_ROOT_CATALOG]   = CTreeViewCtrlEx::InsertItem(L"Remote Catalog", NULL, TVI_ROOT);
	rootNodes[TREE_ROOT_PLAYLISTS] = CTreeViewCtrlEx::InsertItem(L"Remote Playlists", NULL, TVI_ROOT);

	std::wstring alpha = _T("#ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	for (unsigned int i = 0; i < alpha.length(); i++) {
		wchar_t name[2] = { alpha[i] ,'\0' };
		catalogRootNodes[i] = CTreeViewCtrlEx::InsertItem(name, rootNodes[TREE_ROOT_CATALOG], TVI_LAST);
	}

	CTreeViewCtrlEx::Expand(rootNodes[TREE_ROOT_CATALOG]);

	if (loadCachedAlbums) {
		populateTreeWithAlbums(XmlCacheDb::getInstance()->getAllAlbums());
	}
	if (loadCachedPlaylists) {

	}

}

void CSubsonicUi::populateTreeWithAlbums(std::list<Album>* albumList) {
	std::list<Album>::iterator it;

	CTreeViewCtrlEx::DeleteAllItems(); // remove current content
	createRootTree(false, true); // recreate the root tree, remove cached albums but retain the playlists

	for (it = albumList->begin(); it != albumList->end(); it++) {
		//pfc::stringcvt::string_os_from_utf8 albumname(albumList[i]->get_title());

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

				std::list<Track>* trackList = it->getTracks();
				addTracksToAlbum(trackList, albumNode, true);

				Album* store = &*it;
				SetItemData(albumNode, (DWORD_PTR)store); // attach album details
			}
		}

		if (needNewNode) {
			HTREEITEM artistRoot = InsertItem(artistName, rootNode, TVI_LAST); // add artist as new entry
			HTREEITEM albumNode = InsertItem(albumName, artistRoot, TVI_LAST); // add the current album as entry to artist
			
			std::list<Track>* trackList = it->getTracks();
			addTracksToAlbum(trackList, albumNode, true);

			Album* store = &*it;
			SetItemData(albumNode, (DWORD_PTR)store); // attach album details
		}
	}

	// Redraw
	CTreeViewCtrlEx::Invalidate();
}

void CSubsonicUi::populateTreeWithPlaylists(std::list<Playlist>* playlists) {
	std::list<Playlist>::iterator it;

	CTreeViewCtrlEx::DeleteAllItems(); // remove current content
	createRootTree(true, false); // recreate the root tree, remove cached albums but retain the playlists

	for (it = playlists->begin(); it != playlists->end(); it++) {
		//pfc::stringcvt::string_os_from_utf8 albumname(albumList[i]->get_title());

		pfc::stringcvt::string_wide_from_utf8 playlistName(it->get_name());		

		HTREEITEM rootNode = rootNodes[TREE_ROOT_PLAYLISTS];
		
		
		HTREEITEM albumNode = InsertItem(playlistName, rootNode, TVI_LAST); // add the current album as entry to artist
		std::list<Track>* tracks = it->getTracks();
		addTracksToAlbum(tracks, albumNode, false);		
	}

	// Redraw
	CTreeViewCtrlEx::Invalidate();
}

LRESULT CSubsonicUi::OnContextCatalogUpdateDone(UINT, WPARAM, LPARAM, BOOL&) {
	std::list<Album>* albumList = XmlCacheDb::getInstance()->getAllAlbums();

	populateTreeWithAlbums(albumList);

	return 0;
}

LRESULT CSubsonicUi::OnContextPlaylistUpdateDone(UINT, WPARAM, LPARAM, BOOL &)
{	
	std::list<Playlist>* playlists = XmlCacheDb::getInstance()->getAllPlaylists();

	populateTreeWithPlaylists(playlists);

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