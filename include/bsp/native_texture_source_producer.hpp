#pragma once

#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_texture_source_storage.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace bsp {

// Borrow SAME raw string-pool cells/domain as strings, Lua and texture services.
// lua is already active in a retained caller scope throughout synchronous work.
// The table/literals are genuine readable original data, not callable pointers.
struct NativeTextureSourceProducerContext {
    ActualNativeStringPoolStorage& strings;
    NativeStringRawPoolContext& raw_strings;
    NativeLuaServiceBindings& lua;
    NativeTextureCacheContext& textures;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    const char* actual_script_00d79b98;
    const char* actual_animated_00d79b88;
    const char* actual_fps_00d5d1f8;
    const char* actual_textures_00d0d9b0;
};

// Observation only: no extra resource credit or automatic result retirement.
struct NativeTextureSourceChildAcquired final {
    NativeTextureCacheAcquired cache;
    void* captured_renderer{};
    std::uint32_t captured_profile{}, captured_target{};
    bool target_captured{}, call_started{}, call_returned{};
    void* result{};
    bool name_return_started{}, name_return_completed{};
    bool append_started{}, append_completed{};
};

// Retain this nonmovable frame before entry and through any interruption.
// Raw cells are uninitialized byte backing, not preconstructed/reset Lua views.
// Genuine reached constructors/getters return each actual live-object pointer.
// scratch is ONE caller-live actual8B NativeString, shared by native script
// and child-name phases (original frame+10). No unreconciled allocation; raw
// native writes own initialization at each reached constructor/zero point.
// Scratch is distinct from requested-name/source/Lua cells and remains live
// through callbacks; no surviving borrower may use a completed scratch phase.
struct NativeTextureSourceProducerAcquired final {
    enum class Phase { fresh, in_progress_or_unclassified, complete };
    explicit NativeTextureSourceProducerAcquired(NativeString& scratch) noexcept;
    ~NativeTextureSourceProducerAcquired();
    NativeTextureSourceProducerAcquired(const NativeTextureSourceProducerAcquired&) = delete;
    NativeTextureSourceProducerAcquired& operator=(const NativeTextureSourceProducerAcquired&) = delete;
    NativeTextureSourceProducerAcquired(NativeTextureSourceProducerAcquired&&) = delete;
    NativeTextureSourceProducerAcquired& operator=(NativeTextureSourceProducerAcquired&&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1}; // Native map observation, not executed FH3.
    bool body_entered{}, caught_cpp_exception{};
    NativeString& scratch;
    alignas(NativeLuaStateStorage) std::byte owner_cell[sizeof(NativeLuaStateStorage)];
    alignas(NativeLuaObjectStorage) std::byte globals_cell[sizeof(NativeLuaObjectStorage)];
    alignas(NativeLuaObjectStorage) std::byte animated_cell[sizeof(NativeLuaObjectStorage)];
    alignas(NativeLuaObjectStorage) std::byte selected_cell[sizeof(NativeLuaObjectStorage)];
    alignas(NativeLuaObjectStorage) std::byte fps_cell[sizeof(NativeLuaObjectStorage)];
    alignas(NativeLuaObjectStorage) std::byte textures_cell[sizeof(NativeLuaObjectStorage)];
    alignas(NativeLuaObjectStorage) std::byte key_cell[sizeof(NativeLuaObjectStorage)];
    alignas(NativeLuaObjectStorage) std::byte value_cell[sizeof(NativeLuaObjectStorage)];
    NativeLuaStateStorage* owner{};
    NativeLuaObjectStorage *globals{}, *animated{}, *selected{}, *fps{}, *table{}, *key{}, *value{};
    const char* borrowed_value{};
    std::vector<std::unique_ptr<NativeTextureSourceChildAcquired>> children;
};

// Full NORMAL C30570 schedule over an actual C30470-created live payload.
// Raw Lua primitives/current-handler/fatal behavior are unchanged. This has no
// protected replacement, outer Lua handler, transport or universal cleanup.
// Normal return marks complete only after all native normal releases/close.
// Caught C++ exception records metadata/rethrows; it does not classify nested
// foreign Lua C escape or establish safe close/discard/replay. Unknown/foreign
// Lua C escapes are unadmitted; backing retention does not restore C frames.
// Reentrant calls require distinct frames but retain the same live payload;
// the initialized gate skips body work. No positive-FPS/null-child guard,
// count reset, retained credit, rollback or application activation is supplied.
void populate_native_texture_source_00c30570(
    NativeTextureSourcePayload&, const NativeString& requested_animation,
    NativeTextureSourceProducerContext&, NativeTextureSourceProducerAcquired&);

} // namespace bsp
