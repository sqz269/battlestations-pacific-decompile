#include "bsp/font_shader.hpp"
#include <xmmintrin.h>

namespace bsp {
std::string select_font_shader_name_00ab8ce0_fragment(const std::string& override_name,
    std::int32_t reference_height, float font_scale) {
    if (!override_name.empty()) return override_name;
    // Native CVTSI2SS then ordered UCOMISS gates. Keep MXCSR rounding for the
    // integer conversion; a NaN scale fails equality and selects bilinear.
    const float converted = _mm_cvtss_f32(_mm_cvtsi32_ss(_mm_setzero_ps(), reference_height));
    return converted == 720.0f && font_scale == 1.0f
        ? "GuiFont.mshd" : "GuiFontBilinear.mshd";
}
}
