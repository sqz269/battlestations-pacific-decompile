# Particle resource lifetime

## Complete original bodies

| Address | Complete span | Original ABI | Reconstructed behavior |
| --- | --- | --- | --- |
| AF4280 | AF4280..AF435F, 224 bytes | ECX resource, RET | Release inline child rows, name and reference-counted base |
| AF46E0 | AF46E0..AF46FD, 30 bytes | ECX resource, stack flags, EAX original pointer, RET4 | Destroy; free only for flags bit 0 |

The actual resource is 90h bytes. The recovered destructor uses its reference
count at +4, native string header at +8, inline emitter pointers beginning +10
with signed count +30, and inline Layer pointers beginning +34 with signed
count +54. Each inline region has eight physical slots; the native loops do not
enforce an eight-entry bound. Other payload words are preserved.

AF4280 first stamps D5D958. It walks emitter rows **forward**, decrementing each
captured child's actual LONG +4. Zero references dispatch the child's current
vtable slot 0 with ECX child and no flags argument. Each next row pointer and
signed count is read from current storage after callbacks. The layer count is
read before emitter count is zeroed. The Layer loop then follows the same
forward/current-read pattern and zeros its count. The captured name data is
returned through the application's actual string-pool publications. BD30F0
finally stamps CEB130 without changing the remaining resource bytes.

No host owner, copied resource object, generic destructor callback or substitute
string allocator is introduced. The j10 extension binds the proven Layer
D5DC38 identity to genuine BD30E0/AFACE0 source destruction with the same raw
pool; see `NATIVE_PARTICLE_LAYER_RESOURCE_RELEASE_ORCH4.md`. Other child tables
still require actual callable Win32 terminals. Numeric emitter profiles remain
unbound. The resource profiles described below also receive concrete dispatch.

## Unwind ownership

FuncInfo DF29AC and unwind map DF299C have two states. State 1 invokes CBAB78,
which destroys the current name header at resource+8, then state 0 invokes
CBAB70/BD30F0. The source guard is noexcept: another exception from name cleanup
terminates rather than replacing the initial child-release failure or
continuing lower cleanup. Normal name release disarms name cleanup before the
pool getter, so a failure there performs base cleanup alone. Normal base
destruction follows state -1. No remaining children are released by unwind.

The parent repaired AF46F5..AF46F7 (`ADD ESP,4`) after full disk/live preflight.
The source scalar returns the captured original pointer even after flag-1 free.
It does not free when destruction throws.

## Actual loader profile

The loader's final D0D418 table differs from the base D5D958 table:

| Profile | Slot 0 | Slot +4 |
| --- | --- | --- |
| D5D958 | BD30E0 | AF46E0 |
| D0D418 | BD30E0 | 871FA0 |

BD30E0 does no reference decrement. It calls the current table's scalar slot
+4 with flag 1. Derived 871FA0/871CA0 removes the resource from the actual cache
before AF4280. The complete derived/removal and context dispatch work is
documented in `NATIVE_PARTICLE_RESOURCE_CACHE_REMOVAL_ORCH4.md`.

## Evidence and validation scope

The report `reports/native_particle_resource_lifetime_orch4.json` records full
live/disk byte spans, SHA-256 hashes, exact call rows, original comments/names,
source/object pins, and final build/probe results. All live reads verify project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. This worker made
no live Ghidra writes.

The focused probe executes all 224 installed bytes of AF4280 on its normal
path, adapting its imported InterlockedDecrement cell and three direct provider
calls to genuine rebuilt pool/base bodies. It compares callback mutation of
row pointers and counts, release order, retained bytes and pooled name return
against the source body. A separate source exception case verifies name/base
cleanup after a throwing child. Original FH3/SEH transport and injected native
unwind execution are not claimed. These are new C++ APIs, not drop-in binary
replacements or gameplay validation.

Final strict Win32 Release build passed all three existing CTests after eight
native seed byte pairs were verified. The focused original/source and source
cleanup probes passed against that archive. AF46E0 and the direct D5D958
context-dispatch branch have static/build evidence; the focused runtime probe
uses the final D0D418 scalar path instead.
