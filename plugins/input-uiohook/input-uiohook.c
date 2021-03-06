#include <obs-module.h>
#include "platform.h"

#include "libuiohook-async-logger.h"

#include <stdio.h>

#define UIO_LOG(level, format, ...) \
    blog(level, "[UIOhook]: " format, ##__VA_ARGS__)
#define UIO_LOG_S(source, level, format, ...) \
    blog(level, "[UIOhook '%s']: " format, \
    obs_source_get_name(source), ##__VA_ARGS__)
#define UIO_BLOG(level, format, ...) \
    UIO_LOG(level, format, ##__VA_ARGS__)
//UIO_LOG_S(s->source, level, format, ##__VA_ARGS__)

struct input_uiohook {
	obs_source_t *source;
	char         *folder;
	char         *file;
	bool         recording;
	bool         persistent;
};

/// GenerateTimeDateFilename from obs/obs-app.cpp
char* GenerateEventsFilename(const char* path)
{
	time_t    now = time(0);
	struct tm *cur_time;
    int bufSize = 1024;
    char *file = (char*)malloc(bufSize);

#ifdef _WIN32
	char slash = '\\';
#else
	char slash = '/';
#endif

	cur_time = localtime(&now);
    snprintf(file,
             bufSize,
             "%s%c%d-%02d-%02d-%02d-%02d-%02d%c%d-%02d-%02d-%02d-%02d-%02d.txt",
	         path,
	         slash,
	         cur_time->tm_year+1900,
	         cur_time->tm_mon+1,
	         cur_time->tm_mday,
	         cur_time->tm_hour,
	         cur_time->tm_min,
             cur_time->tm_sec,
             slash,
             cur_time->tm_year+1900,
             cur_time->tm_mon+1,
             cur_time->tm_mday,
             cur_time->tm_hour,
             cur_time->tm_min,
             cur_time->tm_sec);
    printf("file '%s'\n",file);
	return file;
}

static const char *input_uiohook_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("InputUIOhook");
}

static void input_uiohook_unload(struct input_uiohook *context)
{
	UNUSED_PARAMETER(context);
}

static void input_uiohook_update(void *data, obs_data_t *settings)
{
	UNUSED_PARAMETER(data);
	const char *folder = obs_data_get_string(settings, "folder");
	printf("input_uiohook_update folder %s\n",folder);
}

static void input_uiohook_defaults(obs_data_t *settings)
{
    obs_data_set_default_bool(settings, "recording", false);
	const char* default_path = GetDefaultEventsSavePath();
	obs_data_set_default_string(settings, "folder", default_path);
	printf("path %s\n",default_path);
}

static void input_uiohook_show(void *data)
{
	UNUSED_PARAMETER(data);
}

static void input_uiohook_hide(void *data)
{
	UNUSED_PARAMETER(data);
}

static void *input_uiohook_create(obs_data_t *settings, obs_source_t *source)
{
	struct input_uiohook *context = bzalloc(sizeof(struct input_uiohook));
	context->source = source;

	int status = test_hooking();
	if(status == UIOHOOK_SUCCESS) {
		UIO_BLOG(LOG_INFO,"system hookable");
	} else {
		UIO_BLOG(LOG_ERROR,"system not hookable");
	}
	stop_logging();

	input_uiohook_update(context, settings);
	return context;
}

static void input_uiohook_destroy(void *data)
{
	struct input_uiohook *context = data;
	input_uiohook_unload(context);
    if(context)
        bfree(context);
}

static obs_properties_t *input_uiohook_properties(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *props = obs_properties_create();

	const char* default_path = GetDefaultEventsSavePath();

	printf("path %s\n",default_path);

	obs_properties_add_path(props,
	                        "folder", obs_module_text("SaveFolder"),
	                        OBS_PATH_DIRECTORY, NULL, default_path);

	return props;
}

static const char * input_uiohook_start(void *data, obs_data_t *settings)
{
	UNUSED_PARAMETER(data);
    //UNUSED_PARAMETER(settings);
    const char *folder = obs_data_get_string(settings, "folder");
    char* filepath = GenerateEventsFilename(folder);
    //printf("filepath %s\n",filepath);
    char* status = start_logging(filepath);
    free(filepath);
    //if(status != 0){
    //    obs_data_set_default_bool(settings, "recording", true);
    //}
    return status;
}

static void input_uiohook_stop(void *data, obs_data_t *settings)
{
	UNUSED_PARAMETER(data);
    UNUSED_PARAMETER(settings);
    //bool recording = obs_data_get_bool(settings,"recording");
    //if(recording){
        stop_logging();
    //    obs_data_set_default_bool(settings, "recording", false);
    //}
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("input-uiohook", "en-US")

OBS_MODULE_AUTHOR("Christian Frisson")

bool obs_module_load(void)
{
    struct obs_source_info input_uiohook_info = {
        .id             = "input_uiohook",
        .type           = OBS_SOURCE_TYPE_INPUT,
        .output_flags   = OBS_SOURCE_INTERACTION,
        .get_name       = input_uiohook_get_name,
        .create         = input_uiohook_create,
        .destroy        = input_uiohook_destroy,
        .update         = input_uiohook_update,
        .start          = input_uiohook_start,
        .stop           = input_uiohook_stop,
        .get_defaults   = input_uiohook_defaults,
        .show           = input_uiohook_show,
        .hide           = input_uiohook_hide,
        .get_properties = input_uiohook_properties
    };

	obs_register_source(&input_uiohook_info);
	return true;
}
