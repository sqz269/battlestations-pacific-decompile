#include "bsp/plane_squadron.hpp"
#include "bsp/plane_squadron_host.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

int run_probe() {
    // One sequence exercises the reentrant tail route and the live latch,
    // then the native no-kill gates and the distinct final-squadron arm.
    std::vector<std::string> events;
    bsp::PlaneSquadronHostRecord squadron;
    squadron.member_units = {0, 1, 2};
    bool latched = false;
    bool clear_leading_seat_on_warning = false;
    int mode = 8, session = 0;
    std::function<void(std::size_t)> invoke;
    invoke = [&](std::size_t plane) {
        bsp::SquadronLeaveMapHost h;
        h.effective_game_mode = mode;
        h.session_mode = session;
        h.leave_latched = [&] { return latched; };
        h.warn_exit_zone = [&] {
            events.push_back("warn" + std::to_string(plane));
            if (clear_leading_seat_on_warning) {
                squadron.member_units[0] = bsp::kPlaneSquadronNoUnit;
                clear_leading_seat_on_warning = false;
            }
        };
        h.member_count = [&] { return static_cast<std::size_t>(squadron.live_count()); };
        h.leader = [&] { return squadron.flight_leader(); };
        h.member = [&](std::size_t i) { return squadron.live_member(i); };
        h.outside_map = [](std::size_t member) {
            if (member == bsp::kPlaneSquadronNoUnit)
                throw std::runtime_error("empty seat exposed as a live member");
            return member == 1;
        };
        h.route_member_exit = [&](std::size_t member) {
            if (member == plane)
                throw std::runtime_error("leader recursively routed its own exit");
            invoke(member);
        };
        h.leaving_on_purpose = [] { return true; };
        h.latch_leave = [&] { latched = true; events.push_back("latch"); };
        h.store_squadron_exit_party = [&] { events.push_back("party"); };
        h.kill_plane = [&] { events.push_back("plane" + std::to_string(plane)); };
        h.kill_squadron = [&] { events.push_back("squadron"); };
        bsp::squadron_on_plane_left_map_007f31a0(plane, h);
    };
    const auto expect = [&](std::vector<std::string> wanted) {
        if (events != wanted) throw std::runtime_error("leave-map call order differs");
        events.clear();
    };
    invoke(0);
    expect({"warn0", "warn1", "latch", "plane1", "plane0"});
    invoke(2);
    expect({"plane2"}); // no later warning; the nested call set the actual latch
    latched = false;
    mode = 9;
    invoke(0);
    expect({});
    mode = 8;
    session = 2;
    invoke(0);
    expect({"warn0"});
    session = 0;
    squadron.member_units = {0};
    invoke(0);
    expect({"warn0", "party", "squadron"});
    // A kill after start-of-step compaction can clear the old leader's seat.
    // The next leader is raw slot1 but must be compact ordinal0 everywhere.
    squadron.member_units = {bsp::kPlaneSquadronNoUnit, 1,
        bsp::kPlaneSquadronNoUnit, 2, bsp::kPlaneSquadronNoUnit};
    invoke(1);
    expect({"warn1", "latch", "plane1"});
    latched = false;
    squadron.member_units = {bsp::kPlaneSquadronNoUnit, 1, bsp::kPlaneSquadronNoUnit};
    invoke(1);
    expect({"warn1", "party", "squadron"}); // holes do not create survivors
    squadron.member_units = {0, 1, 2};
    clear_leading_seat_on_warning = true;
    invoke(1);
    expect({"warn1", "latch", "plane1"}); // reads reflect the callback's mutation
    std::puts("007F31A0 reentrant/sparse-member/latch/owner/gate sequence passed");
    return 0;
}

int main() {
    try { return run_probe(); }
    catch (const std::exception& failure) {
        std::fprintf(stderr, "007F31A0 probe failed: %s\n", failure.what());
        return 1;
    }
}
