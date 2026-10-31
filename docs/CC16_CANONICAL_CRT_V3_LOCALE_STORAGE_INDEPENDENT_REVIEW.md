# Canonical CRT V3 locale storage independent review

No actionable bounded Source finding in `632c11710db5951e0e103dc8281a3fb46477b345`, parent `a536cbd9cd3ad081825ecc8d1a6606232ca6c75e`. This review covers the exact four support files and their new storage/API paths. It does not select V3 in Main, supply a locale setter or close a CRT provider.

The author packet contains four modifications and its two new evidence files. This receipt contains only two new review files. All four complete Source preimages equal the historical71573 proposal preimages; all four postimages equal the approved candidates. Independent Git/raw/frozen reconstruction passed220 checks:22 seams and all intermediate hashes, four complete forward/reverse inverses, five frozen lifecycle witnesses, seven unchanged whole inputs, both Main sites and the full recursive author-tree inverse. The entire canonical owner constructor/joint/initialize suffix and reservation destructor/move/ACK region also remain byte-equal. Whole input pins supply integrity; they do not assert review of unrelated provider bodies.

The3313B author reader was inspected before execution. Its raw script, embedded block and report are equal; exact sealed-revision mode passed25 checks, including the six-file recursive inverse. The independent reconstruction uses its own reader/proof and preserves the original proposal and author reports unchanged.

## Wire, admission and lifetime

The enum appends `CanonicalCrtV3`, preserving old values0/1. V1/V2 still select the exact40B record/view. V3 selects44B in the existing4096B section: the first40B legacy record plus the DWORD descriptor at offset40. It requires version3, unchanged E15000/E16000/109E000 descriptors, total role bits0xF and extra0109D000 descriptor. The selected plan is explicit. Version comparison precedes the V3 tail read; a mismatched V3 record cannot promote a V1/V2 caller. Parent tail stores also require V3. No V1/V2 path reads or writes the extra descriptor.

The same two mutable64K allocation bases, E10000 and1090000, remain responsible for storage. D000 is inside the latter and creates no independent allocation owner. The mutable owner captures a private immutable plan before the existing subset transfer. Both readonly and mutable constructors transfer their disjoint custody before either initializer runs. Only the V3 initializer commits0109D000 with PAGE_READWRITE. Existing range checks handle legitimate D000/E000 page coalescing and still require the actual allocation base, MEM_PRIVATE, committed state and full-page protection.

The supported installed PE is12223752B, SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Its loaded .data begins00E08000, has raw size10000 and virtual size297EDC. The whole0109D000 page lies beyond raw data and inside the virtual tail, supporting its initial4096 zero bytes. V3 seeds and checks these bytes once during private initialization. Later page verification checks custody/protection without reseeding or requiring a permanently zero locale flag; a later legitimate1 remains allowed.

The actual live accessor returns `const volatile std::int32_t&` to0109DE1C. It rejects a non-V3 owner by metadata check, reads no word value and supplies no fallback or setter. Borrow preparation can throw and is not a NoThrow promise. The new enum/private member and API are Source layout changes; Win32 compiler validation remains required.

Existing joint verification, cookie initialization, final page checks, `finish(true)` CAS, permanent owner release/store and failure claim-clear code are exact. Existing parent wait/timeout/cancellation and resource policy are exact. Partial transfer rollback remains at allocation bases: untransferred subsets stay with the reservation, transferred subsets with existing owners, and1090000 is released once rather than releasing a D000 subpage. No new cancellation, cleanup, retry or termination policy is introduced.

## Provenance and remaining gates

Read-only verification used `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe` (PID47268). Native bytes atC02865 are `c7 05 1c de 09 01 01 00 00 00`, matching the installed PE and the recognized `_setlocale` DWORD1 store. Ghidra returned25 recognized references:24 reads and this one write. This is a known static-xref observation, not a claim that no other dynamic writer can exist. No Source setter, full BF7EEC/LocaleUpdate/PTD/encoded-handler domain or genuine used comparator provider is supplied here. The initial zero page does not remove those dependencies.

Both actual Main sites remain explicit `CanonicalCrtV2` at387 and1120; the entire Main file is unchanged. The optional two-token selection patch is excluded. Root still needs actual compiler/Core/link and separately authorized V3 handoff/startup validation. This review executed no compiler, build, tests, probe, provider, Game or startup path and changed no Source, CMake, ledger or Ghidra data. Original CRT/CPU, Native Origin, Original private ABI/aliases/fault/EFLAGS/FH3/SEH, runtime, activation and gameplay remain unproved. New Original/vendor/helper/CG/body/duplicate credit is zero.

Frozen independent proof: `J:\PROG\battlestations-pacific-decompile-codex-canonical-crt-v3-locale-storage-independent-review\local\output\cc16_canonical_crt_v3_locale_storage_independent_review_20261031`. Its manifest has45 artifacts, SHA256 `061f67c3157eac75351f927f39bfb8a67c9d68aa5cfd0d2e75f33096fb5ab5aa`. The report retains exact Source/region pins,22 seam payloads/intermediate hashes and the bounded Native/PE observations.

Read-only review reader:2400B, SHA256 `bc532c822d5cbf311cd3973e191b1a34c4638381f578a856b76b6f878f62fb48`. It validates Git/file/hash facts only; `--review-rev` additionally requires this review's exact two-added-file scope.

```python
import argparse,base64,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--root',default='.');p.add_argument('--review-rev');a=p.parse_args();root=Path(a.root).resolve()
r=json.loads((root/'reports/cc16_canonical_crt_v3_locale_storage_independent_review.json').read_bytes());g=lambda *v:subprocess.check_output(['git','-C',str(root),*v]);h=lambda b:hashlib.sha256(b).hexdigest();get=lambda rev,p:g('show',rev+':'+p);n=0
A=r['reviewed_commit'];B=r['source_base'];assert g('rev-parse',A+'^').decode().strip()==B
for s in r['sources']:
 before=get(B,s['path']);after=get(A,s['path']);assert h(before)==s['before_sha256'] and h(after)==s['after_sha256'];q=before;ss=[x for x in r['seams'] if x['path']==s['path']]
 for x in ss:
  old=base64.b64decode(x['old_base64']);new=base64.b64decode(x['new_base64']);assert h(q)==x['before_sha256'] and q.count(old)==1;q=q.replace(old,new,1);assert h(q)==x['after_sha256']
 assert q==after
 for x in reversed(ss):
  old=base64.b64decode(x['old_base64']);new=base64.b64decode(x['new_base64']);assert q.count(new)==1;q=q.replace(new,old,1);assert h(q)==x['before_sha256']
 assert q==before;n+=1
for x in r['protected_regions']:
 b=base64.b64decode(x['data_base64']);assert h(b)==x['sha256'] and get(B,x['path']).count(b)==get(A,x['path']).count(b)==1;n+=1
for x in r['unchanged_inputs']:
 b=get(A,x['path']);assert b==get(B,x['path']) and h(b)==x['sha256'];n+=1
main=get(A,'src/game_main.cpp');assert main.count(b'GameNativeDataPlan::CanonicalCrtV2')==2 and b'CanonicalCrtV3' not in main;n+=1
before=g('ls-tree','-r','-z',B);bm={x.split(b'\t',1)[1]:x for x in before.split(b'\0') if x};mods={x['path'].encode() for x in r['sources']};adds={x.encode() for x in r['author_added_paths']};restored=[]
for x in g('ls-tree','-r','-z',A).split(b'\0'):
 if not x:continue
 path=x.split(b'\t',1)[1]
 if path not in adds:restored.append(bm[path] if path in mods else x)
assert b'\0'.join(restored)+b'\0'==before;n+=1
assert r['findings']==[] and not any(r['runtime_claims'].values());n+=1
if a.review_rev:
 assert g('rev-parse',a.review_rev+'^').decode().strip()==B;assert sorted(g('diff','--name-status',B,a.review_rev).decode().splitlines())==sorted('A\t'+x for x in r['review_paths']);n+=1
print(json.dumps({'passed':True,'checks':n,'four_full_inverses':True,'author_tree_inverse':True,'review_scope_checked':bool(a.review_rev)}))
```
