#include "foo_subsonic.h"
#include "xmlcachedb.h"
#include "xmlhelper.h"

using namespace XmlHelper;

XmlCacheDb* XmlCacheDb::instance = NULL;

XmlCacheDb::XmlCacheDb() {
	internalDoc = TiXmlDocument("foo_subsonic_cache.xml");
	if (!internalDoc.LoadFile()) {
		console::print("Offline cache no found, creating new");
	}
}

void XmlCacheDb::addAlbumsToSave(std::list<Album>* albumList) {
	std::list<Album>::iterator it;
		
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "1");
	internalDoc.LinkEndChild(decl);

	TiXmlElement* albumRootElement = new TiXmlElement("Albums");
	internalDoc.LinkEndChild(albumRootElement);

	for (it = albumList->begin(); it != albumList->end(); it++) {
		TiXmlElement* album = new TiXmlElement("Album");

		pfc::stringcvt::string_wide_from_utf8 albumName(it->get_title());

		album->SetAttribute("id", it->get_id().c_str());
		album->SetAttribute("artist", it->get_artist().c_str());
		album->SetAttribute("title", it->get_title().c_str());
		album->SetAttribute("artistId", it->get_artistid());
		album->SetAttribute("coverArt", it->get_coverArt().c_str());
		album->SetAttribute("genre", it->get_genre().c_str());
		album->SetAttribute("year", it->get_year().c_str());
		album->SetAttribute("duration", it->get_duration());
		album->SetAttribute("songCount", it->get_songCount());
		album->SetAttribute("parentId", it->get_parentid());
		album->SetAttribute("releaseDate", it->get_releasedate());

		std::list<Track>* trackList = it->getTracks();
		std::list<Track>::iterator trackIterator;
		for (trackIterator = trackList->begin(); trackIterator != trackList->end(); trackIterator++) {
			TiXmlElement* track = new TiXmlElement("Track");

			track->SetAttribute("id", trackIterator->get_id().c_str());
			track->SetAttribute("title", trackIterator->get_title().c_str());
			track->SetAttribute("duration", trackIterator->get_duration());
			track->SetAttribute("bitrate", trackIterator->get_bitrate());
			track->SetAttribute("contentType", trackIterator->get_contentType().c_str());
			track->SetAttribute("coverArt", trackIterator->get_coverArt().c_str());
			track->SetAttribute("genre", trackIterator->get_genre().c_str());
			track->SetAttribute("parentId", trackIterator->get_parentId());
			track->SetAttribute("suffix", trackIterator->get_suffix().c_str());
			track->SetAttribute("tracknumber", trackIterator->get_tracknumber());
			track->SetAttribute("year", trackIterator->get_year().c_str());
			track->SetAttribute("size", trackIterator->get_size());

			// do not save stream url, as it contains username and password!

			album->LinkEndChild(track);
		}
		albumRootElement->LinkEndChild(album);
	}

	internalDoc.SaveFile();
}


std::list<Album>* XmlCacheDb::getAllAlbums() {
	
	TiXmlElement* rootNode = internalDoc.FirstChildElement("Albums");
	if (rootNode) {
		for (TiXmlElement* ae = rootNode->FirstChildElement("Album"); ae != NULL; ae = ae->NextSiblingElement("album")) {
			Album a;

			a.set_artist(XmlStrOrDefault(ae, "artist", ""));
			a.set_title(XmlStrOrDefault(ae, "title", ""));

			a.set_genre(XmlStrOrDefault(ae, "genre", ""));
			a.set_year(XmlStrOrDefault(ae, "year", ""));

			a.set_coverArt(XmlStrOrDefault(ae, "coverArt", ""));

			a.set_artistid(XmlStrOrDefault(ae, "artistId", "0"));
			a.set_id(XmlStrOrDefault(ae, "id", "0"));
			a.set_parentid(XmlStrOrDefault(ae, "parentId", "0"));

			a.set_duration(XmlIntOrDefault(ae, "duration", 0));

			a.set_songCount(XmlIntOrDefault(ae, "songCount", 0));
			a.set_releasedate(XmlStrOrDefault(ae, "releaseDate", 0));

			if (!rootNode->FirstChildElement("Album")->NoChildren()) { // album has tracks

				for (TiXmlElement* e = ae->FirstChildElement("Track"); e != NULL; e = e->NextSiblingElement("Track")) {
					Track t = Track();

					t.set_id(XmlStrOrDefault(e, "id", ""));
					t.set_title(XmlStrOrDefault(e, "title", ""));
					t.set_duration(XmlIntOrDefault(e, "duration", 0));

					t.set_bitrate(XmlIntOrDefault(e, "bitrate", 0));
					t.set_contentType(XmlStrOrDefault(e, "contentType", ""));
					t.set_coverArt(XmlStrOrDefault(e, "coverArt", "0"));

					t.set_genre(XmlStrOrDefault(e, "genre", ""));
					t.set_parentId(XmlStrOrDefault(e, "parentId", "0"));
					t.set_suffix(XmlStrOrDefault(e, "suffix", ""));

					t.set_tracknumber(XmlIntOrDefault(e, "track", 0));
					t.set_size(XmlIntOrDefault(e, "size", 0));

					t.set_artist(XmlStrOrDefault(ae, "artist", ""));
					t.set_album(XmlStrOrDefault(ae, "title", ""));

					// build and add url for streaming
					pfc::string8 idparam = "id=";
					idparam << t.get_id();

					pfc::string8 streamUrl = buildRequestUrl("stream", idparam);
					t.set_streamUrl(streamUrl);

					a.addTrack(t);

				}
			}
			albums.push_back(a);
		}
	}
	return &albums;
}
