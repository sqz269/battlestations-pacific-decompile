# Native instance-generator attachment and lifetime

Addresses: `00B41710`, `00B41780`, `00B417C0`, `00B44FD0`, `00B450A0`,
`00B450D0`, `00B451A0`, `00B451D0`, `00B55B20`, `00B55BE0`, `00B55CB0`,
`00B85610`; seventeen compiler support bodies listed in the report.

Twelve complete ordinary bodies (1,430 bytes) are reconstructed in
`src/native_instance_generator_owner.cpp`, with a tracked Win32 source registration.
The section finalizer now reaches actual generator construction, declaration and
hardware-layout acquisition, binding publication and canonical destruction. This
closes the generator dependency identified by the CJ texture-field packet. The
enclosing subset still requires numeric-profile material/effect admission.

Evidence: `reports/native_instance_generator_ck.json`, its flow/annotation/integration
reports, and frozen artifacts in `local/native_instance_generator_ck/registered/`.
Descriptive symbols are hypotheses; old Ghidra names and comments were recorded.
Existing meaningful names and compiler `Unwind@` names were retained; generic
function/destructor placeholders were given names supported by the producer and
virtual-table evidence.

## Bodies and storage

| Address | Bytes | Native behavior |
|---|---:|---|
| B41710 | 112 | Binding destructor: release and clear generator+C, destroy base |
| B41780 | 59 | Binding generator assignment: identity skip, publish/retain/release |
| B417C0 | 30 | Binding scalar delete, optional raw free |
| B44FD0 | 182 | Generic constructor, exact 17-character declaration name |
| B450A0 | 36 | Generic scalar delete |
| B450D0 | 182 | Building constructor, exact 41-character declaration name |
| B451A0 | 36 | Building scalar delete |
| B451D0 | 339 | Effect-selected generator creation and section binding |
| B55B20 | 189 | Generator base plus instance declaration and combined layout |
| B55BE0 | 205 | Generator resource, name and base destruction |
| B55CB0 | 30 | Base generator scalar delete |
| B85610 | 30 | Section material/effect checks and full attachment call |

The generator occupies `1C` bytes: profile at zero, actual reference count at `+4`,
an embedded eight-byte name at `+8`, instance declaration at `+10`, combined layout
at `+14`, and a retained resource slot at `+18`. Construction clears all fields after
the count before acquiring declarations. This packet does not identify a producer
for nonnull `+18`; destruction nevertheless preserves its observed release order.

The binding occupies `10` bytes: profile `D619F8`, actual count at `+4`, a serial
at `+8` from the current `0108FD30` cell, and generator at `+C`. Serial publication
precedes incrementing the live cell. The binding owns a retained generator; the
section owns a retained binding at `+5C`. The existing `B417E0` section setter is reused.

`B55B20` calls the actual current renderer `+38` (`B317E0`) for the instance
declaration. It then reads the first stream at section `+3C`, obtains its current
`+24` descriptor (`B48CE0`), and creates a two-entry layout key in section-first,
instance-second order. Actual renderer `+40` (`B2F710`) returns the combined layout.
Unused key pointer words remain uninitialized. The opaque middle constructor
argument is forwarded through the wrappers but not read by this base body.

The generic name is `uf44uf44uf44.mvfm`; the building name has nine `uf44` groups
and the same suffix. The derived constructors resize their temporary name to
17/41 characters, copy its current length plus terminator, call the complete base,
return the temporary buffer, and publish `D61BFC`/`D61C1C`. Those tables supply the
existing `B556F0`/`B55780` instance writers. This packet constructs their native
producers; it does not replace those writer implementations.

`B451D0` tests the current effect's primary descriptor at `+C4`, string header `+28`.
An empty string leaves the existing section binding unchanged. It compares
case-insensitively with `building`, then reloads effect `+C4` before testing
`generic`. Other strings also leave the binding unchanged. A selected generator
is constructed in a raw `1C` allocation. The subsequent `10`-byte binding retains
it, the generator creator is released, the section retains the binding, and the
binding creator is released. `B85610` performs only the two material/effect null
checks before invoking this complete path.

All new companions borrow the actual `+4` word and register in the existing
geometry owner domain. There is no second native count, resolver or cache. Current
numeric virtual-table words select the recovered scalar destructor bodies. Host
registration adds no retain or rollback; an interrupted bind retains the creator
and exposes the unbound companion. The concrete graphics/layout services must
share the same renderer publication, declaration/string pools, hardware tree and
canonical owners, and outlive all resulting references.
The acquired frame's `raw_generator` is an allocation audit identity after
construction, not another owning reference; it may be stale after native transfer
and retirement. Its `generator` and `binding` fields track outstanding creators.

## Cleanup and flow recovery

`B55B20`'s constructor unwind protects the embedded name and base only. It does not
release an already acquired `+10` declaration or a layout left in an interrupted
provider frame. The outer attachment's two constructor unwind actions free the
raw allocation only. After generator construction succeeds, no outer native unwind
owns the completed generator or binding. Source acquired records preserve these
effects instead of adding destructor rollback.

The generic/building unwind maps each have state zero (temporary name, then -1),
state one (base generator destructor, then zero), and state two (base generator
destructor, then -1). Normal temporary return occurs in state two, preventing a
second temporary-name release. The existing source string-storage release API is
`noexcept`; a native exception during that return is outside this C++ exception
domain. The complete original state maps are recorded without claiming original
FH3 execution.

`B55BE0` publishes `D62190`, releases then clears `+18`, `+10`, and `+14` in order,
returns the name, and publishes base `CEB130`. On a resource cleanup exception,
only the name and base unwind; later resource slots are not processed and the
failed slot is not retried or cleared. Binding destruction similarly protects
only the base while releasing and clearing generator `+C`. Scalar delete frees
the raw allocation only after completed destruction and only for flags bit zero;
EAX retains the original address bits after free.

The saved/disk audit covers 1,590 code bytes, six unwind-map/FuncInfo records,
profile tables and declaration/selection strings. All 513 instruction addresses
have the expected owner. All 49 direct transfer rows pass the call-site verifier;
18 indirect transfers are recorded separately.

Four scalar deleting destructors had three-byte post-`_free` gaps. Two raw-free
unwind helpers lacked their final `POP ECX; RET`. These six continuations were
restored under the write lock, preserving old comments/names before repair. Six
missing ten-byte exception handlers were created. No bytes were cleared, and no
callee no-return flag was changed. Twenty-nine evidence comments and twelve ABI
views/source records were saved; affected exports were refreshed.

Installed executable SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Validation and limits

The default tracked MSVC Win32 build passed both existing CTests. No project-include
override is cached. The shared CMake registry received one source line without a
whole-file lease, following `docs/COORDINATION.md`. This is an incremental build,
not a fresh empty build directory.

One focused controlled-child fixture used the actual retained-memory mesh reader,
canonical logical stream and sections, declaration aliases, hardware-layout
factory/tree/COM owner and new generator/binding owners. D3D9 HAL readback verified
a first-stream FLOAT3 position, then three or nine second-stream FLOAT4 texture
coordinates at exact offsets and usage indices, followed by END. Instance data
widths are 48/144 bytes; combined strides are 60/156 including the first stream.
Two generic generators shared one cached layout. Replacing one with a building
generator preserved the other's shared layout until its final release.

Empty/unrecognized generator strings and a null material-effect pointer preserved
the existing binding. A canonical stream lookup was made to throw after a real
declaration cache hit: raw generator and temporary name were cleaned, the acquired
declaration reference remained, and the old section binding stayed unchanged.
The fixture explicitly released that retained reference afterward. This is a
source metadata failure observation, not an original `B48CE0` exception oracle.

All fifteen canonical companions retired, the hardware tree and allocator list
were empty, cache stride was zero, and all raw slab slots were returned. Final
D3D9 device/API COM counts were zero. The final fixture passed twice after the
initial device setup failures described below.

Initial `CreateDevice` attempts returned `8876086C` before reaching new production
code. The frozen CJ fixture still passed. Adding read-only display-mode and
client-window diagnostic queries preceded two successful CK runs. The cause has
not been established; the failure and successful logs are retained. Fixture-only
shadowed-variable and unused-parameter warnings were fixed; production build had
already passed.

The material/effect/descriptor prefixes, cache preimage, raw pool initialization,
tree sentinel and platform observation callback are explicit fixture boundaries.
There is no copied-original parent execution, cold vertex-format decoding,
device-loss/retry, full renderer/pool startup, XLive, original ABI/FH3/SEH or gameplay
claim. The real cold hardware-layout factory path and its cache hit are covered.

## Follow-up packets

- Complete numeric-profile `535320` material/effect admission, including the cold
  `B18D60` constructor's texture acquisition. Current callable renderer slots
  cannot be replaced with fabricated tables.
- Audit `B941D0`'s full assembly and exception tail, then compose its material
  path with the completed `B85610/B451D0` generator attachment.
- Continue `B944E0`, `B94710` and registered aggregate parser wrappers when the
  subset contract is ready; proceed toward full game startup and validation.

This packet is published on `agent/orch4-20260910` only. Main was not integrated;
later main deltas remain unreviewed by this worktree.
