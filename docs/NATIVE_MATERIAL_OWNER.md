# Native material storage and lifetime

`NativeMaterialStorage` is the actual `110h` material prefix.
`NativeMaterialReference` borrows its existing atomic at `+04` and dispatches
the real `D5E520` deleting-destructor path. Ordinary construction, cloning and
destruction operate on that same storage. No additional material state, owner
registry, shader, or reference count is created.

This is a bounded owner slice. The initialized material/parameter pools and
the canonical retained resource owners remain required services. The existing
`MaterialCloneState` does not yet alias this native owner, and the module does
not install GUI material callbacks or expose a render-ready material factory.

## Evidence and original ABI

Every analysis/export/byte batch used read-only `python tools/bsp.py ghidra`,
verifying `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Twenty-four
complete function/data spans match the installed PE. Old Ghidra names and
comments, exact lengths and hashes are retained in
`reports/native_material_owner.json`. Names below are hypotheses; library names
remain unchanged. Ghidra was not modified.

| Entry | Original contract | Span, end exclusive | Scope here |
|---|---|---|---|
|`00B18900`|ECX fresh material, stack effect, EAX same material, RET4|`00B189E5`,229 bytes|Actual ordinary constructor |
|`00B18B60`|ECX fresh destination, stack distinct source, EAX destination, RET4|`00B18CF1`,401 bytes|Actual clone constructor |
|`00B192F0`|ECX material, RET|`00B194A1`,433 bytes|Actual destructor, required parameter-pool return |
|`00B194B0`|ECX material, stack flags, EAX original address, RET4|`00B194D0`,32 bytes|Destructor then required material-pool return iff flags bit0 |
|`00B18780`|Discards incoming ECX, sets ECX=`00F8D3AC`, tail-jumps `00B18200`|`00B1878A`,10 bytes|Explicit forwarding to required actual pool |
|`00535320`|ECX native effect-name string, EAX material, RET|`0053539C`,124 bytes|Traced required factory; not implemented |
|`00B17D70`|ECX raw slot, pushes slot, uses pool `00F8D3AC`, RET|`00B17D7C`,12 bytes|Traced constructor-failure return thunk |

The previous name `CG_static_dtor_stub_00b18780` is incorrect: this is the
material allocation thunk. The `110h` size immediate in its caller is discarded.
Pool allocation `00B18200` spans316 bytes through `00B1833B`; `00B17440` is the
67-byte slab initializer and `00B17A80` the104-byte slot-return function. These
pool bodies are evidence for required services, not an implemented pool claim.

The clone listing's unlisted three bytes at `00B18C1D` are `8D 49 00`, skipped
alignment. In pool allocation, `_free` has a false saved no-return boundary:
`00B18296` contains `83 C4 04` (ADD ESP,4), followed by table publication at
`00B18299`. Seven skipped bytes at `00B182F9` are LEA ESP,[ESP]. No missing
behavior was replaced with an early return.

## Storage and transitions

The prefix contains vtable/count `+00/+04`, word `+08`, conditional source owner
`+0C`, nine actual texture pointers `+10..30`, signed16 count `+34`, lighting
record `+38..78`, effect `+7C`,32 actual parameter pointers `+80..FC`, signed
count `+100`, words `+104/+108`, and exact flag bytes `+10C/+10D`. The padding
at `+36..37/+10E..10F` is preserved. Pool-backed instances occupy `114h` slots;
the live slab index at `+110` is not part of the material object.

Ordinary construction publishes base `CEB130`, count1 and material `D5E520`,
then zeros word08, source, texture count and all nine texture pointers. It
reuses the established17-DWORD lighting initializer directly for the actual
record, clears effect/count100, writes `FFFFFFFF` to104/108 and clears both
flag bytes. Effect assignment publishes and retains the actual+04; the final
reload of effect7C writes its actual byte+B4 to1. A null effect is supported.
Uncounted parameter pointers are left as allocation preimage.

Clone construction copies word08, actual source pointer and exact byte10D;
the source pointer is retained only when the byte and pointer are nonzero.
The signed source texture count is reloaded while copying each counted slot,
including nulls, with high-water count growth. It sets byte10C to1 before the
17-DWORD lighting copy, retains the same effect, copies104/108 and leaves
count100 at0. It does not mark effect+B4 or copy parameter records. A negative
source texture count takes the native initial skip; malformed positive extents
over9 are outside the supported storage contract.

Destruction installs D5E520 and captures/releases effect7C before clearing it.
It then releases each nonnull texture, clearing that slot only after its
terminal callback, and reloads the signed texture end after every iteration.
Only then does it reload byte10D and source0C; a nonzero retain byte releases
the actual source+04 and clears0C after its terminal callback. Therefore a
source owner's destruction can observe the material's still-published pointer
and D5E520 phase. A borrowed source remains untouched when byte10D is zero.

The parameter loop reloads count100 after each iteration. Each nonnull record
first releases its actual8-byte string header through existing
`destroy_native_string_header_0041dd20` and the supplied real string pool, then
returns the same parameter slot through its required pool service. Table
pointers, counts and flags are not cleared. Normal completion writes CEB130.
The observed one-state unwind maps for both constructors and destructor point
to funclets `00CBC530/550/600`, each ending at base `00BD30F0`; the C++ cleanup
preserves that base-vtable phase. No general x86 SEH, invalid pointer, malformed
extent, allocation-failure or throwing terminal-owner equivalence is claimed.

Canonical resource release uses the existing `NativeRenderActualOwners`:
decrement actual raw+04 first, resolve only at zero, verify the companion
borrows that exact atomic, and invoke its current terminal profile. The material
companion is registered by the caller in this same owner domain. Its final
callback runs the actual destructor/deleter, returns the physical slot, then
invokes the supplied companion retirement callback. It performs no post-return
storage access and adds no retain. Direct native deletion with flags0 remains
available for a caller that keeps the storage allocation.

## Required integration

`NativeMaterialSlotPool` must operate on actual initialized `00F8D3AC` storage:
`allocate_00b18200()` and `return_slot_00b17a80(void*)`. Its slab has64 slots of
`114h`,4584h bytes total; free-index words start at4500h and free count at4580h.
The pool shares the real allocator list, critical section at+0C, recursion+24,
table+28, slab count+2C, capacity+30 and first-free slab+34.

`NativeMaterialParameterSlots::return_slot_00b193fa_fragment(void*)` must use
the different actual pool at `00F8D3E4`:128 slots of88h, free-index words4400h,
free count4500h, slot slab index84h. Its callback starts after string release,
so it must reload the current slot+84 under the real lock. It must not free
the name again. Neither interface supplies an allocation fallback or a no-op.

Factory `00535320` still needs the actual current renderer `00F8D394` virtual
48h effect acquisition and its canonical effect owner. The native caller
dereferences the returned effect without a null guard. It allocates through
B18780, constructs when the slot is nonnull, then drops the effect temporary;
the state0 constructor-failure funclet `00C6C240` jumps to B17D70 to return the
raw slot. This packet does not replace that acquisition with a shader token.

The existing `MaterialCloneState` owns scalar fields, `MaterialTextureSlots`
owns a vector of shared owners, `MaterialLighting` owns its record/flag, and
`MaterialParameterBindings` owns a private table. None is a borrowed view of
this native prefix. A subsequent shared-storage redesign must reuse these
consumers over the same raw fields and canonical texture/effect/parameter
owners. Copying a second `MaterialCloneState` beside this object would create
two authorities and is deliberately not offered. Native GUI section+20 can
eventually retain this reference, but color/parameter callbacks and rendering
still require that real projection and the separately unresolved widget lifetime.

## Validation

MSVC Win32 `/W4 /WX` and both existing CTests passed after8 native seed spans
were verified. One ignored `/MANIFEST:EMBED` fixture executes captured original
ordinary/clone/destructor/flags0 deleter bytes against the reconstructed bodies.
The native lighting initializer and base destructor are also captured original
code; native atomic imports are bound to real kernel32 exports. Four full114h
comparisons pass, including unwritten bytes and the untouched trailing DWORD.

The fixture uses scratch material storage, null effects/textures, an all-null
texture high-water count9, and an empty parameter table. Its source lighting
includes signed zero, NaN payload and a subnormal. The two comparison source
slots own exactly two references to one actual `NativeRenderContextReference`.
Clone/retirement takes its actual count through2,3,4,3,2,1,zero. The final
reconstructed material release resolves the real context only at zero; its
actual destructor/free runs while material0C is still published, and the
material clears0C afterward. This is a generic retained source owner, not a
fabricated widget or effect.

Material final-zero/pool return, nonnull textures/effects, parameter allocation,
the complete factory, renderer use and gameplay were not tested. The material
pool and parameter pool callbacks reject use in this scratch fixture; they
are never substituted with successful fake lifetime operations. No permanent
test was added. Ignored fixture/scripts/native spans/logs remain under `local/`;
their hashes and commands are in the report. These are new C++ interfaces,
not binary replacements or a completed native material rendering path.

## Integration update from docs/GUI_NATIVE_MATERIAL_INTEGRATION.md

Both required material/parameter pool services now have concrete actual-storage
implementations in native_material_pools. The integrated fixture covers material
final-zero dispatch with real NativeString destruction and both pool returns.
GUI AA6870 color publication resolves the same native material companion through
the model's owner domain and writes actual+38 directly. These additions do not
create a semantic MaterialCloneState projection: parameter registration, effect
acquisition and widget/page lifetime remain separate required integrations.
