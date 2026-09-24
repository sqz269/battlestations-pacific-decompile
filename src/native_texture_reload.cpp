#include "bsp/native_texture_reload.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_logical_texture_named_base.hpp"
#include "bsp/native_memory_stream.hpp"
#include "bsp/native_physical_buffer_owner.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_physical_stream_open.hpp"
#include "bsp/native_raw_inflate_stream.hpp"
#include "bsp/native_render_buffer_unregistration.hpp"
#include "bsp/native_shader_device_reset.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_open_route.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include <d3dx9tex.h>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture reload requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> volatile T& field(void* p, std::size_t offset) {
    return *reinterpret_cast<volatile T*>(static_cast<unsigned char*>(p) + offset);
}
template<class T> const volatile T& field(const void* p, std::size_t offset) {
    return *reinterpret_cast<const volatile T*>(static_cast<const unsigned char*>(p) + offset);
}
void* at(void* p, std::uint32_t offset) {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::uint32_t table(const void* owner) { return field<std::uint32_t>(owner, 0); }
std::uint32_t slot(std::uint32_t table_word, std::uint32_t offset) {
    return *reinterpret_cast<const volatile std::uint32_t*>(table_word + offset);
}
void require_domains(NativeTextureLoadingContext& c) {
    auto& o = c.owners.texture_context();
    auto& n = o.renderer_notification;
    if (!c.cache || &c.cache->textures != &c ||
        &c.strings != &c.opens.physical.strings ||
        &c.current_vfs_0109ceec != &c.opens.physical.manager_0109ceec ||
        c.opens.native_bindings != &c.streams ||
        &c.conversion.memory_owners != &o.retained_memory ||
        n.actual_native_string_pool != &c.strings ||
        &n.actual_string_storage != &c.strings ||
        o.surfaces.actual_string_pool_00419cc0.actual_storage() != &c.strings ||
        &n.actual_renderer_00f8d394 != &c.current_renderer_00f8d394 ||
        reinterpret_cast<const volatile void*>(&o.surfaces.actual_renderer_00f8d394) !=
        reinterpret_cast<const volatile void*>(&c.current_renderer_00f8d394) ||
        &n.synchronization != &c.synchronization_0108d6dc)
        throw std::invalid_argument("texture reload requires the same actual loading/owner domains");
}
void release_stream(void*& stream, bool& started, NativeTextureLoadingContext& c) {
    void* const captured = stream;
    started = true;
    if (InterlockedDecrement(&field<LONG>(captured, 4)) == 0)
        c.streams.zero_reference(table(captured), captured);
    stream = nullptr;
}
void delete_invalid_stream(NativeTextureLoadingContext& c, NativeTextureReloadAcquired& a) {
    const auto profile = table(a.source);
    const auto target = slot(profile, 4);
    if (!((profile == 0x00d691b0 && target == 0x00bf5090) ||
          (profile == 0x00d642c0 && target == 0x00bb8f90) ||
          (profile == 0x00d68db0 && target == 0x00bf1240) ||
          (profile == 0x00d64400 && target == 0x00bbc3e0)))
        throw std::invalid_argument("unsupported current invalid-stream deleting slot");
    a.source_release_started = true;
    if (profile == 0x00d691b0 && target == 0x00bf5090)
        delete_native_physical_stream_00bf5090(a.source, 1);
    else if (profile == 0x00d642c0 && target == 0x00bb8f90)
        delete_native_memory_stream_00bb8f90(a.source, 1, c.conversion.memory_owners);
    else if (profile == 0x00d68db0 && target == 0x00bf1240)
        delete_native_adopted_substream_00bf1240(a.source, 1, c.streams);
    else if (profile == 0x00d64400 && target == 0x00bbc3e0)
        delete_native_raw_inflate_stream_00bbc3e0(a.source, 1, c.streams);
    a.source = nullptr; // Native direct slot+4(1), not a reference decrement.
}
std::uint32_t memory_size(void* memory, NativeTextureLoadingContext& c) {
    return static_cast<std::uint32_t>(c.streams.length(table(memory), memory));
}
bool substring(const void* name, const char* needle) {
    const auto* const data = field<char*>(name, 4);
    if (!data) return false;
    const auto* const found = std::strstr(data, needle);
    if (!found) return false;
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(found) -
        reinterpret_cast<std::uintptr_t>(field<char*>(name, 4))) != 0xffffffffu;
}
std::uint32_t signed_minimum_one(std::uint32_t bits) {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value > 1 ? bits : 1;
}
void support(NativeTextureLoadingContext& c) {
    auto& s = c.owners.texture_context().surfaces;
    resource_support_singleton_00b3e730(s.actual_resource_support_0108fedc,
        s.actual_lifetime_01090aa0);
}
struct DiagnosticCleanup {
    NativeTextureReloadAcquired& acquired;
    ActualNativeStringPoolStorage& strings;
    bool armed{};
    ~DiagnosticCleanup() noexcept {
        if (armed) {
            destroy_native_buffer_diagnostic_record_00b3f4c0(&acquired.diagnostic, strings);
            acquired.diagnostic_released = true;
        }
    }
};
} // namespace

void* construct_native_texture_reload_diagnostic_00b3f5d0(void* output,
    IDirect3DTexture9* const* borrowed, const void* name,
    ActualNativeStringPoolStorage& strings) {
    const auto captured = *borrowed;
    auto* const header = at(output, 4);
    field<IDirect3DTexture9*>(output, 0) = captured;
    field<std::uint32_t>(header, 0) = 0;
    field<char*>(header, 4) = nullptr;
    if (header != name) {
        resize_native_string_header_0041dd40(header, strings,
            field<std::uint32_t>(name, 0), true);
        if (field<std::uint32_t>(name, 0) != 0) {
            const auto count = field<std::uint32_t>(header, 0);
            auto* const source = field<char*>(name, 4);
            auto* const destination = field<char*>(header, 4);
            if (count != 0) std::memmove(destination, source, count);
        }
    }
    return output;
}

void reload_native_texture_00b3fa90(void* owner, NativeTextureLoadingContext& c,
    NativeTextureReloadOutputs outputs, NativeTextureReloadAcquired& a) {
    if (a.phase != NativeTextureReloadAcquired::Phase::not_started)
        throw std::logic_error("texture reload cannot replay a retained invocation");
    require_domains(c);
    a.phase = NativeTextureReloadAcquired::Phase::running;
    a.owner = owner;
    a.name = at(owner, 8);
    try {
        a.native_site = 0x00b3fac1;
        auto* const manager = c.current_vfs_0109ceec;
        a.source = c.streams.open_manager_entry(slot(table(manager), 4), manager, a.name, 2);
        a.native_site = 0x00b3facc;
        if (!c.streams.is_open(table(a.source), a.source)) {
            a.native_site = 0x00b3fadb;
            delete_invalid_stream(c, a);
            a.phase = NativeTextureReloadAcquired::Phase::complete;
            return;
        }
        a.native_site = 0x00b3faf0;
        a.memory = convert_native_stored_stream_00bef750(a.source, c.conversion);
        a.native_site = 0x00b3fafb;
        release_stream(a.source, a.source_release_started, c);

        const auto* const renderer = c.current_renderer_00f8d394;
        auto& owner_context = c.owners.texture_context();
        if (table(renderer) != 0x00d5f0a8 || !owner_context.actual_renderer_profile_00d5f0a8)
            throw std::invalid_argument("unsupported current texture-removal renderer profile");
        const auto* const notification_slot = owner_context.actual_renderer_profile_00d5f0a8 + 0x6c / 4;
        a.native_site = 0x00b3fb1b;
        const auto* const owner_name = native_logical_texture_name_address_00b33e40(owner);
        a.native_site = 0x00b3fb25;
        if (*notification_slot != 0x00b32250)
            throw std::invalid_argument("unsupported current renderer texture-removal slot");
        notify_native_renderer_texture_name_removal_00b32250(
            const_cast<void*>(renderer), owner_name, owner_context.renderer_notification);
        a.native_site = 0x00b3fb2e;
        unregister_native_renderer_texture_00b27d40(
            const_cast<void*>(c.current_renderer_00f8d394), reinterpret_cast<std::uintptr_t>(owner));
        a.native_site = 0x00b3fb33;
        support(c);
        a.native_site = 0x00b3fb44;
        const auto bytes = memory_size(a.memory, c);
        a.native_site = 0x00b3fb49;
        const auto* const data = native_memory_stream_data_00bef610(a.memory, nullptr);
        a.native_site = 0x00b3fb4f;
        if (!c.image_info_00c2dfec) throw std::invalid_argument("missing actual D3DX image-info import");
        a.image_info_result = c.image_info_00c2dfec(data, bytes, &outputs.image_info);
        auto width = 0xffffffffu, height = 0xffffffffu;
        auto saved_width = outputs.image_info.Width, saved_height = outputs.image_info.Height;
        auto mips = outputs.image_info.MipLevels;
        if (mips > 1u && field<std::uint32_t>(c.current_renderer_00f8d394, 0x1d84) != 0) {
            a.native_site = 0x00b3fba5;
            bool reduce = !substring(a.name, "detail.dds");
            if (reduce) { a.native_site = 0x00b3fbca; reduce = !substring(a.name, "noseart"); }
            if (reduce) { a.native_site = 0x00b3fbeb; reduce = !substring(a.name, "interface/textures/gui/units"); }
            if (reduce) {
                const auto quality = field<std::uint32_t>(c.current_renderer_00f8d394, 0x1d84);
                width = saved_width = signed_minimum_one(saved_width >> (quality & 31u));
                height = saved_height = signed_minimum_one(saved_height >> (quality & 31u));
                mips = signed_minimum_one(mips - quality);
            }
        }
        if (auto* const old_com = field<IDirect3DTexture9*>(owner, 0x10)) {
            a.native_site = 0x00b3fc64;
            a.old_com_release_started = true;
            old_com->Release();
            field<IDirect3DTexture9*>(owner, 0x10) = nullptr;
        }
        a.native_site = 0x00b3fc72;
        a.captured_device = static_cast<IDirect3DDevice9*>(get_native_renderer_device_00b1fef0(c.current_renderer_00f8d394));
        a.native_site = 0x00b3fca3;
        const auto current_size = memory_size(a.memory, c);
        a.native_site = 0x00b3fca8;
        const auto* const current_data = native_memory_stream_data_00bef610(a.memory, nullptr);
        a.native_site = 0x00b3fcaf;
        if (!c.create_texture_00c2dfe6) throw std::invalid_argument("missing actual D3DX texture-create import");
        a.create_result = c.create_texture_00c2dfe6(a.captured_device, current_data, current_size,
            width, height, mips, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 0x70004u,
            0xffffffffu, 0, nullptr, nullptr, reinterpret_cast<IDirect3DTexture9**>(at(owner, 0x10)));
        if (field<IDirect3DTexture9*>(owner, 0x10)) {
            a.native_site = 0x00b3fcc1;
            field<std::uint32_t>(owner, 0x24) = memory_size(a.memory, c);
            a.native_site = 0x00b3fcce;
            field<std::uint32_t>(owner, 0x14) = field<IDirect3DTexture9*>(owner, 0x10)->GetLevelCount();
            a.native_site = 0x00b3fce1;
            a.level_desc_result = field<IDirect3DTexture9*>(owner, 0x10)->GetLevelDesc(0, &outputs.level_zero);
            field<std::uint32_t>(owner, 0x28) = outputs.level_zero.Width;
            field<std::uint32_t>(owner, 0x2c) = outputs.level_zero.Height;
            field<std::uint32_t>(owner, 0x18) = outputs.level_zero.Format;
            field<std::uint32_t>(owner, 0x34) = saved_width;
            field<std::uint32_t>(owner, 0x38) = saved_height;
            field<std::uint32_t>(owner, 0x30) = 0;
        }
        a.native_site = 0x00b3fd0d;
        release_stream(a.memory, a.memory_release_started, c);
        IDirect3DTexture9* const diagnostic_texture = field<IDirect3DTexture9*>(owner, 0x10);
        a.native_site = 0x00b3fd33;
        construct_native_texture_reload_diagnostic_00b3f5d0(&a.diagnostic, &diagnostic_texture, a.name, c.strings);
        a.diagnostic_constructed = true;
        DiagnosticCleanup cleanup{a, c.strings, true}; // FH3 state0 only after constructor returns.
        a.native_site = 0x00b3fd3c;
        support(c);
        auto* const captured_data = field<char*>(&a.diagnostic, 8);
        cleanup.armed = false; // Native state -1 precedes normal pool calls.
        if (captured_data) {
            a.native_site = 0x00b3fd5e;
            c.strings.release(captured_data, field<std::uint32_t>(&a.diagnostic, 4) + 1u);
        }
        a.diagnostic_released = true;
        a.phase = NativeTextureReloadAcquired::Phase::complete;
    } catch (...) {
        a.phase = NativeTextureReloadAcquired::Phase::failed;
        throw;
    }
}
} // namespace bsp
