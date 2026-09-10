#pragma once
#include "bsp/compiled_material.hpp"
#include "bsp/instance_geometry.hpp"
#include "bsp/instance_upload.hpp"
#include "bsp/material_clone.hpp"
#include "bsp/render_command_execution.hpp"

namespace bsp {
// Stable wrappers are essential: the renderer borrows their addresses. Keep
// every bound program alive until replaced/unbound or cache.invalidate().
struct MaterialEntryProgram {
    explicit MaterialEntryProgram(std::shared_ptr<CompiledMaterialPass>);
    MaterialEntryProgram(const MaterialEntryProgram&) = delete;
    MaterialEntryProgram& operator=(const MaterialEntryProgram&) = delete;
    std::shared_ptr<CompiledMaterialPass> compiled;
    LogicalVertexShader vertex;
    LogicalPixelShader pixel;
    std::shared_ptr<SamplerStateBlock> sampler_states;
};
// Retained native effect+7C identity shared by ALL mode programs. These fields
// project the actual effect/descriptor; zero initialization is not a loader.
// Missing programs really skip; do not substitute a NORMAL program.
struct MaterialEntryEffect {
    std::shared_ptr<CompiledMaterialEffect> owner;
    std::array<std::shared_ptr<MaterialEntryProgram>, 14> modes; // +C8..FC
    std::shared_ptr<MaterialEntryProgram> alternate_zero; // +100
    std::shared_ptr<MaterialEntryProgram> special_two; // +138
    bool final_lod_fade_out{}; // [[effect+C4]+16], NOT VisilityFade
    bool update_model_state{}; // effect+08 byte
    bool extra_clip_plane{}; // effect+13C byte
    std::array<float, 14> clip_distance{}; // +140..174
};
class MaterialEntryOverride {
public:
    virtual ~MaterialEntryOverride() = default;
    // Actual override virtual+0C, with global0109019C (00B7AAE0 result).
    virtual bool accepts(std::uint32_t, bool& accepted, std::string&) = 0;
};
class MaterialEntryOverrideCollection {
public:
    virtual ~MaterialEntryOverrideCollection() = default;
    virtual std::uint32_t count() const noexcept = 0; // +20, reloaded each turn
    // Reload sentinel->next->value EACH turn, not an indexed traversal.
    virtual MaterialEntryOverride* front() const noexcept = 0;
};
struct MaterialEntryGeometry {
    GeneratedInstanceSection* section{}; // actual captured section/range words
    MaterialCloneState* material{}; // actual section+20 owner
    std::vector<std::shared_ptr<LogicalVertexStream>> streams; // +3C/+4C
    std::shared_ptr<D3D9VertexLayout> layout; // section+50
    std::shared_ptr<LogicalIndexStream> indices; // geometry+60, nullable
    bool indexed{}; // exact nonzero test of section+58 byte
};
struct MaterialEntryConstantState {
    std::uint32_t& first_register; // shared global00E13078, reloaded after VS call
    std::vector<float>& vertex_words; // actual shared0108EBF4 contents
    std::vector<float>& pixel_words; // actual shared0108DBEC contents
};
//00B43541..00B4359F: actual pass metadata and shared constant storage, with
// independent renderer captures for VS and PS. Counts retain the original
// signed differences for diagnostics; PS reloads its gate/start after VS but
// uploads the original pixel_count. COM failure does not suppress PS.
// renderer is the SAME live global00F8D394 slot, never a snapshot or fallback.
bool upload_material_entry_constants_00b43541(const CompiledMaterialPass& pass,
    MaterialEntryConstantState& constants, D3D9StateCache* const volatile& renderer,
    HRESULT& first_failure, std::int32_t& vertex_count, std::int32_t& pixel_count,
    std::string& error);
struct MaterialEntryDrawStatistics {
    std::uint32_t mode{}, primitives{}, vertices{};
    std::int32_t vertex_registers{}, pixel_registers{};
};
// Unresolved native operations are mandatory interfaces, not default no-ops.
// Owner resolution is side-effect-free, returns actual retained identities and
// current camera mode, and must not manufacture shader constants or geometry.
// Entry/model/collection/program storage stays valid across callbacks. Geometry
// owner bindings stay unchanged during each bind/draw; live instance count and
// range words may change. This is a bounded semantic API, not native ABI.
class MaterialEntryOperations {
public:
    virtual ~MaterialEntryOperations() = default;
    virtual bool resolve_effect(InstanceRenderEntry&, MaterialEntryEffect*&,
        std::string&) = 0;
    virtual bool read_camera_mode(const InstanceRenderEntry&, std::uint32_t&,
        std::string&) = 0;
    virtual bool capture_overrides(const InstanceRenderEntry&,
        MaterialEntryOverrideCollection*&, std::string&) = 0;
    virtual bool update_model_and_renderer(InstanceRenderEntry&, std::string&) = 0;
    virtual bool resolve_geometry(InstanceRenderEntry&, MaterialEntryGeometry&,
        std::string&) = 0;
    // Use the actual camera frame companion on this captured renderer. Native
    // saves global00F8D394 at00B4485A before the remaining plane owner/math calls.
    virtual bool restore_pending_planes_00b25080(D3D9StateCache&, std::string&) = 0;
    // Remaining00B4488D..00B44A45 math/owner chain, then00B25040 on the capture.
    virtual bool construct_and_append_effect_plane(D3D9StateCache&, MaterialEntryEffect&,
        InstanceRenderEntry&, float distance, std::string&) = 0;
    virtual bool effect_texture_00b17d90(MaterialEntryEffect&, std::uint32_t,
        std::shared_ptr<LogicalTexture>&, std::string&) = 0;
    // Use recovered build_material_constants_00b42350 with actual retained
    // owner bindings and shared blocks. Native dynamic source implementations
    // and the system prefix are separate dependencies of those bindings.
    // False is an owner/adapter error. Return accumulated COM failures through
    // device_result while continuing subsequent native uploads/callback/draw.
    virtual bool build_constants_00b42350(MaterialEntryProgram&,
        InstanceRenderEntry&, MaterialEntryOverride*, MaterialEntryConstantState&,
        HRESULT& device_result, std::string&) = 0;
    // Selected only when actual material+08 is nonzero. Native CALL EAX has
    // ECX=entry and no explicit stack arguments; callback ABI is unresolved.
    virtual bool material_callback(MaterialCloneState&, InstanceRenderEntry&,
        std::string&) = 0;
    // Actual diagnostic singleton+681 enable gate, effect/current-string
    // record lookup and14-mode accumulator. Native calls even after draw skip
    // or COM failure. A disabled actual singleton may return success unchanged.
    virtual bool record_statistics_00b16f80(const CompiledMaterialEffect&,
        const MaterialEntryDrawStatistics&, std::string&) = 0;
};
struct MaterialEntryEnvironment {
    // Actual shared global00F8D394 slot; keep the slot and any captured renderer
    // alive across callbacks. A replacement takes effect at native reloads.
    D3D9StateCache* const volatile& renderer;
    const D3D9DrawState& draw;
    const void*& material_effect_owner; // SAME global0108FBF4 as batch reset
    const std::uint32_t& override_context; // actual global0109019C
    MaterialEntryConstantState constants;
    MaterialEntryOperations& operations;
};
//00B45360 selector and00B44750/00B43410 orchestration around required native
// operations above. This is not a complete material pipeline. Native ignores
// HRESULTs; COM failures here are accumulated while later stages still run.
// Invalid bindings/adapter failure stop further work and preserve prior effects.
class MaterialEntryDispatcher final : public RenderBatchMaterialDispatch {
public:
    explicit MaterialEntryDispatcher(MaterialEntryEnvironment value) : environment_(value) {}
    bool execute_material_entry(InstanceRenderEntry&, std::string&) override;
private:
    bool mode(const InstanceRenderEntry&, std::uint32_t&, std::string&);
    bool select_and_draw(MaterialEntryEffect&, InstanceRenderEntry&,
        MaterialEntryOverride*, HRESULT&, std::string&);
    bool bind_and_draw(MaterialEntryEffect&, MaterialEntryProgram&,
        InstanceRenderEntry&, MaterialEntryOverride*, HRESULT&, std::string&);
    MaterialEntryEnvironment environment_;
};
}
