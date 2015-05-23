#ifndef __CONFIG__h
#define __CONFIG__h 1

#define VERSION_NUMBER "0.99"
#define CONFIG_CLIENT_ID "a9dc0790f071c1f"
#define CONFIG_CLIENT_SECRET "928a1f1db17543eb1f2d4545aff8a2eba8e25a06"

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
