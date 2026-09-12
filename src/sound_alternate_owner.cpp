#include "bsp/sound_alternate_owner.hpp"
#include "bsp/sound_retained_record.hpp"
#include "bsp/native_vfs_open_logging.hpp"
#include "bsp/unit_motion.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Alternate sound ownership requires MSVC Win32.
#endif
namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t n) noexcept {
    T v; std::memcpy(&v, static_cast<const std::byte*>(p) + n, sizeof v); return v;
}
template<class T> void write(void* p, std::size_t n, T v) noexcept {
    std::memcpy(static_cast<std::byte*>(p) + n, &v, sizeof v);
}
void* at(void* p, std::size_t n) noexcept { return static_cast<std::byte*>(p) + n; }
using ManagerSection = CapturedSoundLifetimeSection;
void* allocate(std::uint32_t n) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, n, n});
}
void release(void* object, SoundAlternateOwnerBindings& b) {
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(at(object, 4))) == 0) {
        // Current native D58F80 slot0 is BD30E0, which invokes slot4 with1.
        // Other current vtables require the same external zero-reference service.
        if (read<std::uint32_t>(object, 0) == 0x00d58f80)
            scalar_delete_sound_alternate_table_00a796f0(object, 1, b.strings);
        else b.references.zero_references_slot_00(object);
    }
}
void clear_reference(void* slot, SoundAlternateOwnerBindings& b) {
    if (void* const captured = read<void*>(slot, 0)) {
        release(captured, b);
        write<void*>(slot, 0, nullptr); // AFTER the zero-reference callback.
    }
}
struct TemporaryName {
    NativeString value;
    NativeStringStorage& strings;
    ~TemporaryName() { destroy_native_string_header_0041dd20(&value, strings); }
};
void destroy_members(NativeSoundAlternateOwnerStorage& owner, SoundAlternateOwnerBindings& b) {
    clear_reference(at(&owner, 0x22c), b);
    for (int i = 17; i >= 0; --i) clear_reference(at(&owner, 0x1c0 + i * 4), b);
    for (int i = 17; i >= 0; --i)
        destroy_sound_alternate_record_004c87f0(at(&owner, 0x10 + i * 0x18), b);
    destroy_sound_alternate_base_00a77970(owner, b);
}
void initialize_logical(void* p, std::uint32_t category) noexcept {
    write<void*>(p, 4, nullptr);
    write<std::uint32_t>(p, 8, 0); write<void*>(p, 0xc, nullptr);
    construct_sound_alternate_record_004c87d0(at(p, 0x10));
    write<void*>(p, 0x28, nullptr);
    construct_sound_alternate_record_004c87d0(at(p, 0x30));
    write<void*>(p, 0x48, nullptr);
    write<std::uint8_t>(p, 0x4c, 0); write<std::uint8_t>(p, 0x4d, 0);
    write<std::uint8_t>(p, 0x4e, 0);
    write(p, 0x50, category); write(p, 0x54, 1.0f);
}
void inspect_diagnostic_name(void* stream, SoundAlternateOwnerBindings& b) {
    TemporaryName temporary{{}, b.strings};
    copy_sound_stream_name_00a77ff0(stream, temporary.value, b.strings);
    // 004254B0 is one RET. The caller still constructs/releases this string.
}
} // namespace

NativeSoundAlternateOwnerStorage& construct_sound_alternate_base_00a778d0(
    NativeSoundAlternateOwnerStorage& owner, SoundAlternateOwnerBindings& b) {
    write<std::uint32_t>(&owner, 0, 0x00d58e58);
    try {
        ManagerSection section(b.domain);
        b.global_00f8bbcc = &owner;
        auto manager = b.domain.get_manager_00415350();
        manager->register_object(b.global_00f8bbcc);
    } catch (...) {
        // DEAB2C state1 releases the captured guard, then state0/412430
        // restores the root vtable. Global publication is not rolled back.
        write<std::uint32_t>(&owner, 0, 0x00ce3818); throw;
    }
    return owner;
}
void destroy_sound_alternate_base_00a77970(
    NativeSoundAlternateOwnerStorage& owner, SoundAlternateOwnerBindings& b) {
    write<std::uint32_t>(&owner, 0, 0x00d58e58);
    try {
        ManagerSection section(b.domain);
        auto manager = b.domain.get_manager_00415350();
        manager->unregister_object(b.global_00f8bbcc);
        b.global_00f8bbcc = nullptr;
    } catch (...) {
        // DEAB60 has the same guard-then-root unwind. Do not clear the
        // publication or undo partial unregister effects on this path.
        write<std::uint32_t>(&owner, 0, 0x00ce3818); throw;
    }
    write<std::uint32_t>(&owner, 0, 0x00ce3818);
}
void* construct_sound_alternate_record_004c87d0(void* p) noexcept {
    for (std::size_t n = 0; n < 0x14; n += 4) write<std::uint32_t>(p, n, 0);
    write(p, 0x14, 1.0f); return p;
}
void destroy_sound_alternate_record_004c87f0(void* p, SoundAlternateOwnerBindings& b) {
    clear_reference(at(p, 0x10), b);
    destroy_native_string_header_0041dd20(at(p, 8), b.strings);
    destroy_native_string_header_0041dd20(p, b.strings);
}
void* construct_sound_alternate_reference_00a778c0(void* p) noexcept {
    write<void*>(p, 0, nullptr); return p;
}
void* construct_sound_alternate_physical_pair_00a77d00(void* p) noexcept {
    write<void*>(p, 0, nullptr); write<void*>(p, 4, nullptr); return p;
}
void destroy_sound_alternate_physical_pair_00a77d10(void* p, SoundAlternateOwnerBindings& b) {
    clear_reference(p, b); clear_reference(at(p, 4), b);
}
void* construct_sound_alternate_logical_00a78150(void* p, std::uint32_t category) noexcept {
    initialize_logical(p, category);
    write<std::uint32_t>(p, 0, 0x00d58e60); write<std::uint8_t>(p, 0x58, 1); return p;
}
void* construct_sound_alternate_logical_00a781c0(void* p, std::uint32_t category) noexcept {
    initialize_logical(p, category);
    write<std::uint8_t>(p, 0x58, 0); write<std::uint32_t>(p, 0, 0x00d58e64); return p;
}
void destroy_sound_alternate_configuration_00a77dd0(void* p, SoundAlternateOwnerBindings& b) {
    clear_reference(at(p, 0x18), b); destroy_sound_alternate_record_004c87f0(p, b);
}
void destroy_sound_alternate_logical_00a780b0(void* p, SoundAlternateOwnerBindings& b) {
    write<std::uint32_t>(p, 0, 0x00d58e5c);
    clear_reference(at(p, 4), b);
    destroy_sound_alternate_configuration_00a77dd0(at(p, 0x30), b);
    destroy_sound_alternate_configuration_00a77dd0(at(p, 0x10), b);
    destroy_native_string_header_0041dd20(at(p, 8), b.strings);
}
void destroy_sound_alternate_table_00a791d0(void* p, NativeStringStorage& strings) {
    // A78EC0(size0), restricted to the constructor-produced valid vector domain.
    while (read<std::int32_t>(p, 0xc) > 0) {
        const auto index = read<std::int32_t>(p, 0xc) - 1;
        write(p, 0xc, index);
        destroy_native_string_header_0041dd20(at(read<void*>(p, 8), index * 0x14), strings);
    }
    write<std::int32_t>(p, 0xc, 0);
    singleton_lifetime_free(read<void*>(p, 8));
    write<std::uint32_t>(p, 0, 0x00ceb130); // BD30F0 after returning BF6989.
}
void* scalar_delete_sound_alternate_table_00a796f0(void* p, std::uint8_t flags,
    NativeStringStorage& strings) {
    destroy_sound_alternate_table_00a791d0(p, strings);
    if (flags & 1u) singleton_lifetime_free(p);
    return p;
}
NativeSoundAlternateOwnerStorage& construct_sound_alternate_owner_00a79230(
    NativeSoundAlternateOwnerStorage& owner, SoundAlternateOwnerBindings& b) {
    construct_sound_alternate_base_00a778d0(owner, b);
    write<std::uint32_t>(&owner, 0, 0x00d58f78);
    for (int i = 0; i < 18; ++i)
        construct_sound_alternate_record_004c87d0(at(&owner, 0x10 + i * 0x18));
    for (int i = 0; i < 18; ++i)
        construct_sound_alternate_reference_00a778c0(at(&owner, 0x1c0 + i * 4));
    for (std::size_t n = 0x218; n <= 0x224; n += 4) write(&owner, n, 1.0f);
    write<std::int32_t>(&owner, 0x208, -1); write<std::uint32_t>(&owner, 0x210, 0);
    write<std::uint8_t>(&owner, 0x214, 1); write<std::uint8_t>(&owner, 0x215, 0);
    write(&owner, 0x228, 0.0f); write<void*>(&owner, 0x22c, nullptr);
    write<void*>(&owner, 0x230, nullptr);
    try {
        void* const table = allocate(0x20);
        write<std::uint32_t>(table, 0, 0x00ceb130); write<std::int32_t>(table, 4, 1);
        write<std::uint32_t>(table, 0, 0x00d58f80);
        write<void*>(table, 8, nullptr); write<std::int32_t>(table, 0xc, 0);
        write<std::int32_t>(table, 0x10, 0); write(table, 0x14, 1.0f);
        write<std::int32_t>(table, 0x18, 0); write<std::uint8_t>(table, 0x1c, 0);
        clear_reference(at(&owner, 0x22c), b);
        write(&owner, 0x22c, table);
        {
            TemporaryName path{{}, b.strings};
            path.value.resize_0041dd40(b.strings, 0x1a, true);
            if (path.value.data()) std::memmove(path.value.data(), "sound/streamed_dialogs.def",
                path.value.length() + 1u);
            b.dependencies.load_stream_table_00a87060(read<void*>(&owner, 0x22c), path.value);
        }
        void* const physical = allocate(0xc);
        write<std::uint32_t>(physical, 0, 1);
        construct_sound_alternate_physical_pair_00a77d00(at(physical, 4));
        write(&owner, 4, at(physical, 4));
        write(&owner, 8, construct_sound_alternate_logical_00a78150(allocate(0x5c), 0));
        write(&owner, 0xc, construct_sound_alternate_logical_00a781c0(allocate(0x5c), 0));
    } catch (...) {
        // Native state3 owns embedded arrays/table/base; its later raw pointer
        // allocations have no destructor state. Do not invent cleanup for them.
        destroy_members(owner, b); throw;
    }
    return owner;
}
void clear_sound_alternate_channels_00a77c10(
    NativeSoundAlternateOwnerStorage& owner, SoundAlternateOwnerBindings& b) {
    for (std::size_t n = 8; n <= 0xc; n += 4) {
        void* logical = read<void*>(&owner, n);
        clear_reference(at(logical, 4), b);
        logical = read<void*>(&owner, n);
        write<std::uint8_t>(logical, 0x4c, 0); write<std::uint8_t>(logical, 0x4d, 0);
        clear_reference(at(logical, 0x20), b); write<void*>(logical, 0x20, nullptr);
        clear_reference(at(logical, 0x28), b); write<void*>(logical, 0x28, nullptr);
    }
    clear_reference(read<void*>(&owner, 4), b);
    write<std::int32_t>(&owner, 0x208, -1);
}
void destroy_sound_alternate_owner_00a78fb0(
    NativeSoundAlternateOwnerStorage& owner, SoundAlternateOwnerBindings& b) {
    write<std::uint32_t>(&owner, 0, 0x00d58f78);
    clear_sound_alternate_channels_00a77c10(owner, b);
    if (void* const pair = read<void*>(&owner, 4)) {
        void* const allocation = static_cast<std::byte*>(pair) - 4;
        const auto count = read<std::uint32_t>(allocation, 0);
        for (auto i = count; i != 0; --i)
            destroy_sound_alternate_physical_pair_00a77d10(at(pair, (i - 1u) * 8u), b);
        singleton_lifetime_free(allocation);
    }
    for (std::size_t n = 8; n <= 0xc; n += 4) {
        if (void* const logical = read<void*>(&owner, n)) {
            destroy_sound_alternate_logical_00a780b0(logical, b);
            singleton_lifetime_free(logical);
        }
    }
    destroy_members(owner, b);
}
NativeSoundAlternateOwnerStorage* scalar_delete_sound_alternate_owner_00a790d0(
    NativeSoundAlternateOwnerStorage* owner, std::uint8_t flags, SoundAlternateOwnerBindings& b) {
    destroy_sound_alternate_owner_00a78fb0(*owner, b);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
void request_sound_stream_fade_00a85c00(void* stream) noexcept {
    if (read<std::uint32_t>(stream, 0x20) - 1u < 2u) {
        write<std::uint8_t>(stream, 0xa, 1); write<std::uint8_t>(stream, 0xb, 0);
    }
}
void request_sound_stream_stop_00a86f40(void* stream, NativeStringStorage& strings) {
    alignas(4) std::byte builder[0x18];
    construct_native_log_builder_00426500(builder, strings);
    try {
        append_native_log_cstring_00bd1a60(builder, "Stream TOSTOP:", strings);
        append_native_log_header_00bd1a20(builder, at(stream, 0x24), strings);
    } catch (...) {
        destroy_native_log_builder_00425f80(builder, strings); throw;
    }
    destroy_native_log_builder_00425f80(builder, strings);
    write<std::uint8_t>(stream, 9, 1);
}
NativeString& copy_sound_stream_name_00a77ff0(void* stream, NativeString& destination,
    NativeStringStorage& strings) {
    write<std::uint32_t>(&destination, 0, 0); write<void*>(&destination, 4, nullptr);
    copy_native_string_header_00be0a30_fragment(&destination, strings, at(stream, 0x24));
    return destination;
}
void update_sound_alternate_owner_00a789c0(
    NativeSoundAlternateOwnerStorage& owner, float dt, SoundAlternateOwnerBindings& b) {
    float step;
    const auto rate = read<float>(&owner, 0x228);
    __asm { fld rate }
    __asm { fmul dt }
    __asm { fstp step }
    write(&owner, 0x220, unit_step_towards_0042ac60(read<float>(&owner, 0x220),
        read<float>(&owner, 0x224), step));
    for (std::size_t n = 8; n <= 0xc; n += 4)
        b.dependencies.update_logical_00a78820(read<void*>(&owner, n), dt, read<float>(&owner, 0x220));
    if (read<std::uint8_t>(&owner, 0x215)) return;
    void* const physical = read<void*>(&owner, 4);
    void* selected = nullptr;
    for (std::size_t n = 8; n <= 0xc; n += 4) {
        void* logical = read<void*>(&owner, n);
        if (read<std::int32_t>(logical, 0x50) != 0) continue;
        if (void* stream = read<void*>(logical, 4)) {
            const auto state = read<std::int32_t>(stream, 0x20);
            if (read<std::uint8_t>(logical, 0x4e) && state != 0 && state != 3)
                request_sound_stream_fade_00a85c00(stream);
            logical = read<void*>(&owner, n);
            stream = read<void*>(logical, 4);
            if (read<std::int32_t>(stream, 0x20) == 3) {
                if (read<void*>(&owner, 0x230)) {
                    TemporaryName name{{}, b.strings};
                    copy_sound_stream_name_00a77ff0(stream, name.value, b.strings);
                    b.dependencies.invoke_callback230_ecx(read<void*>(&owner, 0x230), name.value);
                }
                logical = read<void*>(&owner, n);
                clear_reference(at(logical, 4), b);
            }
        }
        logical = read<void*>(&owner, n);
        if (!read<std::uint8_t>(logical, 0x4e) &&
            (read<void*>(logical, 4) || read<std::uint8_t>(logical, 0x4c))) selected = logical;
    }
    if (!selected) {
        if (void* const stream = read<void*>(physical, 0)) {
            const auto state = read<std::int32_t>(stream, 0x20);
            if (state != 0 && state != 3) {
                inspect_diagnostic_name(stream, b);
                request_sound_stream_stop_00a86f40(read<void*>(physical, 0), b.strings);
            } else {
                release(stream, b); write<void*>(physical, 0, nullptr);
            }
        }
        return;
    }
    if (!read<std::uint8_t>(selected, 0x4c) &&
        (!read<void*>(selected, 4) || read<void*>(selected, 4) == read<void*>(physical, 0))) return;
    if (void* const stream = read<void*>(physical, 0)) {
        inspect_diagnostic_name(stream, b);
        request_sound_stream_fade_00a85c00(read<void*>(physical, 0));
        if (void* const current = read<void*>(physical, 0)) {
            const auto state = read<std::int32_t>(current, 0x20);
            if (state != 0 && state != 3) return;
        }
    }
    // 004254B0("Copying logical ch %d to physical ch %d", selectedIndex,0)
    // is a RET-only diagnostic; ADD ESP,0Ch proves all three stack arguments.
    b.dependencies.start_logical_00a783f0(selected);
    void* const old = read<void*>(physical, 0);
    void* const incoming = read<void*>(selected, 4);
    if (old != incoming) {
        write(physical, 0, incoming);
        if (incoming) InterlockedIncrement(reinterpret_cast<volatile LONG*>(at(incoming, 4)));
        if (old) release(old, b);
    }
}
void SoundAlternateOwnerShutdownRuntime::delete_alternate_slot00(void* p, std::uint32_t flags) {
    if (read<std::uint32_t>(p, 0) != 0x00d58f78)
        throw std::invalid_argument("Unsupported F8BBCC vtable for streamed-dialog destruction");
    scalar_delete_sound_alternate_owner_00a790d0(
        static_cast<NativeSoundAlternateOwnerStorage*>(p), static_cast<std::uint8_t>(flags), bindings_);
}
} // namespace bsp
