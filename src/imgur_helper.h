#ifndef __IMGUR_HELPER__h
#define __IMGUR_HELPER__h 1

#include <FreeImage.h>

int get_access_token(api_config *config_data);

int create_new_album(api_config *config_data, char *new_album_name);

int upload_photo_to_album(api_config *config_data, char *photo_path, char *album_id);

int upload_photo_stream_to_album(api_config *config_data, BYTE *bytes, DWORD size, char *album_id);

int update_albuns_list(api_config *config_data);

int get_album_by_id(api_config *config_data, char *id, char **albumname);

int get_album_by_name(api_config *config_data, char *albumname, char **albumid);

int get_imgur_pin(api_config *config_data);

#endif

