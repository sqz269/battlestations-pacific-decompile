#include "bsp/native_damageable_class_section_index_fragment.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Damageable section Index reconstruction requires MSVC Win32 x87.
#endif

namespace bsp {
namespace {
struct IndexFieldCleanup {
    NativeDamageableClassSectionIndexFragmentScratch& scratch;
    unsigned state = 17;

    ~IndexFieldCleanup() noexcept {
        if (state == 18) {
            state = 17;
            destroy_native_lua_object_00b67700(
                *static_cast<NativeLuaObjectStorage*>(scratch.fresh_field_at_parent_6c));
        }
    }
};
static_assert(sizeof(void*) == 4 && sizeof(unsigned) == 4);
} // namespace

void read_native_damageable_class_section_index_fragment_0087cf35(
    void* row, NativeDamageableClassSectionIndexFragmentScratch& scratch) {
    IndexFieldCleanup cleanup{scratch};
    auto& value = *static_cast<NativeLuaObjectStorage*>(
        scratch.same_msh_scratch.same_iterator_scratch.fresh_value_at_parent_2c);
    auto* field = native_lua_get_by_name_protected(value,
        scratch.fresh_field_at_parent_6c, scratch.actual_index_key_00ce55b4);
    cleanup.state = 18;
    unsigned* const active_state = &cleanup.state;
    const volatile std::uint32_t* const mode = &scratch.actual_crt_conversion_mode_0109eea4;
    _ReadWriteBarrier();
    // Actual C++ cdecl getter returns ST0. Its one pointer argument is removed
    // before the genuine assembly-only fastcall converter consumes that ST0.
    __asm {
        push field
        call native_lua_number_00b66270
        add esp, 4
        mov ecx, mode
        call native_crt_truncate_st0_00bf7420
        mov edx, row
        mov dword ptr [edx + 8], eax // CF60: still18, raw integer bits
        mov edx, active_state
        mov dword ptr [edx], 11h // CF63: lower18->17 after row store
    }
    _ReadWriteBarrier();
    destroy_native_lua_object_00b67700(*field);
}
} // namespace bsp
