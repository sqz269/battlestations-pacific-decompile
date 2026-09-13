#include "bsp/native_renderer_format_check.hpp"
#include "bsp/native_resource_creation_flags.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer format checking requires MSVC Win32.
#endif

namespace bsp {
namespace {

using U32 = std::uint32_t;
using I32 = std::int32_t;
static_assert(sizeof(void*) == 4);

U32 load(const void* base, U32 offset) noexcept {
    const auto address = reinterpret_cast<U32>(base) + offset;
    return *reinterpret_cast<const volatile U32*>(address);
}

using CheckDeviceFormat = I32 (__stdcall*)(
    void*, U32, U32, U32, U32, U32, U32);

} // namespace

bool check_native_renderer_device_format_00b21ec0(
    void* renderer, U32 adapter_format, U32 flags, U32 kind, U32 check_format) {
    // Native output pointers name its flags/kind argument slots, while EDI
    // retains the original kind. These private cells preserve that ordering.
    U32 translated_pool = flags;
    U32 translated_usage = kind;
    translate_native_resource_creation_flags_00b20a80(
        &translated_usage, &translated_pool, flags, kind);

    void* const factory = reinterpret_cast<void*>(load(renderer, 0x1990));
    void* const table = reinterpret_cast<void*>(load(factory, 0));
    const auto method = reinterpret_cast<CheckDeviceFormat>(load(table, 0x28));
    const I32 result = method(
        factory, 0, 1, adapter_format, translated_usage, kind, check_format);
    return result >= 0;
}

bool check_native_renderer_vertex_texture_render_target_00b20190(
    void* renderer, U32 check_format) {
    void* const factory = reinterpret_cast<void*>(load(renderer, 0x1990));
    void* const table = reinterpret_cast<void*>(load(factory, 0));
    const auto method = reinterpret_cast<CheckDeviceFormat>(load(table, 0x28));
    const I32 result = method(factory, 0, 1, 0x16, 0x100001, 3, check_format);
    return result == 0;
}

} // namespace bsp
