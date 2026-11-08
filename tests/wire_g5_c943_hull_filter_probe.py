"""Build one isolated fixture from the actual contact-owner definitions.

MSVC resolves even dead terrain/application references in the full owner TU.
Select its complete storage definitions and filter/lifetime methods verbatim;
do not provide replacement implementations for those unrelated services.
Run from the repo with: python tests/wire_g5_c943_hull_filter_probe.py
"""
import os
from pathlib import Path
import subprocess

repo = Path(__file__).resolve().parents[1]
source = (repo / "src/hull_terrain_contact.cpp").read_text()


def between(start, end):
    first = source.index(start)
    return source[first:source.index(end, first)]


fragment = source[:source.index("namespace bsp {\nnamespace {")]
fragment += "namespace bsp {\n"
fragment += between("struct alignas(16) NativeBody {", "float f32(double v)")
fragment += between("struct HullTerrainContactSolver::Manifold {", "static_assert(sizeof(DynSolverContactPoint)")
fragment += between("struct HullTerrainContactSolver::TerrainTile {", "HullTerrainContactStepResult HullTerrainContactSolver::native_narrow_phase")
fragment += between("struct HullTerrainContactSolver::DraftShape {", "void HullTerrainContactSolver::set_draft_bodies")
fragment += between("HullTerrainContactSolver::HullTerrainContactSolver()", "// 006FAD70's shape build")
fragment += "\n} // namespace bsp\n"
fixture = (repo / "tests/wire_g5_c943_hull_filter_probe.cpp").read_text()
fixture = fixture.replace('#include "../src/hull_terrain_contact.cpp"', fragment)
(repo / "local/c943-probe.cpp").write_text(fixture)
vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
                              "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property",
                              "installationPath"], text=True).strip()
batch = repo / "local/c943-probe.cmd"
batch.write_text('@echo off\ncall "' + vs + '\\VC\\Auxiliary\\Build\\vcvars32.bat" >nul\n'
                 'cl /nologo /std:c++17 /EHsc /MD /O2 /Gy /I include '
                 '/Fo:local/c943-probe.obj /Fe:local/c943-probe.exe '
                 'local/c943-probe.cpp build/win32/Release/bsp_core.lib '
                 '/link /OPT:REF /MANIFEST:EMBED\n'
                 'if errorlevel 1 exit /b 1\nlocal\\c943-probe.exe\n')
result = subprocess.run(["cmd.exe", "/c", str(batch)], cwd=repo, capture_output=True, text=True)
(repo / "local/c943-probe.log").write_text(result.stdout + result.stderr)
print((result.stdout + result.stderr)[-3000:])
raise SystemExit(result.returncode)
