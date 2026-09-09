#pragma once
#include <cstdint>
#include <string>

namespace bsp {
// Selection-only fragment of00ab8ce0. Native ECX text context, RET/boolAL;
// full method skips nonnull cached shader, loads/caches and reports selection
// attempted (even null result). Here inputs replace context/global fields and
// output is a name, not a loaded shader or native ABI. No cache mutation.
std::string select_font_shader_name_00ab8ce0_fragment(const std::string& override_name,
    std::int32_t reference_height, float font_scale);
}
