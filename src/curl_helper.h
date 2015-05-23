#ifndef __CURL_HELPER_h
#define __CURL_HELPER_h 1

#include <curl/curl.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

int my_trace(CURL *handle, curl_infotype type,
              char *data, size_t size,
              void *userp);
#endif
