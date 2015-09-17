#pragma once

namespace Preferences {
	extern const GUID guid_connect_url_data;
	extern cfg_string connect_url_data;

	extern const GUID guid_username_data;
	extern cfg_string username_data;

	extern const GUID guid_password_data;
	extern cfg_string password_data;

	extern const GUID guid_check_selfsignedcerts_data;
	extern cfg_bool check_selfsignedcerts_data;

	// Proxy Stuff
	extern const GUID guid_proxy_url_data;
	extern cfg_string proxy_url_data;

	extern const GUID guid_connect_timeout_data;
	extern cfg_int connect_timeout_data;

	extern const GUID guid_proxy_settings_no_data;
	extern cfg_bool proxy_settings_no_data;

	extern const GUID guid_proxy_settings_system_data;
	extern cfg_bool proxy_settings_system_data;

	extern const GUID guid_proxy_settings_custom_data;
	extern cfg_bool proxy_settings_custom_data;

}
