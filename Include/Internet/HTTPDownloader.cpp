// HTTPDownloader.cpp

#include "HTTPDownloader.h"
#include "unQuoteHTML.h"


HTTPDownloader::HTTPDownloader()
{
	curl = curl_easy_init();
}

HTTPDownloader::~HTTPDownloader() 
{
	curl_easy_cleanup(curl);
}

void HTTPDownloader::setOptions()
{
	// Basic initialization occurs in constructor

	CURLcode res;

	// Initial setup and retrieving version info
	res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	res = curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookies.txt");

	// Default charset - NOT CURRENTLY USED
	charset = _T("UTF-8");

	return;
}

void HTTPDownloader::getCharset(const std::string& url)
{
	// This coding will not run as is
	// Write initial information to temporary file

	CURLcode res;
	char *contentType;
	OString contentTypeOutput;
	FILE *tempFile;

	_wfopen_s(&tempFile, L"InfoQuery.txt", L"wb,ccs=UTF-8");
	res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, tempFile);
	res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	res = curl_easy_perform(curl);
	if (res == CURLE_OK)
	{
		res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &contentType);
		if ((res == CURLE_OK) && (sizeof(contentType) > 0))
		{
			contentTypeOutput = OString(contentType);
			int position = contentTypeOutput.findPosition(_T("charset="));
			if (position != -1)
				charset = contentTypeOutput.right(contentTypeOutput.getLength() - (position + 8) + 1);
		}
	}
	else
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
	}
	fclose(tempFile);

	return;
}

void HTTPDownloader::getVersionInfo()
{
	curl_version_info_data *data;
	data = curl_version_info(CURLVERSION_NOW);

	return;
}

void HTTPDownloader::download(const std::string& url, const OString &filename)
{
	CURLcode res;

	// Setup file and encoding based on charset
	FILE* fp;
	_wfopen_s(&fp, filename.getString(), L"wb,ccs=UTF-8");
	res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	res = curl_easy_perform(curl);
//	printCookies();
	
	if (res != CURLE_OK)
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
	}

	// Close file - SESSION IS STILL OPEN
	fclose(fp);

	return;
}

void HTTPDownloader::printCookies()
{
	CURLcode res;
	struct curl_slist *cookies;
	struct curl_slist *nc;
	int i;

	printf("Cookies, curl knows:\n");
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n",
			curl_easy_strerror(res));
		exit(1);
	}
	nc = cookies, i = 1;
	while (nc) {
		printf("[%d]: %s\n", i, nc->data);
		nc = nc->next;
		i++;
	}
	if (i == 1) {
		printf("(none)\n");
	}
	curl_slist_free_all(cookies);
}

void HTTPDownloader::clearCookies()
{
	curl_easy_setopt(curl, CURLOPT_COOKIELIST, "ALL");
}


