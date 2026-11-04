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
            attacker.command.route_flag_24 = true;
            attacker.command.route_counter_28 = 1;
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
    std::puts("ATTACK construction/retarget/promotion/notification/shutdown lifetime passed; heap valid");
}
''')

vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
    text=True).strip()
vcvars = Path(vs) / "VC/Auxiliary/Build/vcvarsall.bat"
exe = out / "attack544_probe.exe"
args = ["cl.exe", "/nologo", "/std:c++17", "/EHsc", "/O2", "/Gy", "/MD", "/fp:strict",
    "/I" + str(root / "include"), str(probe), "/Fe" + str(exe), "/Fo" + str(out) + "/",
    "/link", "/MANIFEST:EMBED", "/OPT:REF",
    str(root / "build/win32/bsp_game.dir/Release/ai_planners.obj"),
    str(root / "build/win32/bsp_game.dir/Release/cmake_pch.obj"),
    str(root / "build/win32/Release/bsp_core.lib")]
batch = out / "build_probe.cmd"
batch.write_text('@echo off\ncall "' + str(vcvars) + '" x86 >nul\n'
    'if errorlevel 1 exit /b 1\n' + subprocess.list2cmdline(args) + '\nexit /b %errorlevel%\n')
subprocess.run(["cmd.exe", "/d", "/c", str(batch)], check=True, cwd=out)
subprocess.run([str(exe)], check=True, cwd=out)
