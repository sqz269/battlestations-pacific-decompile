# GUI Icon canonical feature preparation

This packet adds an explicit preparation route from the already retained
`GameNativeCanonicalDataOwner` to the Icon delayed-load service. It stores the
reference to the owner's real mutable CRT feature word. It reads no feature value
and selects no raw operation. The exact five production files equal the frozen
8364-byte proposal accepted by Root, SHA256
`fa1947b1af25b7adb8847261c332301256d7ef36f36e30de7b8ada766887aa9b`.

The parent/source base is `1f113e1203aae6200f543c0be21b10ca637bea99`. This isolated packet imports no Source58 or
Source59 branch ancestry. Its diff is five modified production paths plus this
document and the new JSON report. It has no CMake, ledger, Ghidra or Native body
change. Root retains integration/publication while MAINHOLD is active.

The new Game overload is
`prepare_delayed_loads(NativeStringRawPoolContext&, const GameNativeCanonicalDataOwner&)`.
It derives `&canonical_owner.mutable_crt_data().feature_word()` from the genuine
permanent owner. The existing owner accessor names the owned mapped volatile
0109EEA4 cell. Taking its address and passing `*feature` to a reference parameter
does not sample the word. CPU-derived GUI booleans, local feature words, numeric
casts, banks and startup lookups are not used.

StartupHost forwards its existing immutable optional owner. Its existing
constructor checks that this owner supplies the SAME read-only data address;
that check and the process/string/manager checks remain unchanged. Explicit
legacy absence still chooses the old one-argument prepare overload.

The Core overload is
`NativeGuiIconDelayedLoads(strings, publication, const volatile uint32_t&)`.
It only retains an immutable pointer to the reference after the complete old
Impl member prefix. It imports no Game type/header/accessor. The old overload
delegates with explicit absence, never a fallback feature value. Both reach the
same unchanged fresh-null publication guard. Normal getter, allocation,
registration, cancellation and retirement bodies remain byte-exact.

Preparation repeats require the same raw context OBJECT, owner identity and
feature ADDRESS. Exact repeats return the same service without resetting
operation/failure/registration. Either cross-mode reuse is rejected: legacy
preparation cannot be promoted to canonical, and canonical preparation cannot
be silently reused as legacy absence. The latter state is introduced only by
the new overload. Legacy-to-legacy behavior and its wrong-context error remain
unchanged. Potentially throwing metadata allocation/admission finishes before
new borrows are coherently committed. No Native effect is entered by preparation.

The Source declarations/private layouts gain fields and overloads. This is a
C++ interface/layout change, not aggregate or binary ABI compatibility. The
owned canonical mapping gives a genuine live cell reference; it does not prove
Original CRT/CPU initialization, ownership, Origin, private stack aliases,
register/fault/EFLAGS/FH3/SEH behavior, or runtime equivalence.

Static evidence covers twelve exact replacement seams, five full Git/raw/frozen
forward and reverse inverses and every intermediate hash. Seven protected
regions preserve the old invoke/guards/failure logic, all Native operation
bodies, service destructor/public entry wrappers, process status/permanent
owner and Host affinity/preparation suffix/exit/drain policy. Twenty whole
unchanged input files and twelve Source caller/binding sites are pinned.
These twelve are Source sites, not twelve new Native callers. The report stores
all source hashes, exact seam payloads, preserved bytes and this small reader.

No compilation, build, tests, probes, provider/Game execution or Ghidra mutation
was performed. Root must review/build/link and independently admit this packet.
The new Game-layer accessor dependency/private layouts have no compiler proof
here. There is zero new Original/vendor/helper/CG/body/ABI/runtime credit.

Raw57/55/58/59 frames and raw59 observation Acq are deliberately absent. Existing
`make_unique<Operation>()` is unchanged; appending raw no-DMI homes later would
zero them through value-initialization. A separate reviewed default-init or
constructor decision must preserve old DMIs while leaving raw homes untouched.
Actual late flagsBYTE/free-entered/normal-return observations remain a separate
unimplemented binding gate, with no post-HOME rescan, fabricated flags proof or
Origin/ownership authority. Existing profile/domain guards, sticky retention,
retained-service destructor and Host exit policy are unchanged.

`DelayedTextureLoad` still rejects in `gui_icon_runtime.cpp`; the genuine
AB5F00/AB58D0/AB51B0 request/factory/provider/delivery and AB6430/runtime gates
remain open. Retaining a feature reference activates none of them.

## Read-only inverse reader

Run this from an ignored copy with `--root <worktree>`; add `--rev <sealed-commit>`
to inspect an immutable source epoch. It only reads Git/files and checks hashes.
Reader SHA256: `f0fe949bcb059e217239041632ce043cd53d05250e724ce40cabcf1283929d95`.

```python
import argparse,base64,hashlib,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--root',default='.');p.add_argument('--rev');a=p.parse_args()
r=Path(a.root).resolve();d=json.loads((r/'reports/cc16_gui_icon_canonical_feature_preparation.json').read_text())
h=lambda b:hashlib.sha256(b).hexdigest()
g=lambda v,p:subprocess.check_output(['git','-C',str(r),'show',v+':'+p])
actual=lambda p:g(a.rev,p) if a.rev else (r/p).read_bytes()
checks=0
for s in d['static_proof']['sources']:
 b=g(d['source_base'],s['path']);z=actual(s['path']);assert h(b)==s['before']['sha256'] and h(z)==s['after']['sha256']
 q=b;ss=[x for x in d['static_proof']['seams'] if x['path']==s['path']]
 for x in ss:
  o=base64.b64decode(x['old_base64']);n=base64.b64decode(x['new_base64']);assert h(q)==x['before_sha256'] and q.count(o)==1
  q=q.replace(o,n,1);assert h(q)==x['after_sha256']
 assert q==z
 for x in reversed(ss):
  o=base64.b64decode(x['old_base64']);n=base64.b64decode(x['new_base64']);assert q.count(n)==1;q=q.replace(n,o,1)
 assert q==b;checks+=1
for x in d['static_proof']['protected_regions']:
 b=base64.b64decode(x['data_base64']);assert h(b)==x['sha256'] and actual(x['path']).count(b)==1;checks+=1
for x in d['static_proof']['unchanged_input_pins']:
 assert h(g(d['source_base'],x['path']))==x['sha256'] and h(actual(x['path']))==x['sha256'];checks+=1
for x in d['static_proof']['caller_and_binding_sites']:
 b=b''.join(actual(x['path']).splitlines(keepends=True)[x['first_line']-1:x['last_line']]);assert h(b)==x['sha256'];checks+=1
assert h(actual(d['documentation']['path']))==d['documentation']['sha256'];checks+=1
print(json.dumps({'complete_source_pairs':5,'seams':12,'protected_regions':7,'unchanged_inputs':20,'binding_sites':12,'checks':checks,'passed':True}))
```
