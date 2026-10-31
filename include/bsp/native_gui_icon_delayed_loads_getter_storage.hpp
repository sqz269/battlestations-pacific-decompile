#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Icon delayed getter storage requires MSVC Win32.
#endif

namespace bsp {
struct NativeGuiIconDelayedLoadsStorage;

// Ordinary caller-retained eight-byte guard backing, no initializers. The slow
// path writes CE37FC and the captured first manager section. Fast return leaves
// both preimages untouched. No C++ owner/lock policy or cleanup authority lives
// here; this is not an Original private-stack/alias/exception ABI declaration.
struct NativeGuiIconDelayedLoadsGetterStorageFrame {
    std::uint32_t guard_profile_home;
    std::uint32_t guard_section_home;
};
static_assert(sizeof(NativeGuiIconDelayedLoadsGetterStorageFrame) == 8);

// Fresh, retained, nonmoving Source observations only. A reached/returned call,
// pointer observation or phase never authenticates a Native exception, created
// owner, actual manager insertion, replacement generation or physical release.
// Only this whole raw entry writes the record. No reset/replay/setter exists.
class NativeGuiIconDelayedLoadsGetterStorageAcquired final {
public:
    enum class Phase { fresh, entered, complete, unknown_source_escape };
    struct Observation {
        Phase phase{Phase::fresh};
        bool initial_read{}, fast_return{};
        std::uint32_t initial_publication_bits{};
        bool first_manager_entered{}, first_manager_returned{}, section_captured{};
        std::uint32_t first_manager_bits{}, captured_first_section_bits{};
        bool guard_homes_written{}, enter_entered{}, enter_returned{};
        bool depth_incremented{}, state0_reached{}, publication_rechecked{};
        std::uint32_t rechecked_publication_bits{};
        bool allocation_entered{}, allocation_returned{}, object_stores_completed{};
        std::uint32_t allocated_bits{};
        bool publication_stored{}, second_manager_entered{}, second_manager_returned{};
        std::uint32_t second_manager_bits{};
        bool registration_argument_captured{}, registration_entered{}, registration_returned{};
        std::uint32_t pushed_registration_argument_bits{};
        bool depth_decremented{}, leave_entered{}, leave_returned{};
        bool result_captured{};
        std::uint32_t returned_publication_bits{};
    };
    NativeGuiIconDelayedLoadsGetterStorageAcquired() = default;
    NativeGuiIconDelayedLoadsGetterStorageAcquired(
        const NativeGuiIconDelayedLoadsGetterStorageAcquired&) = delete;
    NativeGuiIconDelayedLoadsGetterStorageAcquired& operator=(
        const NativeGuiIconDelayedLoadsGetterStorageAcquired&) = delete;
    NativeGuiIconDelayedLoadsGetterStorageAcquired(
        NativeGuiIconDelayedLoadsGetterStorageAcquired&&) = delete;
    NativeGuiIconDelayedLoadsGetterStorageAcquired& operator=(
        NativeGuiIconDelayedLoadsGetterStorageAcquired&&) = delete;
    const Observation& observation() const { return observation_; }
private:
    friend NativeGuiIconDelayedLoadsStorage*
        get_native_gui_icon_delayed_loads_storage_00ab5700(
            NativeGuiIconDelayedLoadsStorage* volatile&, void* volatile&,
            NativeGuiIconDelayedLoadsGetterStorageFrame&,
            NativeGuiIconDelayedLoadsGetterStorageAcquired&);
    Observation observation_{};
};

// Whole NORMAL AB5700, new ordinary Source C++ interface, not Original no-input
// cdecl/EAX/RET compatibility. Borrow SAME genuine F8BD98 typed publication and
// 01090AA0 manager cell; service Impl can forward its existing private refs.
// No process writable getter, replacement bank, owner registry or feature word
// is created. The raw string context's actual manager-cell ref is sufficient.
//
// Initial nonnull return is captured; slow return rereads AFTER normal leave.
// Preserve nullable allocation, four real object stores, current publication
// after the SECOND manager getter, and FIRST captured section through leave.
// No profile/count/cap/identity/null/overflow filter is installed. A registered
// argument observation is the PUSH value, not proof of what BD0C30 appended.
//
// Caller supplies distinct fresh Acquired and live ordinary Frame backing with
// authentic preimages; cold containing storage must not value-zero the Frame.
// Keep both records and the SAME cells alive; managers, actual Win32 sections/
// counter+18 and allocation backing must be valid at every reached access and
// through required failure disposition. Pointer-only created/pushed/returned
// observations are numeric identities: the getter does not inspect, authenticate
// or protect those pointees. A later consumer separately proves current-object
// liveness/typed birth/custody. Retain the remaining reached graph on unknown
// failure without undoing genuine callee effects. Distinct entered invocations
// need distinct records. Frame, Acquired and Source argument homes are mutually
// disjoint and separate from both actual cells and OTHER Native write/payload/
// callee-private extents. The two genuine cells are distinct and writable where
// their Native bodies require; the explicit Frame guard stores are preserved.
// No T[n] family, created/current ownership equivalence, fake companion, retain
// or protective count follows from a pointer/profile/publication/registration.
//
// ANY escaping Source C++ exception records UNKNOWN and rethrows. No local
// RAII leave, EH411EE0 action, free, rollback, retry or termination is added.
// A possibly held first section/depth and reached partial graph remain debt;
// phase/observations do not grant cleanup. Dependencies retain their existing
// qualified Source exception effects; no Native Origin/FH3/SEH/fault/CRT or
// NoThrow promise is inferred. Hardware/foreign exceptions, Original private
// stack/alias/EFLAGS/ABI, actual consumer/owner admission and runtime remain
// separate. Existing scalar/cancel/default/noexcept routes stay unchanged.
NativeGuiIconDelayedLoadsStorage*
get_native_gui_icon_delayed_loads_storage_00ab5700(
    NativeGuiIconDelayedLoadsStorage* volatile& actual_publication_00f8bd98,
    void* volatile& actual_manager_publication_01090aa0,
    NativeGuiIconDelayedLoadsGetterStorageFrame&,
    NativeGuiIconDelayedLoadsGetterStorageAcquired&);
} // namespace bsp
