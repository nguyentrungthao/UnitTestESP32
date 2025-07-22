
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "ffconf.h"
#include "errno.h"
#ifdef __cplusplus
extern "C"
{
#endif
    

typedef enum
{
    LOG_FILE_OK,
    LOG_FILE_EXIST,
    LOG_FILE_NOT_EXIST,
    LOG_FILE_ERR,
    LOG_FILE_OPENING,
} log_file_err_t;
typedef struct
{
    const char* folder_path;
    char file_path[50];
    const char* mode;
    void* args;
    FILE* file_ptr;
} log_file_t;
log_file_err_t file_create(log_file_t* file_info);
log_file_err_t file_open(log_file_t* file_info);
log_file_err_t file_print(log_file_t* file_info, const char *format, ...);
log_file_err_t file_close(log_file_t* file_info);
log_file_err_t file_remove(const char *file_path);
log_file_err_t file_copy(log_file_t *dest_file_info, log_file_t *src_file_info, const char *src_file_name);

#ifdef __cplusplus
}
#endif
