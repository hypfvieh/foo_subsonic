#pragma once

#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"

#define ID_CONTEXT_UPDATECATALOG			WM_USER + 101
#define ID_CONTEXT_UPDATEPLAYLIST			WM_USER + 102
#define ID_CONTEXT_SEARCHDIALOG 			WM_USER + 103

#define ID_CONTEXT_UPDATECATALOG_DONE       WM_APP  + 201
#define ID_CONTEXT_UPDATEPLAYLIST_DONE      WM_APP  + 202

#define ID_SEARCH_DONE						WM_APP  + 301

#define IDC_TREEVIEWCTRL					2053

#define TREE_ROOT_CATALOG					1
#define TREE_ROOT_PLAYLISTS					2

namespace foo_subsonic {
	class CSubsonicUi : public CWindowImpl<CSubsonicUi, CTreeViewCtrlEx>, public ui_element_instance {
	public:
		DECLARE_WND_CLASS_EX(TEXT("{7C92FF46-60F6-4628-844F-0266BBBAE95A}"), CS_VREDRAW | CS_HREDRAW, (-1));

		void initialize_window(HWND parent) { WIN32_OP(Create(parent, 0, 0, 0, WS_EX_STATICEDGE) != NULL); }	
	
		BEGIN_MSG_MAP(CSubsonicUi)
			//MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown);
			MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick);
			MESSAGE_HANDLER(WM_RBUTTONDOWN, OnContextMenu);
			MESSAGE_HANDLER(WM_CREATE, OnCreate);
			MESSAGE_HANDLER(WM_CONTEXTMENU, OnRButtonDown);
			MESSAGE_HANDLER(ID_CONTEXT_UPDATECATALOG_DONE, OnContextCatalogUpdateDone);
			MESSAGE_HANDLER(ID_CONTEXT_UPDATEPLAYLIST_DONE, OnContextPlaylistUpdateDone);
			COMMAND_ID_HANDLER(ID_CONTEXT_UPDATECATALOG, OnContextCatalogUpdate);
			COMMAND_ID_HANDLER(ID_CONTEXT_UPDATEPLAYLIST, OnContextPlaylistUpdate);
			COMMAND_ID_HANDLER(ID_CONTEXT_SEARCHDIALOG, OnSearchDialogShow);
			COMMAND_ID_HANDLER(TVN_BEGINDRAG, foo);
			MSG_WM_MOUSEMOVE(OnMouseMove);
			MSG_WM_LBUTTONUP(OnLButtonUp);
			//REFLECTED_NOTIFY_CODE_HANDLER(TVN_BEGINDRAG, foo);
//			NOTIFY_HANDLER(IDC_TREEVIEWCTRL, TVN_BEGINDRAG, OnDrag);
		END_MSG_MAP()

		CSubsonicUi(ui_element_config::ptr, ui_element_instance_callback_ptr p_callback);

		LRESULT OnLButtonDown(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnLButtonDblClick(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnRButtonDown(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

		LRESULT OnContextCatalogUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnContextPlaylistUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSearchDialogShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		LRESULT OnContextCatalogUpdateDone(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnContextPlaylistUpdateDone(UINT, WPARAM, LPARAM, BOOL&);
	
		LRESULT OnBeginDrag(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

		LRESULT foo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
			return 0;
		}

		void notify(const GUID & p_what, t_size p_param1, const void * p_param2, t_size p_param2size);

		void OnMouseMove(UINT nFlags, CPoint point);
		void OnLButtonUp(UINT nFlags, CPoint point);

		void populateTreeWithAlbums(std::list<Album>* albumList);
		void populateTreeWithPlaylists(std::list<Playlist>* playlists);

		HWND get_wnd() { return *this; }
		void set_configuration(ui_element_config::ptr config) { m_config = config; }
		ui_element_config::ptr get_configuration() { return m_config; }
		static GUID g_get_guid() {
			static const GUID guid_myelem =	{ 0xbde11872, 0xa32f, 0x4bcd,{ 0x86, 0xfd, 0x74, 0x1f, 0x95, 0x1e, 0xd8, 0xf6 } };			
			return guid_myelem;
		}
		static GUID g_get_subclass() { return ui_element_subclass_utility; }
		static void g_get_name(pfc::string_base & out) { out = "Subsonic Client"; }
		static ui_element_config::ptr g_get_default_configuration() { return ui_element_config::g_create_empty(g_get_guid()); }
		static const char * g_get_description() { return "UI Control to list all content fetched from subsonic server"; }

	private:
		int drag_item;
		ui_element_config::ptr m_config;
		bool m_dragging;
		CImageList *m_dragImage;
		SubsonicLibraryScanner scanner;
		HTREEITEM rootNodes[2] = {NULL, NULL};
		HTREEITEM catalogRootNodes[28];
		HTREEITEM getRootTreeNodeForArtist(wchar_t bgnLetter);

		void addTracksToTreeNode(std::list<Track*>* trackList, HTREEITEM albumNode, bool withTrackNumber);

	protected:
		CSubsonicUi() {
			CTreeViewCtrlEx::SetDlgCtrlID(IDC_TREEVIEWCTRL);
		}
		// this must be declared as protected for ui_element_impl_withpopup<> to work.
		const ui_element_instance_callback_ptr m_callback;
	};

	

	class ui_element_myimpl : public ui_element_impl_withpopup<CSubsonicUi> {};

	static service_factory_single_t<ui_element_myimpl> g_ui_element_myimpl_factory;

} // namespace
