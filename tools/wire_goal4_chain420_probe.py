"""One focused Win32 fixture of the production objective-list adapter; no gameplay claim."""
import os
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[1]
out = root / 'local/output/g4_c420_probe'
out.mkdir(exist_ok=True)
host = (root / 'src/game_hosts_lua.cpp').read_text()
begin = host.index('class ObjectivePositionEraseList final')
end = host.index('\nclass ObjectiveListHost final', begin)
adapter = host[begin:end]
source = r'''
#include "bsp/objective_units.hpp"
#include "bsp/cc15_g24c.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <vector>
'''+adapter+r'''
int main() {
    using bsp::ObjectiveUnitEntry;
    const float nan = std::numeric_limits<float>::quiet_NaN();
    std::vector<ObjectiveUnitEntry> entries{
        {reinterpret_cast<void*>(1u), {}, false},
        {nullptr, {1,2,3}, true},
        {nullptr, {1,2,4}, true},
        {nullptr, {1,2,3}, true},
        {nullptr, {nan,2,3}, true},
        {reinterpret_cast<void*>(2u), {1,2,3}, false},
        {nullptr, {-0.0f,2,3}, true}
    };
    ObjectivePositionEraseList first(entries);
    first.erase(entries, {1,2,3});
    assert(entries.size() == 5);
    assert(entries[0].unit == reinterpret_cast<void*>(1u) && !entries[0].position_valid);
    assert(entries[1].unit == nullptr && entries[1].position.z == 4 && entries[1].position_valid);
    assert(std::memcmp(&entries[2].position.x, &nan, sizeof(float)) == 0);
    assert(entries[3].unit == reinterpret_cast<void*>(2u) && !entries[3].position_valid);
    assert(entries[3].position.x == 1 && entries[3].position.y == 2 && entries[3].position.z == 3);
    assert(entries[4].unit == nullptr && entries[4].position_valid);
    ObjectivePositionEraseList unordered(entries);
    unordered.erase(entries, {nan,2,3});
    assert(entries.size() == 5);
    ObjectivePositionEraseList zeros(entries);
    zeros.erase(entries, {+0.0f,2,3});
    assert(entries.size() == 4 && entries[3].unit == reinterpret_cast<void*>(2u));
    std::vector<ObjectiveUnitEntry> empty;
    ObjectivePositionEraseList empty_view(empty);
    empty_view.erase(empty, {1,2,3});
    assert(empty.empty());
    std::puts("chain420 actual adapter: duplicates removed; units/order/metadata retained; NaN and signed zero passed");
}
'''
cpp = out / 'objective420_probe.cpp'
cpp.write_text(source)
vswhere = Path(os.environ['ProgramFiles(x86)']) / 'Microsoft Visual Studio/Installer/vswhere.exe'
vs = subprocess.check_output([str(vswhere), '-latest', '-products', '*', '-requires',
    'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'], text=True).strip()
vcvars = Path(vs) / 'VC/Auxiliary/Build/vcvarsall.bat'
exe = out / 'objective420_probe.exe'
args = ['cl.exe', '/nologo', '/std:c++17', '/EHsc', '/O2', '/MD', '/fp:strict',
    '/I'+str(root/'include'), str(cpp), '/Fe'+str(exe), '/Fo'+str(out)+'/',
    '/link', '/MANIFEST:EMBED',
    *(str(root/'build/win32/Release'/name) for name in ('bsp_core.lib', 'bsp_lua511.lib', 'bsp_zlib121.lib')),
    'user32.lib', 'd3d9.lib', 'PowrProf.lib']
command = subprocess.list2cmdline(args)
batch = out / 'build_probe.cmd'
batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b 1\n{command}\nexit /b %errorlevel%\n')
subprocess.run(['cmd.exe', '/d', '/c', str(batch)], cwd=root, check=True)
subprocess.run([str(exe)], cwd=root, check=True)
