#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "log.h"
#include "private.h"

#define DEFAULT_CONFIG "xvol.cfg"

config_t *_read_config(int argc, char **argv);

config_t *config;

int main(int argc, char **argv)
{
    config = _read_config(argc, argv);
    char *plugin = config_get_str(config, "plugin");
    char *rekall = config_get_str(config, "rekall_profile");
    char *dom = config_get_str(config, "dom");

    if (!plugin) {
        writelog(LV_ERROR, "Plugin name missing");
        goto error;
    }
    if (!rekall) {
        writelog(LV_ERROR, "Please specify rekall profile");
        goto error;
    }
    if (!dom) {
        writelog(LV_ERROR, "Please specify domain name");
        goto error;
    }
    return 0;
error:
    return -1;
}

config_t *_read_config(int argc, char **argv)
{
    char cfg_file[PATH_MAX_LEN] = "";
    char rekall_profile[PATH_MAX_LEN] = "";
    char dom[PATH_MAX_LEN] = "";
    char plugin[STR_BUFF] = "";

    int c;

    //Read config from command line
    while ((c = getopt(argc, argv, "c:r:d:p:")) != -1) {
        switch (c) {
            case 'c':
                strncpy(cfg_file, optarg, PATH_MAX_LEN);
                break;
            case 'r':
                strncpy(rekall_profile, optarg, PATH_MAX_LEN);
                break;
            case 'd':
                strncpy(dom, optarg, PATH_MAX_LEN);
                break;
            case 'p':
                strncpy(plugin, optarg, STR_BUFF);
                break;
        }
    }
    //Read config from file
    config_t *cfg = config_init(cfg_file[0] ? cfg_file : DEFAULT_CONFIG);
    if (rekall_profile[0])
        config_set_str(cfg, "rekall_profile", rekall_profile);
    if (dom[0])
        config_set_str(cfg, "dom", dom);
    if (plugin[0])
        config_set_str(cfg, "plugin", plugin);
    return cfg;
}
