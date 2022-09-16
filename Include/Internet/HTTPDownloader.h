/**
* HTTPDownloader.hpp
*
* A simple C++ wrapper for the libcurl easy API.
*
* Written by Uli Köhler (techoverflow.net)
* Published under CC0 1.0 Universal (public domain)
*/
#ifndef HTTPDOWNLOADER_H
#define HTTPDOWNLOADER_H

#include <string>
#include <sstream>
#include <iostream>
#include <locale>
#include <codecvt>
#include "stxutif.h"
#include "curl\curl.h"
#include "curl\easy.h"
#include "curl\curlbuild.h"
#include "OString.h"

/**
* A non-threadsafe simple libcURL-easy based HTTP downloader
*/
class HTTPDownloader {
public:
	HTTPDownloader();
	~HTTPDownloader();

	void setOptions();
	void getCharset(const std::string& url);
	void getVersionInfo();
	void download(const std::string& url, const OString &filename);
	void printCookies();
	void clearCookies();

private:
	void* curl;
	OString charset;
};

#endif  /* HTTPDOWNLOADER_H */
