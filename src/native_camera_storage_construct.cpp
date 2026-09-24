#include "bsp/native_camera_storage_construct.hpp"
#include "bsp/camera_plane_initialization.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <array>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw camera construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* p, Word n = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + n);
}
void write(void* p, Word n, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(bits(p) + n) = value;
}
void write_byte(void* p, Word n, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(bits(p) + n) = value;
}
const volatile Word* renderer_table(void* actual, NativeViewportRawRendererAccess& access) {
    const auto* table = access.resolve_profile(read(actual));
    if (!table) throw std::invalid_argument("camera constructor: raw renderer profile is unbound");
    return table;
}
NativeViewportOwner& viewport(Word value) {
    if (!value) throw std::invalid_argument("camera constructor: null viewport is outside normal domain");
    return *static_cast<NativeViewportOwner*>(ptr(value));
}
CameraPlaneSet& begin_planes(void* actual) noexcept {
    static_assert(std::is_aggregate_v<CameraPlaneSet> && std::is_trivially_copyable_v<CameraPlaneSet>);
    static_assert(std::is_aggregate_v<CameraPlaneRecord> && std::is_trivially_copyable_v<CameraPlaneRecord>);
    static_assert(sizeof(CameraPlaneSet) == 0x144 && offsetof(CameraPlaneSet, count) == 0x140);
    // Implicit-lifetime creation by memcpy, without invoking default member
    // initializers. Object representation/preimages are unchanged. No enclosing
    // live camera-tail aggregate overlaps this disjoint +2F4..438 object.
    std::array<std::byte, sizeof(CameraPlaneSet)> representation;
    void* const at = ptr(bits(actual) + 0x2f4u);
    std::memcpy(representation.data(), at, representation.size());
    return *static_cast<CameraPlaneSet*>(std::memcpy(at, representation.data(), representation.size()));
}
void require_domain(void* actual, std::size_t extent,
    const NativeCameraStorageConstructFrame& f, const NativeCameraStorageConstructContext& c,
    const NativeCameraStorageConstructAcquired& a) {
    if (a.started || !actual || (bits(actual) & 3u) || extent < NativeCameraPool::slot_bytes ||
        !f.scratch || !f.pushed_words)
        throw std::invalid_argument("camera constructor requires fresh diagnostics and actual aligned45Ch storage/views");
    if (&c.node_constants.one_00d7a24c != &c.viewport.one_bits_00d7a24c ||
        &c.look_at.one_00d7a24c != &c.viewport.one_bits_00d7a24c ||
        static_cast<NativeNodeBaseWorldDispatch*>(&c.pose_dispatch) != &c.lifetime.node.world ||
        &c.lifetime.increment_00ce221c != &c.lifetime.node.scenes.increment_00ce221c ||
        &c.lifetime.decrement_00ce2220 != &c.lifetime.node.scenes.decrement_00ce2220 ||
        &c.lifetime.decrement_00ce2220 != &c.lifetime.node.trees.decrement_00ce2220 ||
        static_cast<NativeRenderActualOwners*>(&c.lifetime.node.scenes.owners) != &c.lifetime.node.trees.owners)
        throw std::invalid_argument("camera constructor requires SAME constants/world/registry/import domain");
}
} // namespace

void* construct_native_camera_storage_00b71a80(void* actual, std::size_t extent,
    volatile Word& incoming, const NativeCameraStorageConstructFrame& f,
    NativeCameraStorageConstructContext& c, NativeCameraStorageConstructAcquired& a) {
    require_domain(actual, extent, f, c, a); // pure source admission before native work
    a.started = true;
    int state = -1;
    const Word name = incoming; // B71A98: before base construction
    f.pushed_words[1] = name;
    try {
        a.active_call_site = 0x00b71aa7;
        (void)construct_native_node_00b6f5a0(actual, extent, ptr(name),
            c.lifetime.node.strings, c.node_constants);
        a.node_complete = true;
        f.pushed_words[1] = 0x34; // B71AAE, before state0/stamp
        state = 0; a.native_eh_state = state;
        write(actual, 0, 0x00d62cf0);
        a.active_call_site = 0x00b71aba;
        void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x34, 0x34});
        a.viewport_allocation = allocation;
        incoming = bits(allocation); // B71AC2: ORIGINAL name argument cell
        state = 1; a.native_eh_state = state;
        NativeViewportOwner* result = nullptr;
        if (allocation) {
            a.active_call_site = 0x00b71ad1;
            result = initialize_native_viewport_owner_00b1f850(allocation, c.viewport);
            a.viewport_complete = true;
        }
        a.viewport_result = result;
        write(actual, 0x180, bits(result));
        a.viewport_published = true;
        write(actual, 0x184, 0);
        state = 0; a.native_eh_state = state;
        write(actual, 0x1b8, 0); write(actual, 0x1bc, 0); write(actual, 0x1c0, 0);
        auto& planes = begin_planes(actual);
        a.planes_live = true;
        a.active_call_site = 0x00b71b0b;
        (void)construct_camera_plane_set_00b659d0(planes, c.viewport.one_bits_00d7a24c);
        write(actual, 0x438, 0);
        f.pushed_words[1] = bits(f.scratch);
        const Word origin_viewport = read(actual, 0x180); // BEFORE state2/43C/zeros
        state = 2; a.native_eh_state = state;
        write(actual, 0x43c, 0);
        f.scratch[0] = 0; f.scratch[1] = 0;
        a.active_call_site = 0x00b71b34;
        set_native_viewport_origin_00b1f920(viewport(origin_viewport), ptr(f.pushed_words[1]));
        void* const renderer = c.viewport.renderer_00f8d394; // captured ONCE B71B39
        if (!renderer) throw std::invalid_argument("camera constructor: actual renderer is unbound");
        auto& renderer_access = c.viewport.renderer_access;
        const auto* first_table = renderer_table(renderer, renderer_access);
        const Word first_target = first_table[0x30 / 4];
        a.active_call_site = 0x00b71b46;
        void* const first_result = renderer_access.invoke_virtual30(first_target, renderer);
        const auto* second_table = renderer_table(renderer, renderer_access); // B71B48 BEFORE height
        const Word height = read(first_result, 0x10);
        const Word second_target = second_table[0x30 / 4]; // B71B4D AFTER height
        a.active_call_site = 0x00b71b52;
        void* const second_result = renderer_access.invoke_virtual30(second_target, renderer);
        const Word width = read(second_result, 0x0c);
        f.scratch[0] = width;
        const Word dimensions_viewport = read(actual, 0x180); // between local stores
        f.pushed_words[1] = bits(f.scratch);
        f.scratch[1] = height;
        a.active_call_site = 0x00b71b6a;
        set_native_viewport_dimensions_00b1f940(viewport(dimensions_viewport), ptr(f.pushed_words[1]));
        volatile Word* const scalar_argument = f.pushed_words + 1;
        __asm { mov eax, scalar_argument
            fldz
            fstp dword ptr [eax] }
        const Word min_viewport = read(actual, 0x180);
        a.active_call_site = 0x00b71b7b;
        set_native_viewport_min_depth_00b1f750(viewport(min_viewport), *scalar_argument);
        __asm { mov eax, scalar_argument
            fld1
            fstp dword ptr [eax] }
        const Word max_viewport = read(actual, 0x180);
        a.active_call_site = 0x00b71b8c;
        set_native_viewport_max_depth_00b1f760(viewport(max_viewport), *scalar_argument);
        const Word hundred = c.hundred_00ce3d08;
        f.pushed_words[1] = bits(f.scratch);
        f.pushed_words[0] = bits(f.scratch + 3);
        f.scratch[0] = 0; f.scratch[1] = hundred; f.scratch[2] = hundred;
        f.scratch[3] = 0; f.scratch[4] = hundred; f.scratch[5] = 0;
        a.active_call_site = 0x00b71bcc;
        set_native_camera_look_at_storage_00b700e0(actual, f.pushed_words[0], f.pushed_words[1],
            f.pose, c.pose_dispatch, c.look_at);
        const Word scalar_1c4 = c.scalar_00ce7d20;
        const Word scalar_1d8 = c.scalar_00d0c5f8;
        const Word one = c.viewport.one_bits_00d7a24c;
        write(actual, 0x1c4, scalar_1c4);
        const Word scalar_1c8 = c.scalar_00d5bd98;
        write(actual, 0x1c8, scalar_1c8); write(actual, 0x1d8, scalar_1d8);
        const Word scalar_178 = c.scalar_00ce77fc;
        write(actual, 0x1cc, 0); write(actual, 0x1d0, 0);
        write(actual, 0x1d4, one); write(actual, 0x178, scalar_178);
        write(actual, 0x2f0, 1); write(actual, 0x198, 0); write(actual, 0x19c, 1);
        write(actual, 0x188, 0);
        auto* const bytes = reinterpret_cast<volatile std::uint8_t*>(&incoming);
        bytes[2] = 0; bytes[1] = 0; bytes[0] = 0;
        const Word scalar_1dc = c.scalar_00ce3c88;
        write(actual, 0x194, 0); write_byte(actual, 0x17c, 1);
        write(actual, 0x18c, one); write(actual, 0x1dc, scalar_1dc);
        bytes[3] = 0;
        const Word color = incoming;
        write(actual, 0x190, color);
        write(actual, 0x440, 0); write(actual, 0x444, one); write(actual, 0x448, 0);
        write(actual, 0x44c, one); write(actual, 0x450, 0); write(actual, 0x454, 0);
        write_byte(actual, 0x174, 1);
        a.complete = true;
        return actual;
    } catch (...) {
        a.exception_cleanup_started = true;
        try {
            if (state == 2) {
                state = 0; a.native_eh_state = state;
                a.active_call_site = 0x00cc1aec;
                clear_native_camera_retained_storage_00605fd0(
                    *reinterpret_cast<volatile Word*>(bits(actual) + 0x438), c.lifetime);
                a.retained_cleanup_complete = true;
            } else if (state == 1) {
                state = 0; a.native_eh_state = state;
                a.active_call_site = 0x00cc1adc;
                void* const current_allocation = ptr(incoming);
                a.allocation_cleanup_pointer = current_allocation;
                singleton_lifetime_free(current_allocation);
                a.allocation_cleanup_complete = true;
            }
            if (state == 0) {
                state = -1; a.native_eh_state = state;
                a.active_call_site = 0x00cc1ad3;
                destroy_native_raw_node_base_00b6f440(actual,
                    f.cleanup_node, c.lifetime.node, a.cleanup_node);
                a.base_cleanup_complete = true;
            }
        } catch (...) { std::terminate(); }
        throw;
    }
}
} // namespace bsp
