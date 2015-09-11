#include "foo_subsonic.h"
#include "track.h"
#include "album.h"
#include "playlist.h"
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

	bool XmlBoolOrDefault(TiXmlElement* element, const char* attribute, bool default) {
		auto temp = default;
		if (element->QueryBoolAttribute(attribute, &temp) == TIXML_SUCCESS) {
			return temp;
		}
		else {
			return default;
		}
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

	void parseAlbumInfo(TiXmlElement* e, Album* a) {
		if (e == nullptr) {
			return;
		}
		if (a == nullptr) {
			return;
		}
		a->set_artist(XmlStrOrDefault(e, "artist", ""));
		a->set_title(XmlStrOrDefault(e, "title", "")); // use title instead of album, otherwise multi disc albums were mixed

		a->set_genre(XmlStrOrDefault(e, "genre", ""));
		a->set_year(XmlStrOrDefault(e, "year", ""));

		a->set_coverArt(XmlStrOrDefault(e, "coverArt", ""));

		a->set_artistid(XmlStrOrDefault(e, "artistId", "0"));
		a->set_id(XmlStrOrDefault(e, "id", "0"));
		a->set_parentid(XmlStrOrDefault(e, "parent", "0"));

		a->set_duration(XmlIntOrDefault(e, "duration", 0));

		a->set_songCount(XmlIntOrDefault(e, "songCount", 0));
	}

	void parseTrackInfo(TiXmlElement* e, Track* t) {
		if (e == nullptr) {
			return;
		}
		if (t == nullptr) {
			return;
		}
		t->set_duration(XmlIntOrDefault(e, "duration", 0));
		t->set_tracknumber(XmlIntOrDefault(e, "track", 0));
		t->set_parentId(XmlStrOrDefault(e, "parent", "0"));
		t->set_bitrate(XmlIntOrDefault(e, "bitRate", 0));
		t->set_size(XmlIntOrDefault(e, "size", 0));
		t->set_year(XmlStrOrDefault(e, "year", "0"));

		t->set_id(XmlStrOrDefault(e, "id", ""));
		t->set_artist(XmlStrOrDefault(e, "artist", ""));
		t->set_album(XmlStrOrDefault(e, "album", ""));

		t->set_genre(XmlStrOrDefault(e, "genre", ""));
		t->set_contentType(XmlStrOrDefault(e, "contentType", ""));
		t->set_coverArt(XmlStrOrDefault(e, "coverArt", "0"));
		t->set_title(XmlStrOrDefault(e, "title", ""));
		t->set_suffix(XmlStrOrDefault(e, "suffix", ""));

		// build and add url for streaming
		pfc::string8 idparam = "id=";
		idparam << t->get_id();

		pfc::string8 streamUrl = buildRequestUrl("stream", idparam);
		t->set_streamUrl(streamUrl);
	}

	void parsePlaylistInfo(TiXmlElement* e, Playlist* p) {
		if (e == nullptr) {
			return;
		}
		if (p == nullptr) {
			return;
		}
		p->set_comment(XmlStrOrDefault(e, "comment", ""));
		p->set_coverArt(XmlStrOrDefault(e, "covertArt", ""));
		p->set_duration(XmlIntOrDefault(e, "duration", 0));
		p->set_id(XmlStrOrDefault(e, "id", "0"));
		p->set_isPublic(XmlBoolOrDefault(e, "public", false));
		p->set_name(XmlStrOrDefault(e, "name", "No Name given"));
		p->set_owner(XmlStrOrDefault(e, "owner", ""));
		p->set_songcount(XmlIntOrDefault(e, "songCount", 0));
	}
}