"""One Win32 lifecycle case for the formation group's native-layout projection."""
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "local/output/wire_g4_c272_probe"
SOURCE = r'''
#include "bsp/game_unit_group_storage.hpp"
#include "bsp/native_unit_group_direct_views.hpp"
#include "bsp/native_unit_group_lookup.hpp"
#include <cassert>
#include <cstring>
int main() {
    using namespace bsp;
    game::GameUnitGroupStorage group;
    std::vector<ShipAiUnitGroupMember> members(2);
    members[0].entity = 37; members[1].entity = 12;
    members[1].lateral[2] = -17.5f;
    assert(group.capture_members(members));
    assert(native_unit_group_member_index_0070d030(group.data(),
        reinterpret_cast<void*>(12)) == 1);
    ShipAiUnitGroupMember copied;
    std::memcpy(&copied, native_unit_group_record_at_0070d070(
        group.data(), nullptr, 1), sizeof(copied));
    assert(std::memcmp(&copied, &members[1], sizeof(copied)) == 0);
    group.publish_speed_ceiling(-0.0f);
    const float speed = native_unit_group_retained_speed_0070d0f0(group.data(), nullptr);
    unsigned bits; std::memcpy(&bits, &speed, 4);
    assert(bits == 0x80000000u);
    // A leave/compaction must hide the retired record, and capture must not
    // rerun the constructor or overwrite the separately published speed.
    members.erase(members.begin());
    assert(group.capture_members(members));
    assert(native_unit_group_member_index_0070d030(group.data(),
        reinterpret_cast<void*>(37)) == -1);
    assert(native_unit_group_member_index_0070d030(group.data(),
        reinterpret_cast<void*>(12)) == 0);
    const float retained = group.speed_ceiling();
    std::memcpy(&bits, &retained, 4); assert(bits == 0x80000000u);
    members.resize(25);
    assert(!group.capture_members(members));
    assert(native_unit_group_member_index_0070d030(group.data(),
        reinterpret_cast<void*>(12)) == 0);
}
'''


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    source = OUT / "group272_probe.cpp"
    source.write_text(SOURCE)
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip()
    vcvars = Path(vs) / "VC/Auxiliary/Build/vcvarsall.bat"
    inputs = [source, *(ROOT / "src" / name for name in (
        "game_hosts_unit_group.cpp", "native_unit_group_storage.cpp",
        "native_unit_group_lookup.cpp", "native_unit_group_direct_views.cpp"))]
    executable = OUT / "group272_probe.exe"
    command = subprocess.list2cmdline(["cl.exe", "/nologo", "/std:c++17", "/EHsc", "/O2", "/MD",
        "/I" + str(ROOT / "include"), *(str(p) for p in inputs),
        "/Fe" + str(executable), "/Fo" + str(OUT) + "/", "/link", "/MANIFEST:EMBED"])
    batch = OUT / "build_probe.cmd"
    batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b 1\n{command}\nexit /b %errorlevel%\n')
    subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=ROOT, check=True)
    subprocess.run([str(executable)], cwd=ROOT, check=True)
    result = {"status": "passed", "scope": "synthetic Win32 projection lifecycle; no gameplay proof",
        "checks": "record bits, member lookup, compaction refresh, signed-zero speed, capacity fallback",
        "inputs": {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}}
    (OUT / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result))


if __name__ == "__main__":
    main()
