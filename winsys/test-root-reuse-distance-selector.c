#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "../mesa-25/src/nouveau/vulkan/nvk_root_reuse_distance_validation.h"

static void
check(const char *control, const char *variant, bool historical,
      bool expected_valid, bool expected_control, bool expected_variant)
{
   bool control_enabled = false, variant_enabled = false;
   assert(nvk_root_reuse_distance_selectors_valid(
             control, variant, historical,
             &control_enabled, &variant_enabled) == expected_valid);
   assert(control_enabled == expected_control);
   assert(variant_enabled == expected_variant);
}

int
main(void)
{
   check(NULL, NULL, false, true, false, false);
   check("0", "0", false, true, false, false);
   check("1", "0", false, true, true, false);
   check("0", "1", false, true, false, true);
   check("1", "1", false, false, false, false);
   check("x", "0", false, false, false, false);
   check("0", "2", false, false, false, false);
   check("1", "0", true, false, false, false);
   check("0", "1", true, false, false, false);
   return 0;
}
