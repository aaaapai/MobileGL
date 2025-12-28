#ifndef _MOBILEGLUES_CONFIG_H_
#define _MOBILEGLUES_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern const char* mg_directory_path;
extern const char* config_file_path;
extern const char* log_file_path;
extern const char* glsl_cache_file_path;

    extern int initialized;

const char* concatenate(const char* str1, const char* str2);

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
