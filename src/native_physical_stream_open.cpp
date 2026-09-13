#include "bsp/native_physical_stream_open.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_render_batch_lifetime.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical streams require MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
const void* at(const void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
std::uint32_t word(const void* p, std::uint32_t n = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(p, n));
}
void put(void* p, std::uint32_t n, std::uint32_t v) noexcept {
    *static_cast<volatile std::uint32_t*>(at(p, n)) = v;
}
void* pointer(const void* p, std::uint32_t n = 0) noexcept {
    return reinterpret_cast<void*>(word(p, n));
}
std::uint32_t bits(const void* p) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}
bool signed_less(std::uint32_t a, std::uint32_t b) noexcept {
    return static_cast<std::int32_t>(a) < static_cast<std::int32_t>(b);
}
const char* cstring(const void* header) noexcept {
    const auto* data = static_cast<const char*>(pointer(header, 4));
    return data ? data : ""; // Native zero byte0109DC40.
}
void require_slot(void* owner, std::uint32_t identity, std::uint32_t offset,
    std::uint32_t expected) {
    const auto table = word(owner);
    if (table != identity || word(reinterpret_cast<const void*>(table), offset) != expected)
        throw std::invalid_argument("Unimplemented current native physical stream/provider slot");
}
void enter(TrackedCriticalSection* section) {
    if (section) {
        EnterCriticalSection(&section->native);
        put(section, 0x18, word(section, 0x18) + 1u);
    }
}
void leave(TrackedCriticalSection* section) noexcept {
    if (section) {
        put(section, 0x18, word(section, 0x18) - 1u);
        LeaveCriticalSection(&section->native);
    }
}
void diagnostic_name(const void* name) noexcept {
    // 4254B0 is one RET. Preserve the conditional current pointer load;
    // introduce neither formatting nor an invented logging owner.
    (void)word(name, 4);
}
using Failure = void (__fastcall*)(void*, void*);
} // namespace

void* construct_native_physical_stream_00bf50d0(void* stream) noexcept {
    put(stream, 0, 0x00ceb130);
    put(stream, 4, 1);
    put(stream, 0, 0x00d691b0);
    put(stream, 8, 0xffffffffu);
    put(stream, 0x10, 0);
    put(stream, 0x14, 0);
    put(stream, 0x18, 0);
    put(stream, 0x1c, 0);
    return stream;
}

void* native_physical_stream_pool_00bf42a0(NativePhysicalStreamOpenContext& context) {
    if (auto* current = context.pool_0109dc28) return current;
    // The native EH frame releases the first manager's captured section on
    // normal or abnormal exit. Construct in raw stack storage so __finally
    // owns exactly one release in this source exception domain.
    alignas(CapturedSoundLifetimeSection) unsigned char captured_storage[
        sizeof(CapturedSoundLifetimeSection)];
    auto* captured = ::new (captured_storage) CapturedSoundLifetimeSection(context.lifetime);
    __try {
        if (!context.pool_0109dc28) {
            auto* owner = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x10, 0x10});
            if (owner) {
                put(owner, 4, 0);
                put(owner, 8, 0);
                put(owner, 0xc, 0);
                put(owner, 0, 0x00d68ec0);
            }
            context.pool_0109dc28 = owner;
            auto manager = context.lifetime.get_manager_00415350();
            manager->register_object(context.pool_0109dc28);
        }
    } __finally {
        captured->~CapturedSoundLifetimeSection();
    }
    return context.pool_0109dc28; // After captured lock release.
}

void* acquire_native_physical_stream_00bf3770(void* header,
    NativePhysicalStreamOpenContext& context) {
    auto* section = context.batch_lifetime.lock_owner_00b1cd90()->section_04;
    enter(section);
    void* stream = nullptr;
    void* allocation = nullptr;
    bool free_on_unwind = false;
    __try {
        const auto count = word(header, 4);
        if (count != 0) {
            stream = pointer(pointer(header), count * 4u - 4u);
            put(header, 4, count - 1u);
            // FH3 state1 placement-delete00401130 is a verified RET.
            if (stream) construct_native_physical_stream_00bf50d0(stream);
        } else {
            allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x20, 0x20});
            free_on_unwind = true;
            stream = allocation ? construct_native_physical_stream_00bf50d0(allocation) : nullptr;
        }
    } __finally {
        if (AbnormalTermination() && free_on_unwind) singleton_lifetime_free(allocation);
        leave(section);
    }
    return stream;
}

void reserve_native_physical_stream_slots_00bf30c0(void* header, std::uint32_t requested) {
    if (signed_less(requested, 1)) requested = 1;
    if (!signed_less(word(header, 8), requested)) return;
    const auto bytes = requested * 4u;
    auto* replacement = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    std::uint32_t index = 0;
    auto* destination = replacement;
    while (signed_less(index, word(header, 4))) {
        if (destination) put(destination, 0, word(pointer(header), index * 4u));
        ++index;
        destination = at(destination, 4);
    }
    singleton_lifetime_free(pointer(header)); // BF6989 JMP BF65AC.
    put(header, 0, bits(replacement)); // Native post-free continuation.
    put(header, 8, requested);
}

void resize_native_physical_stream_slots_00bf3670(void* header, std::uint32_t requested) {
    if (signed_less(word(header, 8), requested))
        reserve_native_physical_stream_slots_00bf30c0(header, requested);
    auto index = word(header, 4);
    while (signed_less(index, requested)) {
        auto* destination = at(pointer(header), index * 4u);
        if (destination) put(destination, 0, 0);
        ++index;
    }
    while (signed_less(requested, word(header, 4))) put(header, 4, word(header, 4) - 1u);
    put(header, 4, requested);
}

void destroy_native_physical_stream_slots_00bf3930(void* header) {
    std::uint32_t index = 0;
    while (signed_less(index, word(header, 4))) {
        singleton_lifetime_free(pointer(pointer(header), index * 4u));
        ++index;
    }
    resize_native_physical_stream_slots_00bf3670(header, 0);
    singleton_lifetime_free(pointer(header));
}

void append_native_physical_stream_slot_00bf5190(void* header, void* stream,
    NativePhysicalStreamOpenContext& context) {
    auto* section = context.batch_lifetime.lock_owner_00b1cd90()->section_04;
    enter(section);
    __try {
        const auto capacity = word(header, 8);
        if (word(header, 4) == capacity) {
            auto requested = capacity + capacity;
            if (!signed_less(1, requested)) requested = 1;
            reserve_native_physical_stream_slots_00bf30c0(header, requested);
        }
        const auto count = word(header, 4);
        auto* destination = at(pointer(header), count * 4u);
        if (destination) put(destination, 0, bits(stream));
        put(header, 4, word(header, 4) + 1u);
    } __finally {
        leave(section);
    }
}

void* delete_native_physical_stream_pool_00bf4370(void* pool, std::uint32_t flags,
    NativePhysicalStreamOpenContext& context) {
    context.pool_0109dc28 = nullptr;
    put(pool, 0, 0x00ce3818);
    destroy_native_physical_stream_slots_00bf3930(at(pool, 4));
    if (flags & 1u) singleton_lifetime_free(pool);
    return pool;
}

bool valid_native_physical_stream_00bf5020(const void* stream) noexcept {
    return word(stream, 8) != 0xffffffffu;
}

std::uint64_t size_native_physical_stream_00bf4f90(const void* stream) noexcept {
    const auto low = word(stream, 0x18);
    const auto high = word(stream, 0x1c);
    return static_cast<std::uint64_t>(low) | (static_cast<std::uint64_t>(high) << 32);
}

std::uint32_t query_native_physical_file_size_00bf4fa0(void* stream, std::uint32_t) {
    const auto handle = pointer(stream, 8);
    DWORD high = 0;
    return GetFileSize(handle, &high);
}

std::uint32_t read_native_physical_stream_00bf5030(void* stream, void* destination,
    std::uint32_t requested, std::uint32_t* output, NativePhysicalStreamOpenContext& context) {
    DWORD actual = 0;
    if (!ReadFile(pointer(stream, 8), destination, requested, &actual, nullptr)) {
        auto* manager = context.physical.manager_0109ceec;
        auto* field18 = pointer(manager, 0x18);
        // Complete consumed BD9E30: field90 CALL, RET4 ignores stacked field18.
        auto* target = pointer(manager, 0x90);
        reinterpret_cast<Failure>(target)(manager, field18);
    }
    const auto old = word(stream, 0x10);
    const auto low = old + actual;
    put(stream, 0x10, low);
    put(stream, 0x14, word(stream, 0x14) + static_cast<std::uint32_t>(low < old));
    if (output) *output = actual;
    return actual;
}

void* delete_native_physical_stream_00bf5090(void* stream, std::uint32_t flags) {
    const auto handle = pointer(stream, 8);
    put(stream, 0, 0x00d691b0);
    CloseHandle(handle);
    put(stream, 8, 0xffffffffu);
    put(stream, 0, 0x00d5c104);
    put(stream, 0, 0x00ceb130); // Complete consumed seven-byte BD30F0.
    if (flags & 1u) singleton_lifetime_free(stream);
    return stream;
}

void recycle_native_physical_stream_00bf55a0(void* stream,
    NativePhysicalStreamOpenContext& context) {
    require_slot(stream, 0x00d691b0, 4, 0x00bf5090);
    delete_native_physical_stream_00bf5090(stream, 0);
    auto* pool = native_physical_stream_pool_00bf42a0(context);
    append_native_physical_stream_slot_00bf5190(at(pool, 4), stream, context);
}

void open_native_physical_stream_00bf52a0(void* stream, const void* path,
    std::uint32_t flags, ActualNativeStringPoolStorage& strings) {
    const auto access = (flags & 1u) ? GENERIC_WRITE : GENERIC_READ;
    auto disposition = flags;
    switch (flags & 0xeu) {
    case 0: disposition = OPEN_ALWAYS; break;
    case 2: disposition = OPEN_EXISTING; break;
    case 4: disposition = CREATE_ALWAYS; break;
    case 6: disposition = CREATE_NEW; break;
    case 8: disposition = TRUNCATE_EXISTING; break;
    default: break; // Original flags word survives for mask A/C/E.
    }
    const auto share = ~flags & 1u;
    auto handle = CreateFileA(cstring(path), access, share, nullptr, disposition, 0, nullptr);
    put(stream, 8, bits(handle));
    if (handle == INVALID_HANDLE_VALUE) {
        const auto error = GetLastError();
        diagnostic_name(path);
        if (error == ERROR_PATH_NOT_FOUND && access == GENERIC_WRITE) {
            std::uint32_t search = 3;
            for (;;) {
                const auto* data = static_cast<const char*>(pointer(path, 4));
                if (!data) break;
                auto start = search;
                if (signed_less(search, 0)) start = 0;
                else if (search > word(path)) break;
                const auto* found = std::strstr(static_cast<const char*>(at(data, start)), "\\");
                if (!found) break;
                const auto offset = bits(found) - word(path, 4);
                if (offset == 0xffffffffu) break;
                alignas(4) std::uint32_t prefix[2];
                construct_native_string_substring_00469840(path, prefix, 0, offset, strings);
                search = offset + 1u;
                // No state0 store exists in the BF52A0 body. Do not invent
                // exception cleanup for this temporary on the Win32 calls.
                if (!CreateDirectoryA(cstring(prefix), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
                    diagnostic_name(prefix);
                    destroy_native_string_header_0041dd20(prefix, strings);
                    break;
                }
                destroy_native_string_header_0041dd20(prefix, strings);
            }
            handle = CreateFileA(cstring(path), GENERIC_WRITE, share, nullptr, disposition, 0, nullptr);
            put(stream, 8, bits(handle));
            if (handle == INVALID_HANDLE_VALUE) {
                (void)GetLastError();
                diagnostic_name(path);
            }
        }
    }
    handle = pointer(stream, 8);
    if (handle != INVALID_HANDLE_VALUE)
        (void)GetFileSizeEx(handle, static_cast<LARGE_INTEGER*>(at(stream, 0x18)));
    put(stream, 0x14, 0);
    put(stream, 0x10, 0);
}

void open_native_physical_stream_00bf5590(void* stream, const void* path,
    std::uint32_t flags, ActualNativeStringPoolStorage& strings) {
    open_native_physical_stream_00bf52a0(stream, path, flags, strings);
}

void* open_native_physical_provider_00bf4ba0(void* provider, const void* suffix,
    std::uint32_t flags, NativePhysicalStreamOpenContext& context) {
    auto* pool = native_physical_stream_pool_00bf42a0(context);
    auto* stream = acquire_native_physical_stream_00bf3770(at(pool, 4), context);
    alignas(4) std::uint32_t path[2];
    require_slot(provider, 0x00d69168, 0x1c, 0x00bf3970);
    auto* result = build_native_physical_path_00bf3970(provider, path, suffix, context.physical);
    __try {
        open_native_physical_stream_00bf5590(stream, result, flags, context.physical.strings);
    } __finally {
        // Only the temporary path is armed; the native stream is not protected
        // against exceptions from path construction or open.
        destroy_native_string_header_0041dd20(path, context.physical.strings);
    }
    require_slot(stream, 0x00d691b0, 0x18, 0x00bf5020);
    if (valid_native_physical_stream_00bf5020(stream)) return stream;
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(stream, 4))) == 0) {
        require_slot(stream, 0x00d691b0, 0, 0x00bf55a0);
        recycle_native_physical_stream_00bf55a0(stream, context);
    }
    return nullptr;
}
} // namespace bsp
