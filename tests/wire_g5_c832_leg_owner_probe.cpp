// Focused Source ownership fixture: two views use one real registered raw10h
// owner/record array, mission retirement clears identities, and views may be
// retired after the shared drain without dereferencing/recreating that owner.
#include "bsp/leg_altitude_manager_cc13.hpp"
#include "bsp/cc13_seg68.hpp"
#include "bsp/cc13_seg67b.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdio>
#include <cstdlib>
#include <array>
#include <cstring>
#include <new>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "leg owner probe failed: %s\n", message);
        std::abort();
    }
}
struct FixtureUnits final : bsp::LegAltitudeUnitHost {
    bool out_of_action_5d(bsp::LegUnitId) const override { return false; }
    float turn_circle_radius_268(bsp::LegUnitId) const override { return 100.0f; }
};
// Inject only the throwing allocation boundary; verify the original state0
// unwind releases the captured section instead of leaving its depth armed.
struct AllocationFailure final : bsp::PathLegManagerLifetimeCalls {
    std::array<std::uint32_t, 5> manager{};
    std::array<std::uint32_t, 7> section{};
    unsigned enters{}, leaves{};
    AllocationFailure() {
        void* pointer = section.data();
        std::memcpy(reinterpret_cast<char*>(manager.data()) + 0x10, &pointer, sizeof pointer);
    }
    void* path_leg_manager_00f8a720() override { return nullptr; }
    void set_path_leg_manager_00f8a720(void*) override { std::abort(); }
    void* singleton_manager_00415350() override { return manager.data(); }
    void enter_critical_section_ce2218(void* captured) override {
        require(captured == section.data(), "allocation failure captured another section");
        ++enters;
    }
    void leave_critical_section_ce2210(void* captured) override {
        require(captured == section.data(), "unwind released another section");
        ++leaves;
    }
    void* operator_new_00bf681b(std::uint32_t) override { throw std::bad_alloc(); }
    void* bot_path_leg_manager_construct_009faa40(void*) override { std::abort(); }
    void singleton_register_00bd0c30(void*, void*) override { std::abort(); }
};
} // namespace

int main() {
    AllocationFailure failure;
    try {
        bsp::bot_path_leg_manager_get_body_009bd400(failure);
        require(false, "allocation failure did not propagate");
    } catch (const std::bad_alloc&) {
        require(failure.enters == 1 && failure.leaves == 1 && failure.section[6] == 0,
            "allocation failure retained the captured lock/depth");
    }
    void* volatile lifetime_publication = nullptr;
    void* volatile leg_publication = nullptr;
    bsp::NativeSingletonDeletionBindings bindings{};
    bindings.actual_path_leg_manager_publication_00f8a720 = &leg_publication;
    const FixtureUnits units;
    {
        const bsp::LegAltitudeSingletonDomain domain{lifetime_publication, leg_publication};
        bsp::LegAltitudeManager first(domain);
        void* const captured_owner = leg_publication;
        require(captured_owner != nullptr, "getter did not publish the raw owner");
        bsp::LegAltitudeManager second(domain);
        require(leg_publication == captured_owner, "warm getter replaced the owner");
        require(bsp::count_native_singleton_slots_00bcf910(lifetime_publication, nullptr) == 1,
            "warm getter registered a duplicate");
        first.register_009fc000(units, 1, {10.0f, 200.0f, 30.0f});
        const auto* raw = static_cast<const bsp::cc13_seg68::LegManager*>(captured_owner);
        require(second.records().size() == 1 && second.records()[0].unit == 1,
            "views did not share the actual aircraft record");
        require(reinterpret_cast<std::uintptr_t>(second.records().data) == raw->records_04.data_00,
            "altitude consumer read a mirror of the raw record array");
        first.clear(); // same retirement the mission Impl performs before freeing unit slots
        require(second.records().empty(), "mission retirement retained recycled unit identities");
        second.register_009fc000(units, 2, {40.0f, 250.0f, 60.0f});
        bsp::destroy_native_singleton_manager_00bd0400(lifetime_publication, bindings);
        require(leg_publication == nullptr, "D21C80 deletion did not clear the process publication");
        require(bsp::count_native_singleton_slots_00bcf910(lifetime_publication, nullptr) == 0,
            "shared drain retained an owner");
        first.clear();
        second.clear();
        require(leg_publication == nullptr, "post-drain retirement recreated the singleton");
    } // view destructors must not touch the freed captured owner
    bsp::singleton_lifetime_free(lifetime_publication);
    std::puts("leg owner probe passed: shared records, one registration, mission retirement, D21C80 drain");
}
