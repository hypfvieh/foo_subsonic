#include "foo_subsonic.h"
#include "ui.h"
#include "subsoniclibraryscanner.h"
#include "requestThread.h"

using namespace foo_subsonic;

CSubsonicUi::CSubsonicUi(ui_element_config::ptr config, ui_element_instance_callback_ptr p_callback) : m_callback(p_callback), m_config(config) {
	AtlInitCommonControls(ICC_TREEVIEW_CLASSES);
}

LRESULT CSubsonicUi::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

	console::printf("Found text color: %x", m_callback->query_std_color(ui_color_text));
	console::printf("Found bg color: %x", m_callback->query_std_color(ui_color_background));

	COLORREF color = m_callback->query_std_color(ui_color_text);
	CTreeViewCtrlEx::SetTextColor(color);
	CTreeViewCtrlEx::SetBkColor(m_callback->query_std_color(ui_color_background));

	// show +/- and the connecting lines
	CTreeViewCtrlEx::ModifyStyle(0, TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);

	std::wstring alpha = _T("#ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	for (unsigned int i = 0; i < alpha.length(); i++) {
		wchar_t name[2] = { alpha[i] ,'\0' };
		rootNodes[i] = CTreeViewCtrlEx::InsertItem(name, NULL, TVI_ROOT);
	}

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
		::AppendMenu(hMenu, MF_STRING, ID_CONTEXT_UPDATECATALOG, _T("Catalog Update"));
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
	if ((selected != NULL) && !ItemHasChildren(selected)) { // title selected

 		DWORD_PTR track_ptr = GetItemData(selected);

		if (track_ptr != NULL) {
			Track* track = reinterpret_cast<Track*>(track_ptr);
			pfc::string8 tmp = "Got Track=";
			tmp << track->get_title() << ", Artist=" << track->get_artist();
			console::print(tmp);

			//TODO: Add title to playlist
			static_api_ptr_t<playlist_manager> pm;
			t_size playlist = pm->get_active_playlist();

			// TODO: Does not work for HTTPS!
			const char* url = track->get_streamUrl().c_str();

			pm->playlist_undo_backup(playlist);
			pm->playlist_add_locations(playlist, pfc::list_single_ref_t<const char*>(url), TRUE, m_hWnd);

		}

	}
	return 0;
}

BOOL CSubsonicUi::OnEraseBkgnd(CDCHandle dc) {
	CRect rc; 
	WIN32_OP_D(GetClientRect(&rc));
	WTL::CBrush brush;
	WIN32_OP_D(brush.CreateSolidBrush(m_callback->query_std_color(ui_color_background)) != NULL);
	WIN32_OP_D(dc.FillRect(&rc, brush));
	return TRUE;
}

void CSubsonicUi::OnPaint(CDCHandle) {
	
	WTL::CPaintDC dc(*this);
	dc.SetTextColor(m_callback->query_std_color(ui_color_text));
	dc.SetBkMode(TRANSPARENT);
	SelectObjectScope fontScope(dc, (HGDIOBJ)m_callback->query_font_ex(ui_font_default));


//	const UINT format = DT_NOPREFIX | DT_CENTER | DT_VCENTER | DT_SINGLELINE;
//	CRect rc;
//	WIN32_OP_D(GetClientRect(&rc));

	
	
//	WIN32_OP_D(dc.DrawText(_T("TODO: REMOVE ME"), -1, &rc, format) > 0);
	

}

LRESULT CSubsonicUi::OnContextCatalogUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	console::print("Catalog Menu Item clicked");

	threaded_process::g_run_modeless(new service_impl_t<foo_subsonic::RequestThread>(&scanner, m_hWnd),
		threaded_process::flag_show_progress | threaded_process::flag_show_abort, m_hWnd, "Querying data from Subsonic Server");
	
	return 0;
}

HTREEITEM CSubsonicUi::getRootTreeNodeForArtist(wchar_t bgnLetter) {
	std::wstring alpha = _T("#ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	for (unsigned int i = 0; i < alpha.length(); i++) {
		wchar_t c = alpha[i];
		if (c == bgnLetter) {
			return rootNodes[i];
		}
	}
	return rootNodes[0]; // should match for everything which is not beginning with a letter
}

void CSubsonicUi::addTracksToAlbum(std::list<Album>::iterator &it, HTREEITEM albumNode) {
	std::list<Track>* trackList = it->getTracks();
	std::list<Track>::iterator trackIterator;
	for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
		pfc::stringcvt::string_wide_from_utf8 trackName(trackIterator->get_title());
		wchar_t track[250];
		
		swprintf(track, sizeof(track), L"%i) %s", trackIterator->get_tracknumber(), trackName.get_ptr());
		Track* store = &*trackIterator;
		HTREEITEM titleNode = InsertItem(track, albumNode, TVI_LAST);
		SetItemData(titleNode, (DWORD_PTR)store); // attach track meta data to node, so we can use this as shortcut for adding tracks to playlist
	}

}

LRESULT CSubsonicUi::OnContextUpdateDone(UINT, WPARAM, LPARAM, BOOL&) {
	std::list<Album>* albumList = scanner.getFetchedAlbumList();

	std::list<Album>::iterator it;

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
				addTracksToAlbum(it, albumNode);
			}
		}

		if (needNewNode) {
			HTREEITEM artistRoot = InsertItem(artistName, rootNode, TVI_LAST); // add artist as new entry
			HTREEITEM albumNode = InsertItem(albumName, artistRoot, TVI_LAST); // add the current album as entry to artist
			addTracksToAlbum(it, albumNode);
		}
	}

	// Redraw
	CTreeViewCtrlEx::Invalidate();

	return 0;
}