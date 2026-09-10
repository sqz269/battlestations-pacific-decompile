#include "bsp/system_fog_constants.hpp"
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error System fog constants require MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {
// Each original scalar getter is exactly FLD [ECX+offset]; RET. Pair it with
// the caller's single FSTP, retaining x87 exception state and sNaN quieting.
void copy_scalar_x87(float* destination, const float* source) noexcept {
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
// Original colors use an individual integer read/store pair per word. Keep
// source/destination overlap effects and arbitrary NaN payloads exactly.
void copy_color_word(float* destination, const float* source) noexcept {
    __asm {
        mov eax, source
        mov edx, destination
        mov ecx, dword ptr [eax]
        mov dword ptr [edx], ecx
    }
}
const SystemFogState* reload_owner(const SystemFogState* const& slot) noexcept {
    const SystemFogState* value;
    __asm {
        mov eax, slot
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
}

bool write_system_fog_constants_00b46d97(const SystemFogState* captured_fog,
    const SystemFogState* const& current_camera_fog,
    float* prefix, std::size_t word_count, std::string& error) {
    error.clear();
    if (!captured_fog) return true; // Native00B46D7F/00B46D91 gate.
    if (!prefix || word_count < 75 * 4) {
        error = "System fog constants require initialized storage through c74";
        return false;
    }

    copy_scalar_x87(prefix + 72 * 4, &captured_fog->scalar_80); //00B84DA0
    copy_scalar_x87(prefix + 72 * 4 + 1, &captured_fog->scalar_84); //00B84DB0
    copy_scalar_x87(prefix + 73 * 4, &captured_fog->scalar_88); //00B84E20
    copy_scalar_x87(prefix + 73 * 4 + 1, &captured_fog->scalar_8c); //00B84E30
    copy_scalar_x87(prefix + 73 * 4 + 2, &captured_fog->scalar_90); //00B84E40
    copy_scalar_x87(prefix + 35 * 4, &captured_fog->scalar_6c); //00B84CB0
    copy_scalar_x87(prefix + 35 * 4 + 1, &captured_fog->scalar_70); //00B84CC0
    copy_scalar_x87(prefix + 35 * 4 + 2, &captured_fog->scalar_68); //00B84CA0
    copy_scalar_x87(prefix + 36 * 4, &captured_fog->scalar_74); //00B84CD0
    copy_scalar_x87(prefix + 36 * 4 + 1, &captured_fog->scalar_78); //00B84CE0
    copy_scalar_x87(prefix + 36 * 4 + 2, &captured_fog->scalar_7c); //00B84CF0
    for (std::size_t index = 0; index != 4; ++index) { //00B84FD0, RET4
        const float* color = captured_fog->directional_colors_28[index].data();
        for (std::size_t lane = 0; lane != 4; ++lane)
            copy_color_word(prefix + (38 + index) * 4 + lane, color + lane);
    }

    const auto* fog_color_owner = reload_owner(current_camera_fog); //00B46E69
    if (!fog_color_owner) {
        error = "System c37 requires the actual reloaded camera fog owner";
        return false;
    }
    const float* color = fog_color_owner->color_08.data(); //00B84C60
    for (std::size_t lane = 0; lane != 3; ++lane)
        copy_color_word(prefix + 37 * 4 + lane, color + lane);

    //00B46E91 reads color.w, E94 reloads camera+184, E9A stores color.w.
    // Keep this single assembly sequence so a shared owner slot cannot move
    // across the last store, and no floating conversion touches the raw word.
    const SystemFogState* underwater_owner;
    const SystemFogState* const* slot = &current_camera_fog;
    float* color_w_destination = prefix + 37 * 4 + 3;
    __asm {
        mov eax, color
        mov edx, dword ptr [eax + 12]
        mov ecx, slot
        mov ecx, dword ptr [ecx]
        mov underwater_owner, ecx
        mov eax, color_w_destination
        mov dword ptr [eax], edx
    }
    if (!underwater_owner) {
        error = "System c74 requires the actual reloaded camera fog owner";
        return false;
    }
    const float* underwater = underwater_owner->underwater_color_18.data(); //00B84C90
    for (std::size_t lane = 0; lane != 4; ++lane)
        copy_color_word(prefix + 74 * 4 + lane, underwater + lane);
    return true;
}
}
