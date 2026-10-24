#pragma once
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_material_factory.hpp"
#include "bsp/gui_text_effect_consumer.hpp"
#include <exception>
#include <memory>

namespace bsp {
class NativeLuaServiceBindings;
struct NativeMaterialDescriptorSamplersContext;
class NativeMaterialDescriptorSamplersCallFactory;
class GuiNativeGeometryOwners;
}
namespace bsp::game {
class GameHostLog;
class GameNativeRendererApplication;
class GameVfsHost;

// Genuine caller inputs, borrowed unchanged. The root/mode calls have distinct
// native B3B280 entry/release evidence. Unknown B-16 residue stays unknown;
// existing helpers reject its consumption rather than manufacture stack bytes.
// All contexts, globals, providers and applicable evidence must survive every
// retained compiler frame and actual effect retirement. Serialize updates.
struct GameNativeGuiEffectApplicationBindings {
    NativeMaterialDescriptorSamplersContext& root_samplers;
    NativeMaterialDescriptorSamplersContext& mode_samplers;
    void* const volatile& material_manager_00f8bbf0;
    // F8BBF0 above is the existing shadow target's policy+0E publication;
    // "material_manager" is a historical descriptive hypothesis, not a new
    // owner type. This factory supplies genuine per-compiler shared stack state.
    NativeMaterialDescriptorSamplersCallFactory* sampler_calls{};
};

enum class GameNativeGuiEffectLoadPhase { fresh, running, complete, failed };
struct GameNativeGuiEffectLoadOperation final {
    GameNativeGuiEffectLoadPhase phase{GameNativeGuiEffectLoadPhase::fresh};
    // Stable borrowed actual8h header, retained by the caller through failure.
    const void* input_name{};
    void* captured_renderer{};
    // Borrowed existing service identity, not another binding or Lua owner.
    // Acquire activates it synchronously; interrupted Lua cannot be resumed.
    NativeLuaServiceBindings* captured_lua_binding{};
    NativeMaterialEffectCacheAcquired cache;
    // Normal return transfers the actual owning reference to the caller.
    // After that this is an opaque diagnostic identity, not another owner.
    void* result{};
    std::uint32_t result_profile{};
    std::int32_t result_references{}; // Observed actual+04, never a replacement counter.
    std::exception_ptr failure;
    // Appended Source bookkeeping preserves all existing diagnostic offsets.
    bool input_name_borrow_active{};
    bool caller_reference_transferred{};
};

enum class GameNativeGuiMaterialFactoryPhase { fresh, factory, registration, complete, failed };
struct GameNativeGuiMaterialFactoryOperation final {
    GameNativeGuiMaterialFactoryPhase phase{GameNativeGuiMaterialFactoryPhase::fresh};
    // Borrowed actual8h header, never owned or consumed here. It and its live
    // bytes must survive synchronous entry. After return/unwind the caller's
    // native cleanup may consume the name: this pointer is then diagnostic
    // only, even when the factory frame survives failure. Retain the header's
    // storage with the caller frame; never reread/replay a consumed name.
    const void* input_name{};
    bool input_name_borrow_active{};
    void* captured_renderer{};
    NativeLuaServiceBindings* captured_lua_binding{};
    GuiNativeGeometryOwners* geometry{};
    // This SAME access object and every referenced provider outlive the
    // material's final release, retained failures, and shared renderer drain.
    NativeMaterialDestructionAccess* materials{};
    const volatile std::uint32_t* material_profile_00d5e520{};
    NativeMaterialFactoryAcquired factory;
    // On normal return only, transfer the single creator to the caller.
    // Neither this diagnostic alias nor the retained frame adds a reference.
    NativeMaterialStorage* result{};
    std::uint32_t result_profile{};
    std::int32_t result_references{};
    bool caller_reference_transferred{};
    std::exception_ptr failure;
};

// Retained concrete children of the scoped Source callers. The required-null
// acquisition slots stay published even if the nested producer throws. Their
// diagnostic result pointers do not acquire another native reference.
struct GameNativeGuiTextMaterialCall final : GuiTextEffectConsumerMaterialCall {
    const GameNativeGuiMaterialFactoryOperation* acquisition{};
};
struct GameNativeGuiTextShaderCall final : GuiTextEffectConsumerShaderCall {
    const GameNativeGuiEffectLoadOperation* acquisition{};
};

// Real numeric renderer48/cache/loader/program/compiler composition over the
// SAME ready renderer, VFS, string pool, actual effects/passes/shaders/states
// and shared record allocator. No semantic shader/material wrapper, copied
// registry or callable table containing original numeric addresses is built.
// B318B0's fixed0/1/1 route returns an actual held effect (or native null).
// It is the missing production supplier for GuiFont.mshd/GuiFontBilinear.mshd;
// constructing this graph does not yet attach the current sprite frontend.
// Cold binary-mode acquisitions require the caller's existing canonical shader
// cache create/acquire/release bracket. This graph borrows that SAME process
// publication; it does not create a private cache or bypass the native lookup.
//
// Actual B3B3C0, B3C3A0, B43B00, B46950, B2EBB0 and B318B0 bodies are composed
// from existing established helpers. No new raw register/x87 recovery or ABI
// claim is introduced. C++ failures retain their one-shot nested frames and
// prohibit replay/retarget. Preserve this graph with the application's process-
// retention policy on failure; its destructor never performs native rollback.
class GameNativeGuiEffectApplication final {
public:
    // Admit metadata while the renderer is ready with no pending acquisition.
    // The same immutable material profile then survives native preparation;
    // acquiring effects/materials still requires the caller's open cache.
    GameNativeGuiEffectApplication(GameHostLog&, GameNativeRendererApplication&,
        GameVfsHost&, GameNativeGuiEffectApplicationBindings);
    ~GameNativeGuiEffectApplication() noexcept;
    GameNativeGuiEffectApplication(const GameNativeGuiEffectApplication&) = delete;
    GameNativeGuiEffectApplication& operator=(const GameNativeGuiEffectApplication&) = delete;

    // Caller supplies initialized live actual8h name storage. A completed
    // return transfers exactly the native caller reference: no additional
    // retain. Retire it through existing actual-owner paths before shared drain.
    void* acquire_effect_00b318b0(const void* stable_actual_name_header);
    // The fresh output is published BEFORE any native effects and never
    // cleared for retry. Caller cleanup may consume name contents after this
    // entry returns/unwinds; retain header storage and never replay its data.
    void* acquire_effect_00b318b0(const void* stable_actual_name_header,
        const GameNativeGuiEffectLoadOperation*& published_operation);
    std::size_t operation_count() const noexcept;
    const GameNativeGuiEffectLoadOperation& operation(std::size_t) const;
    // Ready-only borrow of this SAME canonical renderer/cache/compiler domain.
    // The caller retains its native acquisition frames and activates the
    // renderer's lua_service_binding() around synchronous cache entry. Serialize
    // borrowed uses with this facade, frames, device recreation and native drain;
    // retain this graph through children, interrupted work and final retirement.
    // No load, extra cache, owner/reference or shader-cache bracket is created.
    NativeMaterialEffectCacheContext& native_effect_cache();
    // Whole numeric535320 plus metadata-only canonical material admission.
    // No Text activation, caller name cleanup, extra retain or rollback. Exact
    // geometry, process material/parameter pools, strings and mapped profile
    // are authenticated before entry. Null native allocation returns null.
    // published_operation must be null. Its fresh stable heap frame is
    // published before native effects, including on later exceptions; retain
    // this application and the caller's name/access/provider frame through
    // failure. The output pointer is never cleared or used to permit replay.
    NativeMaterialStorage* create_material_00535320(const void* stable_actual_name_header,
        GuiNativeGeometryOwners&, NativeMaterialDestructionAccess&,
        const GameNativeGuiMaterialFactoryOperation*& published_operation);
    std::size_t material_operation_count() const noexcept;
    const GameNativeGuiMaterialFactoryOperation& material_operation(std::size_t) const;
    // Bind once to this exact persistent services object, whose consumer slot
    // must be null. This adds a Source capability without constructing Text,
    // opening a shader-cache bracket, changing modes or acquiring an effect.
    // The SAME access object/providers, services and this graph survive every
    // creator/failure and shared drain. This graph also outlives any lifetime
    // whose caller history refers to its frames. No retarget/unbind/replay.
    void bind_text_effect_consumer(GuiTextBufferServices&);
    bool requires_process_retention() const noexcept;
    const std::exception_ptr& failure() const noexcept;
    // Keep this graph alive through renderer cache/effect/compiler retirement.
    // After successful renderer native drain this verifies that same drain and
    // permits metadata destruction. Never clears caches, releases caller refs,
    // discards failures or supplies a second teardown order. A never-entered
    // graph may be destroyed without this call.
    void after_native_renderer_drain();
private:
    void* acquire_effect_for_caller(const void*, const GameNativeGuiEffectLoadOperation*&,
        const GuiTextEffectConsumerShaderCall*);
    NativeMaterialStorage* create_material_for_caller(const void*, GuiNativeGeometryOwners&,
        NativeMaterialDestructionAccess&, const GameNativeGuiMaterialFactoryOperation*&,
        const GuiTextEffectConsumerMaterialCall*);
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
