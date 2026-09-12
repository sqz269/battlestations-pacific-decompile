#pragma once

#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual MPKG directory storage requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringPoolStorage;

// Required existing source/library bindings, with no owned publication or pool.
// The length call receives the CURRENT numeric slot30h target and captured owner.
// Alloc/free bind the established operator-new/CRT-free lifetime services. Pool
// getter/return remain explicit because native wrapper cleanup getters can throw.
class NativeMpkgDirectoryServices {
public:
    virtual ~NativeMpkgDirectoryServices() = default;
    virtual std::uint64_t source_length(std::uintptr_t captured_target,
        void* captured_stream) = 0;
    virtual void* allocate_00bf55be(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void*) = 0;
    virtual NativeStringPoolStorage* string_pool_00419cc0() = 0;
    virtual void return_string_00bd1510(NativeStringPoolStorage*, void* captured_block,
        std::uint32_t bytes, std::uint32_t one) = 0;
};
struct NativeMpkgDirectoryContext {
    NativeStringStorage& strings;
    NativeAdoptedSubstreamDispatch& streams;
    NativeMpkgDirectoryServices& services;
};

// Complete BB87A0..BB8841[162]: ECX actual34h archive; RET. Current stream+0Ch,
// length low DWORD ->+10h; scan last min(length,FFFFh) bytes, tail offsets T-4..1
// only. Current length determines stored hit+1Ch. No hit leaves+1Ch untouched.
// One unchecked read; no cleanup added around throwing seek/read/allocation.
void find_native_mpkg_end_record_00bb87a0(void* actual_archive,
    NativeMpkgDirectoryContext&);

// Complete BB8850..BB8A8D[574]: THREE stack args reader/header/kind; RET0Ch,
// incoming ECX ignored. Actual reader0Ch: base+0, reported count+4, current+8.
// Actual header34h: writes kind+0, consumes46 bytes for zero kind or30 otherwise.
// Each field advances current BEFORE descending-byte reads. No bounds/signature
// checks; absent local fields and header padding+2Ah/+2Bh remain untouched.
void read_native_mpkg_header_00bb8850(void* actual_reader, void* actual_header,
    std::uint32_t kind);

// Complete BB9090..BB90FE[111]: ECX reader, stacked output/count; EAX output;
// RET8. Zero actual8h output, resize(count,1), fill current output with spaces,
// then copy requested bytes with current source/destination pointer reloads on
// EVERY byte. Advance current reader pointer by captured requested count.
void* read_native_mpkg_name_00bb9090(void* actual_reader, void* actual_output,
    std::uint32_t count, NativeStringStorage&);

// Complete BB95B0..BB96F5[326]: ECX archive, stacked reader, RET4. Construct a
// raw24h entry and append to archive+28h via actual BB9520. No normalization,
// signature checks or duplicate filtering. Entry padding remains untouched.
// Normal entry cleanup uses data captured after resize, but current length;
// unwind uses the current embedded string. Temporary-name cleanup precedes
// projecting current archive prefix/header fields and skipping extra/comment.
void parse_native_mpkg_directory_entry_00bb95b0(void* actual_archive,
    void* actual_reader, NativeMpkgDirectoryContext&);

// Complete BB9700..BB97A2[163]: ECX archive, RET. Seek directly to current+24h;
// allocate wrapping(+10h - +24h), retain stream captured BEFORE allocation but
// load its current read target AFTER allocation. One read writes actual count,
// which is not a parsing guard. Loop reloads entry count+14h after each entry.
// Scratch cleanup arms only after read, and frees CURRENT reader base+0.
void load_native_mpkg_directory_00bb9700(void* actual_archive,
    NativeMpkgDirectoryContext&);

// Explicit source interfaces, not original stack/FH3/SEH replacements. Native
// short reads and unchecked storage remain unchecked; no synthetic initialized
// tail, zero defaults, semantic stream/vector, private pool or rollback added.
// Nested NativeStringStorage release is noexcept. Native spill aliases,
// simultaneous unwind failures and game reachability remain separate evidence.
} // namespace bsp
