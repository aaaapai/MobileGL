#ifndef _MOBILEGLUES_CONFIG_H_
#define _MOBILEGLUES_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern char* mg_directory_path;
extern char* config_file_path;
extern char* log_file_path;
extern char* glsl_cache_file_path;

extern int initialized;

char* concatenate(const char* str1, const char* str2);

int check_path(void);

int config_refresh(void);
int config_get_int(const char* name);
const char* config_get_string(const char* name);
void config_cleanup(void);

#ifdef __cplusplus
}
#endif

extern bool is_custom_mg_dir;

#endif // _MOBILEGLUES_CONFIG_H_
