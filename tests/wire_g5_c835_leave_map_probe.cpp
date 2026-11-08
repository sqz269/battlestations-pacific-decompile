#include "bsp/plane_squadron.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

int main() {
    // One sequence exercises the reentrant tail route and the live latch,
    // then the native no-kill gates and the distinct final-squadron arm.
    std::vector<std::string> events;
    std::size_t count = 3;
    bool latched = false;
    int mode = 8, session = 0;
    std::function<void(std::size_t)> invoke;
    invoke = [&](std::size_t plane) {
        bsp::SquadronLeaveMapHost h;
        h.effective_game_mode = mode;
        h.session_mode = session;
        h.leave_latched = [&] { return latched; };
        h.warn_exit_zone = [&] { events.push_back("warn" + std::to_string(plane)); };
        h.member_count = [&] { return count; };
        h.leader = [] { return std::size_t{0}; };
        h.member = [](std::size_t i) { return i; };
        h.outside_map = [](std::size_t member) { return member == 1; };
        h.route_member_exit = [&](std::size_t member) { invoke(member); };
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
    count = 1;
    invoke(0);
    expect({"warn0", "party", "squadron"});
    std::puts("007F31A0 reentrant leave-map/latch/owner/gate sequence passed");
}
