#pragma once
#include "bsp/native_texture_atlas_item_storage.hpp"
#include <cstdint>
#include <exception>
#include <memory>
#include <vector>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw atlas registry storage requires MSVC Win32.
#endif

struct _RTL_CRITICAL_SECTION;
namespace bsp {
namespace game { class GameNativeReadOnlyData; }
class NativeRenderActualOwnerRegistry;
class NativeTextureLoadOwners;
class RenderCommandReference;
using NativeTextureAtlasRegistrySectionCall = void (__stdcall*)(::_RTL_CRITICAL_SECTION*);
using NativeTextureAtlasRegistryDecrement = long (__stdcall*)(volatile long*);

// Cold borrowed bindings. Publication cells and typed callable OS-import cells
// are the actual shared domains, not private copies or numeric code words.
// textures owns the genuine Source-created D61948/D61870/D618B0 companions in
// THIS concrete registry; its 2D context must enable this same canonical domain.
// The item context is the SAME raw pool/live feature as item construction.
// No allocation, registration, retain, constructor or publication occurs here.
struct NativeTextureAtlasRegistryStorageContext final {
    NativeTextureAtlasRegistryStorageContext(const game::GameNativeReadOnlyData&,
        void* volatile& actual_manager_publication_01090aa0,
        void* volatile& actual_atlas_publication_00f8c26c,
        NativeTextureAtlasItemStorageContext&, NativeRenderActualOwnerRegistry&,
        NativeTextureLoadOwners&,
        NativeTextureAtlasRegistrySectionCall const volatile& actual_enter_iat_00ce2218,
        NativeTextureAtlasRegistrySectionCall const volatile& actual_leave_iat_00ce2210,
        NativeTextureAtlasRegistryDecrement const volatile& actual_decrement_iat_00ce2220);
    const game::GameNativeReadOnlyData& data;
    void* volatile& manager_publication;
    void* volatile& atlas_publication;
    NativeTextureAtlasItemStorageContext& items;
    NativeRenderActualOwnerRegistry& owners;
    NativeTextureLoadOwners& textures;
    NativeTextureAtlasRegistrySectionCall const volatile& enter_iat;
    NativeTextureAtlasRegistrySectionCall const volatile& leave_iat;
    NativeTextureAtlasRegistryDecrement const volatile& decrement_iat;
    const volatile std::uint32_t* const profile_00d61948;
    const volatile std::uint32_t* const profile_00d61870;
    const volatile std::uint32_t* const profile_00d618b0;
};

enum class NativeTextureAtlasRegistryPhase { fresh, entered, complete, failed };
// Retained raw8 guard scratch plus Source history. No destructor unlocks,
// unregisters, clears a cell, frees or retries. Native guard scratch is unseeded
// until the corresponding two reached stores. Source controls are separate.
struct NativeTextureAtlasRegistryBaseFrame final {
    NativeTextureAtlasRegistryBaseFrame() {}
    NativeTextureAtlasRegistryBaseFrame(const NativeTextureAtlasRegistryBaseFrame&) = delete;
    NativeTextureAtlasRegistryBaseFrame& operator=(const NativeTextureAtlasRegistryBaseFrame&) = delete;
    std::uint32_t guard[2];
    NativeTextureAtlasRegistryPhase phase{NativeTextureAtlasRegistryPhase::fresh};
    void* actual_registry{};
    void* first_manager{};
    void* second_manager{};
    void* current_published_registry{};
    void* captured_section{};
    NativeTextureAtlasRegistrySectionCall entered_target{}, left_target{};
    std::uint32_t active_site{};
    std::int32_t native_state{-1};
    bool entered{}, depth_incremented{}, published{}, registered{}, unregistered{};
    bool cleared{}, depth_decremented{}, left{};
    std::exception_ptr failure;
};
struct NativeTextureAtlasRegistryConstructFrame final {
    NativeTextureAtlasRegistryBaseFrame base;
    NativeTextureAtlasRegistryPhase phase{NativeTextureAtlasRegistryPhase::fresh};
    void* actual_registry{};
    std::uint32_t active_site{};
    std::exception_ptr failure;
};
struct NativeTextureAtlasRegistryItemRetirement final {
    NativeTextureAtlasItemDestroyFrame destruction;
    void* actual_item{};
    bool destroy_started{}, destroy_returned{}, free_started{}, free_returned{};
};
struct NativeTextureAtlasRegistryTextureRetirement final {
    void* actual_texture{};
    RenderCommandReference* terminal_companion{}; // opaque after terminal returns
    NativeTextureAtlasRegistryDecrement decrement{};
    long decrement_result{};
    std::uint32_t profile{}, slot00{}, deleting_profile{}, slot04{}, active_site{};
    bool decrement_started{}, decrement_returned{}, terminal_started{}, terminal_returned{};
};
struct NativeTextureAtlasRegistryArrayRetirement final {
    std::uint32_t captured_capacity{}, copy_index{}, destination{}, fill_displacement{};
    void* replacement{};
    void* replaced_buffer{};
    void* final_buffer{};
    bool started{}, allocation_started{}, allocation_returned{}, replaced_free_returned{};
    bool replacement_published{}, count_cleared{}, final_free_started{}, final_free_returned{};
};
struct NativeTextureAtlasRegistryDestroyFrame final {
    NativeTextureAtlasRegistryDestroyFrame() = default;
    NativeTextureAtlasRegistryDestroyFrame(const NativeTextureAtlasRegistryDestroyFrame&) = delete;
    NativeTextureAtlasRegistryDestroyFrame& operator=(const NativeTextureAtlasRegistryDestroyFrame&) = delete;
    NativeTextureAtlasRegistryPhase phase{NativeTextureAtlasRegistryPhase::fresh};
    void* actual_registry{};
    void* captured_item{};
    void* captured_texture{};
    NativeTextureAtlasRegistryDecrement captured_decrement{};
    std::uint32_t item_index{}, texture_index{}, first_item_count{}, active_site{};
    std::int32_t native_state{-1};
    // Stable pointees are adopted before each reached child's first Native
    // effect. Cold metadata allocation must not inspect/mutate/reenter Native
    // domains. It may fail; the enclosing Frame retains all prior operations.
    std::vector<std::unique_ptr<NativeTextureAtlasRegistryItemRetirement>> item_calls;
    std::vector<std::unique_ptr<NativeTextureAtlasRegistryTextureRetirement>> texture_calls;
    NativeTextureAtlasRegistryArrayRetirement texture_array, item_array;
    NativeTextureAtlasRegistryBaseFrame base;
    std::exception_ptr failure;
};
struct NativeTextureAtlasRegistryScalarFrame final {
    NativeTextureAtlasRegistryDestroyFrame destruction;
    NativeTextureAtlasRegistryPhase phase{NativeTextureAtlasRegistryPhase::fresh};
    void* actual_registry{};
    std::uint32_t active_site{};
    std::uint8_t reached_flags_byte{};
    bool free_started{}, free_returned{};
    std::exception_ptr failure;
};

// Whole ordinary NORMAL 7364A0: ECX actual1C, no stack args, EAX same/RET.
// CFEA58; first actual manager/captured section; enter/+18; publish F8C26C;
// second manager, CURRENT publication, real BD0C30; same section -18/leave.
void* register_native_texture_atlas_registry_base_007364a0(void* actual_registry1c,
    NativeTextureAtlasRegistryBaseFrame&, NativeTextureAtlasRegistryStorageContext&);
// Whole ordinary NORMAL 737EE0: ECX actual1C, EAX same/RET. Call7364A0 then
// stamp CFEAE8 and six individual zero DWORD stores+04,+08,+0C,+10,+14,+18.
void* construct_native_texture_atlas_registry_00737ee0(void* actual_registry1c,
    NativeTextureAtlasRegistryConstructFrame&, NativeTextureAtlasRegistryStorageContext&);
// Whole ordinary NORMAL 7375E0 base: ECX same/RET. CFEA58, two fresh manager
// lookups, unregister CURRENT F8C26C, clear it, release captured lock, CE3818.
void destroy_native_texture_atlas_registry_base_007375e0(void* actual_registry1c,
    NativeTextureAtlasRegistryBaseFrame&, NativeTextureAtlasRegistryStorageContext&);
// Whole ordinary NORMAL AEF090: real owning item4/count8/capC and owning
// actual-texture10/count14/cap18. Item AEE220 then BF65AC; texture decrement
// target is captured ONCE only if initial signed count14>0. Every iteration
// reloads current begin/count. Zero dispatch admits only genuine same-registry
// Source-created 2D/cube/volume companions with the exact captured actual+04.
// Negative capacities allocate4/copy/free/publish cap1; negative counts perform
// raw32 displaced zero stores; positive counts decrement, then count0/free.
// Texture array precedes item array; stale begin/cap/header words stay intact.
// Finally call the complete7375E0. No C++ typed/semantic atlas is overlaid.
void destroy_native_texture_atlas_registry_00aef090(void* actual_registry1c,
    NativeTextureAtlasRegistryDestroyFrame&, NativeTextureAtlasRegistryStorageContext&);
// Whole ordinary NORMAL 737F10: ECX actual1C, mutable DWORD flags HOME/RET4,
// EAX original opaque identity. Read ONLY its low BYTE AFTER fullAEF090;
// BF65AC only when bit0 is then set. The returned address can be freed.
void* delete_native_texture_atlas_registry_00737f10(void* actual_registry1c,
    const volatile std::uint32_t& actual_flags_home,
    NativeTextureAtlasRegistryScalarFrame&, NativeTextureAtlasRegistryStorageContext&);

// Caller proves genuine allocation/array/item/texture provenance, all reached
// physical extents, raw-string/feature/heap/import/canonical/publication domains
// and serialized native liveness; profile/count bits do not supply that proof.
// Fresh address-stable Frames/HOME/private controls are disjoint from actual
// objects, provider-write extents and each other. Real source aliases elsewhere
// follow the underlying leaf contracts. Retain whole graph after every escape.
// CPP catch records/barethrows only: no new FH3/SEH unwind, guard release,
// unregister/free/replay/Origin/NoThrow authority. Existing canonical texture
// terminals are noexcept and can terminate on their own escaping child fault.
// No allocation by737EE0, common.ats load, parser, GUI producer activation,
// binary ABI, Native Origin, fixture/runtime/Game or fullHost claim follows.
} // namespace bsp
