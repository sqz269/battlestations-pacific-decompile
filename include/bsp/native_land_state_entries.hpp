#pragma once
#include "bsp/native_land_task_constructor.hpp"
#include "bsp/observer_lifetime.hpp"
#include "bsp/plane_squadron_host.hpp"

namespace bsp {

// Borrow actual constructed state fields; no defaults, owned sidecars,
// endpoint construction, layout overlays or lifetime extension.
struct NativeLandParkEntryView {
    float volatile& field_1c;
    std::uint8_t volatile& flag_18;
    float volatile& timer_28;
};
struct NativeLandFollowEntryView {
    const void* const volatile& approach_04;
    void* const volatile& parameters_6c;
    const void* volatile& watched_leader_2c;
    NativeObserverOwnerStorage& callback_18;
    float volatile& field_94;
    float volatile& field_90;
    float volatile& amplitude_88;
    std::uint8_t volatile& flag_84;
    float volatile& field_8c;
    std::uint8_t volatile& flag_85;
};
struct NativeLandFollowSquadronView {
    PlaneSquadronEntity& entity;
    std::uint32_t volatile& formation_shape_3e4;
    const void* const volatile& leader_3d0;
};

class NativeLandStateEntryServices : public PlaneSquadronPromotionReceiptHost {
   public:
    // All mappings below are REQUIRED PURE identity/address-to-borrowed-field
    // mappings: no represented field reads, callbacks, native calls, allocation,
    // default receiver/profile or translated-pointer caches. Objects/fields
    // remain live through the corresponding segment, including observer work.
    virtual NativeLandParkEntryView park_entry_fields(const void* state) = 0;
    virtual NativeLandFollowEntryView follow_entry_fields(const void* state) = 0;
    virtual const void* const volatile& state_profile_cell_00(const void* state) = 0;
    virtual const void* const volatile& approach_squadron_cell_0c(
        const void* approach) = 0;
    virtual NativeLandFollowSquadronView squadron_entry_fields(
        const void* squadron) = 0;
    virtual const float volatile* parameter_amplitude_address_08(
        const void* parameters) = 0;
    virtual NativeObserverOwnerStorage& plane_observed_endpoint(
        const void* plane) = 0;
    virtual NativeObserverLifetime& observer_lifetime() = 0;

    // REQUIRED complete actual singleton acquisition, then its +380h block
    // identity. State+6Ch is read AFTER this call; it is NOT assigned here.
    virtual const void* follow_tuning_block_380_0042e740() = 0;
    // REQUIRED COMPLETE009BE150: ECX=existing destination block, stack source,
    // RET4. Forty ordered x87 FLD/FSTP float lanes and bytes68/69; no raw
    // memcpy, pointer replacement, invented provider stub or fresh/default block.
    virtual void copy_follow_parameters_009be150(void* destination,
        const void* source) = 0;
    // Inherited assign_formation_indices_007ed260 is REQUIRED COMPLETE on the
    // actual retained squadron, including its real field/provider effects.
};

// Complete ordinary entries; new SOURCE APIs, not original binary entries.
// Follow: ECXstate,RET,preservesEBX/ESI/EBP/EDI. Existing6C copy ->reload6C,
// capturestate4 ->94/90zero ->actual+8 FLD/FSTP88 ->84zero/8Cone ->captured
// approachC. Nonnull firstC:3E4=1/reindex ->FRESHstate4/C/leader3D0; preserve
// captured newleader across old unregister,2Cstore,new register. Clear85 LAST.
// Actual borrowed callback18 and old/new observed endpoints/global lifetime
// context must remain valid throughout. No current plane membership or queue
// lookup is supplied. State4,6C,tuning380 and all accessed field addresses are
// valid nonnull; only the explicitly guarded approachC/leader values may be null.
// Admit masked FP exceptions/no pendingunmasked exception
// and one available x87 slot; ordinary returns/no structural alias invalidation,
// reentry, concurrency, fault/unwind or observer/death lifetime guarantee.
void enter_native_land_follow_009bed80(
    const NativeLandFollowEntryView&, NativeLandStateEntryServices&);
// Park: ECXstate,RET; exact positivezero1C, byte18zero, binary32three28 stores.
void enter_native_land_park_009b21a0(const NativeLandParkEntryView&) noexcept;
// MoveTo007B3DB0 is proved one-byte RET.
void enter_native_land_moveto_007b3db0() noexcept;

// REQUIRED nonnull actual executable tables with these known entry+4 targets.
// No fabricated address literals/default profile or metadata-only table.
struct NativeLandInitialExecutableProfiles {
    const void* moveto_00d20aec; // +4 ->007B3DB0
    const void* follow_00d20ab8; // +4 ->009BED80
    const void* park_00d1ff60;   // +4 ->009B21A0
};

// Opt-in CONNECTED adoption for construct_native_land_task_009b3240. Borrow
// its SAME view and actual profiles; final entry dispatch reads actual state
// profile cell and supports only its matched MoveTo4C4/Follow500/Park620.
// All base/composite/owner/control/leader/entry-service methods remain abstract.
// Unsupported identity/profile is an excluded domain and reports a source
// logic error, never a no-op/default entry. No arena/provider/game adapter.
class NativeLandTaskStateEntryConstructorCalls
    : public NativeLandTaskConstructorCalls, public NativeLandStateEntryServices {
   public:
    NativeLandTaskStateEntryConstructorCalls(const NativeLandTaskConstructorView&,
        NativeLandInitialExecutableProfiles) noexcept;
    void enter_state_04(const void* state) final;

   private:
    const NativeLandTaskConstructorView& task_;
    const NativeLandInitialExecutableProfiles profiles_;
};

} // namespace bsp
