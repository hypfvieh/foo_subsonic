#pragma once

#include "foo_subsonic.h"

namespace XmlHelper {

#ifndef XML_INT_OR_DEFAULT_
	/*
	Retrieve a int value of a XML Element, or return a default value if element could not be found/read.
	*/
	int XmlIntOrDefault(TiXmlElement* element, const char* attribute, unsigned int default);
#define XML_INT_OR_DEFAULT_
#endif

#ifndef XML_STR_OR_DEFAULT_
	/*
	Retrieve a String value of a XML Element, or return a default value if element could not be found/read.
	*/
	pfc::string8 XmlStrOrDefault(TiXmlElement* element, const char* attribute, const char* default);
#define XML_STR_OR_DEFAULT_
#endif

#ifndef TO_HEX_	
	/*
	Turn char to Hex-representation.
	*/
	char to_hex(char c);
#define TO_HEX_
#endif


#ifndef URL_ENCODE_
	/*
	Build the request URL required for subsonic.
	This will build the URL using the configured server and add the required parameters like client (c), user (u) and password (p).
	*/
	pfc::string8 url_encode(const char *in);
#define URL_ENCODE_
#endif

#ifndef BUILD_REQUEST_URL_
	/*
	Encode a URL (which means mask all none ASCII characters).
	*/
	pfc::string8 buildRequestUrl(const char* restMethod, pfc::string8 urlparams);
#define BUILD_REQUEST_URL_
#endif

}