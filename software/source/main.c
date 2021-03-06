/**
 * USB-Thing Utility
 */

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "libusb-1.0/libusb.h"

#include "usbthing.h"
#include "selftest.h"
#include "version.h"

#define DEFAULT_VID     0x0001
#define DEFAULT_PID     0x0001

enum mode_e {
    MODE_UNRECOGNIZED = 0,
    MODE_LIST = 1,
    MODE_SELFTEST = 2,
    MODE_VERSION = 3
};

struct config_s {
    int mode;
    int device;
    int quiet;
    uint32_t vid;
    uint32_t pid;
};

int mode_selftest(usbthing_t usbthing, struct config_s *config);
int mode_version(usbthing_t usbthing, struct config_s *config);
int mode_list(usbthing_t usbthing, struct config_s *config);

void parse(int argc, char** argv, struct config_s* config);
void print_help();

int main(int argc, char **argv)
{
    usbthing_t usbthing;
    struct config_s config;

    int res;

    parse(argc, argv, &config);

    USBTHING_init();

    switch (config.mode) {
    case MODE_LIST:
        mode_list(usbthing, &config);
        break;

    case MODE_SELFTEST:
        mode_selftest(usbthing, &config);
        break;

    case MODE_VERSION:
        mode_version(usbthing, &config);
        break;

    case MODE_UNRECOGNIZED:
        print_help();
        break;
    }

    USBTHING_close();

    return 0;
}

int mode_list(usbthing_t usbthing, struct config_s *config)
{
    int res;

    printf("Listing devices: \r\n");
    res = USBTHING_list_devices(config->vid, config->pid);

    return res;
}

int mode_selftest(usbthing_t usbthing, struct config_s *config)
{
    int res;

    res = USBTHING_connect(&usbthing, config->vid, config->pid);
    if (res < 0) {
        printf("Error opening USB thing\n");
        return -1;
    }

    printf("Running self tests\r\n");

    int interactive = (config->quiet == 0)? 1 : 0;
    self_test(usbthing, interactive);

    res = USBTHING_disconnect(&usbthing);
    if (res < 0) {
        printf("Error closing USB thing\n");
        return -2;
    }

    return 0;
}

int mode_version(usbthing_t usbthing, struct config_s *config)
{
    int res;
    char version[32];

    res = USBTHING_connect(&usbthing, config->vid, config->pid);
    if (res < 0) {
        printf("Error opening USB thing\n");
        return -1;
    }

    USBTHING_get_firmware_version(usbthing, sizeof(version), version);

    printf("Firmware version: %s\r\n", version);

    res = USBTHING_disconnect(&usbthing);
    if (res < 0) {
        printf("Error closing USB thing\n");
        return -2;
    }

    return 0;
}

void parse(int argc, char** argv, struct config_s* config) {

    int c;
    int option_index = 0;
    static struct option long_options[] =
    {
        {"help", no_argument,          0, 'h'},
        {"mode", required_argument,    0, 'm'},
        {"vid",  required_argument,    0, 'v'},
        {"pid",  required_argument,    0, 'p'},
        {"device",  required_argument, 0, 'd'},
        {"quiet", no_argument,         0, 'q'},
        {0, 0, 0, 0}
    };

    config->vid = DEFAULT_VID;
    config->pid = DEFAULT_PID;

    while (1) {
        c = getopt_long (argc, argv, "h",
                         long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            print_help();
            exit(0);
            break;

        case 'm':
            if (strncmp(optarg, "selftest", 8) == 0) {
                config->mode = MODE_SELFTEST;
            } else if (strncmp(optarg, "version", 7) == 0) {
                config->mode = MODE_VERSION;
            } else if (strncmp(optarg, "list", 4) == 0) {
                config->mode = MODE_LIST;
            } else {
                printf("unrecognized mode option\r\n");
                config->mode = MODE_UNRECOGNIZED;
            }

            break;

        case 'v':
            config->vid = atoi(optarg);
            break;

        case 'p':
            config->pid = atoi(optarg);
            break;

        case 'd':
            config->device = atoi(optarg);
            break;

        case 's':
            //TODO: parse serial
            break;

        case 'q':
            config->quiet = 1;
            break;

        default:
            printf("Unrecognized option %s\r\n", long_options[option_index].name);
            break;
        }
    }
}

void print_help() {
    printf("\r\nUSBThing Utility (Version: %s)\r\n", SOFTWARE_VERSION);
    printf("\r\n");
    printf("Arguments:\r\n");
    printf("--mode [mode], required\r\n");
    printf("\tversion - fetch the connected device version\r\n");
    printf("\tselftest - run self test mode\r\n");
    printf("\tlist - list attached devices\r\n");
    printf("--vid [VID], device Vendor ID\r\n");
    printf("--pid [PID], device Product ID\r\n");
    printf("--quiet, removes prompts where possible\r\n");
    printf("\r\n");
}
