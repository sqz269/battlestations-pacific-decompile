#include "bsp/game_native_gui_effect_probe.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_gui_sampler_application.hpp"
#include "bsp/game_native_shadow_target_application.hpp"
#include "bsp/native_effect_registry_destroy.hpp"
#include "bsp/native_material_effect_owner.hpp"
#include "bsp/native_material_program_compiler_actual.hpp"
#include "bsp/native_shader_descriptor_reader.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_shader_binary_cache.hpp"
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp::game {
namespace {
using Stage = GameNativeGuiEffectProbeStage;
static_assert(sizeof(NativeString) == 8);
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
const char* stage_name(Stage stage) noexcept {
    switch (stage) {
    case Stage::prepared: return "prepared";
    case Stage::shadow: return "shadow";
    case Stage::shader_cache: return "shader-cache-create";
    case Stage::sampler_parent: return "sampler-parent";
    case Stage::effect_parent: return "effect-parent";
    case Stage::names: return "names";
    case Stage::first_effect: return "GuiFont.mshd";
    case Stage::second_effect: return "GuiFontBilinear.mshd";
    case Stage::shader_cache_release: return "shader-cache-release";
    case Stage::ready: return "ready";
    case Stage::retiring_references: return "retiring-references";
    case Stage::retiring_names: return "retiring-names";
    case Stage::retired: return "retired";
    case Stage::closing: return "closing";
    case Stage::closed: return "closed";
    case Stage::failed: return "failed";
    case Stage::fakemotion_preload: return "fakemotion-preload";
    case Stage::fakemotion_warm_hit: return "fakemotion-warm-hit";
    }
    return "unknown";
}
void note_exception(GameHostLog& log, const std::exception_ptr& failure) {
    if (!failure) return;
    try { std::rethrow_exception(failure); }
    catch (const std::exception& error) { log.notef("native GUI effect probe exception: %s", error.what()); }
    catch (...) { log.note("native GUI effect probe exception: nonstandard C++ exception"); }
}
std::int32_t inspect_effect(void* identity, NativeRenderActualOwnerRegistry& owners,
    std::uint32_t& profile) {
    require(identity && (reinterpret_cast<std::uintptr_t>(identity) & 3u) == 0,
        "GUI effect probe requires a nonnull aligned actual effect");
    auto* companion = dynamic_cast<NativeMaterialEffectReference*>(&owners.resolve_actual(identity));
    require(companion && &companion->storage() == identity &&
        &companion->reference_count == &companion->storage().references_04,
        "GUI effect probe requires the existing canonical actual+04 effect companion");
    auto& storage = companion->storage();
    profile = storage.vtable_00;
    const auto references = storage.references_04.load();
    require(references > 0 && (profile == 0x00d5e534 || profile == 0x00d61a00),
        "GUI effect probe returned no live supported actual effect");
    return references;
}
void observe_cache(NativeShaderBinaryCacheContext& cache,
    GameNativeGuiEffectProbeCacheObservation& observed) noexcept {
    observed.owner = cache.actual_cache_0108d6ec;
    observed.source_mode = cache.source_mode_0108d6f1;
    observed.load_variants = cache.load_variants_0108d6f0;
    if (observed.owner) {
        const auto* actual = static_cast<const volatile NativeShaderBinaryCacheStorage*>(observed.owner);
        observed.cursor = actual->cursor_00;
        observed.count = actual->count_04;
    }
    observed.sampled = true;
}
bool name_is(const NativeString& name, const char* expected) noexcept {
    const auto length = std::strlen(expected);
    return name.length() == length && name.data() &&
        std::memcmp(name.data(), expected, length) == 0;
}
void note_resource(GameHostLog& log, const GameNativeGuiEffectProbeResource& slot) {
    log.notef("native GUI probe slot=%s header=%p name=%d/%d acquire=%d op=%zu frame=%p held=%p identity=%p release=%d/%d",
        slot.requested_name, static_cast<const void*>(slot.input_name), slot.name_entered ? 1 : 0,
        slot.name_live ? 1 : 0, slot.acquisition_entered ? 1 : 0, slot.operation_index,
        slot.retained_operation, slot.caller_reference, slot.returned_identity,
        slot.release_entered ? 1 : 0, slot.caller_released ? 1 : 0);
    log.notef("native GUI probe cache resource=%s sampled=%d/%d owners=%p/%p cursors=%u/%u counts=%u/%u modes=%u/%u variants=%u/%u site=%08x published=%d cold=%d",
        slot.requested_name, slot.cache_before.sampled ? 1 : 0, slot.cache_after.sampled ? 1 : 0,
        slot.cache_before.owner, slot.cache_after.owner, slot.cache_before.cursor, slot.cache_after.cursor,
        slot.cache_before.count, slot.cache_after.count, static_cast<unsigned>(slot.cache_before.source_mode),
        static_cast<unsigned>(slot.cache_after.source_mode), static_cast<unsigned>(slot.cache_before.load_variants),
        static_cast<unsigned>(slot.cache_after.load_variants), slot.cache_native_site,
        slot.cache_published ? 1 : 0, slot.cold_loader_entered ? 1 : 0);
}
void note_program(GameHostLog& log, const NativeMaterialEffectProgramOperation& programs) {
    auto* child = programs.active_child();
    log.notef("native GUI program phase=%u site=%08x slots=%u initialized=%d live=%d child=%p",
        static_cast<unsigned>(programs.phase()), programs.active_call_site(), programs.primary_slots_written(),
        programs.pass_slots_initialized() ? 1 : 0, programs.has_live_bindings() ? 1 : 0, child);
    if (auto* descriptor = dynamic_cast<NativeShaderDescriptorReadOperation*>(child)) {
        log.notef("native GUI descriptor phase=%u site=%08x retained=%d descriptor=%p name=%p lua=%p",
            static_cast<unsigned>(descriptor->phase()), descriptor->active_call_site(),
            descriptor->retains_native_state() ? 1 : 0, descriptor->descriptor(),
            descriptor->original_name_header(), descriptor->retained_lua_state());
    }
    if (auto* compiler = dynamic_cast<NativeMaterialProgramCompileOperation*>(child)) {
        const auto& frame = compiler->frame();
        log.notef("native GUI compiler phase=%u site=%08x resume=%08x builder=%p result=%p tail=%p retained=%d",
            static_cast<unsigned>(frame.phase), frame.native_site, static_cast<unsigned>(frame.resume),
            static_cast<const void*>(&frame.builder), frame.result, frame.tail_child.get(),
            compiler->retains_native_state() ? 1 : 0);
        if (auto* tail = dynamic_cast<NativeMaterialProgramCompilerActualFrame*>(frame.tail_child.get())) {
            log.notef("native GUI compiler-tail phase=%u site=%08x state=%08x raw_pass=%p pass=%p registered=%d native_null=%d",
                static_cast<unsigned>(tail->phase), tail->native_site, tail->exception_state,
                tail->raw_pass, tail->pass, tail->pass_registered ? 1 : 0, tail->native_null_return ? 1 : 0);
            // Read only retained compiler metadata. Never probe or release a native owner.
            const auto& entry = tail->sampler_entry;
            log.notef("native GUI sampler-entry cached=%d profile=%d boundary=%d/%d pixel_com_released=%d known=%d published=%d cache=%p row=%p builder=%p pass=%p owner=%p shader=%p device=%p",
                entry.cached_path ? 1 : 0, entry.profile_matched ? 1 : 0,
                entry.release_entered ? 1 : 0, entry.release_returned ? 1 : 0,
                tail->pixel_com_released ? 1 : 0,
                entry.known ? 1 : 0, entry.published ? 1 : 0,
                static_cast<const void*>(entry.cache), static_cast<const void*>(entry.row),
                static_cast<const void*>(entry.builder), static_cast<const void*>(entry.pass),
                static_cast<const void*>(entry.owner), static_cast<const void*>(entry.shader), static_cast<const void*>(entry.device));
            log.notef("native GUI sampler-entry modules d3d9=%p ntdll=%p kernel32=%p release_target=%p result=%u shader_before=%u device=%u/%u owner_ctor=%d owner_release_previous=%d",
                reinterpret_cast<const void*>(entry.d3d9_module), reinterpret_cast<const void*>(entry.ntdll_module),
                reinterpret_cast<const void*>(entry.kernel32_module), reinterpret_cast<const void*>(entry.release_target),
                entry.com_release_result, entry.shader_count_before, entry.device_count_before, entry.device_count_after,
                entry.owner_count_at_constructor, entry.owner_release_previous_count);
            if (entry.release_returned)
                log.notef("native GUI sampler-entry captured_word=%08x qualified=%d", entry.word, entry.known ? 1 : 0);
            for (std::size_t index = 0; index < tail->samplers.size(); ++index) {
                const auto* sampler = tail->samplers[index].get();
                if (!sampler) continue;
                const auto* context = sampler->context;
                const bool known = context && context->stack_knowledge && context->stack_knowledge->known;
                log.notef("native GUI sampler arm=%zu phase=%u site=%08x index=%u steps=%zu stack_known=%d",
                    index, static_cast<unsigned>(sampler->phase), sampler->native_site,
                    sampler->sampler_index, sampler->steps.size(), known ? 1 : 0);
                // Never read the actual private word when its knowledge is unknown.
                if (!sampler->steps.empty()) {
                    const auto& step = sampler->steps.back();
                    log.notef("native GUI sampler step index=%u site=%08x owner=%p obligation=%d release=%d/%d",
                        step.sampler_index, step.native_site, step.captured_owner, step.owner_obligation ? 1 : 0,
                        step.release_entered ? 1 : 0, step.release_returned ? 1 : 0);
                    if (step.sampler)
                        log.notef("native GUI reached sampler actual=%p source=%d index=%d type=%d vertex=%u",
                            step.sampler, step.sampler->texture_source_14, step.sampler->index_20,
                            step.sampler->type_10, static_cast<unsigned>(step.sampler->vertex_sampler_0c));
                }
            }
        }
    }
}
void note_cache_chain(GameHostLog& log, const NativeMaterialEffectCacheAcquired& first) {
    std::size_t depth = 0;
    for (const auto* cache = &first; cache; cache = cache->loader.fallback.get(), ++depth) {
        const auto& loader = cache->loader;
        log.notef("native GUI cache depth=%zu phase=%u site=%08x loaded=%p result=%p published=%d caller=%d",
            depth, static_cast<unsigned>(cache->phase), cache->native_site, cache->loaded, cache->result,
            cache->cache_published ? 1 : 0, cache->caller_acquired ? 1 : 0);
        if (const auto* key = cache->key_resolution.get()) {
            log.notef("native GUI cache key phase=%u step=%u site=%08x original=%d changed=%d output=%d used_original=%d",
                static_cast<unsigned>(key->phase), static_cast<unsigned>(key->step), key->native_site,
                key->original_live ? 1 : 0, key->changed_live ? 1 : 0, key->output_live ? 1 : 0,
                key->used_original ? 1 : 0);
            note_exception(log, key->failure);
            const NativeMaterialEffectNameResolutionCall* calls[] = {
                &key->changed_resolution, &key->original_resolution};
            for (std::size_t arm = 0; arm < 2; ++arm) {
                const auto& call = *calls[arm];
                log.notef("native GUI cache key VFS arm=%zu manager=%p header=%p started=%d returned=%d failed=%d result=%d",
                    arm, call.manager, call.name, call.started ? 1 : 0, call.returned ? 1 : 0,
                    call.failed ? 1 : 0, call.result ? 1 : 0);
                if (call.acquired)
                    log.notef("native GUI cache key VFS arm=%zu phase=%u site=%08x failure_site=%08x",
                        arm, static_cast<unsigned>(call.acquired->phase()), call.acquired->active_call_site(),
                        call.acquired->failure_site());
            }
        }
        log.notef("native GUI loader phase=%u site=%08x raw=%p creator=%p companion=%p owner_record=%p constructed=%d registered=%d returned=%d fallback=%p",
            static_cast<unsigned>(loader.phase), loader.native_site, loader.raw_slot, loader.creator,
            loader.companion, loader.owner_record, loader.constructor_complete ? 1 : 0,
            loader.registered ? 1 : 0, loader.returned ? 1 : 0, loader.fallback.get());
        if (loader.programs) note_program(log, *loader.programs);
    }
}
}

struct GameNativeGuiEffectProbe::Impl {
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& vfs;
    GameSingletonHost& singletons;
    GameNativeReadOnlyData& data;
    GameNativeSettingsProcess& settings;
    GameInputRuntime& input;
    NativePlatformLoadPublication platform;
    GameNativeVfsRawServices raw;
    GameNativeGuiEffectProbeObservation observed;
    // Reverse destruction preserves parent scopes/header addresses through effect metadata.
    std::unique_ptr<GameNativeShadowTargetApplication> shadow;
    std::unique_ptr<GameNativeGuiSamplerApplication> sampler;
    std::unique_ptr<GameNativeGuiEffectApplication> effects;
    Impl(GameHostLog& l, GameNativeRendererApplication& r, GameVfsHost& v,
        GameSingletonHost& s, GameNativeReadOnlyData& d, GameNativeSettingsProcess& settings_process,
        GameInputRuntime& i, NativePlatformLoadPublication p)
        : log(l), renderer(r), vfs(v), singletons(s), data(d), settings(settings_process),
          input(i), platform(p), raw(v.borrow_raw_services()) {
        observed.resources[0].requested_name = "GuiFont.mshd";
        observed.resources[1].requested_name = "GuiFontBilinear.mshd";
        observed.fakemotion_preload.requested_name = "fakemotionblur.mshd";
        observed.fakemotion_warm.requested_name = "fakemotionblur.mshd (warm)";
        for (auto& slot : observed.resources) slot.input_name = &slot.native_name;
        observed.fakemotion_preload.input_name = &observed.fakemotion_preload.native_name;
        observed.fakemotion_warm.input_name = &observed.fakemotion_preload.native_name;
        // No native allocation, name construction, shadow/effect/sampler entry here.
    }
    void acquire(GameNativeGuiEffectProbeResource& slot, const NativeString& name) {
        slot.input_name = &name;
        slot.operation_index = effects->operation_count();
        observe_cache(renderer.shader_cache_context(), slot.cache_before);
        slot.acquisition_entered = true;
        slot.caller_reference = effects->acquire_effect_00b318b0(slot.input_name);
        slot.returned_identity = slot.caller_reference;
        observe_cache(renderer.shader_cache_context(), slot.cache_after);
        slot.references_on_return = inspect_effect(slot.caller_reference, renderer.actual_owners(), slot.profile);
        const auto& operation = effects->operation(slot.operation_index);
        slot.retained_operation = &operation;
        require(operation.cache.caller_acquired,
            "GUI effect probe received no exact native caller credit");
        slot.cache_native_site = operation.cache.native_site;
        slot.cache_published = operation.cache.cache_published;
        slot.cold_loader_entered = operation.cache.loader.phase != NativeMaterialEffectLoadPhase::not_started;
        slot.fallback_call_seen = operation.cache.loader.fallback != nullptr;
        log.notef("native GUI effect probe resource=%s header=%p actual=%p profile=%08x refs=%d fallback_call=%d",
            slot.requested_name, static_cast<const void*>(slot.input_name), slot.caller_reference,
            slot.profile, slot.references_on_return, slot.fallback_call_seen ? 1 : 0);
        note_resource(log, slot);
    }
    void inspect_fakemotion_descriptor(NativeShaderDescriptorStorage* descriptor,
        GameNativeGuiEffectProbeSelectedSampler& result) {
        result.descriptor = descriptor;
        require(descriptor != nullptr, "fakemotion probe has no current selected descriptor");
        result.count = descriptor->virtual_owners_c4.count_04;
        for (std::size_t index = 0; index < 14; ++index)
            if (descriptor->mode_names_48[index].length()) result.mode_mask |= 1u << index;
        result.dummy_combiner = name_is(descriptor->mode_names_48[0], "dummy.shfx");
        require(result.count == 1 && descriptor->virtual_owners_c4.data_00,
            "fakemotion probe requires the selected single SceneTexture sampler");
        auto* selected_sampler = *static_cast<NativeShaderSamplerStorage**>(descriptor->virtual_owners_c4.data_00);
        result.sampler = selected_sampler;
        require(selected_sampler != nullptr, "fakemotion probe selected a null sampler");
        result.texture_source = selected_sampler->texture_source_14;
        result.index = selected_sampler->index_20;
        result.type = selected_sampler->type_10;
        result.vertex_sampler = selected_sampler->vertex_sampler_0c;
        result.scene_texture_name = name_is(selected_sampler->name_04, "SceneTexture");
        result.inspected = true;
        log.notef("native fakemotion selected descriptor=%p sampler=%p count=%d source=%d index=%d type=%d vertex=%u SceneTexture=%d mode_mask=%08x dummy=%d",
            result.descriptor, result.sampler, result.count, result.texture_source, result.index, result.type,
            static_cast<unsigned>(result.vertex_sampler), result.scene_texture_name ? 1 : 0,
            result.mode_mask, result.dummy_combiner ? 1 : 0);
        require(result.texture_source == 0 && result.index == 0 && result.type == 2 &&
            result.vertex_sampler == 0 && result.scene_texture_name && result.mode_mask == 1 && result.dummy_combiner,
            "fakemotion probe selected a descriptor outside the bounded SceneTexture Source0 contract");
    }
    static GameNativeGuiSamplerCallEvidence observe_fakemotion_sampler_call(void* context,
        NativeMaterialProgramBuilderStorage& builder, NativeMaterialPassStorage& pass,
        NativeShaderDescriptorStorage& root, NativeShaderDescriptorStorage& mode) {
        auto& p = *static_cast<Impl*>(context);
        auto& o = p.observed;
        if (o.include_fakemotion && o.stage == Stage::fakemotion_preload) {
            require(o.fakemotion_sampler_call_count < o.fakemotion_sampler_calls.size(),
                "fakemotion probe exceeded its two retained variant sampler observations");
            const auto index = o.fakemotion_sampler_call_count++;
            auto& call = o.fakemotion_sampler_calls[index];
            call.builder = &builder;
            call.pass = &pass;
            call.mode_descriptor = &mode;
            call.following_root_site = 0x00b3c018;
            call.following_mode_site = 0x00b3c024;
            call.mode_sampler_count = mode.virtual_owners_c4.count_04;
            p.log.notef("native fakemotion sampler observation=%zu callback=host-final-states before_root=%08x before_mode=%08x graph=%p builder=%p pass=%p root=%p mode=%p mode_samplers=%d evidence=empty",
                index, call.following_root_site, call.following_mode_site, o.effect_application_identity,
                call.builder, call.pass, static_cast<void*>(&root), call.mode_descriptor, call.mode_sampler_count);
            p.inspect_fakemotion_descriptor(&root, call.root);
            require(call.mode_sampler_count == 0,
                "fakemotion probe selected a mode sampler outside the bounded empty-mode contract");
        }
        // Observation only. Never supply W, entry knowledge, release scratch,
        // a provider or a default. final_states still publishes its qualified
        // actual cached predecessor capture and enforces the existing guard.
        return GameNativeGuiSamplerCallEvidence{};
    }
    void retire(GameNativeGuiEffectProbeResource& slot) {
        require(slot.caller_reference && !slot.release_entered,
            "GUI effect probe caller credit is absent or already being retired");
        std::uint32_t profile{};
        slot.references_before_release = inspect_effect(slot.caller_reference, renderer.actual_owners(), profile);
        slot.release_entered = true;
        release_native_cached_effect_00b32010(slot.caller_reference, renderer.actual_owners());
        slot.caller_reference = nullptr;
        slot.caller_released = true;
        slot.companion_present_after_release = renderer.actual_owners().find(slot.returned_identity) != nullptr;
        if (slot.companion_present_after_release)
            slot.references_after_release = inspect_effect(slot.returned_identity, renderer.actual_owners(), profile);
        if (slot.references_after_release)
            require(*slot.references_after_release == slot.references_before_release - 1,
                "GUI effect caller retirement did not remove exactly one actual credit");
        else require(slot.references_before_release == 1,
            "GUI effect companion disappeared before its actual count reached zero");
        // Missing companion means retired identity; do not dereference freed raw storage.
        log.notef("native GUI effect probe retired caller=%s identity=%p before=%d companion=%d after=%d",
            slot.requested_name, slot.returned_identity, slot.references_before_release,
            slot.companion_present_after_release ? 1 : 0, slot.references_after_release.value_or(-1));
    }
    void record_failure() noexcept {
        observed.failure = std::current_exception();
        observed.failure_stage = observed.stage;
        observed.stage = Stage::failed;
    }
};

GameNativeGuiEffectProbe::GameNativeGuiEffectProbe(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs, GameSingletonHost& singletons,
    GameNativeReadOnlyData& data, GameNativeSettingsProcess& settings, GameInputRuntime& input,
    NativePlatformLoadPublication platform)
    : impl_(std::make_unique<Impl>(log, renderer, vfs, singletons, data, settings, input, platform)) {}

GameNativeGuiEffectProbe::~GameNativeGuiEffectProbe() noexcept {
    if (impl_->observed.stage != Stage::prepared && impl_->observed.stage != Stage::closed)
        std::terminate();
}

const GameNativeGuiEffectProbeObservation& GameNativeGuiEffectProbe::run(bool include_fakemotion) {
    auto& p = *impl_;
    auto& o = p.observed;
    require(o.stage == Stage::prepared, "GUI effect probe cannot replay production");
    o.include_fakemotion = include_fakemotion;
    try {
        p.log.notef("native GUI effect probe options include_fakemotion=%d", include_fakemotion ? 1 : 0);
        o.stage = Stage::shadow;
        p.shadow = std::make_unique<GameNativeShadowTargetApplication>(p.log, p.renderer, p.settings);
        p.shadow->construct();
        auto* target = p.shadow->target();
        require(target != nullptr, "actual shadow allocation returned null; GUI effect probe cannot enter policy consumer");
        p.log.notef("native GUI effect probe shadow owner=%p profile=%08x extent=%u/%u policy=%u",
            target, target->profile_00, target->width_04, target->height_08,
            static_cast<unsigned>(target->policy_0e));
        o.stage = Stage::shader_cache;
        require(!p.renderer.shader_cache_creation_operation() &&
            !p.renderer.shader_cache_context().actual_cache_0108d6ec,
            "GUI effect probe requires an unused canonical shader-cache bracket");
        // Original73BF80 brackets its material acquisitions with B3A600/B3B140.
        // This bounded resource probe schedules those same existing producers;
        // it does not run the preload script/vector or change either mode byte.
        o.source_mode_on_create = p.renderer.shader_cache_context().source_mode_0108d6f1;
        o.load_variants_on_create = p.renderer.shader_cache_context().load_variants_0108d6f0;
        if (include_fakemotion)
            require(o.source_mode_on_create == 0,
                "fakemotion binary preload probe requires the existing compiler source mode0");
        o.cache_creation_entered = true;
        p.renderer.create_shader_cache();
        o.cache_creation_returned = true;
        o.cache_identity = p.renderer.shader_cache_context().actual_cache_0108d6ec;
        require(o.cache_identity != nullptr,
            "actual shader-cache allocation returned null; GUI effect probe cannot enter binary consumers");
        p.log.notef("native GUI effect probe shader cache owner=%p source_mode=%u load_variants=%u",
            o.cache_identity, static_cast<unsigned>(o.source_mode_on_create),
            static_cast<unsigned>(o.load_variants_on_create));
        o.stage = Stage::sampler_parent;
        GameNativeGuiSamplerApplicationBindings sampler_bindings{p.input, p.platform};
        if (include_fakemotion) {
            sampler_bindings.evidence_context = &p;
            sampler_bindings.evidence = &Impl::observe_fakemotion_sampler_call;
        }
        p.sampler = std::make_unique<GameNativeGuiSamplerApplication>(p.log, p.renderer, p.vfs,
            p.singletons, p.data, sampler_bindings);
        // EMPTY MSG output preimage and no invocation/root/release stack capture.
        o.stage = Stage::effect_parent;
        p.effects = std::make_unique<GameNativeGuiEffectApplication>(p.log, p.renderer, p.vfs,
            p.sampler->effect_bindings());
        o.effect_application_identity = p.effects.get();
        require(&p.renderer.effect_registry_domain().actual_records.strings == &p.raw.strings,
            "GUI effect probe names require the same actual renderer/VFS string pool");
        o.stage = Stage::names;
        for (auto& slot : o.resources) {
            slot.name_entered = slot.name_live = true;
            slot.native_name.assign_0041e870(p.raw.strings, slot.requested_name);
        }
        for (std::size_t index = 0; index < o.resources.size(); ++index) {
            auto& slot = o.resources[index];
            o.stage = index == 0 ? Stage::first_effect : Stage::second_effect;
            p.acquire(slot, slot.native_name);
        }
        if (include_fakemotion) {
            o.stage = Stage::fakemotion_preload;
            auto& slot = o.fakemotion_preload;
            require(p.effects->operation_count() == o.resources.size(),
                "fakemotion preload must follow exactly the two genuine font acquisitions");
            slot.name_entered = slot.name_live = true;
            slot.native_name.assign_0041e870(p.raw.strings, slot.requested_name);
            p.acquire(slot, slot.native_name);
            const auto& operation = p.effects->operation(slot.operation_index);
            require(operation.cache.cache_published && operation.cache.loaded == slot.caller_reference &&
                operation.cache.loader.creator == slot.caller_reference && operation.cache.loader.registered &&
                operation.cache.loader.returned && operation.cache.loader.phase == NativeMaterialEffectLoadPhase::complete &&
                !slot.fallback_call_seen,
                "fakemotion preload requires its genuine completed cold creator without fallback");
            require(slot.cache_before.owner == o.cache_identity && slot.cache_after.owner == o.cache_identity &&
                slot.cache_before.source_mode == 0 && slot.cache_after.source_mode == 0 &&
                slot.cache_before.load_variants == o.load_variants_on_create &&
                slot.cache_after.load_variants == o.load_variants_on_create &&
                slot.cache_before.count == slot.cache_after.count &&
                slot.cache_after.cursor > slot.cache_before.cursor && slot.cache_after.cursor <= slot.cache_after.count,
                "fakemotion preload changed its canonical cache, mode or forward cursor domain");
            require(slot.profile == 0x00d61a00 && o.fakemotion_sampler_call_count != 0,
                "fakemotion preload requires an actual derived creator and observed compiler sampler calls");
            auto* effect = static_cast<NativeMaterialEffectStorage*>(slot.caller_reference);
            p.inspect_fakemotion_descriptor(static_cast<NativeShaderDescriptorStorage*>(effect->descriptor_c4),
                o.fakemotion_sampler);
            std::uint32_t profile{};
            o.fakemotion_references_before_cache_release =
                inspect_effect(slot.caller_reference, p.renderer.actual_owners(), profile);
        }
        o.stage = Stage::shader_cache_release;
        o.cache_release_entered = true;
        p.renderer.release_shader_cache();
        o.cache_release_returned = true;
        require(p.renderer.shader_cache_released() &&
            !p.renderer.shader_cache_context().actual_cache_0108d6ec,
            "GUI effect probe shader cache survived its native release bracket");
        if (include_fakemotion) {
            o.stage = Stage::fakemotion_warm_hit;
            auto& cold = o.fakemotion_preload;
            auto& warm = o.fakemotion_warm;
            std::uint32_t profile{};
            o.fakemotion_references_after_cache_release =
                inspect_effect(cold.caller_reference, p.renderer.actual_owners(), profile);
            require(o.fakemotion_references_after_cache_release == o.fakemotion_references_before_cache_release &&
                o.fakemotion_references_after_cache_release < std::numeric_limits<std::int32_t>::max(),
                "normal binary cache close changed the retained fakemotion caller credit");
            require(p.effects.get() == o.effect_application_identity &&
                p.effects->operation_count() == o.resources.size() + 1u &&
                &p.effects->operation(cold.operation_index) == cold.retained_operation,
                "fakemotion warm acquisition requires its same application and retained cold frame");
            p.acquire(warm, cold.native_name); // SAME actual8h header; no second name acquisition.
            const auto& operation = p.effects->operation(warm.operation_index);
            require(warm.input_name == cold.input_name && operation.input_name == cold.input_name &&
                warm.retained_operation != cold.retained_operation && warm.caller_reference == cold.caller_reference &&
                operation.cache.native_site == 0x00b31436 && !operation.cache.cache_published &&
                !operation.cache.key_resolution && operation.cache.loader.phase == NativeMaterialEffectLoadPhase::not_started &&
                !warm.fallback_call_seen && warm.references_on_return == o.fakemotion_references_after_cache_release + 1,
                "fakemotion warm acquisition did not return the same owner and one B31436 caller credit");
            require(!warm.cache_before.owner && !warm.cache_after.owner &&
                warm.cache_before.source_mode == 0 && warm.cache_after.source_mode == 0 &&
                warm.cache_before.load_variants == o.load_variants_on_create &&
                warm.cache_after.load_variants == o.load_variants_on_create && p.renderer.shader_cache_released(),
                "fakemotion warm acquisition reopened or changed its closed canonical cache domain");
            o.fakemotion_warm_verified = true;
            p.log.notef("native fakemotion warm verified graph=%p cold_frame=%p warm_frame=%p header=%p actual=%p refs_before_close=%d after_close=%d after_warm=%d cache=%p site=%08x",
                o.effect_application_identity, cold.retained_operation, warm.retained_operation,
                static_cast<const void*>(warm.input_name), warm.caller_reference,
                o.fakemotion_references_before_cache_release, o.fakemotion_references_after_cache_release,
                warm.references_on_return, warm.cache_after.owner, warm.cache_native_site);
        }
        o.stage = Stage::ready;
        return o;
    } catch (...) {
        p.record_failure(); emit_failure_observations(); throw;
    }
}

void GameNativeGuiEffectProbe::retire_caller_references() {
    auto& p = *impl_;
    auto& o = p.observed;
    require(o.stage == Stage::ready, "GUI effect probe retirement requires complete unreleased acquisitions");
    try {
        o.stage = Stage::retiring_references;
        if (o.include_fakemotion) {
            require(o.fakemotion_warm_verified, "fakemotion credits require the completed warm-hit proof before teardown");
            p.retire(o.fakemotion_warm);
            p.retire(o.fakemotion_preload);
        }
        for (std::size_t index = o.resources.size(); index != 0; --index)
            p.retire(o.resources[index - 1]);
        o.stage = Stage::retiring_names;
        if (o.include_fakemotion) {
            require(o.fakemotion_preload.name_live && !o.fakemotion_warm.name_live,
                "fakemotion warm slot must borrow the single live cold name");
            destroy_native_string_header_0041dd20(&o.fakemotion_preload.native_name, p.raw.strings);
            o.fakemotion_preload.name_live = false;
        }
        for (std::size_t index = o.resources.size(); index != 0; --index) {
            auto& slot = o.resources[index - 1];
            require(slot.name_live, "GUI effect probe input name is already retired");
            destroy_native_string_header_0041dd20(&slot.native_name, p.raw.strings);
            slot.name_live = false;
        }
        o.stage = Stage::retired;
    } catch (...) {
        p.record_failure(); emit_failure_observations(); throw;
    }
}

void GameNativeGuiEffectProbe::after_native_renderer_drain() {
    auto& p = *impl_;
    if (p.observed.stage == Stage::closed) return;
    require(p.observed.stage == Stage::retired, "GUI effect probe caller credits/names must retire before shared drain");
    try {
        p.observed.stage = Stage::closing;
        p.effects->after_native_renderer_drain();
        p.sampler->after_native_renderer_drain();
        p.shadow->after_native_renderer_drain();
        p.observed.stage = Stage::closed;
        p.log.note("native GUI effect probe closed after genuine shared native drain");
    } catch (...) {
        p.record_failure(); emit_failure_observations(); throw;
    }
}

const GameNativeGuiEffectProbeObservation& GameNativeGuiEffectProbe::observation() const noexcept {
    return impl_->observed;
}
bool GameNativeGuiEffectProbe::requires_process_retention() const noexcept {
    const auto& p = *impl_;
    const auto stage = p.observed.stage;
    return stage == Stage::failed ||
        (stage != Stage::prepared && stage != Stage::ready && stage != Stage::retired && stage != Stage::closed) ||
        (p.shadow && p.shadow->requires_process_retention()) ||
        (p.sampler && p.sampler->requires_process_retention()) ||
        (p.effects && p.effects->requires_process_retention());
}
void GameNativeGuiEffectProbe::emit_failure_observations() const noexcept {
    const auto& p = *impl_;
    try {
        p.log.notef("native GUI shader cache create=%d/%d release=%d/%d identity=%p current=%p source_mode=%u load_variants=%u",
            p.observed.cache_creation_entered ? 1 : 0, p.observed.cache_creation_returned ? 1 : 0,
            p.observed.cache_release_entered ? 1 : 0, p.observed.cache_release_returned ? 1 : 0,
            p.observed.cache_identity, p.renderer.shader_cache_context().actual_cache_0108d6ec,
            static_cast<unsigned>(p.observed.source_mode_on_create),
            static_cast<unsigned>(p.observed.load_variants_on_create));
        if (const auto* cache = p.renderer.shader_cache_creation_operation())
            p.log.notef("native GUI shader cache operation phase=%u function=%08x site=%08x cache=%p allocation=%p records=%u index=%u temporary_live=%d",
                static_cast<unsigned>(cache->phase), cache->function, cache->native_site, cache->cache,
                cache->allocation, cache->completed_records, cache->index, cache->temporary_live ? 1 : 0);
        p.log.notef("native GUI effect probe state=%s failed_stage=%s retention=%d",
            stage_name(p.observed.stage), stage_name(p.observed.failure_stage), requires_process_retention() ? 1 : 0);
        note_exception(p.log, p.observed.failure);
        if (p.observed.include_fakemotion) {
            const auto& sampler = p.observed.fakemotion_sampler;
            p.log.notef("native fakemotion probe graph=%p warm_verified=%d refs_before_close=%d after_close=%d descriptor=%p sampler=%p inspected=%d count=%d source=%d index=%d mode_mask=%08x",
                p.observed.effect_application_identity, p.observed.fakemotion_warm_verified ? 1 : 0,
                p.observed.fakemotion_references_before_cache_release,
                p.observed.fakemotion_references_after_cache_release, sampler.descriptor, sampler.sampler,
                sampler.inspected ? 1 : 0, sampler.count, sampler.texture_source, sampler.index, sampler.mode_mask);
            note_resource(p.log, p.observed.fakemotion_preload);
            note_resource(p.log, p.observed.fakemotion_warm);
            for (std::size_t index = 0; index < p.observed.fakemotion_sampler_call_count; ++index) {
                const auto& call = p.observed.fakemotion_sampler_calls[index];
                p.log.notef("native fakemotion retained sampler observation=%zu before_root=%08x before_mode=%08x builder=%p pass=%p root=%p sampler=%p inspected=%d root_samplers=%d source=%d mode=%p mode_samplers=%d evidence=empty",
                    index, call.following_root_site, call.following_mode_site, call.builder, call.pass,
                    call.root.descriptor, call.root.sampler, call.root.inspected ? 1 : 0, call.root.count, call.root.texture_source,
                    call.mode_descriptor, call.mode_sampler_count);
            }
        }
        if (p.shadow) {
            const auto& shadow = p.shadow->observation();
            p.log.notef("native GUI shadow stage=%u failure_stage=%u allocated=%p returned=%p ctor=%d extent=%u/%u hires=%u",
                static_cast<unsigned>(shadow.stage), static_cast<unsigned>(shadow.failure_stage), shadow.allocated,
                shadow.returned, shadow.constructor_entered ? 1 : 0, shadow.dimensions.width, shadow.dimensions.height,
                static_cast<unsigned>(shadow.hires_shadow));
            if (const auto* operation = p.shadow->constructor_operation())
                p.log.notef("native GUI shadow operation phase=%u site=%08x state=%08x singleton_state=%08x section=%p entered=%d published=%d",
                    static_cast<unsigned>(operation->phase), operation->native_site, operation->exception_state,
                    operation->singleton_exception_state, operation->captured_section,
                    operation->section_entered ? 1 : 0, operation->publication_written ? 1 : 0);
        }
        for (const auto& slot : p.observed.resources) note_resource(p.log, slot);
        if (p.sampler)
            p.log.notef("native GUI sampler parent scopes=%zu procedural_creators=%zu retention=%d",
                p.sampler->live_call_scopes(), p.sampler->live_procedural_creators(),
                p.sampler->requires_process_retention() ? 1 : 0);
        if (p.effects) for (std::size_t index = 0; index < p.effects->operation_count(); ++index) {
            const auto& operation = p.effects->operation(index);
            p.log.notef("native GUI effect op=%zu phase=%u name=%p result=%p lua_binding=%p",
                index, static_cast<unsigned>(operation.phase), operation.input_name, operation.result,
                operation.captured_lua_binding);
            note_exception(p.log, operation.failure);
            note_cache_chain(p.log, operation.cache);
        }
    } catch (...) {
        std::fputs("bsp_game: native GUI effect probe observations incomplete; retain entered graph\n", stderr);
        std::fflush(stderr);
    }
}
} // namespace bsp::game
