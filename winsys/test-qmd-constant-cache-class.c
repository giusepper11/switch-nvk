/* SPDX-License-Identifier: MIT */
#include <assert.h>

#include "../mesa-25/src/nouveau/vulkan/nvk_qmd_constant_cache_validation.h"

int
main(void)
{
   const unsigned maxwell_compute_b = 0xb1c0;
   assert(nvk_qmd_constant_cache_compute_class_valid(false, 0, maxwell_compute_b));
   assert(nvk_qmd_constant_cache_compute_class_valid(true, maxwell_compute_b,
                                                      maxwell_compute_b));
   assert(!nvk_qmd_constant_cache_compute_class_valid(true, 0xa0c0,
                                                       maxwell_compute_b));
   return 0;
}
