// Focused identity fixture: the marked plane, its squadron and the +140h
// subject have different detection/name state. Exercise the production resolver.
// Link against the built Win32 bsp_core.lib with /OPT:REF /MANIFEST:EMBED.
#include "bsp/cc13_seg49.hpp"
#include "bsp/gunnery_recon_detection.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
struct Subject {
    bsp::ReconDetectionRecord record;
    const char* name;
};
struct NameHost final : bsp::cc13_seg49::MarkerDisplayNameHost {
    Subject marked{{}, "fixture.plane"};
    Subject squadron{{}, "fixture.squadron"};
    Subject root{{}, "fixture.root"};
    bool aircraft = true, leader = false, squadron_bound = true, root_bound = true;
    int detection_calls = 0, squadron_queries = 0, root_queries = 0;
    int name_calls = 0;
    void* last_detection = nullptr;
    void* last_name = nullptr;

    int display_name_detection_level(int party, void* unit) override {
        if (party != 1 || (unit != &squadron && unit != &root)) std::abort();
        ++detection_calls;
        last_detection = unit;
        return static_cast<int>(bsp::recon_record_effective_level(static_cast<Subject*>(unit)->record));
    }
    const char* unit_name_14(void* unit) override {
        if (unit != last_detection) std::abort();
        ++name_calls;
        last_name = unit;
        return static_cast<Subject*>(unit)->name;
    }
    bool marked_unit_is_aircraft(void* unit) override {
        if (unit != &marked) std::abort();
        return aircraft;
    }
    bool marked_plane_is_flight_leader(void* unit) override {
        if (unit != &marked) std::abort();
        return leader;
    }
    void* aircraft_squadron_9d4(void* unit) override {
        if (unit != &marked) std::abort();
        ++squadron_queries;
        return squadron_bound ? &squadron : nullptr;
    }
    void* unit_subject_140(void* unit) override {
        if (unit != &marked) std::abort();
        ++root_queries;
        return root_bound ? &root : nullptr;
    }
};
void require(bool condition) { if (!condition) std::abort(); }
}

int main() {
    NameHost host;
    const auto read = [&](bool self, bool friendly, bool target) {
        return bsp::cc13_seg49::marker_display_name(host, 1, &host.marked, self, friendly, target);
    };
    host.marked.record.level = bsp::ReconDetectionLevel::identified;
    // A self wing that is not a flight leader is not eligible.
    require(read(true, false, false) == nullptr && host.squadron_queries == 0);
    require(host.detection_calls == 0 && host.name_calls == 0);
    host.leader = true;
    require(read(false, false, false) == nullptr && host.squadron_queries == 0);
    // Plane identified, squadron unknown: the name must still be unknown.
    require(std::strcmp(read(true, false, false), "globals.unknown") == 0);
    require(host.last_detection == &host.squadron && host.name_calls == 0 && host.root_queries == 0);
    // Reverse the levels: forcing the squadron reveals its own name only.
    host.marked.record.level = bsp::ReconDetectionLevel::none;
    host.squadron.record.forced = true;
    host.squadron.record.forced_level = static_cast<bsp::ReconDetectionLevel>(4);
    require(read(true, false, false) == host.squadron.name && host.last_name == &host.squadron);
    host.squadron.record.forced = false;
    require(std::strcmp(read(true, false, false), "globals.unknown") == 0 && host.name_calls == 1);
    // Target and friendly bypass the leader/self conjunction.
    host.leader = false;
    host.squadron.record.level = bsp::ReconDetectionLevel::identified;
    require(read(false, false, true) == host.squadron.name);
    host.squadron.name = "fixture.squadron.reused";
    require(read(false, true, false) == host.squadron.name);
    // An unbound live squadron must not fall back to its identified wing.
    host.squadron_bound = false;
    host.marked.record.level = bsp::ReconDetectionLevel::identified;
    const int previous_calls = host.detection_calls;
    require(read(false, false, true) == nullptr && host.detection_calls == previous_calls);
    // Non-aircraft reads both fields from the +140h result, not the marker.
    host.aircraft = false;
    require(std::strcmp(read(false, false, false), "globals.unknown") == 0);
    require(host.last_detection == &host.root);
    host.root.record.level = bsp::ReconDetectionLevel::identified;
    require(read(false, false, false) == host.root.name && host.last_name == &host.root);
    host.root_bound = false;
    require(read(false, false, false) == nullptr);
    std::puts("recon marker name: subject identity, aircraft eligibility and forced levels passed");
}
