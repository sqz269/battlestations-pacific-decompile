# Canonical CRT V3 locale storage support

This packet applies the exact approved11713B proposal SHA256 `5031290f1c39ddd69ed02609689bb5d221eab09b5a3babb3526f1e5b1f50335a` to FOUR existing support files. Its actual published parent is `a536cbd9cd3ad081825ecc8d1a6606232ca6c75e`; all four preimages and unchanged Main equal the historical proposal base71573f30. It adds storage support only. No Main selection, locale setter, comparator provider, Original CRT initialization, ABI or runtime closure follows.

## Versioned wire and physical ownership

`GameNativeDataPlan::CanonicalCrtV3` is explicit. V1/V2 retain their original40B record and view sizes, page descriptors, role bits, defaults and admission paths. Neither reads nor writes the new tail. V3 uses a44B record inside the existing4096B shared section: the unchanged40B legacy first member plus a DWORD D000-page descriptor at offset40. Version3 requires exact old E15000/E16000/109E000 descriptors, total role mask0xF and extra descriptor0109D000; V2 stays0x7. A plan mismatch rejects before adoption rather than promoting/inferencing a plan from a record.

The SAME two mutable allocation bases remain E10000 and1090000; the existing64K1090000 custody already encloses D000. This support adds no allocation band, second bank or standalone page owner. The mutable owner captures the immutable plan before its subset transfer. Read-only and mutable constructors still transfer their disjoint custody first; only afterward may initialization commit D000 with PAGE_READWRITE. Existing range/protection checks accept legitimate coalesced D000/E000 committed regions and require the same allocation base, MEM_PRIVATE and complete page coverage.

Supported installed PE has .data RVA A08000, raw size10000 and virtual size297EDC. The entire0109D000 page is beyond raw data and within the virtual tail: its loader preimage is4096 zero bytes. V3 initializes and verifies that full preimage once during private initialization. `verify_pages` subsequently checks ownership/protection, not permanent zero contents. No later access, repeated operation or view reseeds the page or rejects a legitimate later1. This proves initial storage bytes, not a lasting ASCII domain or a complete CRT service.

## Borrow and existing lifetime sequence

`const volatile int32_t& GameNativeMutableCrtData::locale_changed_0109de1c() const` returns only the actual live reference. It reads no cell value and supplies no substitute word, setter or public writable mode flag. Calling it without the explicit V3 role throws a metadata error; callers must obtain the qualified borrow before Native work. The private constructor/noncopyable owner and permanent canonical lifetime remain the underlying storage contract, not an Original fault/NoThrow promise.

The existing joint verification, security-cookie initialization, page verification, `owned.finish(true)` readiness CAS and final permanent pointer/atomic publication stay byte-exact. All heap ownership metadata exists before the successful ACK. The existing CAS never overwrites a parent's cancelled state. Existing parent wait/timeout/fail-child policy is untouched; no extra handshake, retry or terminal policy is added.

Rollback remains at actual allocation bases. A subset not transferred stays with the capability; transferred subsets belong to their existing owners. Initializer failure releases the whole1090000 allocation once, including D000/E000, rather than releasing a subpage. Existing pending-owner deletion, process_claim.clear on failure and resource release semantics are preserved; no new retry loop or Native-payload compensation is supplied.

## Main and semantic gates remain unchanged

`run_bootstrap_parent` line387 and `WinMain` line1120 both still explicitly select `CanonicalCrtV2`. The optional two-token Main-selection patch is excluded. No runtime flag, automatic V3 adoption, guard flip or default upgrade is added. V3 support must be explicitly selected at both ends in a later separately authorized integration before any V3 startup exercise.

Original0109DE1C has24 recognized reads and one recognized write: successful nonnull non-C `_setlocale` stores1 atC02865, before later thread/global locale publication. Lua os.setlocale and C++ locale constructors/destructors can reach it. Returning to C does not reset that recognized sticky bit. Mapping its loader0 does not make the Native BF7FBF nonzero arm unnecessary. The real Source setter event, full BF7EEC/LocaleUpdate/PTD/handler domain and used NativeCrtStricmpCallees provider remain missing. Existing owning-Source &_errno and SDK noinfo services keep their prior qualified contracts; no Host_stricmp, synthetic handler or fake producer is installed.

## Evidence and limits

The four raw Source postimages exactly match the approved candidates. All22 seams and intermediate forward/reverse hashes and complete four-file inverses are frozen. Five exact witnesses preserve ACK/cancel, parent wait, joint/cookie/publication/rollback and allocation/protection code. Seven unchanged whole-file pins include Main, read-only/cookie providers and the existing comparator parent; integrity pins do not claim unrelated-body review. Full recursive six-file commit inversion is checked after sealing by replacing the four changed blobs with exact preimages and removing only this doc/report.

Historical read-only locale audit SHA256 `9e297fdaa7a8457e79094c52181acac8e7c7a9f08145e214635c4a12ae4bf14a` retains verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, native producer/caller/table epochs and installed-PE evidence. It is an earlier source epoch, not a current-input claim. Its initial-page blob is DATA, not an executable. No new Ghidra query, mutation or vendor body recovery is required for this exact support patch.

This is ordinary Source metadata/mapping composition. Source overload/private layout and40/44B wire assertions require Root's actual MSVC Win32 validation; no compiler, Core-object, handoff/startup, probe, provider, Game or test execution was performed here. Root independent peer, compiler/Core and controlled startup verification are deferred. Setter/provider/CRT readiness, Native Origin, Original ABI/private aliases/register/fault/EFLAGS/FH3/SEH, runtime and gameplay remain false. Zero new Original/vendor/helper/CG/body/duplicate credit. No CMake, ledger, Ghidra or other Source edits are included.

Read-only static reader SHA256 `55588c5558dd580f70f8ee52463799c32a047b8e526672d18e69f34190c987e3` follows; it executes only Git/file/hash/text checks, with optional exact sealed-revision inverse.

```python
import argparse,base64,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--root',default='.');p.add_argument('--rev');a=p.parse_args();root=Path(a.root).resolve()
r=json.loads((root/'reports/cc16_canonical_crt_v3_locale_storage.json').read_bytes());h=lambda b:hashlib.sha256(b).hexdigest()
g=lambda *v:subprocess.check_output(['git','-C',str(root),*v]);actual=lambda p:g('show',a.rev+':'+p) if a.rev else (root/p).read_bytes();checks=0
for s in r['sources']:
 before=g('show',r['source_base']+':'+s['path']);after=actual(s['path']);assert len(before)==s['before']['bytes'] and h(before)==s['before']['sha256'] and before==Path(s['before']['frozen']).read_bytes();assert len(after)==s['after']['bytes'] and h(after)==s['after']['sha256'] and after==Path(s['after']['frozen']).read_bytes()
 q=before;ss=[x for x in r['seams'] if x['path']==s['path']]
 for x in ss:
  old=base64.b64decode(x['old_base64']);new=base64.b64decode(x['new_base64']);assert h(q)==x['before_sha256'] and q.count(old)==1;q=q.replace(old,new,1);assert h(q)==x['after_sha256']
 assert q==after
 for x in reversed(ss):
  old=base64.b64decode(x['old_base64']);new=base64.b64decode(x['new_base64']);assert q.count(new)==1;q=q.replace(new,old,1);assert h(q)==x['before_sha256']
 assert q==before;checks+=1
for x in r['protected_regions']:
 b=base64.b64decode(x['data_base64']);assert len(b)==x['bytes'] and h(b)==x['sha256'] and actual(x['path']).count(b)==1 and Path(x['frozen']).read_bytes()==b;checks+=1
for x in r['unchanged_inputs']:
 b=actual(x['path']);assert len(b)==x['bytes'] and h(b)==x['sha256'] and b==g('show',r['source_base']+':'+x['path']) and Path(x['frozen']).read_bytes()==b;checks+=1
for x in r['Main_sites']:
 b=b''.join(actual(x['path']).splitlines(keepends=True)[x['first_line']-1:x['last_line']]);assert len(b)==x['bytes'] and h(b)==x['sha256'];checks+=1
assert actual('src/game_main.cpp').count(b'GameNativeDataPlan::CanonicalCrtV2')==2 and b'CanonicalCrtV3' not in actual('src/game_main.cpp');checks+=1
for x in [r['approved_patch'],r['historical_readiness'],r['page_preimage']]:
 b=Path(x['path']).read_bytes();assert len(b)==x['bytes'] and h(b)==x['sha256'];checks+=1
x=r['document_pin'];assert h(actual(x['path']))==x['sha256'];checks+=1
assert r['compiled'] is False and r['Main_V3_selected'] is False and r['setter_or_provider_closed'] is False;checks+=1
inverse=False
if a.rev:
 assert g('rev-parse',a.rev+'^').decode().strip()==r['source_base'];before=g('ls-tree','-r','-z',r['source_base']);bm={x.split(b'\t',1)[1]:x for x in before.split(b'\0') if x};mods={x['path'].encode() for x in r['sources']};adds={r['documentation_path'].encode(),r['report_path'].encode()};restored=[]
 for x in g('ls-tree','-r','-z',a.rev).split(b'\0'):
  if not x:continue
  path=x.split(b'\t',1)[1]
  if path not in adds:restored.append(bm[path] if path in mods else x)
 assert b'\0'.join(restored)+b'\0'==before;assert sorted(g('diff','--name-status',r['source_base'],a.rev).decode().splitlines())==sorted(['M\t'+x['path'] for x in r['sources']]+['A\t'+x for x in [r['documentation_path'],r['report_path']]]);inverse=True;checks+=1
print(json.dumps({'passed':True,'checks':checks,'pairs':4,'seams':22,'protected_regions':5,'unchanged_inputs':7,'Main_sites':2,'recursive_inverse':inverse}))
```
