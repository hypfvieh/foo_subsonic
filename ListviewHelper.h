#pragma once
#include "foo_subsonic.h"

/*
	This code is copied from foo_musicbrainz
	(c) Dremora
	--> https://github.com/Dremora/foo_musicbrainz/
*/

namespace listviewHelper {
	bool set_item_text(HWND p_listview, unsigned p_index, unsigned p_column, const char * p_name)
	{
		LVITEM item = {};

		pfc::stringcvt::string_os_from_utf8 os_string_temp(p_name);

		item.mask = LVIF_TEXT;
		item.iItem = p_index;
		item.iSubItem = p_column;
		item.pszText = const_cast<TCHAR*>(os_string_temp.get_ptr());
		return uSendMessage(p_listview, LVM_SETITEM, 0, (LPARAM)&item) ? true : false;
	}

	unsigned insert_item(HWND p_listview, unsigned p_index, const char * p_name, LPARAM p_param)
	{
		if (p_index == ~0) p_index = ListView_GetItemCount(p_listview);
		LVITEM item = {};

		pfc::stringcvt::string_os_from_utf8 os_string_temp(p_name);

		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = p_index;
		item.lParam = p_param;
		item.pszText = const_cast<TCHAR*>(os_string_temp.get_ptr());

		LRESULT ret = uSendMessage(p_listview, LVM_INSERTITEM, 0, (LPARAM)&item);
		if (ret < 0) return ~0;
		else return (unsigned)ret;
	}

	unsigned insert_item2(HWND p_listview, unsigned p_index, const char * col0, const char * col1, LPARAM p_param) {
		unsigned i = insert_item(p_listview, p_index, col0, p_param);
		if (i != ~0) {
			set_item_text(p_listview, i, 1, col1);
		}
		return i;
	}

	unsigned insert_item3(HWND p_listview, unsigned p_index, const char * col0, const char * col1, const char * col2, LPARAM p_param) {
		unsigned i = insert_item(p_listview, p_index, col0, p_param);
		if (i != ~0) {
			set_item_text(p_listview, i, 1, col1);
			set_item_text(p_listview, i, 2, col2);
		}
		return i;
	}

	unsigned insert_item4(HWND p_listview, unsigned p_index, const char * col0, const char * col1, const char * col2, const char * col3, LPARAM p_param) {
		unsigned i = insert_item(p_listview, p_index, col0, p_param);
		if (i != ~0) {
			set_item_text(p_listview, i, 1, col1);
			set_item_text(p_listview, i, 2, col2);
			set_item_text(p_listview, i, 3, col3);
		}
		return i;
	}


	unsigned insert_column(HWND p_listview, unsigned p_index, const char * p_name, LONG p_width_dlu)
	{
		pfc::stringcvt::string_os_from_utf8 os_string_temp(p_name);

		RECT rect = { 0,0,p_width_dlu,0 };
		MapDialogRect(GetParent(p_listview), &rect);

		LVCOLUMN data = {};
		data.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
		data.fmt = LVCFMT_LEFT;
		data.cx = rect.right;
		data.pszText = const_cast<TCHAR*>(os_string_temp.get_ptr());

		LRESULT ret = uSendMessage(p_listview, LVM_INSERTCOLUMN, p_index, (LPARAM)&data);
		if (ret < 0) return ~0;
		else return (unsigned)ret;
	}

	void get_item_text(HWND p_listview, unsigned p_index, unsigned p_column, pfc::string_base & p_out) {
		enum { buffer_length = 1024 * 64 };
		pfc::array_t<TCHAR> buffer; buffer.set_size(buffer_length);
		ListView_GetItemText(p_listview, p_index, p_column, buffer.get_ptr(), buffer_length);
		p_out = pfc::stringcvt::string_utf8_from_os(buffer.get_ptr(), buffer_length);
	}	

	bool is_item_selected(HWND p_listview, unsigned p_index)
	{
		LVITEM item = {};
		item.mask = LVIF_STATE;
		item.iItem = p_index;
		item.stateMask = LVIS_SELECTED;
		if (!uSendMessage(p_listview, LVM_GETITEM, 0, (LPARAM)&item)) return false;
		return (item.state & LVIS_SELECTED) ? true : false;
	}

	void set_item_selection(HWND p_listview, unsigned p_index, bool p_state)
	{
		PFC_ASSERT(::IsWindow(p_listview));
		LVITEM item = {};
		item.stateMask = LVIS_SELECTED;
		item.state = p_state ? LVIS_SELECTED : 0;
		WIN32_OP_D(SendMessage(p_listview, LVM_SETITEMSTATE, (WPARAM)p_index, (LPARAM)&item));
	}

	bool ensure_visible(HWND p_listview, unsigned p_index)
	{
		return uSendMessage(p_listview, LVM_ENSUREVISIBLE, p_index, FALSE) ? true : false;
	}

	bool select_single_item(HWND p_listview, unsigned p_index)
	{
		LRESULT temp = SendMessage(p_listview, LVM_GETITEMCOUNT, 0, 0);
		if (temp < 0) return false;
		ListView_SetSelectionMark(p_listview, p_index);
		unsigned n; const unsigned m = pfc::downcast_guarded<unsigned>(temp);
		for (n = 0; n<m; n++) {
			enum { mask = LVIS_FOCUSED | LVIS_SELECTED };
			ListView_SetItemState(p_listview, n, n == p_index ? mask : 0, mask);
		}
		return ensure_visible(p_listview, p_index);
	}


}