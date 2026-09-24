# Raw material-name header: current hot-cache composition

Address: `00535320`. This packet changes no production code. It verifies the
existing raw-header overload with one current-source fixture derived from the
genuine CL base-effect/cache fixture. The strict Win32 build and all three
existing CTests passed. The standalone D3D9 fixture exited 0 and drained its
owners, pools, window and COM objects.

The entry remains the shared typed/raw numeric cache implementation in
`native_material_factory.cpp`. This is source composition evidence, not copied
original execution, binary ABI compatibility or full post-effect admission.

## Exact native scope

| Entry | Native extent | Evidence |
|---|---|---|
| `535320` material factory | `[535320,53539C)`, 124 bytes | Fresh live Ghidra bytes equal installed PE; 43 instructions, final one-byte RET at `53539B` |

Original input is ECX pointing at the actual eight-byte length/data header;
EAX returns the material. The existing source overload accepts `const void*`
and keeps the typed wrapper. This packet adds no new entry, cleanup or dispatch.
The earlier argument-view packet retains the native compiler-boundary evidence.
Ghidra was read-only; the wrapper verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` before reading bytes.

| Native call site | Target | Reached in this composition |
|---|---|---|
| `535345` | Current renderer virtual `+48`, admitted `B318B0` | Genuine numeric effect-cache provider |
| `53534E` | `B18780` | Actual material pool slot allocation |
| `535364` | `B18900` | Actual material construction and effect retention |
| `535377` | Imported decrement through `CE2220` | Existing same-actual canonical ownership release; temporary effect credit becomes 2 |
| `535387` | Current effect virtual `+0`, only at zero | Not reached at this factory site; later real creator retirement is exercised separately |

The two direct call rows are live-verifier checked. The three indirect rows
remain exact instruction inventory; the report does not equate them with
original indirect instruction execution or mutable import/profile proof.

## Fixture and ownership

`local/cc10_material_raw_header_hot_probe/probe.cpp` preserves the CL scaffold's
genuine numeric providers and explicit rejection of cold operations. Its
`scaffold.diff` removes the derived poison-only constructor and the separate
failed metadata-bind case. The single factory input is two initialized live
DWORD cells `{9, pointer to "TeSt.ShFx"}`, adjacent to `13579BDF` and `2468ACE0`
canaries. No `NativeString` aggregate lifetime is started over these cells.
All four words and the input text remain unchanged through construction and
material retirement.

The isolated renderer storage is an aligned `1B90h` field fixture with the
borrowed original `D5F0A8` profile, actual initialized `19F4h` critical section,
real D3D9 device at `+1A10`, and actual texture/effect cache headers. It is not a
full `B32410` renderer constructor or application bootstrap. The child reserves
original read-only data regions before normal startup and loads their installed
PE bytes using the existing data bootstrap; it does not execute original code.

The windowed 64x64 HAL device used software vertex processing and
`D3DCREATE_FPU_PRESERVE` on NVIDIA RTX 5090 (`10DE:2B85`). A real managed 4x4
`A8R8G8B8` D3D texture and actual texture pool owner seed the `error.tga` cache.
The complete raw `B18D60` C4 base-effect constructor then obtains that texture,
stamps `D5E534`, preserves its unwritten fields, produces its real creator count
of one, wraps the current serial from `FFFFFFFF` to zero, and initializes
`dirty+B4` to zero. This is a constructed base effect, not a derived effect with
unproven pass fields.

The effect registry has its actual `D5F074` identity at renderer `+1A98`. Its
real record and alias-list providers append `test.shfx` through `B301A0`; the
record borrows the constructed effect and contributes no owner credit. The
mixed-case raw input resolves that same effect. The acquired cache is complete
and its loader remains `not_started`; cold resolver/program/cache methods throw
if reached. Null cold-route bindings are never treated as successful providers.

The actual `B18780` material pool and `B18900` constructor yield `D5E520`,
material count one, and the same effect at material `+7C`. After the factory's
temporary release, effect count is two, texture count is two, and `dirty+B4`
is one. Canonical material metadata admission uses the same actual-owner
registry and introduces no extra native credit.

Canonical material retirement returns its slot and leaves effect count one.
The fixture then destroys/removes the borrowed effect row and alias storage
before releasing the effect creator. Real base-effect retirement releases its
texture credit. Real texture creator retirement removes its cache row. The
fixture verifies the material slab's 64 free slots, three canonical bindings
and three retirements, an empty owner registry, zero texture tracking and cache
accounting, and a drained allocator list. All initialized material, parameter,
texture, surface, section, mesh and raw string pools are destroyed. Final
device/API `Release` returns are `0/0`, and the window closes.

## Reproduction and evidence limits

Baseline is `2e2223fad` (full hash in the report). Run `./scripts/build.ps1`, then
the exact retained `compile.cmd` from this worktree. It links the three current
project libraries with `/MD /fp:strict /W4 /WX /MANIFEST:EMBED`; `CHECK` is always
active and an `NDEBUG` build is rejected. `run.json` pins the exact executable
command, ten binary/source/build inputs and their unchanged post-run hashes.
`inputs.json` additionally pins 22 relevant provider source files, the installed
PE, the five historical CL artifacts and the frozen readiness. The link map
identifies the raw `const void*` overload from `native_material_factory.obj`.

The standalone parent launched one controlled bootstrap child, PID 99332. Both
returned zero, and the process inventory found zero remaining matching
processes. `run.log` records the assertions and final pool/COM drain. This was
not an application or original-game launch. No new tracked test was added.

The fixture does not exercise cold program/effect loading, source0, declarations
or geometry upload, `B4E470`, model/camera/scene construction, the distortion
initializer, application startup or gameplay. It does not exercise failure
unwind, metadata bind rejection, native FH3, callback mutation of the header or
mutable import/profile epochs. The input is disjoint from owner/cache/metadata
storage; no private native stack alias compatibility is claimed. Historical CL
runtime coverage remains separately attributed in its original report.

All 23 prior evidence archives and the three hot-cache readiness files were
hash-verified unchanged. The new report records the frozen archive/manifest;
the ignored fixture is retained for review without adding a test suite.
