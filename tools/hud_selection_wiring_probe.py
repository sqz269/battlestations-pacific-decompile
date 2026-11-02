"""Exercise the actual HUD wiring methods with bounded callback fixtures.

Compiles extracted host methods plus the existing hud_root_rows.cpp bodies.
This tests tuple transport, not real input devices, mission selection commits,
or Original ABI equivalence. --revision can demonstrate a prior regression.
"""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]


def block(source: str, anchor: str) -> str:
    start = source.index(anchor)
    opening = source.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


PRELUDE = r'''
#include "bsp/hud_root_rows.hpp"
#include "bsp/hud_updates.hpp"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>

namespace bsp::game {
inline constexpr bool kWireGoal1Chain12Bound = PROBE_ON;
struct FixtureUnits { std::uint32_t controlled{22}; };
struct FixtureMenu {
    std::set<int> edges;
    bool panel{};
    bool input_action_pressed(int action) { return edges.count(action) != 0; }
    bool in_game_screen_applied(int) { return panel; }
};
struct GameHudHost {
    struct Impl {
        FixtureUnits storage;
        FixtureUnits* units{&storage};
        FixtureMenu menu;
        bsp::HudRootUnitLists root_lists;
        bsp::HudRootSelectionState root_selection;
        bsp::HudRootUpdateState hud_root;
        std::map<std::uint32_t, std::uint32_t> leaders;
        int record_pending_id_20{};
        int concrete{}, records{};
        bool unit_vtable_124(std::size_t, std::uint32_t) { return true; }
        void done(const char*, std::uint32_t) { ++concrete; }
        void record(const char*, std::uint32_t) { ++records; }
    };
};
// Only external callbacks are fixtures. The selection adapter, caller methods
// and all selection/lookup bodies are compiled directly from repository code.
class HudRootListsBinding {
public:
    explicit HudRootListsBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    bool unit_is_group_leader_00778890(std::uint32_t unit) {
        for (const auto& pair : owner_.leaders) if (pair.second == unit) return true;
        return false;
    }
    bool unit_is_group_member_007788b0(std::uint32_t unit) {
        return owner_.leaders.count(unit) != 0;
    }
    std::uint32_t group_leader_007788d0(std::uint32_t unit) {
        const auto found = owner_.leaders.find(unit);
        return found == owner_.leaders.end() ? 0 : found->second;
    }
    std::uint32_t controlled_unit() { return owner_.units->controlled; }
private:
    GameHudHost::Impl& owner_;
};
'''

CHECKS = r'''
void check(bool value, const char* message) {
    if (!value) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
std::uint32_t resolve(HudRootUpdateBinding& binding) {
    bool fallback = false;
    const auto value = binding.resolve_weapon_info(fallback);
    return fallback ? binding.resolve_weapon_info_fallback() : value;
}
int run_probe() {
    GameHudHost::Impl owner;
    owner.root_lists.primary_8c = {11, 22, 33};
    owner.root_lists.cursor = {false, 0};
    HudRootUpdateBinding binding(owner);
    if constexpr (!kWireGoal1Chain12Bound) {
        bool fallback = true;
        check(binding.resolve_weapon_info(fallback) == 0xffff0000u && !fallback,
            "OFF resolver changed");
        check(binding.resolve_weapon_info_fallback() == 0xffff0000u,
            "OFF fallback changed");
        owner.hud_root.weapon_info_field = 0x0001a500u;
        binding.update_unit_rows(0.05f);
        check(owner.hud_root.weapon_info_field == 0x0001a500u &&
            owner.root_lists.cursor.index == 0 && !owner.root_lists.force_pending_c0 &&
            owner.concrete == 0 && owner.records == 2, "OFF poll changed state");
        std::puts("OFF: legacy resolver and record-only poll preserved");
        return 0;
    } else {
    // Start with a caller-written tuple that disagrees with the list cursor.
    owner.hud_root.weapon_info_field = 0x0001a500u;
    const auto neutral = owner.hud_root.weapon_info_field;
    binding.update_unit_rows(0.05f);
    check(owner.hud_root.weapon_info_field == neutral &&
        owner.root_lists.cursor.index == 1 && owner.root_selection.selection.index == 1 &&
        !owner.root_lists.force_pending_c0, "neutral poll did not preserve incoming tuple");
    std::printf("neutral: controlled=22 stale_cursor=0 incoming=%08x after=%08x cursor=%u pending=0\n",
        neutral, owner.hud_root.weapon_info_field, static_cast<unsigned>(owner.root_lists.cursor.index));

    // A nongroup unit must use the primary fallback, not the stale cursor.
    check(resolve(binding) == 0x00010000u, "controlled 22 must resolve to primary[1]");

    owner.storage.controlled = 33;
    owner.root_lists.cursor = {false, 0};
    owner.hud_root.weapon_info_field = resolve(binding);
    check(owner.hud_root.weapon_info_field == 0x00020000u, "controlled change used stale cursor");
    binding.update_unit_rows(0.05f);
    check(owner.hud_root.weapon_info_field == 0x00020000u &&
        owner.root_lists.cursor.index == 2, "controlled change not synchronized");
    std::puts("controlled change: 22 -> 33, incoming=00020000 after=00020000 cursor=2");

    owner.storage.controlled = 22;
    for (int press = 1; press <= 2; ++press) {
        owner.menu.edges = {0x8c};
        owner.hud_root.weapon_info_field = resolve(binding);
        check(owner.hud_root.weapon_info_field == 0x00010000u,
            "repeated press must start from controlled unit, not previous poll result");
        binding.update_unit_rows(0.05f);
        check(owner.hud_root.weapon_info_field == 0x00020000u &&
            owner.root_lists.cursor.index == 2 && owner.root_lists.force_pending_c0,
            "primary-next result wrong");
        std::printf("press %d: controlled=22 incoming=00010000 after=00020000 cursor=2 pending=1\n", press);
        owner.menu.edges.clear();
        owner.hud_root.weapon_info_field = resolve(binding);
        binding.update_unit_rows(0.05f);
        check(owner.hud_root.weapon_info_field == 0x00010000u,
            "neutral release did not preserve freshly resolved tuple");
    }
    // Simulate the downstream controlled-unit writer, then another press.
    owner.storage.controlled = 33;
    owner.menu.edges = {0x8c};
    owner.hud_root.weapon_info_field = resolve(binding);
    binding.update_unit_rows(0.05f);
    check(owner.hud_root.weapon_info_field == 0 && owner.root_lists.cursor.index == 0,
        "next after changed control must wrap from primary[2]");
    std::puts("press after controlled change: controlled=33 incoming=00020000 after=00000000 cursor=0");

    // The poll's selected_unit callback must also see canonical +C2h before
    // converting a secondary selection to the leader's primary index.
    owner.leaders = {{22, 11}};
    owner.root_lists.primary_8c = {11, 33};
    owner.root_lists.secondary_9c = {11, 22};
    owner.root_lists.cursor = {false, 1};
    owner.storage.controlled = 22;
    owner.hud_root.weapon_info_field = resolve(binding);
    check(owner.hud_root.weapon_info_field == 0x00010001u, "secondary lookup not used");
    binding.update_unit_rows(0.05f);
    check(owner.hud_root.weapon_info_field == 0x00010000u &&
        !owner.root_lists.cursor.secondary && owner.root_lists.cursor.index == 1,
        "selected_unit callback used stale cursor during secondary-to-primary conversion");
    std::puts("secondary conversion: stale_cursor=primary[1] incoming=00010001 after=00010000");
    std::puts("PASS: actual host resolver/fallback/poll methods, neutral/change/repeated presses");
    return 0;
    }
}
} // namespace bsp::game
int main() { return bsp::game::run_probe(); }
'''


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--revision", help="Read host methods from this git revision instead of the worktree")
    parser.add_argument("--out", default="local/output/g1-c12-review-probe")
    args = parser.parse_args()
    if args.revision:
        source = subprocess.check_output(
            ["git", "show", f"{args.revision}:src/game_hosts_hud.cpp"], cwd=ROOT, text=True)
    else:
        source = (ROOT / "src/game_hosts_hud.cpp").read_text(encoding="utf-8")
    adapter = block(source, "class HudRootSelectionBinding final") + ";\n"
    caller = source[source.index("class HudRootUpdateBinding final"):]
    methods = "\n".join(block(caller, anchor).replace(" override", "") for anchor in (
        "std::uint32_t resolve_weapon_info(bool& needs_fallback)",
        "std::uint32_t resolve_weapon_info_fallback()", "void update_unit_rows(float)"))
    cpp = PRELUDE + adapter + "\nclass HudRootUpdateBinding {\npublic:\n" + (
        "explicit HudRootUpdateBinding(GameHudHost::Impl& owner) : owner_(owner) {}\n"
    ) + methods + "\nprivate: GameHudHost::Impl& owner_;\n};\n" + CHECKS
    output = (ROOT / args.out).resolve()
    output.mkdir(parents=True, exist_ok=True)
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip()
    vcvars = Path(vs) / "VC/Auxiliary/Build/vcvarsall.bat"
    for enabled in (False, True):
        label = "on" if enabled else "off"
        unit = output / f"hud_selection_{label}_probe.cpp"
        unit.write_text(cpp.replace("PROBE_ON", "true" if enabled else "false"), encoding="utf-8")
        executable = output / f"hud_selection_{label}_probe.exe"
        command = subprocess.list2cmdline(["cl", "/nologo", "/std:c++17", "/EHsc", "/W4", "/WX",
            f"/I{ROOT / 'include'}", str(unit), str(ROOT / "src/hud_root_rows.cpp"),
            f"/Fe{executable}", f"/Fo{output.as_posix()}/", "/link", "/MANIFEST:EMBED"])
        batch = output / f"compile_{label}.cmd"
        batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b 1\n'
            + command + "\nexit /b %errorlevel%\n", encoding="utf-8")
        built = subprocess.run(["cmd", "/d", "/c", str(batch)], cwd=output, text=True, capture_output=True)
        (output / f"build_{label}.txt").write_text(built.stdout + built.stderr, encoding="utf-8")
        if built.returncode:
            print(built.stdout + built.stderr, file=sys.stderr)
            return built.returncode
        run = subprocess.run([str(executable)], cwd=output, text=True, capture_output=True)
        print(run.stdout + run.stderr, end="")
        (output / f"run_{label}.txt").write_text(run.stdout + run.stderr, encoding="utf-8")
        if run.returncode:
            return run.returncode
    print(f"source={args.revision or 'worktree'}; evidence={output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
