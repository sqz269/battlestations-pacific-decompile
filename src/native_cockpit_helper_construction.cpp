#include "bsp/native_cockpit_helper_construction.hpp"
#include "bsp/native_camera_configuration_leaves.hpp"
#include "bsp/native_cockpit_helper_lifetime.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <array>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native cockpit helper construction requires MSVC Win32 and x87.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(offsetof(NativeCockpitHelperStorage, camera_0c) == 0x0c);
static_assert(offsetof(NativeCockpitHelperStorage, preserved_10) == 0x10);
static_assert(offsetof(NativeCockpitHelperStorage, preserved_14) == 0x14);
static_assert(offsetof(NativeCockpitHelperStorage, preserved_1c) == 0x1c);
static_assert(offsetof(NativeCockpitHelperStorage, preserved_20) == 0x20);

namespace {
// DF74E8: 0->-1 base, 1->0 raw camera, 2->1 conditional string,
// 3->0 conditional string, 4->0 raw viewport. Native normal code never assigns
// state3. The shared viewport factory owns state4's exact allocation/free pair.
enum class NativeUnwindState {
    base = 0, raw_camera = 1, name_and_raw_camera = 2,
    name_only = 3, raw_viewport_in_factory = 4
};

NativeCameraOwner& current_camera(GeneratedModelLifetimeRuntime& runtime,
    std::uint32_t actual_key) {
    auto* const binding = runtime.find_actual_node(actual_key);
    auto* const reference = dynamic_cast<NativeCameraReference*>(binding);
    if (!reference)
        throw std::logic_error("cockpit current camera has no canonical camera reference");
    auto& owner = reference->camera_owner();
    // Inspect host phase before using any potentially ended actual storage.
    if (owner.phase != NativeCameraOwner::Phase::live ||
        reinterpret_cast<std::uintptr_t>(&owner.storage.node) != actual_key ||
        &owner.environment.nodes.attachments != &runtime)
        throw std::logic_error("cockpit current camera binding is not the live canonical owner");
    return owner;
}

void release_local_viewport(NativeViewportOwner* const captured,
    const NativeCockpitViewportReleaseContext& context) {
    // B3C928..B3C92C use the local EDI allocation, never current camera+180.
    if (!captured)
        throw std::logic_error("cockpit local viewport is outside the nonnull allocation domain");
    const auto decrement = context.decrement_iat_00ce2220;
    if (!decrement)
        throw std::logic_error("cockpit current CE2220 decrement IAT is unbound");
    if (decrement(&captured->references_04) != 0) return;

    // B3C936..B3C93C: current owner profile and slot0 only AFTER zero. The
    // concrete BD30E0 body then loads current slot4 and forwards flags1.
    const auto* const table = context.actual_two_word_table_00d5e5f8;
    if (!table || captured->native_vtable_00 != kNativeViewportVtable ||
        table[0] != 0x00bd30e0u)
        throw std::logic_error("cockpit current viewport slot0 is outside the concrete profile");
    if (captured->native_vtable_00 != kNativeViewportVtable ||
        table[1] != 0x00b1f8f0u)
        throw std::logic_error("cockpit current viewport deleting slot is outside the concrete profile");
    invoke_native_viewport_deleting_destructor_00bd30e0(captured);
}

void finish_helper_base(NativeCockpitHelperStorage& helper) noexcept {
    helper.profile_00 = 0x00ceb130u; // CBECE3 -> BD30F0
    helper.~NativeCockpitHelperStorage();
}
} // namespace

void* construct_native_cockpit_helper_00b3c800(void* actual_helper,
    std::size_t helper_bytes, const volatile std::uint32_t& near_00d7a2f0,
    const volatile std::uint32_t& far_00ce38b8,
    const NativeCockpitViewportReleaseContext& releases,
    NativeCockpitConstructionBlock::Admission&& admission) {
    if (!actual_helper || helper_bytes < sizeof(NativeCockpitHelperStorage) ||
        reinterpret_cast<std::uintptr_t>(actual_helper) % alignof(NativeCockpitHelperStorage))
        throw std::invalid_argument("cockpit constructor requires aligned unused actual 24h storage");
    if (!releases.actual_two_word_table_00d5e5f8)
        throw std::invalid_argument("cockpit constructor requires the actual viewport table binding");
    admission.validate_prepared();
    auto attempt = std::move(admission);
    attempt.begin_execution(); // caller token empty before any native callback
    auto& block = *attempt.block_;
    auto& environment = *attempt.camera_environment_;
    auto& name_pool = environment.nodes.require_raw_name_pool();

    // Only these scalar words are preserved by B3C800. The atomic counter is
    // not a trivially copyable object; establish its lifetime normally and set
    // its value through store below instead of restoring its representation.
    std::array<std::uint32_t, 4> preserved;
    const auto* const raw_bytes = static_cast<const std::byte*>(actual_helper);
    std::memcpy(preserved.data(), raw_bytes + 0x10, 8);
    std::memcpy(preserved.data() + 2, raw_bytes + 0x1c, 8);
    auto* const helper = ::new (actual_helper) NativeCockpitHelperStorage;
    helper->preserved_10 = preserved[0];
    helper->preserved_14 = preserved[1];
    helper->preserved_1c = preserved[2];
    helper->preserved_20 = preserved[3];
    helper->profile_00 = 0x00ceb130u;                 // B3C823
    helper->references_04.store(1, std::memory_order_relaxed); // B3C830
    auto state = NativeUnwindState::base;           // B3C83C
    helper->profile_00 = 0x00d61854u;                // B3C840
    helper->attached_08 = 0;                        // B3C846
    helper->camera_0c = 0;                          // B3C849
    helper->retained_18 = 0;                        // B3C84C

    void* raw_camera = nullptr;
    alignas(4) std::array<std::byte, 8> local_name;
    bool name_live = false; // native saved EBX bit0
    bool camera_completed = false; // HOST-only fact; not native EH state3
    try {
        raw_camera = allocate_native_camera_slot_00b71930(); // B3C84F
        state = NativeUnwindState::raw_camera;      // B3C85C
        void* camera_result = nullptr;
        if (raw_camera) {                           // B3C85A..B3C861
            construct_native_string_cstring_0041e870(local_name.data(),
                "CockpitCamera", name_pool);       // B3C86C, literal D6185C
            name_live = true;                      // B3C875/B3C882
            state = NativeUnwindState::name_and_raw_camera; // B3C87D
            block.camera_owner_.emplace(raw_camera, 0x45c, environment,
                std::move(attempt.scene_admission_));
            camera_result = construct_native_camera_00b71a80(*block.camera_owner_,
                local_name.data(), *attempt.node_constants_,
                std::move(attempt.viewport_admissions_[0])); // B3C886
            camera_completed = true;
            // Success-only host registration before B3C892. No native retain,
            // allocation, ambient token, or raw slot ownership transfer occurs.
            block.camera_reference_.emplace(*block.camera_owner_,
                NativeCameraCompanionDisposal{&block,
                    NativeCockpitConstructionBlock::record_camera_retirement},
                std::move(attempt.lifetime_admission_));
        }
        helper->camera_0c = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(camera_result)); // B3C892
        state = NativeUnwindState::base;            // B3C895, BEFORE string return
        if (name_live) {
            name_live = false; // never retry a throwing normal raw41DD20 return
            destroy_native_string_header_0041dd20(local_name.data(), name_pool);
            // Inlined original B3C89B..B3C8B5 captures data/length+1 before the
            // current 419CC0 getter, then BD1510; raw41DD20 preserves that order.
        }

        std::uint32_t camera_key, near_word, far_word, depth_word;
        const auto* const near_cell = &near_00d7a2f0;
        __asm {
            mov eax, near_cell
            fld dword ptr [eax]                    // B3C8BA, CURRENT D7A2F0
            mov ecx, helper
            mov edx, dword ptr [ecx + 0ch]          // B3C8C1
            mov camera_key, edx
            fstp dword ptr [near_word]             // B3C8C4
        }
        set_native_camera_near_00b6fbf0(
            &current_camera(environment.nodes.attachments, camera_key).storage.node,
            &near_word);                           // B3C8C7
        const auto* const far_cell = &far_00ce38b8;
        __asm {
            mov eax, far_cell
            fld dword ptr [eax]                    // B3C8CC, CURRENT CE38B8
            mov ecx, helper
            mov edx, dword ptr [ecx + 0ch]          // B3C8D3
            mov camera_key, edx
            fstp dword ptr [far_word]              // B3C8D6
        }
        set_native_camera_far_00b6fc10(
            &current_camera(environment.nodes.attachments, camera_key).storage.node,
            &far_word);                            // B3C8D9
        const std::uint32_t clear_flags = 6;
        (void)set_native_camera_clear_flags_00b6fe10(
            &current_camera(environment.nodes.attachments, helper->camera_0c).storage.node,
            &clear_flags);                         // B3C8DE..B3C8E3
        __asm {
            fld1                                  // B3C8E8
            fstp dword ptr [depth_word]             // B3C8EB
            mov ecx, helper
            mov edx, dword ptr [ecx + 0ch]          // B3C8EE
            mov camera_key, edx
        }
        set_native_camera_clear_depth_00b6fe20(
            &current_camera(environment.nodes.attachments, camera_key).storage.node,
            &depth_word);                          // B3C8F1

        state = NativeUnwindState::raw_viewport_in_factory;
        auto* const local_viewport = allocate_native_viewport_owner(environment.viewport,
            std::move(attempt.viewport_admissions_[1])); // B3C8F8/B3C90F
        // The factory frees only failed raw construction (state4 ->0). A
        // returned live viewport has no automatic local release from here on.
        state = NativeUnwindState::base;            // B3C91E
        set_native_camera_viewport_00b71990(
            current_camera(environment.nodes.attachments, helper->camera_0c),
            local_viewport);                       // B3C91A/B3C923
        release_local_viewport(local_viewport, releases); // B3C92C/B3C93C
    } catch (...) {
        try {
            if ((state == NativeUnwindState::name_and_raw_camera ||
                state == NativeUnwindState::name_only) && name_live) {
                name_live = false; // CBECFC consumes bit before CBED03 ->41DD20
                destroy_native_string_header_0041dd20(local_name.data(), name_pool);
            }
            if ((state == NativeUnwindState::raw_camera ||
                state == NativeUnwindState::name_and_raw_camera) && !camera_completed) {
                // Source preflight may reject before B71A80 enters its native
                // body. Abandon only still-prepared typed/scene registration.
                // A callee-unwound dead companion stays until explicit reset.
                if (block.camera_owner_ &&
                    block.camera_owner_->phase == NativeCameraOwner::Phase::prepared)
                    block.camera_owner_.reset();
                return_native_camera_slot_00b71350(raw_camera); // CBECEB
            }
            // If reference admission unexpectedly fails after camera success,
            // camera_completed preserves that live camera/slot. This HOST-only
            // diagnostic is outside admitted equivalence; the block may remain
            // pinned awaiting external recovery. Native state3 is not assigned.
            // State4 raw free has already run inside the viewport factory;
            // state0 adds no camera or local-viewport release, including on a
            // throwing B71990 or final current-IAT/profile dispatch.
            finish_helper_base(*helper);            // CBECE3 -> BD30F0
        } catch (...) { std::terminate(); }         // nested throwing cleanup boundary
        throw;
    }
    return actual_helper;                          // B3C943/B3C952
}

} // namespace bsp
