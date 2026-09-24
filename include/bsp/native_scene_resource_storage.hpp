#pragma once
#include "bsp/native_ambient_lifetime.hpp"
#include "bsp/native_scene_registry_storage.hpp"
#include "bsp/native_scene_resource_ambient.hpp"
#include "bsp/native_string.hpp"
#include <optional>

namespace bsp {
struct NativeSceneResourceStorageContext {
    NativeStringRawPoolContext& strings;
    NativeSceneRegistryStorageBindings& registry_storage;
    NativeSceneResourceAmbientContext& ambient;
    // Must use exactly ambient.owners. Metadata admission borrows actual+4,
    // adds no count or native stores, and has no successful/no-op fallback.
    NativeAmbientIdentityContext& ambient_identity;
    const void* one_00d7a24c;
};
struct NativeSceneResourceConstructFrame {
    // Actual incoming name-pointer cell, captured AFTER base/count writes.
    // B83CFA overwrites this SAME word with the ambient allocation; CC23AE
    // reads its current value if native constructor state3 needs raw free.
    volatile std::uint32_t name_or_ambient_argument;
    // Native local bytes are never initialized by B83C50. Caller supplies
    // actual initialized preimages; first is passed as B83600's used byte.
    unsigned char allocator_first;
    unsigned char allocator_second;
    NativeSceneRegistryConstructScratch registry;
    volatile std::uint32_t requested_ambient_argument;
    NativeSceneResourceAmbientFrame setter;
};
struct NativeSceneResourceAcquired {
    enum class Failure { none, native_call, host_metadata_admission };
    bool started{};
    bool complete{};
    bool creator_decrement_completed{};
    std::uint32_t active_call_site{};
    std::int32_t native_eh_state{-1};
    Failure failure{Failure::none};
    void* raw_ambient{};
    void* ambient_creator{};
    // Caller-owned, address-stable canonical metadata. Keep this record AND
    // contexts alive until final ambient retirement and external quiescence,
    // even after resource destruction. Not another native ownership credit.
    // On metadata failure, raw completed ambient remains for explicit recovery;
    // on later failure the native prefix cleanup does NOT release that creator.
    std::optional<NativeAmbientReference> ambient_reference;
};

// Full253B B83C50 normal schedule on caller-owned actual3Ch storage:
// profile0/count4/name8,C/ambient10/registry14..3B. Exact raw name copy,
// registry construction, new98h/B7C290, setter and CAPTURED creator decrement.
// Fresh Acquired/frame/contexts must be stable, disjoint from owner storage and
// each other except explicit caller-cell aliases described above. Constructor
// name may alias owner+8; both header words are still cleared before skipping.
// Name/pool callbacks use current fields. No semantic scene projection occurs.
void* construct_native_scene_resource_00b83c50(void* actual_resource,
    NativeSceneResourceConstructFrame&, NativeSceneResourceStorageContext&,
    NativeSceneResourceAcquired&);
// Complete178B B82ED0. Capture/release actual ambient through current import
// and current0 canonical dispatch, clear+10 after callback, current vector
// free/reset then list; captured name block/size return; base CEB130. A surviving
// ambient's borrowed backlink is deliberately NOT removed by this destructor.
void destroy_native_scene_resource_00b82ed0(void* actual_resource,
    NativeSceneResourceStorageContext&);
// Complete30B B83410; late low-byte flags read AFTER destructor, same CRT free
// only for bit0. Return numeric identity; no owner reads after possible free.
void* delete_native_scene_resource_00b83410(void* actual_resource,
    const volatile std::uint32_t& actual_flags_slot,
    NativeSceneResourceStorageContext&);

struct NativeSceneResourceIdentityContext {
    NativeSceneResourceStorageContext& storage;
    const volatile std::uint32_t* resource_profile_00d63168;
};
// Separate postconstruction metadata, not part of B83C50. Bind the SAME actual
// +4 atomic in storage.ambient.owners; require CURRENT D63168[0/4] to be
// BD30E0/B83410. No copied count, retain or enlarged3Ch allocation. Contexts and
// metadata survive terminal/explicit deletion; retire without dead owner reads.
class NativeSceneResourceReference final : public RenderCommandReference {
public:
    NativeSceneResourceReference(void*, NativeSceneResourceIdentityContext&);
    ~NativeSceneResourceReference() override;
    NativeSceneResourceReference(const NativeSceneResourceReference&) = delete;
    NativeSceneResourceReference& operator=(const NativeSceneResourceReference&) = delete;
    void release_zero_references() noexcept override;
    void* delete_scalar_00b83410(const volatile std::uint32_t&);
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeSceneResourceIdentityContext& context_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};
// New source ABI and C++ cleanup projection. Native FH3/SEH/private EBP frames,
// logical GuiSceneOwner/SceneAttachmentRuntime closure and gameplay excluded.
} // namespace bsp
