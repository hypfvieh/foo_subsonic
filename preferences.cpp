#include "foo_subsonic.h"
#include "preferences.h"

namespace Preferences {
	const GUID guid_connect_url_data = { 0x4dcf833, 0x78b7, 0x4549,{ 0xaf, 0x34, 0xfd, 0xd3, 0x9c, 0x13, 0x9, 0xde } };
	cfg_string connect_url_data(guid_connect_url_data, "");

	const GUID guid_username_data = { 0x27dab537, 0xfe3a, 0x4119,{ 0xa0, 0x64, 0x3c, 0x43, 0xb5, 0x91, 0xa5, 0xb8 } };
	cfg_string username_data(guid_username_data, "");

	const GUID guid_password_data = { 0x3c3a9af4, 0xd496, 0x49da,{ 0xb6, 0xc, 0xe7, 0xa, 0x97, 0xd5, 0xde, 0x16 } };
	cfg_string password_data(guid_password_data, "");

	const GUID guid_check_selfsignedcerts_data = { 0x141b2ec3, 0x6b59, 0x49b3,{ 0xb7, 0xd7, 0x52, 0xcf, 0xb3, 0x8d, 0xe2, 0xc } };
	cfg_bool check_selfsignedcerts_data(guid_check_selfsignedcerts_data, FALSE);

	const GUID guid_connect_timeout_data = { 0x5d359a74, 0x8010, 0x4fe0,{ 0xaf, 0x90, 0x45, 0x4, 0x9b, 0x1a, 0xb9, 0x84 } };
	cfg_int connect_timeout_data(guid_connect_timeout_data, 10);

	// Proxy Stuff
	// {91582C5C-E5CF-4B21-9518-591CDD43E245}
	const GUID guid_proxy_url_data = { 0x91582c5c, 0xe5cf, 0x4b21,{ 0x95, 0x18, 0x59, 0x1c, 0xdd, 0x43, 0xe2, 0x45 } };
	cfg_string proxy_url_data(guid_proxy_url_data, "");

	// {C837B708-F51F-4E22-AEE2-9707DB78CAA5}
	const GUID guid_proxy_settings_no_data = { 0xc837b708, 0xf51f, 0x4e22,{ 0xae, 0xe2, 0x97, 0x7, 0xdb, 0x78, 0xca, 0xa5 } };
	cfg_bool proxy_settings_no_data(guid_proxy_settings_no_data, "");

	// {355211D1-5DE2-46AD-AFB1-9DFE18F8D4CB}
	const GUID guid_proxy_settings_system_data = { 0x355211d1, 0x5de2, 0x46ad,{ 0xaf, 0xb1, 0x9d, 0xfe, 0x18, 0xf8, 0xd4, 0xcb } };
	cfg_bool proxy_settings_system_data(guid_proxy_settings_system_data, "");

	// {2BE7E55F-CC61-4368-A53F-47685B9783D2}
	const GUID guid_proxy_settings_custom_data = { 0x2be7e55f, 0xcc61, 0x4368,{ 0xa5, 0x3f, 0x47, 0x68, 0x5b, 0x97, 0x83, 0xd2 } };
	cfg_bool proxy_settings_custom_data(guid_proxy_settings_custom_data, "");

}

class PreferencesPageInstance : public CDialogImpl<PreferencesPageInstance>, public preferences_page_instance {
private:
	CEdit connect_url;
	CEdit username;
	CEdit password;
	CCheckBox use_selfsignedcerts;

	CEdit proxy_url;
	CEdit connect_timeout;

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
		COMMAND_HANDLER_EX(IDC_RADIO_PROXY_CUSTOM, BN_CLICKED, OnChanged)
		COMMAND_HANDLER_EX(IDC_RADIO_PROXY_SYSTEM, BN_CLICKED, OnChanged)
		COMMAND_HANDLER_EX(IDC_RADIO_PROXY_NO, BN_CLICKED, OnChanged)
		COMMAND_HANDLER_EX(IDC_PROXY_URL_DATA, EN_UPDATE, OnChanged)
		COMMAND_HANDLER_EX(IDC_CONNECT_TIMEOUT_DATA, EN_UPDATE, OnChanged)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {

		connect_url = GetDlgItem(IDC_CONNECT_URL_DATA);
		username = GetDlgItem(IDC_USERNAME_DATA);
		password = GetDlgItem(IDC_PASSWORD_DATA);

		use_selfsignedcerts = GetDlgItem(IDC_CHECK_SELFSIGNED);

		proxy_url = GetDlgItem(IDC_PROXY_HOSTNAME_DATA);
		connect_timeout = GetDlgItem(IDC_CONNECT_TIMEOUT_DATA);

		proxy_settings_custom = GetDlgItem(IDC_RADIO_PROXY_CUSTOM);
		proxy_settings_system = GetDlgItem(IDC_RADIO_PROXY_SYSTEM);
		proxy_settings_no = GetDlgItem(IDC_RADIO_PROXY_NO);

		uSetWindowText(connect_url, Preferences::connect_url_data);
		uSetWindowText(username, Preferences::username_data);
		uSetWindowText(password, Preferences::password_data);

		uSetWindowText(proxy_url, Preferences::proxy_url_data);

		char foo[20];
		snprintf(foo, sizeof(foo), "%i", Preferences::connect_timeout_data.get_value());		
		uSetWindowText(connect_timeout, foo);

		//CheckRadioButton(IDC_RADIO_PROXY_NO, IDC_RADIO_PROXY_CUSTOM, IDC_RADIO)
		CheckDlgButton(IDC_RADIO_PROXY_NO, Preferences::proxy_settings_no_data);
		CheckDlgButton(IDC_RADIO_PROXY_SYSTEM, Preferences::proxy_settings_system_data);
		CheckDlgButton(IDC_RADIO_PROXY_CUSTOM, Preferences::proxy_settings_custom_data);

		CheckDlgButton(IDC_CHECK_SELFSIGNED, Preferences::check_selfsignedcerts_data);

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
		
		//CheckDlgButton(IDC_CHECK_CUSTOMPORT, 0);

		data = IsDlgButtonChecked(IDC_CHECK_SELFSIGNED) == BST_CHECKED;
		if (Preferences::check_selfsignedcerts_data != data) return true;

		data = IsDlgButtonChecked(IDC_RADIO_PROXY_CUSTOM) == BST_CHECKED;
		if (Preferences::proxy_settings_custom_data != data) return true;

		data = IsDlgButtonChecked(IDC_RADIO_PROXY_SYSTEM) == BST_CHECKED;
		if (Preferences::proxy_settings_system_data != data) return true;

		data = IsDlgButtonChecked(IDC_RADIO_PROXY_NO) == BST_CHECKED;
		if (Preferences::proxy_settings_no_data != data) return true;

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

		pfc::string8 foo;
		uGetWindowText(connect_timeout, foo);
		Preferences::connect_timeout_data = atoi(foo.c_str());
		
		//Preferences::check_customport_data = GetDlgItemInt(IDC_CHECK_CUSTOMPORT, NULL, FALSE);
		Preferences::check_selfsignedcerts_data = IsDlgButtonChecked(IDC_CHECK_SELFSIGNED) == BST_CHECKED;

		Preferences::proxy_settings_no_data = IsDlgButtonChecked(IDC_RADIO_PROXY_NO) == BST_CHECKED;
		Preferences::proxy_settings_system_data = IsDlgButtonChecked(IDC_RADIO_PROXY_SYSTEM) == BST_CHECKED;
		Preferences::proxy_settings_custom_data = IsDlgButtonChecked(IDC_RADIO_PROXY_CUSTOM) == BST_CHECKED;
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

		CheckDlgButton(IDC_CHECK_CUSTOMPORT, FALSE);
		CheckDlgButton(IDC_CHECK_SELFSIGNED, FALSE);

		CheckRadioButton(IDC_RADIO_PROXY_NO, IDC_RADIO_PROXY_CUSTOM, IDC_RADIO_PROXY_NO);

		on_change();
	}

	void OnChanged(UINT, int, HWND) {
		on_change();
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
