#ifndef RENDER_H
#define RENDER_H

#include <vulkan/vulkan.h>

#include "common.h"

void render_create_sync_objects(void);
void render_loop(void);

#endif
