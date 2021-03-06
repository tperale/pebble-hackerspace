#pragma once

#include <pebble.h>
#include "../globals.h"
#include "../libs/basic/basic.h"

extern BasicInfo* basic_info;

void win_basic_show(void);
void win_basic_init(void);
void win_basic_deinit(void);
