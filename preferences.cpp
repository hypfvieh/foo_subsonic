#include "foo_subsonic.h"
#include "preferences.h"
#include "sqliteCacheDb.h"

namespace Preferences {
	const GUID guid_connect_url_data = { 0x4dcf833, 0x78b7, 0x4549,{ 0xaf, 0x34, 0xfd, 0xd3, 0x9c, 0x13, 0x9, 0xde } };
	cfg_string connect_url_data(guid_connect_url_data, "");

	const GUID guid_username_data = { 0x27dab537, 0xfe3a, 0x4119,{ 0xa0, 0x64, 0x3c, 0x43, 0xb5, 0x91, 0xa5, 0xb8 } };
	cfg_string username_data(guid_username_data, "");

	const GUID guid_password_data = { 0x3c3a9af4, 0xd496, 0x49da,{ 0xb6, 0xc, 0xe7, 0xa, 0x97, 0xd5, 0xde, 0x16 } };
	cfg_string password_data(guid_password_data, "");

	const GUID guid_check_selfsignedcerts_data = { 0x141b2ec3, 0x6b59, 0x49b3,{ 0xb7, 0xd7, 0x52, 0xcf, 0xb3, 0x8d, 0xe2, 0xc } };
	cfg_bool check_selfsignedcerts_data(guid_check_selfsignedcerts_data, false);

	const GUID guid_check_pass_as_hex_data = { 0x6f05d961, 0x344e, 0x49f3,{ 0x82, 0xf, 0x43, 0x58, 0x69, 0x80, 0xa0, 0xdb } };
	cfg_bool check_pass_as_hex_data(guid_check_pass_as_hex_data, true);

	const GUID guid_connect_timeout_data = { 0x5d359a74, 0x8010, 0x4fe0,{ 0xaf, 0x90, 0x45, 0x4, 0x9b, 0x1a, 0xb9, 0x84 } };
	cfg_int connect_timeout_data(guid_connect_timeout_data, 10);

	// Proxy Stuff
	const GUID guid_proxy_url_data = { 0x91582c5c, 0xe5cf, 0x4b21,{ 0x95, 0x18, 0x59, 0x1c, 0xdd, 0x43, 0xe2, 0x45 } };
	cfg_string proxy_url_data(guid_proxy_url_data, "");

	const GUID guid_proxy_settings_no_data = { 0xc837b708, 0xf51f, 0x4e22,{ 0xae, 0xe2, 0x97, 0x7, 0xdb, 0x78, 0xca, 0xa5 } };
	cfg_bool proxy_settings_no_data(guid_proxy_settings_no_data, true);

	const GUID guid_proxy_settings_system_data = { 0x355211d1, 0x5de2, 0x46ad,{ 0xaf, 0xb1, 0x9d, 0xfe, 0x18, 0xf8, 0xd4, 0xcb } };
	cfg_bool proxy_settings_system_data(guid_proxy_settings_system_data, false);

	const GUID guid_proxy_settings_custom_data = { 0x2be7e55f, 0xcc61, 0x4368,{ 0xa5, 0x3f, 0x47, 0x68, 0x5b, 0x97, 0x83, 0xd2 } };
	cfg_bool proxy_settings_custom_data(guid_proxy_settings_custom_data, false);

	// CoverArt Stuff
	const GUID guid_coverart_size_data = { 0xf79bf5f6, 0x6630, 0x415b,{ 0x80, 0xcc, 0x38, 0x3c, 0xc8, 0x67, 0x46, 0x86 } };
	cfg_int coverart_size_data(guid_coverart_size_data, 500);

	const GUID guid_coverart_download = { 0x1a4e3073, 0x6af4, 0x4d9c,{ 0x89, 0xb5, 0x6c, 0xe0, 0xb0, 0x17, 0xdb, 0x3f } };
	cfg_bool coverart_download(guid_coverart_download, true);

	const GUID guid_coverart_resize = { 0x4ed09771, 0x2bc5, 0x4f63,{ 0xba, 0xa3, 0x46, 0x18, 0x60, 0xec, 0x28, 0xa } };
	cfg_bool coverart_resize(guid_coverart_resize, true);

}
class PreferencesPageInstance : public CDialogImpl<PreferencesPageInstance>, public preferences_page_instance {
private:
	CEdit connect_url;
	CEdit username;
	CEdit password;

	CEdit proxy_url;
	CEdit connect_timeout;
	CEdit coverart_size;

	CCheckBox use_selfsignedcerts;
	CCheckBox use_pass_as_hex;
	CCheckBox use_coverart_dl;
	CCheckBox use_coverart_resize;

	CButton proxy_settings_no;
	CButton proxy_settings_system;
	CButton proxy_settings_custom;

	preferences_page_callback::ptr on_change_callback;

public:
	PreferencesPageInstance(preferences_page_callback::ptr callback) : on_change_callback(callback)
	{}

	enum { IDD = IDD_PREFERENCES };

	BEGIN_MSG_MAP(CPreferencesDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_HANDLER_EX(IDC_CONNECT_URL_DATA, EN_UPDATE, OnChanged)
		COMMAND_HANDLER_EX(IDC_USERNAME_DATA, EN_UPDATE, OnChanged)
		COMMAND_HANDLER_EX(IDC_PASSWORD_DATA, EN_UPDATE, OnChanged)		
		COMMAND_HANDLER_EX(IDC_CHECK_SELFSIGNED, BN_CLICKED, OnChanged)
		COMMAND_HANDLER_EX(IDC_CHK_PASSWORD_AS_HASH, BN_CLICKED, OnChanged)
		COMMAND_HANDLER_EX(IDC_RADIO_PROXY_CUSTOM, BN_CLICKED, OnChanged)
		COMMAND_HANDLER_EX(IDC_RADIO_PROXY_SYSTEM, BN_CLICKED, OnChanged)
		COMMAND_HANDLER_EX(IDC_RADIO_PROXY_NO, BN_CLICKED, OnChanged)
		COMMAND_HANDLER_EX(IDC_PROXY_URL_DATA, EN_UPDATE, OnChanged)
		COMMAND_HANDLER_EX(IDC_CONNECT_TIMEOUT_DATA, EN_UPDATE, OnChanged)
		COMMAND_HANDLER_EX(IDC_CHK_RESIZECOVERART, EN_UPDATE, OnChanged)
		COMMAND_HANDLER_EX(IDC_CHK_DLCOVERART, EN_UPDATE, OnChanged)
		COMMAND_HANDLER_EX(IDC_TXT_COVERARTSIZE, EN_UPDATE, OnChanged)
		COMMAND_HANDLER(IDC_BTN_RESET_COVERART_CACHE, BN_CLICKED, OnBnClickedBtnResetCoverartCache)
		COMMAND_HANDLER(IDC_BTN_RESET_CACHE, BN_CLICKED, OnBnClickedBtnResetCache)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {

		connect_url = GetDlgItem(IDC_CONNECT_URL_DATA);
		username = GetDlgItem(IDC_USERNAME_DATA);
		password = GetDlgItem(IDC_PASSWORD_DATA);

		coverart_size = GetDlgItem(IDC_TXT_COVERARTSIZE);

		use_selfsignedcerts = GetDlgItem(IDC_CHECK_SELFSIGNED);
		use_coverart_dl = GetDlgItem(IDC_CHK_DLCOVERART);
		use_coverart_resize = GetDlgItem(IDC_CHK_RESIZECOVERART);

		use_pass_as_hex = GetDlgItem(IDC_CHK_PASSWORD_AS_HASH);

		proxy_url = GetDlgItem(IDC_PROXY_HOSTNAME_DATA);
		connect_timeout = GetDlgItem(IDC_CONNECT_TIMEOUT_DATA);

		proxy_settings_custom = GetDlgItem(IDC_RADIO_PROXY_CUSTOM);
		proxy_settings_system = GetDlgItem(IDC_RADIO_PROXY_SYSTEM);
		proxy_settings_no = GetDlgItem(IDC_RADIO_PROXY_NO);

		uSetWindowText(connect_url, Preferences::connect_url_data);
		uSetWindowText(username, Preferences::username_data);
		uSetWindowText(password, Preferences::password_data);

		uSetWindowText(proxy_url, Preferences::proxy_url_data);

		char tmp[20];
		snprintf(tmp, 20, "%i", Preferences::connect_timeout_data.get_value());
		uSetWindowText(connect_timeout, tmp);
		snprintf(tmp, 20, "%i", Preferences::coverart_size_data.get_value());
		uSetWindowText(coverart_size, tmp);

		//CheckRadioButton(IDC_RADIO_PROXY_NO, IDC_RADIO_PROXY_CUSTOM, IDC_RADIO)
		CheckDlgButton(IDC_RADIO_PROXY_NO, Preferences::proxy_settings_no_data);
		CheckDlgButton(IDC_RADIO_PROXY_SYSTEM, Preferences::proxy_settings_system_data);
		CheckDlgButton(IDC_RADIO_PROXY_CUSTOM, Preferences::proxy_settings_custom_data);

		CheckDlgButton(IDC_CHECK_SELFSIGNED, Preferences::check_selfsignedcerts_data);
		CheckDlgButton(IDC_CHK_PASSWORD_AS_HASH, Preferences::check_pass_as_hex_data);

		CheckDlgButton(IDC_CHK_DLCOVERART, Preferences::coverart_download);
		CheckDlgButton(IDC_CHK_RESIZECOVERART, Preferences::coverart_resize);

		return 0;
	}

	bool has_changed() {

		pfc::string8 temp;

		uGetWindowText(proxy_url, temp);
		if (Preferences::proxy_url_data != temp && checkProxyPrefix(temp)) return true;

		uGetWindowText(connect_url, temp);
		if (Preferences::connect_url_data != temp) return true;

		uGetWindowText(username, temp);
		if (Preferences::username_data != temp) return true;

		uGetWindowText(password, temp);
		if (Preferences::password_data != temp) return true;

		uGetWindowText(connect_timeout, temp);
		int tInt = atoi(temp.c_str());
		if (Preferences::connect_timeout_data != tInt) return true;

		bool data;
		
		data = IsDlgButtonChecked(IDC_CHECK_SELFSIGNED) == BST_CHECKED;
		if (Preferences::check_selfsignedcerts_data != data) return true;

		data = IsDlgButtonChecked(IDC_CHK_PASSWORD_AS_HASH) == BST_CHECKED;
		if (Preferences::check_pass_as_hex_data != data) return true;

		data = IsDlgButtonChecked(IDC_RADIO_PROXY_CUSTOM) == BST_CHECKED;
		if (Preferences::proxy_settings_custom_data != data) return true;

		data = IsDlgButtonChecked(IDC_RADIO_PROXY_SYSTEM) == BST_CHECKED;
		if (Preferences::proxy_settings_system_data != data) return true;

		data = IsDlgButtonChecked(IDC_RADIO_PROXY_NO) == BST_CHECKED;
		if (Preferences::proxy_settings_no_data != data) return true;

		data = IsDlgButtonChecked(IDC_CHK_DLCOVERART) == BST_CHECKED;
		if (Preferences::coverart_download != data) return true;

		data = IsDlgButtonChecked(IDC_CHK_RESIZECOVERART) == BST_CHECKED;
		if (Preferences::coverart_resize != data) return true;

		return false;
	}

	bool checkProxyPrefix(pfc::string8 proxyHost) {
		pfc::string workstr = proxyHost;

		workstr = workstr.toLower();
		return workstr.startsWith("http://") || workstr.startsWith("https://") || workstr.startsWith("socks://") || workstr.startsWith("socks5://");
	}

	t_uint32 get_state() {
		t_uint32 state = preferences_state::resettable;
		if (has_changed()) state |= preferences_state::changed;
		return state;
	}

	void apply() {
		uGetWindowText(connect_url, Preferences::connect_url_data);
		uGetWindowText(username, Preferences::username_data);
		uGetWindowText(password, Preferences::password_data);

		uGetWindowText(proxy_url, Preferences::proxy_url_data);

		pfc::string8 tmp;
		uGetWindowText(connect_timeout, tmp);
		Preferences::connect_timeout_data = atoi(tmp.c_str());

		uGetWindowText(coverart_size, tmp);
		Preferences::coverart_size_data = atoi(tmp.c_str());
		
		Preferences::check_pass_as_hex_data = IsDlgButtonChecked(IDC_CHK_PASSWORD_AS_HASH) == BST_CHECKED;
		Preferences::check_selfsignedcerts_data = IsDlgButtonChecked(IDC_CHECK_SELFSIGNED) == BST_CHECKED;

		Preferences::proxy_settings_no_data = IsDlgButtonChecked(IDC_RADIO_PROXY_NO) == BST_CHECKED;
		Preferences::proxy_settings_system_data = IsDlgButtonChecked(IDC_RADIO_PROXY_SYSTEM) == BST_CHECKED;
		Preferences::proxy_settings_custom_data = IsDlgButtonChecked(IDC_RADIO_PROXY_CUSTOM) == BST_CHECKED;

		Preferences::coverart_download = IsDlgButtonChecked(IDC_CHK_DLCOVERART) == BST_CHECKED;
		Preferences::coverart_resize = IsDlgButtonChecked(IDC_CHK_RESIZECOVERART) == BST_CHECKED;
	}

	void on_change() {
		on_change_callback->on_state_changed();
	}

	void reset() {
		uSetWindowText(connect_url, "");
		uSetWindowText(username, "");
		uSetWindowText(password, "");

		uSetWindowText(proxy_url, "");
		uSetWindowText(connect_timeout, "10");

		uSetWindowText(coverart_size, "500");

		CheckDlgButton(IDC_CHECK_SELFSIGNED, FALSE);
		CheckDlgButton(IDC_CHK_PASSWORD_AS_HASH, TRUE);

		CheckDlgButton(IDC_CHK_DLCOVERART, TRUE);
		CheckDlgButton(IDC_CHK_RESIZECOVERART, TRUE);

		CheckDlgButton(IDC_RADIO_PROXY_NO, TRUE);
		CheckDlgButton(IDC_RADIO_PROXY_CUSTOM, FALSE);
		CheckDlgButton(IDC_RADIO_PROXY_SYSTEM, FALSE);

		on_change();
	}

	void OnChanged(UINT, int, HWND) {
		on_change();
	}
	
	LRESULT OnBnClickedBtnResetCoverartCache(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SqliteCacheDb::getInstance()->clearCoverArtCache();
		return 0;
	}

	LRESULT OnBnClickedBtnResetCache(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{		
		SqliteCacheDb::getInstance()->clearCache();

		return 0;
	}
};

class PreferencesPage : public preferences_page_impl<PreferencesPageInstance> {
public:
	const char * get_name() {
		return COMPONENT_TITLE;
	}

	GUID get_guid() {
		static const GUID guid = { 0x6829a87b, 0xd15f, 0x4ee7,{ 0xb7, 0xa, 0xe9, 0x6a, 0xfe, 0xd4, 0x2d, 0xd7 } };
		return guid;
	}

	GUID get_parent_guid() {
		return preferences_page::guid_media_library;
	}
};

preferences_page_factory_t<PreferencesPage> _;