"""One Win32 fixture of the production ATTACK command/observer lifetime; no game run."""
import os
from pathlib import Path
import subprocess


root = Path(__file__).resolve().parents[1]
out = root / "local/output/g4_c544_probe"
out.mkdir(exist_ok=True)
source = (root / "src/game_hosts_ai.cpp").read_text()
start = source.index("    struct Planner;\n")
end = source.index("    struct Planner {", start)
owner = source[start:end]

# Exercise the actual notification loop as well as the actual retained command
# implementation. No reimplementation of constructor, projection or teardown.
start = source.index("while (g->attack_endpoint && !g->attack_endpoint->commands.empty()) {")
end = start + source[start:].index("{") + 1
depth = 1
while depth:
    depth += (source[end] == "{") - (source[end] == "}")
    end += 1
notification = source[start:end]
probe = out / "attack544_probe.cpp"
probe.write_text(r'''
#include "bsp/cc13_seg69.hpp"
#include "bsp/ai_command_object.hpp"
#include "bsp/ai_command_tick.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <malloc.h>

struct Owner {
''' + owner + r'''
    static Group* group_at(void* group) { return static_cast<Group*>(group); }
    unsigned notifications = 0;
    void done(const char*, std::uint32_t) { ++notifications; }
    void notify(Group* g) {
''' + notification + r'''
    }
};

// A single route-building fixture. Callbacks outside this route arm fail if
// reached, rather than supplying unrelated gameplay behavior.
struct RouteHost final : bsp::AiCommandTickHost {
    Owner::Group& group;
    int leader{};
    unsigned sent{};
    explicit RouteHost(Owner::Group& g) : group(g) {}
    std::uint32_t tick_group_population(void* g) override { assert(g == &group); return 1; }
    void* tick_member_at(void* g, std::size_t index) override {
        assert(g == &group && index == 0); return &leader;
    }
    bool tick_member_is_groupable_combatant(void* member) override {
        assert(member == &leader); return true;
    }
    bool tick_route_slot_present(void* member) override { assert(member == &leader); return true; }
    bool tick_route_attached_0071fc40(void* member) override { assert(member == &leader); return false; }
    int tick_group_team(void* g) override { assert(g == &group); return group.team; }
    bool tick_leader_point(void* g, float out[3]) override {
        assert(g == &group);
        std::copy(std::begin(group.leader_position), std::end(group.leader_position), out);
        return true;
    }
    float tick_danger_cost_00a010f0(const float[3], int team) override {
        assert(team == (group.team == 0 ? 1 : 0)); return 0.0f;
    }
    void tick_route_send_point_0071d340(void* member, const float[3]) override {
        assert(member == &leader); ++sent;
    }
    static std::logic_error unexpected() { return std::logic_error("Unexpected route fixture callback"); }
    std::size_t tick_member_count(void*) override { throw unexpected(); }
    bool tick_member_position(void*, float[3]) override { throw unexpected(); }
    bool tick_member_is_ship_base(void*) override { throw unexpected(); }
    bool tick_member_is_plane_squadron(void*) override { throw unexpected(); }
    bool tick_squadron_excluded_007eda90(void*) override { throw unexpected(); }
    bool tick_squadron_excluded_009ffeb0(void*) override { throw unexpected(); }
    bool tick_issue_moveto(void*, const float[3]) override { throw unexpected(); }
    bool tick_request_join_formation(void*, void*) override { throw unexpected(); }
    bool tick_avoid_zone_point(void*, const float[3], float[2]) override { throw unexpected(); }
    float tick_tuning_field(std::uint32_t) override { throw unexpected(); }
    float tick_horizontal_distance(const float[3], const float[3]) override { throw unexpected(); }
    void tick_replace_command(void*, bsp::AiCommandType) override { throw unexpected(); }
    bsp::AiCommandObject* tick_command_state(void*) override { throw unexpected(); }
    int tick_route_remaining_0071d2a0(void*) override { throw unexpected(); }
    bool tick_issue_clearorders(void*) override { throw unexpected(); }
    bool tick_member_forward_row(void*, float[3]) override { throw unexpected(); }
    bool tick_member_has_formation_0284(void*) override { throw unexpected(); }
    int tick_formation_shape0_0070efd0(void*) override { throw unexpected(); }
    bool tick_formation_set_column0_0070d080(void*, void*, float, float) override { throw unexpected(); }
    bool tick_formation_has_record_0070d080(void*, void*) override { throw unexpected(); }
};

int main() {
    using Type = bsp::AiCommandType;
    Owner owner;
    Owner::Group attacker, second, target, successor;
    owner.notify(&target); // a never-observed group has no allocated endpoint
    assert(owner.notifications == 0);
    const auto install = [&](Owner::Group& group, Owner::Group& observed, Type type) {
        bsp::AiCommandObject command;
        command.type = type;
        command.owner_group = &group;
        command.target_group = &observed;
        owner.replace_command(&group, command);
        assert(group.command.type == type);
        assert(group.command.owner_group == &group);
        assert(group.command.target_group == &observed);
        assert(group.native_attack != nullptr);
    };

    // The three actual constructor outputs feed the host's type/owner/target;
    // retargeting removes the old pair and registers exactly one new pair.
    for (Type type : {Type::MoveToAttack, Type::CautiousAttack, Type::CloseAttack}) {
        install(attacker, target, type);
        assert(target.attack_endpoint->commands.size() == 1);
        if (type == Type::CautiousAttack) {
            assert(!attacker.command.route_flag_24);
            assert(attacker.command.route_counter_28 == 4);
            RouteHost route_host(attacker);
            bsp::AiCommandTickResult tick;
            const float destination[3] = {400.0f, 0.0f, 0.0f};
            const auto route = bsp::ai_cautious_approach_pass_00a14dd0(route_host,
                &attacker, destination, attacker.command, tick);
            assert(route.built && route.points == 3 && route_host.sent == 4);
            assert(attacker.command.route_flag_24 && attacker.command.route_counter_28 == 3);
            owner.store_command_route_state(&attacker);
            const auto retained = attacker.native_attack->state();
            assert(retained.route_flag_24 == attacker.command.route_flag_24);
            assert(retained.route_counter_28 == attacker.command.route_counter_28);
            const bsp::cc13_seg69::Raw bytes{attacker.native_attack->bytes.data()};
            assert(bytes.at<std::uint8_t>(0x24) == 1);
            assert(bytes.at<std::int32_t>(0x28) == 3);
        }
        install(attacker, successor, type);
        assert(target.attack_endpoint->commands.empty());
        assert(successor.attack_endpoint->commands.size() == 1);
        if (type == Type::CautiousAttack) {
            assert(!attacker.command.route_flag_24);
            assert(attacker.command.route_counter_28 == 4);
        }
        bsp::AiCommandObject idle;
        idle.type = Type::Idle;
        idle.owner_group = &attacker;
        owner.replace_command(&attacker, idle);
        assert(!attacker.native_attack);
        assert(successor.attack_endpoint->commands.empty());
    }

    // Promotion replaces a command on the same endpoint. Target destruction
    // consumes both registered observers and installs IDLE once for each.
    install(attacker, target, Type::CautiousAttack);
    install(attacker, target, Type::CloseAttack);
    install(second, target, Type::MoveToAttack);
    assert(target.attack_endpoint->commands.size() == 2);
    owner.notify(&target);
    assert(owner.notifications == 2);
    assert(target.attack_endpoint->commands.empty());
    assert(attacker.command.type == Type::Idle && !attacker.native_attack);
    assert(second.command.type == Type::Idle && !second.native_attack);
    owner.notify(&target);
    assert(owner.notifications == 2);

    // Whole-mission teardown can free a target group before its attacker.
    std::weak_ptr<Owner::AttackObserverEndpoint> retained;
    {
        auto early_target = std::make_unique<Owner::Group>();
        install(attacker, *early_target, Type::MoveToAttack);
        retained = early_target->attack_endpoint;
    }
    assert(!retained.expired());
    attacker.native_attack.reset();
    assert(retained.expired());
    assert(_heapchk() == _HEAPOK);
    std::puts("ATTACK construction/route-state synchronization/retarget/promotion/notification/shutdown lifetime passed; heap valid");
}
''')

vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
    text=True).strip()
vcvars = Path(vs) / "VC/Auxiliary/Build/vcvarsall.bat"
exe = out / "attack544_probe.exe"
args = ["cl.exe", "/nologo", "/std:c++17", "/EHsc", "/O2", "/Gy", "/MD", "/fp:strict",
    "/I" + str(root / "include"), str(probe),
    "/Fe" + str(exe), "/Fo" + str(out) + "/",
    "/link", "/MANIFEST:EMBED", "/OPT:REF",
    str(root / "build/win32/bsp_game.dir/Release/ai_planners.obj"),
    str(root / "build/win32/bsp_game.dir/Release/ai_command_tick.obj"),
    str(root / "build/win32/bsp_game.dir/Release/gameplay_seg69_70_cc13.obj"),
    str(root / "build/win32/bsp_game.dir/Release/band_009f_cc13.obj"),
    str(root / "build/win32/bsp_game.dir/Release/cmake_pch.obj"),
    str(root / "build/win32/Release/bsp_core.lib")]
batch = out / "build_probe.cmd"
batch.write_text('@echo off\ncall "' + str(vcvars) + '" x86 >nul\n'
    'if errorlevel 1 exit /b 1\n' + subprocess.list2cmdline(args) + '\nexit /b %errorlevel%\n')
subprocess.run(["cmd.exe", "/d", "/c", str(batch)], check=True, cwd=out)
subprocess.run([str(exe)], check=True, cwd=out)
