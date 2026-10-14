#include "bsp/game_native_font_probe.hpp"
#include "bsp/game_native_font_application.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/gui_startup.hpp"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <new>
#include <stdexcept>
#include <vector>

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4, "native font resource probe requires MSVC Win32");
struct ImageObservation {
    void* identity{}; // Borrowed opaque key after retirement; no post-zero read.
    std::uint32_t profile{};
    std::int32_t count_before{};
    std::size_t font_slots{}; // Expected owned slots, not a replacement refcount.
};
struct ProbeState {
    // Descriptor storage precedes its borrower, so successful destruction is
    // producer first then registry. Failed state is deliberately never deleted.
    FontRegistryStartup registry;
    std::unique_ptr<GameNativeFontApplication> producer;
    std::vector<ImageObservation> images;
    GameNativeFontProbeSummary summary;
    const char* stage{"construct"};
};
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
const char* phase_name(GameNativeFontLoadPhase phase) noexcept {
    switch (phase) {
    case GameNativeFontLoadPhase::fresh: return "fresh";
    case GameNativeFontLoadPhase::gfx: return "gfx";
    case GameNativeFontLoadPhase::alpha: return "alpha";
    case GameNativeFontLoadPhase::data: return "data";
    case GameNativeFontLoadPhase::decode: return "decode";
    case GameNativeFontLoadPhase::adoption: return "adoption";
    case GameNativeFontLoadPhase::publication: return "publication";
    case GameNativeFontLoadPhase::complete: return "complete";
    case GameNativeFontLoadPhase::failed: return "failed";
    }
    return "unknown";
}
void log_exception(GameHostLog& log, const std::exception_ptr& failure) {
    if (!failure) return;
    try { std::rethrow_exception(failure); }
    catch (const std::exception& error) { log.notef("native font probe exception: %s", error.what()); }
    catch (...) { log.note("native font probe exception: nonstandard C++ exception"); }
}
std::int32_t observe_image(void* image, NativeRenderActualOwners& owners,
    std::uint32_t& profile) {
    require(image != nullptr && (reinterpret_cast<std::uintptr_t>(image) & 3u) == 0,
        "Native font probe requires a nonnull aligned actual image");
    // Canonical lookup authenticates live raw+04 before any observation. No
    // native retain/release, COM interface cast or copied counter is installed.
    auto& companion = owners.resolve_actual(image);
    auto* raw_count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(image) + 4));
    require(&companion.reference_count == raw_count,
        "Native font image companion does not alias its actual atomic+04");
    const auto count = raw_count->load();
    require(count > 0, "Native font image has no live actual reference");
    std::memcpy(&profile, image, sizeof(profile));
    require(profile == 0x00d61948 || profile == 0x00d61870 || profile == 0x00d618b0,
        "Native font image has an unsupported actual native texture profile");
    return count;
}
void note_image_slot(ProbeState& state, void* image, NativeRenderActualOwners& owners) {
    for (auto& record : state.images) {
        if (record.identity == image) { ++record.font_slots; return; }
    }
    ImageObservation observation;
    observation.identity = image;
    observation.count_before = observe_image(image, owners, observation.profile);
    observation.font_slots = 1;
    state.images.push_back(observation);
}
[[noreturn]] void retain_failed_probe(GameHostLog& log, ProbeState* retained,
    const std::exception_ptr& failure) noexcept {
    try {
        log.notef("native font probe FAILED stage=%s; retaining producer/registry/renderer/VFS until process exit",
            retained ? retained->stage : "heap-allocation");
        log_exception(log, failure);
        if (retained && retained->producer) {
            log.notef("native font probe retained producer ready=%d retention=%d completed=%zu registry_rows=%zu",
                retained->producer->ready() ? 1 : 0,
                retained->producer->requires_process_retention() ? 1 : 0,
                retained->producer->produced_fonts(), retained->registry.fonts().size());
            // registry_rows bounds the producer's operations; an allocation/name
            // preflight failure may append a descriptor before creating a frame.
            for (std::size_t index = 0; index < retained->registry.fonts().size(); ++index) {
                const GameNativeFontLoadOperation* operation{};
                try { operation = &retained->producer->operation(index); }
                catch (const std::out_of_range&) { break; }
                const auto& op = *operation;
                log.notef("native font probe retained font=%s phase=%s region=%08x gfx_owned=%p alpha_owned=%p gfx_result=%p alpha_result=%p decoded=%p unpublished=%p published=%p gfx_cache=%u/%08x alpha_cache=%u/%08x",
                    op.descriptor ? op.descriptor->name.c_str() : "<no descriptor>", phase_name(op.phase),
                    op.native_site, op.gfx, op.alpha, op.gfx_result, op.alpha_result,
                    static_cast<void*>(op.decoded.get()), static_cast<void*>(op.unpublished.get()),
                    static_cast<const void*>(op.published),
                    static_cast<unsigned>(op.gfx_cache.phase), op.gfx_cache.native_site,
                    static_cast<unsigned>(op.alpha_cache.phase), op.alpha_cache.native_site);
                log.notef("native font probe retained paths gfx=%s alpha=%s data=%s loader_sites=%08x/%08x HRESULT=%08x/%08x creators=%p/%p registered=%d/%d",
                    op.gfx_path.c_str(), op.alpha_path.c_str(), op.data_path.c_str(),
                    op.gfx_cache.loader.native_site, op.alpha_cache.loader.native_site,
                    static_cast<unsigned>(op.gfx_cache.loader.last_hresult),
                    static_cast<unsigned>(op.alpha_cache.loader.last_hresult),
                    op.gfx_cache.loader.creator, op.alpha_cache.loader.creator,
                    op.gfx_cache.loader.registered ? 1 : 0, op.alpha_cache.loader.registered ? 1 : 0);
                log_exception(log, op.failure);
            }
        }
        log.close();
    } catch (...) {
        std::fputs("bsp_game: failed native font probe requires process retention\n", stderr);
        std::fflush(stderr);
    }
    // Same no-unwind exit policy as GameStartupHost's incomplete native owners.
    // The retained heap state and all caller-owned service graphs remain live.
    std::_Exit(1);
}
} // namespace

GameNativeFontProbeSummary probe_actual_native_fonts(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs, GameScriptHost& scripts,
    const std::string& language_font_path) {
    std::unique_ptr<ProbeState> state;
    try {
        state = std::make_unique<ProbeState>();
        state->summary.owners_before = renderer.actual_owners().size();
        log.notef("native font probe begin root=%s descriptor=%s language=%s owners=%zu",
            std::string(kFontRootPrefix).c_str(), std::string(kFontDescriptorPath).c_str(),
            language_font_path.c_str(), state->summary.owners_before);
        state->producer = std::make_unique<GameNativeFontApplication>(log, renderer, vfs, state->registry);
        state->stage = "actual-load";
        state->producer->load_descriptors_00ac3910(scripts.files(), scripts.globals(),
            std::string(kFontRootPrefix), std::string(kFontDescriptorPath), language_font_path);
        state->stage = "resource-inspection";
        state->summary.fonts = state->producer->produced_fonts();
        require(state->producer->ready() && state->summary.fonts > 0 &&
            state->summary.fonts == state->registry.fonts().size(),
            "Native font probe did not produce a nonempty complete descriptor/resource sequence");
        std::vector<const FontData*> sole_allocations; // Metadata identities only.
        auto& owners = renderer.actual_owners();
        for (std::size_t index = 0; index < state->summary.fonts; ++index) {
            const auto& op = state->producer->operation(index);
            const auto& row = *state->registry.fonts().at(index);
            require(op.phase == GameNativeFontLoadPhase::complete && op.published && op.published->live() &&
                op.descriptor == &row.descriptor && row.native_resources == op.published && !row.resources &&
                &op.published->descriptor() == &row.descriptor && !op.decoded && !op.unpublished &&
                !op.gfx && !op.alpha && !op.gfx_name_live && !op.alpha_name_live &&
                op.gfx_cache.phase == NativeTextureCacheAcquired::Phase::complete &&
                op.alpha_cache.phase == NativeTextureCacheAcquired::Phase::complete,
                "Native font probe found incomplete ownership transfer or descriptor/cache identity");
            const auto& data = op.published->data();
            for (const auto* prior : sole_allocations)
                require(prior != &data, "Distinct native fonts unexpectedly share one decoded owner allocation");
            sole_allocations.push_back(&data);
            require(&op.published->actual_owners() == &owners && !data.glyphs.empty() &&
                data.source_record_count >= data.glyphs.size(), "Native font probe found a foreign or empty glyph owner");
            const auto fallback = data.glyphs.find(0x0091);
            require(fallback != data.glyphs.end(), "Native font probe lacks original fallback key0091");
            require(data.space_lf_glyph.gfx_texture_18 == op.gfx_result &&
                data.space_lf_glyph.alpha_texture_1c == op.alpha_result &&
                data.carriage_return_glyph.gfx_texture_18 == op.gfx_result &&
                data.carriage_return_glyph.alpha_texture_1c == op.alpha_result,
                "Native font embedded ownership/alias slots differ from the actual load results");
            for (const auto& glyph : data.glyphs)
                require(glyph.second.gfx_texture_18 == op.gfx_result && glyph.second.alpha_texture_1c == op.alpha_result,
                    "Native font glyph aliases differ from the actual image owner pair");
            const auto& missing = data.missing_glyph;
            const auto& original = fallback->second;
            require(std::memcmp(missing.fields_00_0c.data(), original.fields_00_0c.data(), sizeof(float) * 4) == 0 &&
                missing.field_10 == original.field_10 && missing.scaled_field_12 == original.scaled_field_12 &&
                missing.scaled_field_14 == original.scaled_field_14 &&
                missing.gfx_texture_18 == original.gfx_texture_18 && missing.alpha_texture_1c == original.alpha_texture_1c,
                "Native font known fallback fields differ from their original glyph");
            std::uint32_t gfx_profile{}, alpha_profile{};
            const auto gfx_count = observe_image(op.gfx_result, owners, gfx_profile);
            const auto alpha_count = observe_image(op.alpha_result, owners, alpha_profile);
            note_image_slot(*state, op.gfx_result, owners);
            note_image_slot(*state, op.alpha_result, owners);
            state->summary.glyph_records += data.glyphs.size();
            log.notef("native font probe font=%s records=%u glyphs=%zu height=%d data=%p gfx=%p/%08x/ref%d alpha=%p/%08x/ref%d fallback0091=verified",
                row.descriptor.name.c_str(), data.source_record_count, data.glyphs.size(),
                op.published->signed_height_14(), static_cast<const void*>(&data), op.gfx_result, gfx_profile, gfx_count,
                op.alpha_result, alpha_profile, alpha_count);
        }
        // Observe counts once after all slot aggregation. Counts are native+04;
        // font_slots is solely a lower-bound/delta expectation for this probe.
        for (auto& image : state->images) {
            image.count_before = observe_image(image.identity, owners, image.profile);
            require(static_cast<std::size_t>(image.count_before) >= image.font_slots,
                "Actual image count is below the probe's returned owning font slots");
        }
        state->summary.unique_images = state->images.size();
        state->summary.resources_verified = true;
        state->stage = "actual-retirement";
        state->producer->retire_fonts(); // No native_fonts export and no Text factory.
        state->stage = "retirement-inspection";
        require(!state->producer->requires_process_retention(), "Completed native font retirement still requires retention");
        for (const auto& row : state->registry.fonts())
            require(!row->native_resources && !row->resources, "Retired registry retained an actual/semantic resource binding");
        for (const auto& image : state->images) {
            const auto expected = image.count_before - static_cast<std::int32_t>(image.font_slots);
            auto* survivor = owners.find(image.identity); // Metadata lookup, no raw post-zero read.
            if (expected == 0) {
                require(!survivor, "Zero-reference native font image did not retire from its canonical registry");
                log.notef("native font probe image=%p profile=%08x refs=%d->retired slots=%zu",
                    image.identity, image.profile, image.count_before, image.font_slots);
            } else {
                require(survivor != nullptr && survivor->reference_count.load() == expected,
                    "Surviving actual font image count did not release the probe's owning slots");
                log.notef("native font probe image=%p profile=%08x refs=%d->%d slots=%zu",
                    image.identity, image.profile, image.count_before, expected, image.font_slots);
            }
        }
        state->summary.owners_after = owners.size(); // Lazy cache fallback may survive.
        state->summary.retired = true;
        log.notef("native font probe PASS fonts=%zu glyphs=%zu images=%zu owners=%zu->%zu actual-retirement=verified rendering=unverified",
            state->summary.fonts, state->summary.glyph_records, state->summary.unique_images,
            state->summary.owners_before, state->summary.owners_after);
        return state->summary;
    } catch (...) {
        auto failure = std::current_exception();
        retain_failed_probe(log, state.release(), failure);
    }
}
} // namespace bsp::game
