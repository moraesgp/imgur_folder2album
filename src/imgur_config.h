#ifndef __CONFIG__h
#define __CONFIG__h 1

#define VERSION_NUMBER "0.99"
#define CONFIG_CLIENT_ID "aa814050621ea14"
#define CONFIG_CLIENT_SECRET "0fa4854a020bf6bc51419beed04281a7a3f8f192"

#define CONFIG_IMGUR_AUTHORIZE_URL "https://api.imgur.com/oauth2/authorize"
#define CONFIG_IMGUR_TOKEN_URL "https://api.imgur.com/oauth2/token"
#define CONFIG_IMGUR_ALBUNS "https://api.imgur.com/3/account/%s/albums/"
#define CONFIG_IMGUR_ALBUM "https://api.imgur.com/3/album"
#define CONFIG_IMGUR_FILEUPLOAD "https://api.imgur.com/3/image"

#define API_DATA_FILE "apidatafile"
#define ALBUM_DATA_FILE "albumidtitlefile"

#define WORD1SIZE 30
#define WORD2SIZE 60
#define DIRPATHSIZE 2048

#define IMAGE_SQUARE_SIZE 768

#if defined(_WIN32) || defined(_WIN64)
  #define snprintf _snprintf
  #define vsnprintf _vsnprintf
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
  #define PATH_SEPARATOR "\\"
#else
	#define PATH_SEPARATOR "/"
#endif

struct s_api_config {
	char pin[WORD1SIZE];
	char access_token[WORD2SIZE];
	time_t valid_until;
	char refresh_token[WORD2SIZE];
	char account_username[WORD1SIZE];
	char images_directory[DIRPATHSIZE];
};

typedef struct s_api_config api_config;

struct s_photo_linked_list {
	char photo_path[DIRPATHSIZE];
	struct s_photo_linked_list *next;
	char type;
};

typedef struct s_photo_linked_list photo_linked_list;

#endif
