#include "bsp/native_post_effect_frame_binding.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native post-effect frame binding requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
Word word(const volatile void* actual, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, actual
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* actual, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, actual
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
volatile long* count(void* actual) noexcept {
    return reinterpret_cast<volatile long*>(reinterpret_cast<Word>(actual) + 4u);
}
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
}

void set_native_post_effect_frame_target_00b4e3d0(void* actual,
    void* incoming, NativePostEffectFrameBindingContext& context) {
    void* const old = reinterpret_cast<void*>(word(actual, 8)); // B4E3D5.
    if (old == incoming) return;
    put(actual, 8, reinterpret_cast<Word>(incoming)); // B4E3DE, before either call.
    if (incoming) {
        const auto increment = context.actual_increment_00ce221c; // B4E3E7.
        require(increment != nullptr, "post-effect frame binding requires its current increment IAT target");
        increment(count(incoming));
    }
    if (old) {
        const auto decrement = context.actual_decrement_00ce2220; // B4E3F5, after retain.
        require(decrement != nullptr, "post-effect frame binding requires its current decrement IAT target");
        if (decrement(count(old)) != 0) return;
        require(word(old) == 0x00d5e600u && context.actual_frame_profile_00d5e600 &&
            word(context.actual_frame_profile_00d5e600) == 0x00bd30e0u,
            "post-effect frame binding requires the current concrete frame virtual-zero profile");
        // BD30E0 rereads the actual profile before its deleting slot.
        require(word(old) == 0x00d5e600u &&
            word(context.actual_frame_profile_00d5e600, 4) == 0x00b1fcf0u,
            "post-effect frame binding requires the current concrete frame deleting slot");
        delete_native_frame_target_owner_00b1fcf0(
            *static_cast<NativeFrameTargetOwnerStorage*>(old), 1, context.frame_targets);
        // The provider may free old. Native performs no further access or clear.
    }
}

__declspec(naked) std::uint32_t __fastcall get_native_post_effect_count_00b4cc00(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 0x20]
        ret
    }
}
} // namespace bsp
