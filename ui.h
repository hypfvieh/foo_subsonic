#pragma once

#include "foo_subsonic.h"
#include "subsoniclibraryscanner.h"

#define ID_CONTEXT_UPDATECATALOG	WM_USER + 101
#define ID_CONTEXT_UPDATEDONE       WM_APP + 102

namespace foo_subsonic {
	class CSubsonicUi : public ui_element_instance, public CWindowImpl<CSubsonicUi, CTreeViewCtrlEx> {
	public:
		DECLARE_WND_CLASS_EX(TEXT("{7C92FF46-60F6-4628-844F-0266BBBAE95A}"), CS_VREDRAW | CS_HREDRAW, (-1));
	
		// AtlInitCommonControls(ICC_TREEVIEW_CLASSES);

		void initialize_window(HWND parent) { WIN32_OP(Create(parent, 0, 0, 0, WS_EX_STATICEDGE) != NULL); }	
	
		BEGIN_MSG_MAP(CSubsonicUi)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown);
			MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick);
			MESSAGE_HANDLER(WM_RBUTTONDOWN, OnContextMenu);
			MESSAGE_HANDLER(WM_CREATE, OnCreate);
			MESSAGE_HANDLER(WM_CONTEXTMENU, OnRButtonDown);
			MESSAGE_HANDLER(ID_CONTEXT_UPDATEDONE, OnContextUpdateDone);
			COMMAND_ID_HANDLER(ID_CONTEXT_UPDATECATALOG, OnContextCatalogUpdate);
			//COMMAND_ID_HANDLER(ID_CONTEXT_UPDATEDONE, OnContextUpdateDone);
	//	    MSG_WM_ERASEBKGND(OnEraseBkgnd)
	//		MSG_WM_PAINT(OnPaint)				
		END_MSG_MAP()

		CSubsonicUi(ui_element_config::ptr, ui_element_instance_callback_ptr p_callback);

		LRESULT OnLButtonDown(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnLButtonDblClick(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnRButtonDown(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

		LRESULT OnContextCatalogUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnContextUpdateDone(UINT, WPARAM, LPARAM, BOOL&);
	
		void OnPaint(CDCHandle);
		BOOL OnEraseBkgnd(CDCHandle dc);

		HWND get_wnd() { return *this; }
		void set_configuration(ui_element_config::ptr config) { m_config = config; }
		ui_element_config::ptr get_configuration() { return m_config; }
		static GUID g_get_guid() {
			static const GUID guid_myelem = { 0x7c92ff46, 0x60f6, 0x4628,{ 0x84, 0x4f, 0x2, 0x66, 0xbb, 0xba, 0xe9, 0x5a } };
			return guid_myelem;
		}
		static GUID g_get_subclass() { return ui_element_subclass_utility; }
		static void g_get_name(pfc::string_base & out) { out = "Subsonic Streaming Client"; }
		static ui_element_config::ptr g_get_default_configuration() { return ui_element_config::g_create_empty(g_get_guid()); }
		static const char * g_get_description() { return "Control to list all content fetched from subsonic server"; }

	private:
		ui_element_config::ptr m_config;
		SubsonicLibraryScanner scanner;
		HTREEITEM rootNodes[28];
		HTREEITEM getRootTreeNodeForArtist(wchar_t bgnLetter);

		void CSubsonicUi::addTracksToAlbum(std::list<Album>::iterator &it, HTREEITEM albumNode);

	protected:
		// this must be declared as protected for ui_element_impl_withpopup<> to work.
		const ui_element_instance_callback_ptr m_callback;
	};

	class ui_element_myimpl : public ui_element_impl_withpopup<CSubsonicUi> {};

	static service_factory_single_t<ui_element_myimpl> g_ui_element_myimpl_factory;

}
