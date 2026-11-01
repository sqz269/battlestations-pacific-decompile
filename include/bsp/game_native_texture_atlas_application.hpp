#pragma once
#include "bsp/native_texture_atlas_deletion_lifetime.hpp"
#include "bsp/native_texture_atlas_registry_storage.hpp"
#include "bsp/native_texture_atlas_loader_storage.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>

namespace bsp {
struct NativePathCanonicalizerRawContext;
struct NativeStringRawPoolContext;
namespace game {
class GameNativeReadOnlyData;
class GameSingletonHost;
class GameStartupHost;
class GameNativeVfsApplication;
class GameNativeRendererApplication;

// One reached AF0060 call, retained before any Native effect. Ordinary backing
// for the internal common.ats request/HOME is separate from its frame/private
// homes and every provider-write extent. Other requests borrow the caller's
// actual initialized DWORD HOME and raw8/payload, retained on every escape.
// There is no destructor cleanup, reset, replay or inferred successful load.
struct GameNativeTextureAtlasLoadCall final {
    GameNativeTextureAtlasLoadCall() {}
    GameNativeTextureAtlasLoadCall(const GameNativeTextureAtlasLoadCall&) = delete;
    GameNativeTextureAtlasLoadCall& operator=(const GameNativeTextureAtlasLoadCall&) = delete;
    std::uint32_t common_request[2], common_request_home;
    NativeTextureAtlasLoaderFrame loader;
    volatile std::uint32_t* actual_request_home{};
    void* captured_manager{};
    bool is_common{}, request_constructor_started{}, request_constructor_returned{};
    bool loader_started{}, loader_returned{}, request_return_started{}, request_returned{};
    std::exception_ptr failure;
};
struct GameNativeTextureAtlasDeleteCall final {
    GameNativeTextureAtlasDeleteCall() {}
    GameNativeTextureAtlasDeleteCall(const GameNativeTextureAtlasDeleteCall&) = delete;
    GameNativeTextureAtlasDeleteCall& operator=(const GameNativeTextureAtlasDeleteCall&) = delete;
    std::uint32_t flags_home; // initialized ONCE from the full captured flags
    NativeTextureAtlasRegistryScalarFrame scalar;
    void* captured_receiver{};
    void* opaque_return{}; // never dereferenced after scalar returns
    bool scalar_started{}, scalar_returned{};
    std::exception_ptr failure;
};

// One deliberately retained Source process-role owner. Original F8C26C starts
// as zero, F8C270 is an initial NUL role, and AF5740 writes shared F8C2C8. These
// Source objects have ordinary allocated addresses: NO Original VA reservation,
// binary identity, original scratch declaration/capacity or overflow is claimed.
//
// The caller must select a NONZERO explicit scratch extent before any consumer
// borrows it. There is no default. Every AF5740 consumer in the application must
// borrow THIS exact storage and publication; no parallel role bank is admitted.
// Prove every reached normalized-line write including NUL fits that extent for
// the ACTUAL loaded buffers before activation. No clamp, resize, fallback buffer
// or line-acceptance policy is added. Calls are externally serialized/nonreentrant.
class GameNativeTextureAtlasApplication final : public NativeTextureAtlasDeletionLifetime {
public:
    // One attempt per process, including an allocation failure. Metadata and
    // role allocation only; no manager, string, renderer or atlas is constructed.
    // Subsequent access uses process_roles(), never repeats this operation.
    static GameNativeTextureAtlasApplication& create_process_roles_once(
        std::size_t explicit_source_line_scratch_extent);
    static GameNativeTextureAtlasApplication& process_roles();
    GameNativeTextureAtlasApplication(const GameNativeTextureAtlasApplication&) = delete;
    GameNativeTextureAtlasApplication& operator=(const GameNativeTextureAtlasApplication&) = delete;
    GameNativeTextureAtlasApplication(GameNativeTextureAtlasApplication&&) = delete;
    GameNativeTextureAtlasApplication& operator=(GameNativeTextureAtlasApplication&&) = delete;

    void* volatile& atlas_publication_00f8c26c() noexcept { return atlas_publication_; }
    const char* empty_name_00f8c270() const noexcept { return empty_name_; }
    // Distinct initial-NUL F8C2C1 role in this SAME permanent process owner.
    // It aliases neither the F8C270 empty name nor F8C2C8 scratch.
    const char* empty_atlas_stem_00f8c2c1() const noexcept { return empty_atlas_stem_; }
    char* shared_line_scratch_00f8c2c8() noexcept { return scratch_.get(); }
    std::size_t source_line_scratch_extent() const noexcept { return scratch_extent_; }

    // Cold one-attempt composition of the SAME real host/VFS/RO/pool/live
    // feature/canonicalizer/renderer graph. The existing renderer owns the ONE
    // actual-owner registry and genuine NativeTextureLoadOwners. The Root-owned
    // ready-only atlas_process_texture_cache supplier checks that renderer's
    // private normal birth without inventing parser dispatch observations. Import args
    // borrow actual typed callable IAT cells; the caller retains these exact
    // references, all code/providers and this whole graph through shared drain.
    // No graph constructor, cache acquisition or publication occurs here.
    // The actual heap Root must prove it owns THIS host/data/VFS/renderer.
    // Its negative marker is installed before throwing binding admission.
    void bind_graph_once(GameStartupHost&, GameSingletonHost&, GameNativeReadOnlyData&,
        GameNativeVfsApplication&, GameNativeRendererApplication&,
        const volatile std::uint32_t& actual_feature_0109eea4,
        NativePathCanonicalizerRawContext&,
        NativeTextureAtlasRegistrySectionCall const volatile& actual_enter_iat_00ce2218,
        NativeTextureAtlasRegistrySectionCall const volatile& actual_leave_iat_00ce2210,
        NativeTextureAtlasRegistryDecrement const volatile& actual_decrement_iat_00ce2220);

    // Actual BF681B(1Ch) is adopted before the complete 737EE0/7364A0 call.
    // Private normal allocation+constructor history establishes this source
    // birth; an empty atlas, count, profile or external pointer cannot adopt it.
    void construct_normal();
    // Genuine AF0060 on interface/textures/common.ats through the retained raw
    // VFS/parser/renderer graph, distinct from construction. Invoke BEFORE GUI.
    // A normal return is only AF0060 completion: enumeration can find nothing.
    // The record exposes actual reached descriptor/parser/creator history for
    // later review. This method supplies NO blanket Page/readiness permission.
    void load_common_normal();
    // Fresh retained frame for each later reached loader call; no common.ats
    // substitution and no reset/retry. Caller retains incoming HOME/raw8/data
    // after an escape and proves all scratch/Native extents/provider contracts.
    void load_normal(volatile std::uint32_t& actual_requested_header_home);
    const GameNativeTextureAtlasLoadCall* common_load_record() const noexcept;

    // Explicit normal schedule only. Arm while the genuine renderer/provider
    // graph is ready, before beginning its shared reverse drain; no Native work
    // or derived cleanup permission occurs while arming. Shared deletion then
    // uses the captured popped owner, independent of CURRENT F8C26C identity.
    // retire_normal() instead executes this same real scalar flags1 explicitly;
    // the full normal body unregisters/clears/releases/frees in native order.
    // Binding also installs the distinct actual Host-owned atlas manager
    // attempt. The real core arms it before every throwing admission/effect;
    // unknown Source escape skips BD0220 and reaches the real Root _Exit guard.
    // Explicit retire_normal() remains outside the shared-manager wrapper.
    void prepare_normal_retirement();
    void retire_normal();
    void* delete_registered_00737f10(void*, std::uint32_t full_flags) override;
    const NativeTextureAtlasRegistryConstructFrame* construction_record() const noexcept;
    const GameNativeTextureAtlasDeleteCall* deletion_record() const noexcept;

private:
    friend class GameStartupHost;
    bool requires_host_retention() const noexcept;
    void note_host_exception() noexcept;
    void after_host_shared_drain(GameSingletonHost&);
    explicit GameNativeTextureAtlasApplication(std::size_t);
    ~GameNativeTextureAtlasApplication() = default; // never invoked by process owner
    struct Impl;
    void* volatile atlas_publication_{};
    char empty_name_[1]{};
    const std::size_t scratch_extent_;
    std::unique_ptr<char[]> scratch_;
    bool bind_attempted_{};
    std::exception_ptr bind_failure_;
    Impl* graph_{}; // intentionally permanent; never destructs partial Native debt
    // Appended Source storage preserves the earlier role declarations.
    // Original F8C2C1 begins in the supported PE virtual .data zero tail.
    char empty_atlas_stem_[1]{};
};
} // namespace game
} // namespace bsp
