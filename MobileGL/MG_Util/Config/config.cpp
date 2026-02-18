#include "config.h"

#include "cJSON.h"
#include <cerrno>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>

#include "Includes.h"
#include <MG_Impl/Init.h>
#include <MG_State/GLState/Core.h>

#define DEBUG 0

const char* DEFAULT_MG_DIRECTORY_PATH = "/sdcard/MGL";

bool is_custom_mg_dir = false;
const char* mg_directory_path = nullptr;
const char* config_file_path = nullptr;
const char* log_file_path = nullptr;
const char* glsl_cache_file_path = nullptr;

static cJSON* config_json = nullptr;

int initialized = 0;

const char* concatenate(const char* str1, const char* str2) {
    std::string str = std::string(str1) + str2;
    char* result = new char[str.size() + 1];
    strcpy(result, str.c_str());
    return result;
}

int check_path(void) {
    if (!mg_directory_path) {
        char* var = std::getenv("MGL_DIR_PATH");
        is_custom_mg_dir = var ? true : false;
        mg_directory_path = var ? strdup(var) : DEFAULT_MG_DIRECTORY_PATH;
        unsetenv("MGL_DIR_PATH");
    }
    config_file_path = concatenate(mg_directory_path, "/config.json");
    log_file_path = concatenate(mg_directory_path, "/latest.log");
    glsl_cache_file_path = concatenate(mg_directory_path, "/glsl_cache.tmp");

    if (mkdir(mg_directory_path, 0755) != 0 && errno != EEXIST) {
        MGLOG_E("Error creating MG directory.\n");
        return 0;
    }
    return 1;
}

int config_refresh(void) {
    MGLOG_D("MGL_DIRECTORY_PATH=%s", mg_directory_path);
    MGLOG_D("CONFIG_FILE_PATH=%s", config_file_path);
    MGLOG_D("LOG_FILE_PATH=%s", log_file_path);

    FILE* file = fopen(config_file_path, "r");
    if (file == nullptr) {
        MGLOG_E("Unable to open config file %s", config_file_path);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* file_content = (char*)malloc(file_size + 1);
    if (file_content == nullptr) {
        MGLOG_E("Unable to allocate memory for file content");
        fclose(file);
        return 0;
    }

    fread(file_content, 1, file_size, file);
    fclose(file);
    file_content[file_size] = '\0';

    config_json = cJSON_Parse(file_content);
    free(file_content);

    if (config_json == nullptr) {
        MGLOG_E("Error parsing config JSON: %s\n", cJSON_GetErrorPtr());
        return 0;
    }

    initialized = 1;
    return 1;
}

int config_get_int(const char* name) {
    if (config_json == nullptr) {
        return -1;
    }

    cJSON* item = cJSON_GetObjectItem(config_json, name);
    if (item == nullptr || !cJSON_IsNumber(item)) {
        MGLOG_D("Config item '%s' not found or not an integer.\n", name);
        return -1;
    }

    return item->valueint;
}

const char* config_get_string(const char* name) {
    if (config_json == nullptr) {
        return nullptr;
    }

    cJSON* item = cJSON_GetObjectItem(config_json, name);
    if (item == nullptr || !cJSON_IsString(item)) {
        MGLOG_D("Config item '%s' not found or not a string.\n", name);
        return "";
    }

    return item->valuestring;
}

void config_cleanup(void) {
    if (config_json != nullptr) {
        cJSON_Delete(config_json);
        config_json = nullptr;
    }
}
