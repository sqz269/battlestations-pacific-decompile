#pragma once

#include "bsp/native_mpak_factory.hpp"
#include <cstdint>

namespace bsp {
class NativeStringStorage;
class NativePathCanonicalizerServices;
class NativeAdoptedSubstreamDispatch;
struct NativePakRegistryContext;

// Original STL specialization boundaries. These operations receive the actual
// raw storage; implementations must preserve the original allocator, iterator,
// copy and exception contracts. No successful fallback is supplied here.
// File vectors are 10h headers with begin/end/capacity at +4/+8/+C and 24h
// elements. Directory vectors have the same header and 14h elements. The
// allocator words at header+0 and file-record+14h are not initialized by BB8240.
class NativeMpakContainerLibrary {
public:
    virtual ~NativeMpakContainerLibrary() = default;
    // ECX vector, stack actual record, RET4.
    virtual void append_file_00bb7a20(void* vector, const void* record) = 0;
    virtual void append_directory_00bb7ba0(void* vector, const void* record) = 0;
    // ECX vector; stacked result iterator, where-container, where-pointer,
    // source DWORD; RET10h. The parser ignores the returned iterator.
    virtual void insert_offset_00a40d60(void* vector, void* result_iterator,
        void* where_container, void* where_pointer, const void* value) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    // ECX begin, EDX end, two ignored stack words (vector and owner), RET8.
    virtual void destroy_file_range_00bb6220(void* begin, void* end,
        void* actual_vector, void* actual_owner) = 0;
    // ECX actual vector, RET. BB7580 is a five-byte jump to BB71F0.
    virtual void destroy_file_vector_00bb6e60(void* vector) = 0;
    virtual void destroy_directory_vector_00bb71f0(void* vector) = 0;
};

struct NativeMpakDirectoryContext {
    NativeStringStorage& strings;
    NativeAdoptedSubstreamDispatch& streams;
    NativePathCanonicalizerServices& allocation_and_pool;
    NativeMpakContainerLibrary& containers;
};

// BB5630[52]: ECX actual cursor DWORD, stack output8h, EAX output, RET4.
// Increment cursor before reading the unsigned prefix byte; construct from
// the following NUL-terminated text; then add prefix+1 to CURRENT cursor.
// The prefix is cursor movement, not a bound on the string copy.
void* read_native_mpak_string_00bb5630(void* actual_cursor, void* actual_output,
    NativeStringStorage&);

// BB6870[127]: ECX actual24h record; stack name, first, second, byte flag;
// EAX record, RET10h. Name copy precedes numeric fields. Leave padding11..13
// and allocator14..17 untouched, zero vector18/1C/20. Self-alias zeroes name.
void* construct_native_mpak_file_00bb6870(void* actual_record,
    const void* actual_name, std::uint32_t first, std::uint32_t second,
    std::uint8_t flag, NativeStringStorage&);
void destroy_native_mpak_file_00bb5430(void* actual_record,
    NativeMpakDirectoryContext&);
void destroy_native_mpak_directory_00bb6500(void* actual_record,
    NativeMpakDirectoryContext&);

// Full BB7C50..BB8239[1514], including the recovered 807F and 8221 tails.
// ECX actual44h provider, RET. Read16 bytes, allocate wrapping(header+4 +
// header+8 +4), read once, parse file offsets and named member lists. Marker
// comparisons do not reject mismatches. No bounds, short-read, null-stream,
// offset or overflow validation is added. Scratch is freed only on success.
// Existing actual pooled strings and actual0Ch string vectors are reused;
// the three original STL insertion operations above remain library contracts.
void load_native_mpak_directory_00bb7c50(void* actual_provider,
    NativeMpakDirectoryContext&);

class NativeMpakProviderDispatch {
public:
    virtual ~NativeMpakProviderDispatch() = default;
    virtual void* open_manager_00bb82c8(std::uintptr_t captured_entry,
        void* captured_manager, const void* embedded_name,
        std::uint32_t flags) = 0;
};
struct NativeMpakProviderContext {
    ActualNativeStringPoolStorage& strings;
    NativePakRegistryContext& registry;
    NativeMpakDirectoryContext& directory;
    void* volatile& actual_manager_publication_0109ceec;
    NativeMpakProviderDispatch& dispatch;
};

// BB8240[170]: base/name, D641F8, actual registry ordinal+0C -> provider+18,
// vector words, open CURRENT manager slot4 with embedded name+8/mode2,
// publish stream+14, then the actual parser above. EAX owner, RET4.
void* construct_native_mpak_provider_00bb8240(void* actual_owner,
    const void* original_system_name, NativeMpakProviderContext&);
// BB7920[180]: release captured stream, directory vector, file range/current
// backing, base. No cache clear, stale stream clear or provider allocation free.
void destroy_native_mpak_provider_00bb7920(void* actual_owner,
    NativeMpakProviderContext&);
// BB7B80[30]: destroy, optional flags bit0 free, EAX captured owner, RET4.
void* delete_native_mpak_provider_00bb7b80(void* actual_owner,
    std::uint32_t flags, NativeMpakProviderContext&);

struct NativeMpakCacheContext {
    NativeMpakProviderContext& provider;
    void* volatile& actual_lock_publication_010904e0;
    void* volatile& actual_cached_provider_010904dc;
};
// BB82F0[161]: ECX actual8h name, RET. Enter captured lock; increment depth;
// clear cache for null/empty data, else allocate44h/construct/publish. Reload
// lock for decrement/leave. Do not release old cache or unlock on exception.
void replace_native_mpak_cache_00bb82f0(const void* actual_system_name,
    NativeMpakCacheContext&);

class NativeMpakRuntimeProviderOperations final : public NativeMpakProviderOperations {
public:
    explicit NativeMpakRuntimeProviderOperations(NativeMpakCacheContext&) noexcept;
    void* construct_00bb8240(void*, const void*) override;
    void replace_cached_00bb82f0(const void*) override;
private:
    NativeMpakCacheContext& context_;
};

// New source interfaces, not drop-in Win32/FH3 replacements. All contexts
// borrow existing owners and one actual pool/lifetime domain. Native stack
// spill aliases, short-read indeterminate bytes, simultaneous cleanup failure,
// original STL implementation and in-game reachability remain separate proof.
} // namespace bsp
