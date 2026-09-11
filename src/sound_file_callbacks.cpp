#include "bsp/sound_file_callbacks.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/physical_file.hpp"

#include <atomic>
#include <cstddef>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Sound file callbacks require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);
std::atomic<NativeSoundFileContext*> current_context{};

template<class Function>
Function stream_slot(void* owner, std::size_t byte_offset) noexcept {
    auto* const table = *static_cast<const std::uintptr_t* const*>(owner);
    return reinterpret_cast<Function>(table[byte_offset / 4]);
}

struct StringCleanup {
    NativeString& value;
    NativeStringStorage& storage;
    ~StringCleanup() noexcept { value.release_to(storage); }
};

// Host-only partial callable stream shape. Unused slots are explicitly outside
// its domain; no no-op implementations stand in for unrecovered virtuals.
struct Adapter {
    const std::uintptr_t* table;
    volatile LONG references = 1;
    std::shared_ptr<MemoryStream> memory;
    std::shared_ptr<PhysicalFile> physical;
    SoundFileStreamAdapterStatus status;
};
static_assert(offsetof(Adapter, references) == 4);

void __fastcall adapter_release(Adapter* owner, void*) {
    delete owner;
}
void __fastcall adapter_seek(Adapter* owner, void*, std::uint32_t low,
    std::uint32_t high, std::uint32_t origin) noexcept {
    const auto distance = static_cast<std::int64_t>(
        (static_cast<std::uint64_t>(high) << 32) | low);
    DWORD error = ERROR_SUCCESS;
    const bool ok = owner->physical
        ? owner->physical->seek_00bf4f20(distance, origin, error)
        : owner->memory->seek_00bef540(distance, origin);
    owner->status = {ok, error};
}
void __fastcall adapter_read(Adapter* owner, void*, void* destination,
    std::uint32_t requested, std::uint32_t* actual) noexcept {
    DWORD error = ERROR_SUCCESS;
    const bool ok = owner->physical
        ? owner->physical->read_00bf5030(destination, requested, *actual, error)
        : owner->memory->read_00bef590(destination, requested, actual);
    owner->status = {ok, error};
}
std::uint64_t __fastcall adapter_size64(Adapter* owner, void*) noexcept {
    return owner->physical ? owner->physical->size_00bf4f90()
        : static_cast<std::uint64_t>(owner->memory->size_00bef600());
}
std::uint32_t __fastcall adapter_size_low(Adapter* owner, void*,
    std::uint32_t* high) noexcept {
    return sound_stream_size_low_00be41a0(owner, high);
}
const std::uintptr_t adapter_table[] = {
    reinterpret_cast<std::uintptr_t>(&adapter_release), // +00 final reference
    0, 0, 0, 0, 0, 0, // +04..+18 outside this adapter's callable domain
    reinterpret_cast<std::uintptr_t>(&adapter_seek), // +1C
    0, // +20 outside domain
    reinterpret_cast<std::uintptr_t>(&adapter_read), // +24
    0, // +28 outside domain
    reinterpret_cast<std::uintptr_t>(&adapter_size_low), // +2C
    reinterpret_cast<std::uintptr_t>(&adapter_size64) // +30
};
} // namespace

NativeSoundFileContext* bind_sound_file_context(NativeSoundFileContext* context) noexcept {
    return current_context.exchange(context);
}

std::int32_t __stdcall sound_file_open_00a7d410(const char* name, std::int32_t,
    std::uint32_t* size, void** handle, void**) {
    auto& context = *current_context.load();
    NativeString resolved;
    StringCleanup resolved_cleanup{resolved, context.strings};
    resolved.assign_0041e870(context.strings, name);
    context.resolve(context.user, resolved); // AL deliberately ignored.
    {
        NativeString copy;
        StringCleanup copy_cleanup{copy, context.strings};
        copy.copy_from_00be0a30_fragment(context.strings, resolved);
        *handle = context.open_read_only(context.user, copy, 2);
    } // Native copied-name storage is released before testing the handle.
    if (!*handle) return 0x17;
    using Size = std::uint32_t (__thiscall*)(void*, std::uint32_t*);
    *size = stream_slot<Size>(*handle, 0x2c)(*handle, nullptr);
    return 0;
}

std::int32_t __stdcall sound_file_close_00a7b750(void* handle, void*) {
    if (handle && InterlockedDecrement(reinterpret_cast<volatile LONG*>(
        static_cast<unsigned char*>(handle) + 4)) == 0) {
        using Release = void (__thiscall*)(void*);
        stream_slot<Release>(handle, 0)(handle);
    }
    return 0;
}

std::int32_t __stdcall sound_file_read_00a79930(void* handle, void* destination,
    std::uint32_t requested, std::uint32_t* output, void*) {
    // Native reuses the incoming stack handle slot as the actual-count local.
    auto actual = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(handle));
    using Read = void (__thiscall*)(void*, void*, std::uint32_t, std::uint32_t*);
    stream_slot<Read>(handle, 0x24)(handle, destination, requested, &actual);
    if (output) *output = actual;
    return actual < requested ? 0x16 : 0;
}

std::int32_t __stdcall sound_file_seek_00a79970(void* handle,
    std::uint32_t position, void*) {
    using Seek = void (__thiscall*)(void*, std::uint32_t, std::uint32_t, std::uint32_t);
    stream_slot<Seek>(handle, 0x1c)(handle, position, 0, 0);
    return 0;
}

std::uint32_t sound_stream_size_low_00be41a0(void* handle, std::uint32_t* high) {
    using Size = std::uint64_t (__thiscall*)(void*);
    const auto size = stream_slot<Size>(handle, 0x30)(handle);
    if (high) *high = static_cast<std::uint32_t>(size >> 32);
    return static_cast<std::uint32_t>(size);
}

void* create_sound_memory_stream_adapter(std::shared_ptr<MemoryStream> stream) {
    if (!stream || !stream->has_backing()) return nullptr;
    return new Adapter{adapter_table, 1, std::move(stream), {}, {}};
}
void* create_sound_physical_stream_adapter(std::shared_ptr<PhysicalFile> stream) {
    if (!stream || !stream->valid_00bf5020()) return nullptr;
    return new Adapter{adapter_table, 1, {}, std::move(stream), {}};
}
SoundFileStreamAdapterStatus sound_file_stream_adapter_status(void* handle) noexcept {
    return static_cast<Adapter*>(handle)->status;
}
} // namespace bsp
