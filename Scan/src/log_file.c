#include "log_file.h"

static const char* TAG = "LOG_FILE";

log_file_err_t file_create(log_file_t* file_info)
{
    struct stat st;
    // Kiểm tra sự tồn tại của thư mục
    if (stat(file_info->folder_path, &st) == 0)
    {
        struct stat s = { 0 };
        if (stat(file_info->file_path, &s) != 0) {
            file_info->file_ptr = fopen(file_info->file_path, "w");
            if (file_info->file_ptr == NULL) {
                ESP_LOGE(TAG, "Failed to open file for writing");
                return LOG_FILE_ERR;
            }
            // fprintf(file_info->file_ptr, "Thu muc da ton tai, tao file moi\n");
            fclose(file_info->file_ptr);
            file_info->file_ptr = NULL;
            ESP_LOGI(TAG, "Thu muc ton tai, tao moi file");
            return LOG_FILE_OK;
        }
        ESP_LOGE(TAG, "Thu muc ton tai, file ton tai");
        return LOG_FILE_EXIST;
    }
    else
    {
        // Thư mục chưa tồn tại, tạo mới thư mục
        if (mkdir(file_info->folder_path, 0777) == 0)
        {
            file_info->file_ptr = fopen(file_info->file_path, "w");
            if (file_info->file_ptr == NULL) {
                ESP_LOGE(TAG, "Failed to open file for writing");
                return LOG_FILE_ERR;
            }
            // fprintf(file_info->file_ptr, "Tao moi thu muc, tao moi file\n");
            fclose(file_info->file_ptr);
            file_info->file_ptr = NULL;
            ESP_LOGI(TAG, "Tao moi thu muc, tao moi file");
            return LOG_FILE_OK;
        }
        else
        {
            ESP_LOGE(TAG, "LOG_FILE_ERR");
            return LOG_FILE_ERR;
        }
    }
}

log_file_err_t file_open(log_file_t* file_info)
{
    struct stat s = { 0 };
    if (stat(file_info->file_path, &s) == 0)
    {
        ESP_LOGI(TAG, "Opening %s file ", file_info->file_path);
        if (file_info->mode == NULL) file_info->mode = "a+";
        file_info->file_ptr = fopen(file_info->file_path, file_info->mode);
        if (file_info->file_ptr == NULL)
        {
            ESP_LOGE(TAG, "Failed to open %s file", file_info->file_path);
            return LOG_FILE_ERR;
        }
        ESP_LOGI(TAG, "Success!");
        return LOG_FILE_OK;
    }
    return LOG_FILE_NOT_EXIST;
}

log_file_err_t file_print(log_file_t* file_info, const char* format, ...) {
    if (file_info->file_ptr != NULL)
    {
        va_list args;
        va_start(args, format);
        if(vfprintf(file_info->file_ptr, format, args) < 0)
		{
			va_end(args);
			return LOG_FILE_ERR;
		}
        va_end(args);
        return LOG_FILE_OK;
    }
    else return LOG_FILE_ERR;
}
log_file_err_t file_close(log_file_t* file_info)
{
    if (file_info->file_ptr != NULL)
    {
        if (fclose(file_info->file_ptr) == 0) return LOG_FILE_OK;
    }
    return LOG_FILE_ERR;
}

log_file_err_t file_remove(const char* file_path)
{
    if (remove(file_path) == 0) return LOG_FILE_OK;
    return LOG_FILE_ERR;
}

log_file_err_t file_copy(log_file_t *dest_file_info, log_file_t *src_file_info, const char *src_file_name)
{
	uint16_t count_byte = 0;
	sprintf(src_file_info->file_path, "%s/%s", src_file_info->folder_path, src_file_name);
	FILE *f_src = fopen(src_file_info->file_path, "r");

	ESP_LOGI(TAG, "Opening %s file ", src_file_info->file_path);

	if (f_src == NULL)
	{
		ESP_LOGE(TAG, "Failed to open %s file", src_file_info->file_path);
		return LOG_FILE_ERR;
	}
	sprintf(dest_file_info->file_path, "%s/%s", dest_file_info->folder_path, src_file_name);
	log_file_err_t file_err = file_create(dest_file_info);
	if (file_err == LOG_FILE_ERR)
	{
		ESP_LOGE(TAG, "Failed to open %s file", dest_file_info->file_path);
		fclose(f_src);
		return file_err;
	}
	else if (file_err == LOG_FILE_EXIST)
	{
		fclose(f_src);
		return file_err;
	}
	file_err = file_open(dest_file_info);
	if(file_err == LOG_FILE_ERR)
	{
		ESP_LOGE(TAG, "Failed to open %s file", dest_file_info->file_path);
		fclose(f_src);
		return LOG_FILE_ERR;
	}
	char c;
	ESP_LOGI(TAG, "Copying %s file to %s file", src_file_info->file_path, dest_file_info->file_path);
	c=fgetc(f_src);
	printf("%c\n",c);
	while(c >= 0x01 && c <= 0x7F)
	{
		fputc(c, dest_file_info->file_ptr);
		count_byte++;
		printf("%c", c);
		if(count_byte >= 127)
		{
			fclose( dest_file_info->file_ptr);
			count_byte = 0;
			vTaskDelay(pdMS_TO_TICKS(10));
			dest_file_info->file_ptr = fopen(dest_file_info->file_path, "a");
			if(dest_file_info->file_ptr == NULL)
			{
				ESP_LOGE(TAG, "Error during copying");
				fclose(f_src);
				return LOG_FILE_ERR;
			}
		}
		c=fgetc(f_src);
	}

	fclose(f_src);
	fclose(dest_file_info->file_ptr);

	ESP_LOGI(TAG, "Finished copying !");
	return LOG_FILE_OK;
}

