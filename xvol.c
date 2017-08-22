#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "config.h"
#include "log.h"
#include "private.h"
#include "libxvol.h"
#include "constants.h"

#define DEFAULT_CONFIG "xvol.cfg"

config_t *_read_config(int argc, char **argv);

config_t *config;

int main(int argc, char **argv)
{
    config = _read_config(argc, argv);
    char *plugin = config_get_str(config, "plugin");
    char *dom = config_get_str(config, "dom");
    char *rekall = config_get_str(config, "rekall_profile");
    char *param = config_get_str(config, "param");

    if (!plugin) {
        writelog(LV_ERROR, "Plugin name missing");
        goto error;
    }
    if (!dom) {
        writelog(LV_ERROR, "Please specify domain name");
        goto error;
    }
    if (!rekall) {
        writelog(LV_ERROR, "Please specify rekall profile");
        goto error;
    }

    constants_init(rekall);
    if (!strncmp(plugin, "pslist", STR_BUFF)) {
        pslist_exec();
    }
    else if (!strncmp(plugin, "handle", STR_BUFF)) {
        handle_exec(param);
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
    char param[STR_BUFF] = "";

    int c;

    static struct option long_options[] = {
        {"config", required_argument, 0, 'c'},
        {"rekall", required_argument, 0, 'r'},
        {"domain", required_argument, 0, 'd'},
        {"plugin", required_argument, 0, 'p'},
        {"param", required_argument, 0, 'r'},
        {0, 0, 0, 0}
    };
    int option_index = 0;

    //Read config from command line
    while ((c = getopt_long(argc, argv, "c:r:d:p:a:", long_options, &option_index)) != -1) {
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
            case 'a':
                strncpy(param, optarg, STR_BUFF);
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
    if (param[0])
        config_set_str(cfg, "param", param);
    return cfg;
}
