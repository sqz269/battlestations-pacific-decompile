#pragma once
#include "bsp/native_shader_compiler_lookups.hpp"
#include "bsp/native_resource_stream_reads.hpp"

namespace bsp {
class NativeVfsRuntimeBindings;
using NativeShaderCacheDecrement = long (__stdcall*)(volatile long*);
// Borrow the SAME raw string/VFS domain and source-mode cells used by the
// material compiler. No private cache, provider, allocator or stream is made.
// Allocation callbacks preserve the existing application's malloc/free domain;
// original static CRT/new-handler and FH3 identities remain explicit boundaries.
struct NativeShaderBinaryCacheContext {
    NativeResourceStreamReadContext& reads;
    NativeVfsRuntimeBindings& vfs;
    void* volatile& actual_vfs_0109ceec;
    const volatile std::uint8_t& load_variants_0108d6f0;
    const volatile std::uint8_t& source_mode_0108d6f1;
    NativeShaderBinaryCacheStorage* volatile& actual_cache_0108d6ec;
    const char* actual_filename_00d60d30;
    const volatile std::uint32_t* actual_physical_profile_00d691b0;
    NativeShaderCacheDecrement const volatile& decrement_00ce2220;
    void* (*allocate_object_00bf681b)(std::uint32_t);
    void* (*allocate_array_00bf55be)(std::uint32_t);
    void (*free_array_00bf6989)(void*);
    void (*free_object_00bf65ac)(void*);
};
// Persistent normal-path invocation state, retained BEFORE side effects.
// Failure does not roll back native resources or permit replay. The caller
// must retain a failed frame and its context; original FH3 is not reproduced.
struct NativeShaderBinaryCacheOperation final {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{}, index{}, completed_records{};
    std::uint32_t allocation_bytes{}, constructed_records{};
    NativeShaderBinaryCacheStorage* cache{};
    void* allocation{};
    NativeString temporary;
    bool temporary_live{};
    NativeShaderBinaryCacheOperation()=default;
    ~NativeShaderBinaryCacheOperation();
    NativeShaderBinaryCacheOperation(const NativeShaderBinaryCacheOperation&)=delete;
    NativeShaderBinaryCacheOperation& operator=(const NativeShaderBinaryCacheOperation&)=delete;
};
// B34C70: ECX actual10h row; EAX same row, RET. Clear00/04/08 ONLY.
NativeShaderBinaryCacheRecordStorage* initialize_native_shader_cache_record_00b34c70(void*) noexcept;
// B34C80: ECX row, RET. Free captured blob, clear CURRENT08, then release
// current name through419CC0/BD1510; name header and byte_count remain stale.
void destroy_native_shader_cache_record_00b34c80(NativeShaderBinaryCacheRecordStorage&,
    NativeShaderBinaryCacheContext&);
// BE45F0: ECX stream, stacked output/actual, RET8; call CURRENT48 and return
// captured output, ignoring callee EAX. Numeric BE4620 is the supported entry.
void* read_native_shader_cache_string_00be45f0(void*,void*,std::uint32_t*,
    NativeResourceStreamReadContext&);
// B35340: ECX cache, RET. Gate BEFORE dereferencing owner; cursor0, count,
// overflow-saturated count*10h+4 cookie allocation, prefix construction, then
// current-stream slot60 name/slot38 size/slot24 blob reads in unsigned order.
void load_native_shader_cache_records_00b35340(NativeShaderBinaryCacheStorage*,
    NativeShaderBinaryCacheContext&,NativeShaderBinaryCacheOperation&);
// B38A70: ECX raw10h, EAX same, RET. Leave cursor preimage; zero04/08/0C.
// source-mode skips I/O; variants selects flags5 and writes zero header;
// otherwise flags2 and full record loader. Re-sample variants after name free.
NativeShaderBinaryCacheStorage* construct_native_shader_binary_cache_00b38a70(void*,
    NativeShaderBinaryCacheContext&,NativeShaderBinaryCacheOperation&);
// B352B0: ECX cache, RET; source-mode gates ALL cleanup. Variants seeks and
// writes count; reverse cookie destruction/free, then CURRENT stream release.
void destroy_native_shader_binary_cache_00b352b0(NativeShaderBinaryCacheStorage*,
    NativeShaderBinaryCacheContext&);
// B3A600/B3B140: no consumed input, RET. Publish only after construction;
// destroy captured publication, free it, then clear CURRENT publication.
void create_native_shader_binary_cache_00b3a600(NativeShaderBinaryCacheContext&,
    NativeShaderBinaryCacheOperation&);
void release_native_shader_binary_cache_00b3b140(NativeShaderBinaryCacheContext&);
} // namespace bsp
