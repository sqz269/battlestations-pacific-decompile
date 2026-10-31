#pragma once
#include "bsp/native_model_storage_lifetime.hpp"
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_model_resource_storage.hpp"
#include <cstdint>
#include <exception>

namespace bsp {
// Same real initialized01090390/raw node/string/owner/import/constant domain.
// Context construction does not read any factory input HOME or object payload.
class NativeMatrixStorageContext final {
public:
    explicit NativeMatrixStorageContext(NativeMatrixModelStorageContext& prefix) noexcept
        : prefix_(prefix) {}
    NativeMatrixStorageContext(const NativeMatrixStorageContext&) = delete;
    NativeMatrixStorageContext& operator=(const NativeMatrixStorageContext&) = delete;
    NativeMatrixModelStorageContext& prefix() const noexcept { return prefix_; }
private:
    NativeMatrixModelStorageContext& prefix_;
};
struct NativeMatrixStorageAccess;
class NativeMatrixStorageOwner;
class NativeMatrixStorageFrame final {
public:
    enum class Phase { fresh, creating, live, destroying, payload_dead, returned, null_result, failed };
    struct Progress {
        Phase phase;
        void* actual; // identity only after a return; never a late payload borrow
        void* captured_bones_data; // historical address only after free
        bool poisoned, allocation_entered, allocation_returned, name_captured;
        bool constructor_entered, constructor_returned, destructor_entered, destructor_returned;
        bool resize_entered, resize_returned, free_entered, free_returned;
        bool normal_base_entered, normal_base_returned;
        bool selected_base_entered, selected_base_returned;
        bool base_payload_started, base_payload_complete;
        bool factory_return_entered, factory_return_returned;
        bool flags_read, scalar_returned, pool_return_entered, pool_returned;
        std::uint8_t flags_low_byte;
        bool prefix_live, bones_live;
        bool creator_issued, creator_transferred, creator_release_entered;
        bool creator_release_returned, creator_decrement_completed, disposition_entered;
        const void* source_owner; // opaque historical identity, never a live receiver borrow
        bool creator_receiver_ended;
    };
    NativeMatrixStorageFrame() noexcept = default;
    ~NativeMatrixStorageFrame() = default; // metadata only; no native/extent cleanup
    NativeMatrixStorageFrame(const NativeMatrixStorageFrame&) = delete;
    NativeMatrixStorageFrame& operator=(const NativeMatrixStorageFrame&) = delete;
    NativeMatrixStorageFrame(NativeMatrixStorageFrame&&) = delete;
    NativeMatrixStorageFrame& operator=(NativeMatrixStorageFrame&&) = delete;
    Progress progress() const noexcept;
private:
    friend struct NativeMatrixStorageAccess;
    const NativeMatrixStorageContext* context_{};
    Phase phase_{Phase::fresh};
    void* actual_{};
    void* captured_bones_data_{};
    NativeMatrixModelPrefixExtent prefix_extent_;
    NativeRenderPointerArrayStorage* bones_{};
    NativeMatrixModelStorageConstructAcquired prefix_construct_;
    NativeModelStorageLifetimeFrame base_frame_{};
    NativeModelStorageLifetimeAcquired base_;
    std::exception_ptr primary_, secondary_;
    bool poisoned_{}, bones_live_{}, factory_edge_{}, base_edge_{};
    bool allocation_entered_{}, allocation_returned_{}, name_captured_{};
    bool constructor_entered_{}, constructor_returned_{}, destructor_entered_{}, destructor_returned_{};
    bool resize_entered_{}, resize_returned_{}, free_entered_{}, free_returned_{};
    bool normal_base_entered_{}, normal_base_returned_{};
    bool selected_base_entered_{}, selected_base_returned_{};
    bool factory_return_entered_{}, factory_return_returned_{};
    bool flags_read_{}, scalar_returned_{}, pool_return_entered_{}, pool_returned_{};
    std::uint8_t flags_low_byte_{};
    // Issued only by the actual completed B91220. Public progress/count/pool
    // membership cannot create, transfer or clear any of these private facts.
    struct CreatorFacts {
        NativeNodeStorage* node{};
        std::atomic<std::int32_t>* count{};
        NativeMatrixStorageOwner* receiver{};
        const void* receiver_identity{}; // diagnostic identity only; never dereferenced
        bool issued{}, transferred{}, release_entered{}, release_returned{};
        bool decrement_completed{}, release_active{}, disposition_entered{};
        bool receiver_ended{};
    } creator_;
    NativeNodeTreeRetirementFrame* creator_release_frame_{};
    NativeNodeTreeRetirementAcquired creator_release_; // private actual lower-call receipt
    std::uint32_t geometry_depth_{};
};

// Specific Source custody receiver for this factory's creator obligation. The
// private once-only move has NO Native retain/decrement/publication effect.
// Keep owner/frame/context and all affected records address-stable to actual
// physical disposition; an interrupted/flags0 owner requires process retention.
// The actual Frame and Context also outlive the COMPLETE Owner C++ destructor:
// legal settled end retires its private backpointer in the still-live Frame.
// No destructor cleanup, generic capability, public receipt setter or canonical
// is synthesized. Its exact release invokes the genuine full B6F310 provider.
class NativeMatrixStorageOwner final {
public:
    ~NativeMatrixStorageOwner() noexcept;
    NativeMatrixStorageOwner(const NativeMatrixStorageOwner&) = delete;
    NativeMatrixStorageOwner& operator=(const NativeMatrixStorageOwner&) = delete;
    NativeMatrixStorageOwner(NativeMatrixStorageOwner&&) = delete;
    NativeMatrixStorageOwner& operator=(NativeMatrixStorageOwner&&) = delete;
    void* capture_current_raw_scene_170();
    void set_geometry_00b75170(const NativeMatrixStorageGeometryContext&,
        NativeModelStorageGeometryFrame&, NativeModelStorageGeometryAcquired&);
    void release_creator_00b6f310(NativeNodeTreeRetirementFrame& actual_argument_frame);
    void destroy_00b91280();
    void* delete_00b915a0(const volatile std::uint32_t& actual_flags_home);
    void return_slot();
    bool requires_process_retention() const noexcept { return !disposition_complete_; }
private:
    friend struct NativeMatrixStorageAccess;
    friend NativeMatrixStorageOwner transfer_native_matrix_creator(
        const NativeMatrixStorageContext&, NativeMatrixStorageFrame&);
    NativeMatrixStorageOwner(const NativeMatrixStorageContext&, NativeMatrixStorageFrame&);
    const NativeMatrixStorageContext& context_;
    NativeMatrixStorageFrame& frame_;
    bool disposition_complete_{};
};

NativeMatrixStorageOwner transfer_native_matrix_creator(
    const NativeMatrixStorageContext&, NativeMatrixStorageFrame&);
// Original factory-frame custody is usable before a Source transfer; after the
// move only the exact receiver can use creator/disposition entries. Four input
// Word HOMEs and genuine geometry service backing stay live through callbacks.
void set_native_matrix_creator_geometry_00b75170(const NativeMatrixStorageContext&,
    NativeMatrixStorageFrame&, const NativeMatrixStorageGeometryContext&,
    NativeModelStorageGeometryFrame&, NativeModelStorageGeometryAcquired&);
void release_native_matrix_creator_00b6f310(const NativeMatrixStorageContext&,
    NativeMatrixStorageFrame&, NativeNodeTreeRetirementFrame& actual_argument_frame);

// Complete B931F0: actual initialized4B Source pointer HOME is sampled ONLY
// after a nonnull actual allocation. NULL skips that HOME and the constructor.
// The pointed-to real raw8 header is borrowed through the synchronous ctor.
void* create_native_matrix_storage_00b931f0(const void* const volatile& actual_name_pointer_home,
    const NativeMatrixStorageContext&, NativeMatrixStorageFrame&);
// Complete B91220 producer entry; only the real factory's private allocated
// phase can enter it. No default frame or arbitrary/prepared owner adoption.
void* construct_native_matrix_storage_00b91220(const void* captured_name_header,
    const NativeMatrixStorageContext&, NativeMatrixStorageFrame&);
// Complete B91280 normal/selected payload; same Matrix base admission, no pool
// return. Success leaves physical slot custody with this frame/caller.
void destroy_native_matrix_storage_00b91280(
    const NativeMatrixStorageContext&, NativeMatrixStorageFrame&);
// Complete B915A0: CURRENT actual flags LOW BYTE after destruction, then
// no-store typed completion, conditional SAME Matrix B90150 and identity return.
// Flags may alias still-live payload; never contexts/frame/diagnostics.
void* delete_native_matrix_storage_00b915a0(const volatile std::uint32_t& actual_flags_home,
    const NativeMatrixStorageContext&, NativeMatrixStorageFrame&);
// Source-only explicit disposition of a normally destroyed flags0/direct slot.
// No destructor retry; only the matched B90150 return. No new Native credit.
void return_native_matrix_storage_slot(
    const NativeMatrixStorageContext&, NativeMatrixStorageFrame&);

// Caller retains nonmoving frame/context, real pool, actual allocation and all
// reached graph/CRT/import/backing until genuine disposition. No asynchronous
// reuse, metadata overlap, count repair, rollback, scene/type/mission selection
// or default provider. Rejected reentry is outside owning cleanup scopes;
// entered failures are sticky. Late progress/destruction reads metadata only.
// C++ cleanup is the existing qualified Source projection, NOT Original
// private HOME/FH3/processingThrow/SEH/allocator/runtime compatibility.
} // namespace bsp
