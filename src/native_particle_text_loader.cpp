#include "bsp/native_particle_text_loader.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle text loading requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(const void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(base) + offset);
}
volatile Word& word(const void* base, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(base, offset));
}
void* pointer(const void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
} // namespace

struct NativeParticleTextBufferReadAccess final {
    static void begin(NativeParticleTextBufferReadFrame& f, void* text, const char* name) {
        if (f.phase_ != NativeParticleTextReadPhase::fresh)
            throw std::logic_error("actual AF5850 read frame cannot replay");
        f.phase_ = NativeParticleTextReadPhase::entered;
        f.text_ = text;
        f.actual_count_home_ = reinterpret_cast<Word>(name);
    }
    static Word& count_home(NativeParticleTextBufferReadFrame& f) noexcept { return f.actual_count_home_; }
    static void allocation_enter(NativeParticleTextBufferReadFrame& f, void* stream, Word extent) noexcept {
        f.stream_ = stream; f.allocation_bytes_ = extent; f.allocation_entered_ = true;
    }
    static void allocation_return(NativeParticleTextBufferReadFrame& f, void* p) noexcept {
        f.allocation_ = p; f.allocation_returned_ = true;
    }
    static void read_enter(NativeParticleTextBufferReadFrame& f, Word requested) noexcept {
        f.read_requested_ = requested; f.read_entered_ = true;
    }
    static void read_return(NativeParticleTextBufferReadFrame& f, Word count) noexcept {
        f.read_count_at_return_ = count; f.read_returned_ = true;
    }
    static void free_enter(NativeParticleTextBufferReadFrame& f, void* p) noexcept {
        f.mismatch_free_pointer_ = p; f.mismatch_free_entered_ = true;
    }
    static void free_return(NativeParticleTextBufferReadFrame& f) noexcept { f.mismatch_free_returned_ = true; }
    static void complete(NativeParticleTextBufferReadFrame& f, Word result) noexcept {
        f.returned_count_ = result; f.phase_ = NativeParticleTextReadPhase::complete;
    }
    static void fail(NativeParticleTextBufferReadFrame& f) noexcept {
        f.phase_ = NativeParticleTextReadPhase::failed; f.failure_ = std::current_exception();
    }
};
namespace {
std::uint32_t load_text_buffer_body(
    void* text, const char* name, void* volatile& actual_vfs_publication_0109ceec,
    NativeVfsRuntimeBindings& vfs, NativeVfsNameResolutionContext& name_resolution,
    NativeVfsNameResolutionAcquired& invocation, NativeStringRawPoolContext& strings,
    Word& actual_count, NativeParticleTextBufferReadFrame* record) {
    // Native reuses its incoming name argument word at [ESP+10] for the
    // output-count pointer. Retain its input bits until a real read replaces it.
    void* const name_header = at(text, 0x0c);
    Word length = 0;
    if (name) {
        while (name[length] != '\0') ++length;
    }
    resize_native_string_header_0041dd40(name_header, strings, length, false);
    void* const current_name_bytes = pointer(name_header, 4);
    if (current_name_bytes) {
        const Word current_length = word(name_header);
        // BF7680's overlapping-range branch is part of the native primitive.
        if (current_length != 0) std::memmove(current_name_bytes, name, current_length);
    }

    void* const resolve_manager = actual_vfs_publication_0109ceec;
    (void)resolve_native_vfs_existing_name_00bdf4c0(
        resolve_manager, name_header, name_resolution, invocation);
    void* const open_manager = actual_vfs_publication_0109ceec;
    const Word open_target = word(pointer(open_manager), 4);
    void* const stream = vfs.open_manager_entry(open_target, open_manager, name_header, 0x32);

    const Word length_target = word(pointer(stream), 0x30);
    const Word extent = static_cast<Word>(vfs.stream_length_entry(length_target, stream));
    word(text) = extent;
    word(text, 8) = extent;
    word(text, 4) = 0;
    word(text, 0x18) = 0;
    // BF55BE is only JMP BF681B. The existing CRT allocation provider owns
    // this exact byte extent; no array header, terminator or zero fill is added.
    if (record) NativeParticleTextBufferReadAccess::allocation_enter(*record, stream, extent);
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, extent, extent});
    if (record) NativeParticleTextBufferReadAccess::allocation_return(*record, allocation);
    const Word requested = word(text);
    word(text, 0x14) = reinterpret_cast<Word>(allocation);
    const Word read_target = word(pointer(stream), 0x24);
    if (record) NativeParticleTextBufferReadAccess::read_enter(*record, requested);
    vfs.source_read(read_target, stream, allocation, requested, &actual_count);
    const Word compared_count = actual_count; // Same reached count read used by comparison.
    if (record) NativeParticleTextBufferReadAccess::read_return(*record, compared_count);
    if (compared_count != word(text)) {
        void* const current_free = pointer(text, 0x14); // Same single CURRENT load.
        if (record) NativeParticleTextBufferReadAccess::free_enter(*record, current_free);
        singleton_lifetime_free(current_free);
        if (record) NativeParticleTextBufferReadAccess::free_return(*record);
        word(text, 0x14) = 0;
        return 0;
    }

    if (InterlockedDecrement(static_cast<volatile LONG*>(at(stream, 4))) == 0) {
        const Word table = word(stream);
        const Word terminal = word(reinterpret_cast<void*>(table));
        vfs.source_zero_reference(terminal, stream, table);
    }
    return actual_count;
}
} // namespace

std::uint32_t load_native_particle_text_buffer_00af5850(
    void* text, const char* name, void* volatile& publication,
    NativeVfsRuntimeBindings& vfs, NativeVfsNameResolutionContext& resolution,
    NativeVfsNameResolutionAcquired& invocation, NativeStringRawPoolContext& strings) {
    Word actual_count = reinterpret_cast<Word>(name);
    return load_text_buffer_body(text, name, publication, vfs, resolution, invocation,
        strings, actual_count, nullptr);
}
std::uint32_t load_native_particle_text_buffer_00af5850(
    void* text, const char* name, void* volatile& publication,
    NativeVfsRuntimeBindings& vfs, NativeVfsNameResolutionContext& resolution,
    NativeVfsNameResolutionAcquired& invocation, NativeStringRawPoolContext& strings,
    NativeParticleTextBufferReadFrame& record) {
    NativeParticleTextBufferReadAccess::begin(record, text, name);
    try {
        const Word result = load_text_buffer_body(text, name, publication, vfs, resolution,
            invocation, strings, NativeParticleTextBufferReadAccess::count_home(record), &record);
        NativeParticleTextBufferReadAccess::complete(record, result);
        return result;
    } catch (...) {
        NativeParticleTextBufferReadAccess::fail(record);
        throw;
    }
}
} // namespace bsp
