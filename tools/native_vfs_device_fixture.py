"""Focused device-route native/source replay; every run writes a NEW output directory."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

SPANS = [('00bdd850',305),('00bdbc70',396),('00bdb680',88),('00bdb670',4)]
DATA = [('00ce7898',2),('00d683f4',12)]

def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def run(args, **kwargs):
    return subprocess.run(args, check=True, text=True, encoding='utf-8', errors='replace',
        creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0), **kwargs)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--baseline',type=Path,required=True,help='Read-only earlier output directory or composition report with fixed_baseline')
    args=parser.parse_args()
    repo=args.repo.resolve();out=args.output.resolve()
    candidate_head=run(['git','rev-parse','HEAD'],cwd=repo,capture_output=True).stdout.strip()
    if out.exists(): parser.error('--output must be a new directory; existing baselines are never overwritten')
    if any('"' in str(p) for p in (repo,out)): parser.error('quotation marks in paths are unsupported')
    if not (repo/'build/win32/Release/bsp_core.lib').exists(): parser.error('Build the selected repository with scripts/build.ps1 first')
    baseline=args.baseline.resolve()
    fixed_baseline=None
    baseline_report_bytes=None
    if baseline:
        if baseline.is_file():
            baseline_report_bytes=baseline.read_bytes()
            baseline_report=json.loads(baseline_report_bytes)
            frozen_rows=baseline_report['fixed_baseline']
            baseline_spans=baseline_report.get('original_spans',baseline_report['functions'])
            fixed_baseline={name:frozen_rows[name.replace('.','_')].encode('utf-8') for name in ('original_inputs.jsonl','original_outcomes.jsonl')}
        else:
            fixed_baseline={name:(baseline/name).read_bytes() for name in ('original_inputs.jsonl','original_outcomes.jsonl')}
            baseline_spans=json.loads((baseline/'inputs/manifest.json').read_bytes())['original_spans']
    out.mkdir(parents=True)
    frozen=out/'inputs';frozen.mkdir()
    for name,content in fixed_baseline.items(): (frozen/('baseline_'+name)).write_bytes(content)
    if baseline_report_bytes is not None: (frozen/'baseline_report.json').write_bytes(baseline_report_bytes)
    sys.path.insert(0,str(repo/'tools'))
    from ghidra_export import pe_summary
    cfg=json.loads((repo/'config/target.json').read_text())
    binary=Path(cfg['binary']);image=binary.read_bytes();pe=pe_summary(binary)
    captured=[];header=['#pragma once','#include <cstdint>','#include <cstddef>',
        'struct OriginalSpan { std::uint32_t address; const unsigned char* bytes; std::size_t size; };']
    for index,(address,size) in enumerate(SPANS+DATA):
        # This CLI verifies the existing project/program before each live read.
        live_text=run([sys.executable,str(repo/'tools/bsp.py'),'ghidra','bytes',address,'--length',str(size)],
            cwd=repo,capture_output=True).stdout
        (frozen/f'{address}.live.txt').write_text(live_text)
        live=b''.join(bytes.fromhex(m.group(1)) for line in live_text.splitlines()
            if (m:=re.match(r'^[0-9a-f]{8}  ((?:[0-9a-f]{2} ?)+)',line)))
        rva=int(address,16)-int(pe['image_base'],16)
        section=next(s for s in pe['sections'] if int(s['rva'],16)<=rva and rva+size<=int(s['rva'],16)+s['raw_size'])
        offset=section['raw_offset']+rva-int(section['rva'],16);disk=image[offset:offset+size]
        if disk!=live: raise RuntimeError(f'Original live/disk mismatch at {address}')
        (frozen/f'{address}.bin').write_bytes(disk)
        captured.append(dict(address=address,bytes=size,sha256=hashlib.sha256(disk).hexdigest(),live_matches_disk=True))
        if index<len(SPANS): header.append(f'const unsigned char bytes_{address}[]={{'+','.join(f'0x{b:02x}' for b in disk)+'};')
    captured_by_address={p['address']:p for p in captured}
    for pin in baseline_spans:
        current=captured_by_address[pin['address']]
        if any(current[k]!=pin[k] for k in ('bytes','sha256')): raise RuntimeError('Original bytes differ from fixed baseline')
    header.append('const OriginalSpan spans[]={'+','.join('{0x'+address+',bytes_'+address+','+str(size)+'}' for address,size in SPANS)+'};')
    (frozen/'original_data.hpp').write_text('\n'.join(header)+'\n')
    source_paths=['tools/native_vfs_device_fixture.cpp','src/native_vfs_device_route.cpp','src/native_vfs_lookup_routes.cpp']
    source_pins=[]
    for relative in source_paths:
        target=frozen/Path(relative).name;shutil.copy2(repo/relative,target)
        source_pins.append(dict(path=relative,sha256=digest(repo/relative),frozen_copy=str(target),
            compiled=relative.startswith('tools/')))
    # The target routines come from exact candidate build objects, without recompilation.
    candidate_objects=[]
    for name in ('native_vfs_device_route','native_vfs_lookup_routes'):
        current=repo/'build/win32/bsp_core.dir/Release'/(name+'.obj')
        target=frozen/(name+'.obj');shutil.copy2(current,target)
        if digest(current)!=digest(target): raise RuntimeError(f'Object changed while freezing: {current}')
        candidate_objects.append(dict(path=str(current),frozen=str(target.relative_to(out)),sha256=digest(target)))
    shutil.copy2(Path(__file__),frozen/'native_vfs_device_fixture.py')
    # Before compilation, retain fingerprints for candidate include dependencies.
    before_headers={p.resolve():digest(p) for p in (repo/'include').rglob('*.hpp')}
    libs=[repo/'build/win32/Release/bsp_core.lib']
    for name in ('bsp_lua511.lib','bsp_zlib121.lib'):
        matches=list((repo/'build/win32').rglob(name))
        if len(matches)!=1: raise RuntimeError(f'Expected exactly one built {name}: {matches}')
        libs.append(matches[0])
    manifest=dict(repo=str(repo),head=candidate_head,
        project=cfg['project_file'],program=cfg['program_path'],installed_executable=str(binary),installed_sha256=pe['sha256'],
        original_spans=captured,source_pins=source_pins,candidate_objects=candidate_objects,
        libraries=[dict(path=str(p),sha256=digest(p)) for p in libs],
        support_contracts=['Actual shared resize, cstring construction, concat, normalization, pool getter/return and current host memmove',
            'BDD850 original calls shared actual BDD0A0 traversal and source BDBC70; BDBC70 original is compared independently',
            'Instrumented provider+24 returns raw result, optionally writes b, and replaces current payload provider',
            'No native exception propagation; aliased callback output case does not destroy the externally backed visitor name'],
        cases=['empty success raw80','nonempty success raw80','nonempty false concatenation','false result becomes current m via aliased output',
            'true result becomes current zero via embedded-NUL aliased output','empty false preserves old name',
            'BDD850 normalized successful device and cleanup','BDD850 normalized failure and cleanup'],
        fixed_mappings=dict(arena='31000000',pool='32000000',original_code='34000000',profile='00d683f4',slash='00ce7898'),
        baseline=str(baseline) if baseline else None)
    (frozen/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    vswhere=Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio/Installer/vswhere.exe'
    vs=Path(run([str(vswhere),'-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
        '-property','installationPath'],capture_output=True).stdout.strip())
    vcvars=vs/'VC/Auxiliary/Build/vcvarsall.bat'
    objects=[out/p['frozen'] for p in candidate_objects]
    for relative in source_paths[:1]:
        copied=frozen/Path(relative).name;obj=out/(copied.stem+'.obj');objects.append(obj)
        rsp=out/(copied.stem+'.compile.rsp')
        rsp.write_text('\n'.join(['/nologo','/std:c++17','/EHsc','/MD','/O2','/W4','/WX','/fp:strict','/showIncludes','/c',
            f'/I"{repo / "include"}"',f'/I"{frozen}"',f'/Fo"{obj}"',f'"{copied}"'])+'\n')
        with (out/(copied.stem+'.includes.log')).open('w',encoding='utf-8') as log:
            batch=out/(copied.stem+'.compile.cmd')
            batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b %errorlevel%\ncl.exe @"{rsp}"\nexit /b %errorlevel%\n')
            run(['cmd','/d','/c',str(batch)],cwd=out,stdout=log,stderr=subprocess.STDOUT)
    include_pins={}
    for log in out.glob('*.includes.log'):
        for line in log.read_text().splitlines():
            if 'including file:' not in line: continue
            path=Path(line.split('including file:',1)[1].strip()).resolve()
            if path in before_headers:
                if digest(path)!=before_headers[path]: raise RuntimeError(f'Header changed during compilation: {path}')
                relative=path.relative_to(repo).as_posix();target=frozen/'headers'/relative
                target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(path,target)
                include_pins[relative]=before_headers[path]
    manifest['compiled_include_pins']=[dict(path=p,sha256=h) for p,h in sorted(include_pins.items())]
    exe=out/'device_probe.exe';link_rsp=out/'link.rsp'
    link_rsp.write_text('\n'.join(['/NOLOGO','/MACHINE:X86','/MANIFEST:EMBED','/DYNAMICBASE:NO','/BASE:0x20000000',
        '/INCREMENTAL:NO','/VERBOSE:LIB',f'/MAP:"{out / "device_probe.map"}"',f'/OUT:"{exe}"']+
        [f'"{p}"' for p in objects+libs]+['user32.lib','d3d9.lib'])+'\n')
    with (out/'link.log').open('w',encoding='utf-8') as log:
        batch=out/'link_fixture.cmd'
        batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b %errorlevel%\nlink.exe @"{link_rsp}"\nexit /b %errorlevel%\n')
        run(['cmd','/d','/c',str(batch)],cwd=out,stdout=log,stderr=subprocess.STDOUT)
    for pin in source_pins:
        if digest(repo/pin['path'])!=pin['sha256']: raise RuntimeError(f'Candidate source changed: {pin["path"]}')
    for pin in manifest['libraries']:
        if digest(pin['path'])!=pin['sha256']: raise RuntimeError('Library changed while compiling fixture')
    for pin in candidate_objects:
        if digest(pin['path'])!=pin['sha256']: raise RuntimeError('Candidate object changed while compiling fixture')
    manifest['objects']=[dict(path=str(p),sha256=digest(p)) for p in objects]
    manifest['executable_sha256']=digest(exe)
    manifest['map_sha256']=digest(out/'device_probe.map')
    manifest['link_log_sha256']=digest(out/'link.log')
    # The final input manifest is sealed before either native or candidate execution.
    (frozen/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    with (out/'execution.log').open('w',encoding='utf-8') as log:
        run([str(exe),str(out)],cwd=out,stdout=log,stderr=subprocess.STDOUT)
    original=(out/'original_outcomes.jsonl').read_bytes();actual=(out/'source_outcomes.jsonl').read_bytes()
    if original!=actual: raise RuntimeError('Source outcomes differ from original outcomes')
    if fixed_baseline:
        for name,content in fixed_baseline.items():
            if (out/name).read_bytes()!=content: raise RuntimeError(f'Replay differs from fixed baseline {name}')
            if baseline_report_bytes is None and (baseline/name).read_bytes()!=content:
                raise RuntimeError('Earlier baseline was modified')
        if baseline_report_bytes is not None and baseline.read_bytes()!=baseline_report_bytes:
            raise RuntimeError('Earlier baseline report was modified')
    result=dict(status='passed',variants=8,original_routines=4,original_code_bytes=793,profile_and_literal_bytes=14,
        normalized_arena_bytes_per_variant=4096,actual_pool_prefix_bytes_per_variant=0x8ad484,
        current_include_pins=len(include_pins),baseline_replayed=True,original_source_outcomes_match=True,
        exact_candidate_objects_linked=len(candidate_objects),
        input_manifest_sha256=digest(frozen/'manifest.json'),original_inputs_sha256=digest(out/'original_inputs.jsonl'),
        original_outcomes_sha256=digest(out/'original_outcomes.jsonl'),source_outcomes_sha256=digest(out/'source_outcomes.jsonl'),
        installed_or_gameplay_validation=False)
    (out/'result.json').write_text(json.dumps(result,indent=2)+'\n')
    after_head=run(['git','rev-parse','HEAD'],cwd=repo,capture_output=True).stdout.strip()
    if after_head!=manifest['head']: raise RuntimeError('Candidate HEAD changed during fixture')
    for pin in candidate_objects+manifest['libraries']:
        if digest(pin['path'])!=pin['sha256']: raise RuntimeError('Candidate linked input changed during fixture')
    inputs=[dict(path=str((repo/p['path']).resolve()),frozen=str(Path(p['frozen_copy']).relative_to(out)),sha256=p['sha256']) for p in source_pins]
    inputs += candidate_objects
    inputs += [dict(path=str(repo/p),frozen=str((frozen/'headers'/p).relative_to(out)),sha256=h) for p,h in sorted(include_pins.items())]
    inputs.append(dict(path=str(Path(__file__).resolve()),frozen='inputs/native_vfs_device_fixture.py',sha256=digest(frozen/'native_vfs_device_fixture.py')))
    if baseline_report_bytes is not None:
        inputs.append(dict(path=str(baseline),frozen='inputs/baseline_report.json',sha256=digest(frozen/'baseline_report.json')))
    replay=dict(status='passed',repo=str(repo),candidate_head=after_head,inputs=inputs,
        libraries={Path(p['path']).name:p for p in manifest['libraries']},
        result=dict(variants=8,original_routines=4,original_code_bytes=793,
            outcomes=[json.loads(line) for line in original.decode().splitlines()]),
        executable_sha256=digest(exe),link_map_sha256=digest(out/'device_probe.map'))
    (out/'replay.json').write_text(json.dumps(replay,indent=2)+'\n')
    print(json.dumps(result))

if __name__=='__main__':
    main()
