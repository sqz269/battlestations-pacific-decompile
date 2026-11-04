"""One actual native mounted-cache fixture; build first. Does not run the game."""
SOURCE = r'''
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_vfs_application.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/game_native_data_bootstrap.hpp"
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <exception>
#include <cstdlib>

void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
int main(int argc, char** argv) {
    std::set_terminate([] {
        try { if (auto error = std::current_exception()) std::rethrow_exception(error); }
        catch (const std::exception& error) { std::fprintf(stderr, "probe terminate: %s\n", error.what()); }
        std::_Exit(90);
    });
    require(argc >= 3, "original exe and fixture root required");
    const std::filesystem::path binary(argv[1]), root(argv[2]);
    const bsp::game::GameNativeDataSpan spans[]{
        {0x00ce2000u, 1}, {0x00cf0000u, 1}, {0x00d00000u, 1},
        {0x00d10000u, 1}, {0x00d50000u, 1}, {0x00d60000u, 1}, {0x00d70000u, 1}};
    if (argc == 3) {
        const auto arguments = L"\"" + binary.wstring() + L"\" \"" + root.wstring() + L"\" --cache418-child";
        bsp::game::GameNativeDataBootstrapChild child(std::filesystem::absolute(argv[0]), arguments, spans, 7);
        child.reserve_and_resume();
        child.wait_for_mapping(30000);
        require(WaitForSingleObject(child.process_handle(), 30000) == WAIT_OBJECT_0, "fixture child timed out");
        DWORD code;
        require(GetExitCodeProcess(child.process_handle(), &code) != 0, "no fixture child status");
        return static_cast<int>(code);
    }
    auto reservation = bsp::game::accept_native_data_handoff(spans, 7);
    std::filesystem::create_directories(root);
    std::ofstream(root / "sample.lua", std::ios::binary) << "first";
    std::ofstream(root / "empty.lua", std::ios::binary);
    bsp::game::GameHostLog log;
    bsp::game::GameNativeReadOnlyData data(binary, spans, 7, std::move(reservation));
    bsp::game::GameSingletonHost singletons(log);
    bsp::game::GameNativeVfsApplication application(log, singletons, data, binary);
    std::puts("probe initialize"); std::fflush(stdout);
    application.initialize_core();
    auto& runtime = application.runtime();
    std::string directory = root.string() + "\\";
    runtime.mount_phase2_loose_paths(directory.c_str());
    std::puts("probe cache"); std::fflush(stdout);
    runtime.cache_file_store("sample.lua", 0x32);
    runtime.cache_file_store("empty.lua", 0x32);
    std::ofstream(root / "sample.lua", std::ios::binary | std::ios::trunc) << "second";
    std::ofstream(root / "empty.lua", std::ios::binary | std::ios::trunc) << "later";
    auto cached = runtime.read_all("sample.lua", 2);
    auto empty = runtime.read_all("empty.lua", 2);
    require(cached && std::string(cached->begin(), cached->end()) == "first",
        "mounted readers did not consume the native FileStore");
    require(empty && empty->empty(), "opened empty file lost its retained backing");
    runtime.cache_file_store("sample.lua", 0x32);
    cached = runtime.read_all("sample.lua", 2);
    require(cached && std::string(cached->begin(), cached->end()) == "first",
        "duplicate cache changed the retained entry");
    require(!runtime.file_store_request_failure_site(), "cache retained a failed invocation");
    singletons.shutdown();
    std::ofstream(root / "result.json") << "{\"status\":\"passed\",\"native_cache_calls\":3,"
        "\"mounted_cache_consumed\":true,\"duplicate_preserved\":true,"
        "\"empty_backing_preserved\":true,\"shared_drain_completed\":true,\"game_launched\":false}\n";
    std::puts("chain418 native mounted-cache/duplicate/empty/shared-drain passed");
}
'''

import json, os, pathlib, subprocess, xml.etree.ElementTree as ET
root = pathlib.Path(__file__).resolve().parents[1]
out = root / 'local/output/wire_g4_c418_probe'
out.mkdir(parents=True, exist_ok=True)
(out/'cache418_probe.cpp').write_text(SOURCE)
result = out/'cache418_fixture/result.json'
result.unlink(missing_ok=True)
ns = {'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
project = ET.parse(root / 'build/win32/bsp_game.vcxproj')
objects = [root/'build/win32/bsp_game.dir/Release'/ (pathlib.Path(x.attrib['Include']).stem+'.obj')
    for x in project.findall('.//m:ClCompile[@Include]',ns)]
vswhere = pathlib.Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio/Installer/vswhere.exe'
vs = pathlib.Path(subprocess.check_output([str(vswhere),'-latest','-products','*','-requires',
    'Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip())
command = subprocess.list2cmdline(['cl.exe','/nologo','/std:c++17','/EHsc','/O2','/MD',
    '/I'+str(root/'include'),str(out/'cache418_probe.cpp'),'/Fo'+str(out/'cache418_probe.obj'),
    '/Fe'+str(out/'cache418_probe.exe'),'/link','/MANIFEST:EMBED','/BASE:0x10000000',
    '/DYNAMICBASE:NO','/ENTRY:mainCRTStartup','/SUBSYSTEM:CONSOLE','/OPT:REF','/OPT:ICF',
    *map(str,objects),*(str(root/'build/win32/Release'/lib) for lib in
    ['bsp_core.lib','bsp_lua511.lib','bsp_zlib121.lib']),
    'd3d9.lib','user32.lib','gdi32.lib','ole32.lib','advapi32.lib','shell32.lib','PowrProf.lib'])
batch = out/'build_cache418_probe.cmd'
batch.write_text('@echo off\ncall "'+str(vs/'VC/Auxiliary/Build/vcvarsall.bat')+'" x86 >nul\n'
    'if errorlevel 1 exit /b 1\n'+command+'\nexit /b %errorlevel%\n')
subprocess.run(['cmd.exe','/d','/c',str(batch)],check=True)
subprocess.run([str(out/'cache418_probe.exe'),
    json.loads((root/'config/target.json').read_text())['binary'],
    str(out/'cache418_fixture')],check=True)

report = json.loads(result.read_text())
assert report['status'] == 'passed'
print(json.dumps(report))
