//
// Created by 张育 on 2022/1/9.
//

#include "config.h"

static config_t default_config = {
        9000,
        9001,
        9002,
};

config_t *load_config(const char* path) {
    return &default_config;
}