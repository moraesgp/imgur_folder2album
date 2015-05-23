#ifndef __dbg_h__
#define __dbg_h__

typedef enum {
	UNDETERMINED,
	SUCCESS,
	CONFIG_FILE_DOESNOT_EXIST,
	CANT_OPEN_FILE,
	CANT_READ_FILE,
	CANT_OPEN_DIR,
	CURL_ERROR,
	OUTDATE_ACCESS_TOKEN,
	OUT_OF_MEMORY,
	IMAGE_NOT_LOADED,
	API_ERROR,
	APP_ERROR,
	PATH_NOT_FOUND,
	JSON_PARSE_ERROR,
	IMAGE_RESIZE_ERROR
} error_code;

error_code last_error;

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) fprintf(stderr, "[ERROR·] [%s:%d: errno: %s] " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stderr, "[WARN··] [%s:%d: errno: %s] " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stderr, "[INFO··] [%s:%d] " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define throw_error(A, E, M, ...) if(A) { log_err(M, ##__VA_ARGS__); last_error=E; goto error; }

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

#endif
