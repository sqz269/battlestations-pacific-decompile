#pragma once
#include <cstdint>

#include "bsp/storage_pool.hpp"

// Host projection of the game's native string helper layer, recovered from
// BSP_NativeString_Resize 0041dd40, BSP_NativeString_Assign 0041e870,
// BSP_NativeString_Duplicate 00438e40 and the copy fragment inside
// BSP_FileBlock_Construct 00be0a30.
//
// The native object is eight bytes and carries no allocator, no capacity and no
// small-buffer area: dword 0 is the length, dword 4 is the buffer. Storage is
// therefore passed explicitly into every operation instead of being cached in
// the object or read from a process global. Evidence and native ABI:
// docs/APP_INIT_ALLOC_STRINGS.md.
namespace bsp {

// The allocation contract 0041dd40 depends on. Every release repeats the size
// that was allocated, because the native pool picks its free list from that
// number. The native large-block malloc can return null; these native string
// bodies do not translate that failure into an exception or a fallback buffer.
// Host allocators may have a stronger throwing contract. See also the actual
// pool bridge in native_string_pool_storage.hpp and its exception-domain limit.
class NativeStringStorage {
public:
    virtual ~NativeStringStorage() = default;
    virtual char* allocate(std::uint32_t size) = 0;
    virtual void release(char* block, std::uint32_t size) noexcept = 0;
};

// Semantic SizedStoragePool projection; not the actual 00419cc0 owner layout.
class PooledStringStorage final : public NativeStringStorage {
public:
    explicit PooledStringStorage(SizedStoragePool& pool) noexcept : pool_(&pool) {}
    char* allocate(std::uint32_t size) override;
    void release(char* block, std::uint32_t size) noexcept override;

private:
    SizedStoragePool* pool_;
};

// std::malloc / std::free for hosts that do not stand a pool up.
NativeStringStorage& crt_string_storage() noexcept;

// Full 0041DD20; ECX actual8h header, RET. Capture its nonnull data pointer,
// then length+1 (DWORD wrap), and release through supplied storage. Leave the
// header untouched, including changes performed during release. Null data
// skips the length read and storage call. No implicit header destruction.
void destroy_native_string_header_0041dd20(void* actual_header,
    NativeStringStorage& storage) noexcept;

// Full 004BCC00; ECX actual8h header, RET. Capture data once; walk bytes by the
// current unsigned length, reread after every store. Convert only ASCII A..Z,
// including after embedded NUL, and still store unchanged bytes. No allocation.
void lowercase_native_string_header_004bcc00(void* actual_header) noexcept;

// Full 0041dd40 against an existing native header: uint32 length at +0 and
// char* data at +4, in the Win32 build. The header is neither initialized nor
// copied into a temporary owner. Allocation/release can observe or change its
// fields; later reads use those current fields. Equal length returns without
// touching the pointer, including a stale or null one. No null-header guard.
// NativeStringStorage remains the explicit host boundary for 00419cc0 and the
// sized pool calls. The original __thiscall/RET 8 ABI is not this C++ interface.
// A zero-byte native memcpy is omitted, as in the existing NativeString API.
// Evidence and the remaining boundaries: docs/NATIVE_POOLED_STRING_ACTUAL_RESIZE.md.
void resize_native_string_header_0041dd40(void* actual_header,
    NativeStringStorage& storage, std::uint32_t length, bool preserve);

// Existing BE0A30 copy fragment against actual8h headers, without starting a
// NativeString object or copying a source header. Self-copy returns; otherwise
// resize from source length, then reload source length/data and destination
// length/data AFTER allocation callbacks. A zero-byte memcpy is omitted.
void copy_native_string_header_00be0a30_fragment(void* actual_destination,
    NativeStringStorage&, const void* actual_source);

// Exact layout of the native eight-byte string. This host wrapper has no
// implicit destructor cleanup. Its owner calls release_to, resize(0), or the
// actual-header destruction body before discarding owned storage.
class NativeString {
public:
    NativeString() noexcept = default;
    NativeString(const NativeString&) = delete;
    NativeString& operator=(const NativeString&) = delete;
    NativeString(NativeString&& other) noexcept : length_(other.length_), data_(other.data_) {
        other.length_ = 0;
        other.data_ = nullptr;
    }

    // 0041dd40, __thiscall, RET 8. A request for the length the string already
    // has does nothing at all, even when the buffer is null; that quirk is
    // native. Length zero releases the buffer and clears both fields. Any other
    // length allocates length + 1, optionally copies min(old, new) bytes, then
    // releases the old buffer with old_length + 1 and writes the terminator.
    // Delegates to resize_native_string_header_0041dd40 on this actual object.
    void resize_0041dd40(NativeStringStorage& storage, std::uint32_t length, bool preserve);

    // 0041e870, __thiscall, RET 4, returns this. This is a constructor body,
    // not an assignment: it zeroes both fields before resizing, so calling it
    // on a live string leaks that string's buffer. text must not be null.
    NativeString& assign_0041e870(NativeStringStorage& storage, const char* text);

    // The copy fragment at 00be0a82 inside BSP_FileBlock_Construct: guard
    // against self-copy, resize to the source length with preserve set, then
    // copy exactly length bytes over the terminator resize already wrote.
    void copy_from_00be0a30_fragment(NativeStringStorage& storage, const NativeString& source);

    // resize to zero: the release path the owners use.
    void release_to(NativeStringStorage& storage) noexcept;

    std::uint32_t length() const noexcept { return length_; }
    // Null for a string that has never held a non-empty value; the native
    // callers check this pointer rather than the length.
    const char* data() const noexcept { return data_; }
    char* data() noexcept { return data_; }
    // Size the buffer occupies in the pool, and the number release has to
    // repeat. There is no separate capacity: it is always length + 1.
    std::uint32_t block_size() const noexcept { return length_ + 1u; }

private:
    std::uint32_t length_{};
    char* data_{};
};

// 00438e40, __fastcall in ECX, RET 0. Returns null for a null argument,
// otherwise a std::malloc copy of strlen + 1 bytes. This one does not touch the
// pool: the native allocates through 00bf681b and its callers release the
// result with _free 00bf9dc8.
char* duplicate_00438e40(const char* text);
void release_duplicate_00438e40(char* copy) noexcept;

}
