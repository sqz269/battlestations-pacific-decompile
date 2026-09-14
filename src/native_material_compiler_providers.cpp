#include "bsp/native_material_compiler_providers.hpp"
#include "bsp/native_vertex_shader_compilation.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native compiler providers require MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
U word(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(static_cast<const char*>(p) + offset);
}
void entry(void* stream, U slot, U expected, const volatile U* profile) {
    if (word(stream) != 0x00d691b0 || !profile || profile[slot / 4u] != expected)
        throw std::logic_error("compiler stream requires the actual recovered physical write entry");
}
}
U write_native_compiler_stream_word_00be40d0(void* stream, U value, U* count,
    const volatile U* profile) {
    entry(stream, 0x28, 0x00bf4f50, profile);
    return write_native_physical_stream_00bf4f50(stream, &value, 4, count);
}
U write_native_compiler_stream_string_00be4460(void* stream, const void* name,
    U* count, const volatile U* profile, const char* empty) {
    const U length = word(name);
    entry(stream, 0x54, 0x00be40d0, profile);
    (void)write_native_compiler_stream_word_00be40d0(stream, length, count, profile);
    const char* data = reinterpret_cast<const char*>(word(name, 4));
    if (!data) data = empty;
    entry(stream, 0x28, 0x00bf4f50, profile);
    return write_native_physical_stream_00bf4f50(stream, data, length, count);
}
const void* __fastcall get_native_compiler_renderer_capabilities_00b1ff50(const void* renderer) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<U>(renderer) + 0x1b18u);
}
NativeMaterialCompilerPassConstructionFrame::~NativeMaterialCompilerPassConstructionFrame() {
    if (phase == Phase::running || phase == Phase::failed) std::terminate();
}
NativeMaterialPassStorage* initialize_native_compiler_pass_00b44b10(void* raw,
    NativeMaterialPassConstructionAccess& access, NativeTextureCacheContext& cache,
    const volatile U* profile, NativeMaterialCompilerPassConstructionFrame& a) {
    using Phase = NativeMaterialCompilerPassConstructionFrame::Phase;
    if (a.phase != Phase::fresh || !access.state_registration.bind)
        throw std::logic_error("actual pass construction requires a fresh persistent frame and canonical states");
    a.phase = Phase::running;
    a.pass = static_cast<NativeMaterialPassStorage*>(raw);
    try {
        auto* pass = ::new(raw) NativeMaterialPassStorage;
        a.native_site = 0x00b44b30;
        initialize_native_material_pass_base_00b5f720(&pass->base);
        a.base_constructed = true;
        pass->base.root.vtable_00 = 0x00d61be8;
        pass->binding_count_6c = 0;
        a.exception_state = 0;
        pass->vertex_shader_70 = nullptr; pass->pixel_shader_74 = nullptr;
        pass->index_78 = -1; pass->index_7c = -1;
        pass->byte_80 = 0; pass->fallback_84 = nullptr;
        access.state_registration.bind(access.state_registration.context, pass->base);
        a.states_registered = true;
        a.native_site = 0x00b44b6f;
        resize_native_string_header_0041dd40(&a.temporary, access.lifetime.strings, 9, true);
        a.temporary_live = true;
        if (a.temporary.data()) std::memcpy(a.temporary.data(), "white.tga", a.temporary.length() + 1u);
        // The native cache call may fail with retained names/resources. Keep
        // this exact child and actual input temporary alive through that state.
        a.texture = std::make_unique<NativeTextureCacheAcquired>();
        void* renderer = access.current_renderer_00f8d394;
        const U current_table = word(renderer);
        const U target = profile ? profile[0x64 / 4] : 0;
        a.exception_state = 1; a.native_site = 0x00b44ba8;
        if (current_table != 0x00d5f0a8 || target != 0x00b319b0)
            throw std::logic_error("actual pass constructor requires current numeric D5F0A8/B319B0 texture entry");
        void* acquired = load_native_renderer_texture_00b319b0(renderer, &a.temporary, 0, cache, a.texture.get());
        char* data = a.temporary.data();
        pass->fallback_84 = acquired; a.texture_published = true;
        a.exception_state = 0;
        if (data) {
            const U size = a.temporary.length() + 1u;
            a.native_site = 0x00b44bc7;
            access.lifetime.strings.release(data, size);
        }
        a.temporary_live = false;
        a.phase = Phase::complete;
        return pass;
    } catch (...) { a.phase = Phase::failed; throw; }
}
} // namespace bsp
