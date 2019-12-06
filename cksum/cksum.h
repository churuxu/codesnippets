#pragma once

#include <stdint.h>
#include <stdlib.h>

uint32_t cksum_update(uint32_t last, const void* data, size_t len);

uint32_t cksum_final(uint32_t last, size_t total);
