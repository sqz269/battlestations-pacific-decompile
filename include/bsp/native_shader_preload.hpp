#pragma once
#include "bsp/native_lua_bootstrap.hpp"
#include "bsp/native_lua_file_loading.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_shader_binary_cache.hpp"
#include <memory>

namespace bsp {
// Actual 0Ch pointer vector at application+08; count/capacity are signed.
// Entries own the material loader's returned caller references, including null.
struct NativeApplicationPointerVectorStorage {
    void** data_00;
    std::int32_t count_04,capacity_08;
};
static_assert(sizeof(NativeApplicationPointerVectorStorage)==12);
struct NativeApplicationPointerVectorAllocation {
    void* (*allocate_00bf55be)(std::uint32_t);
    void (*free_00bf6989)(void*);
};
// 735EC0: ECX actual0Ch header; stack signed request; RET4. Minimum1,
// signed capacity comparison, wrapping request*4 allocation, live source/count
// reads, free CURRENT old data, THEN publish new data/capacity. Count unchanged.
// Valid readable extents and native allocator/free contracts are required.
// No rollback or original CRT/new-handler/FH3 identity is added.
void reserve_native_application_pointer_vector_00735ec0(
    NativeApplicationPointerVectorStorage&,std::int32_t,
    const NativeApplicationPointerVectorAllocation&);

// All providers borrow the SAME application string/VFS/cache/renderer and
// material owner domains. Install the existing Lua service activation before
// entry. The material context is the real B318B0 provider, never a preload stub.
struct NativeShaderPreloadContext {
    NativeStringStorage& strings;
    const NativeLuaBootstrapInputs& bootstrap;
    const NativeLuaFileServices& files;
    NativeShaderBinaryCacheContext& cache;
    NativeMaterialEffectCacheContext& materials;
    void* const volatile& actual_renderer_00f8d394;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
};
enum class NativeShaderPreloadPhase {
    fresh,cache,bootstrap,script,globals,iteration,material,append,
    cache_release,cleanup,complete,failed
};
// Own before entry and retain on failure, together with application/context.
// Tracks actual4C8h Lua/14h objects, stable8h name, cache and per-load frames.
// A completed frame owns metadata only: vector entries belong to application.
// No automatic native rollback, result release, vector cleanup or retry.
class NativeShaderPreloadOperation final {
public:
    NativeShaderPreloadOperation();
    ~NativeShaderPreloadOperation();
    NativeShaderPreloadOperation(const NativeShaderPreloadOperation&)=delete;
    NativeShaderPreloadOperation& operator=(const NativeShaderPreloadOperation&)=delete;
    NativeShaderPreloadPhase phase() const noexcept;
    std::uint32_t active_call_site() const noexcept;
    std::uint32_t appended_entries() const noexcept;
    bool retains_native_state() const noexcept;
    NativeLuaStateStorage* retained_lua_state() noexcept;
    void* unpublished_material_result() const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void preload_native_application_shaders_0073bf80(
        void*,NativeShaderPreloadContext&,NativeShaderPreloadOperation&);
};
// Full normal 73BF80: ECX actual application, RET. Cache create, Lua mask1,
// actual shaderfx/shaderpreload.lua, FileNames native iteration (all keys),
// current renderer+48 per value, append owned/null results to application+08.
// Cache release precedes reverse value/key/table/Lua cleanup. Original4254B0
// bracket diagnostics are RET-only. Existing vector entries are preserved.
// Source normal-path ABI; original private frame/SEH and game parity unproved.
void preload_native_application_shaders_0073bf80(
    void* actual_application,NativeShaderPreloadContext&,NativeShaderPreloadOperation&);
} // namespace bsp
