#include "foo_subsonic.h"
#include "xmlhelper.h"


namespace XmlHelper {

	int XmlIntOrDefault(TiXmlElement* element, const char* attribute, unsigned int default) {
		auto temp = default;
		if (element->QueryUnsignedAttribute(attribute, &temp) == TIXML_SUCCESS) {
			return temp;
		}
		else {
			return default;
		}
	}
	
	pfc::string8 XmlStrOrDefault(TiXmlElement* element, const char* attribute, const char* default) {
		auto tmp = element->Attribute(attribute);
		return tmp == nullptr ? "" : tmp;
	}

	
	char to_hex(char c) {
		return c < 0xa ? '0' + c : 'a' - 0xa + c;
	}

	
	pfc::string8 buildRequestUrl(const char* restMethod, pfc::string8 urlparams) {
		pfc::string8 url;
		url << Preferences::connect_url_data;
		url << "/rest/";
		url << restMethod << ".view";
		url << "?v=1.8.0";
		url << "&c=" << COMPONENT_SHORT_NAME;
		url << "&u=" << Preferences::username_data;
		url << "&p=" << url_encode(Preferences::password_data);

		if (sizeof(urlparams) > 0) {
			url << "&" << urlparams;
		}

		return url;
	}

	
	pfc::string8 url_encode(const char *in) {
		pfc::string8 out;
		out.prealloc(strlen(in) * 3 + 1);

		for (register const char *tmp = in; *tmp != '\0'; tmp++) {
			auto c = static_cast<unsigned char>(*tmp);
			if (isalnum(c)) {
				out.add_char(c);
			}
			else if (isspace(c)) {
				out.add_char('+');
			}
			else {
				out.add_char('%');
				out.add_char(to_hex(c >> 4));
				out.add_char(to_hex(c % 16));
			}
		}
		return out;
	}
}