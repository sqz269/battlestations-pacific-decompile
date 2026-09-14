# Actual material-effect descriptor terminal

The raw material-effect lifetime can now delete its numeric-profile descriptor
through concrete shared providers. `NativeMaterialEffectDestructionAccess` appends
an optional `actual_descriptor` context; null retains the existing callable-table
API. The context borrows the same `NativeStringStorage`, actual fixed0108FEE4
state-list pool storage, and the immutable slot0 words for D61A44 and D621F4.
It owns no allocator list, companion, callback registry, or allocation.

| Entry | Complete inclusive native range | Contract |
| --- | --- | --- |
| B41B10 | B41B10..B41BCD, 190 bytes | ECX effect, RET; direct descriptor deletion then retained slots |
| B41F80 | B41F80..B41FCE, 79 bytes | ECX effect, RET; D61A00 stamp and armed base cleanup |
| B46930 | B46930..B4694D, 30 bytes | ECX descriptor, stack flags, EAX old address, RET4 |
| B458A0 | B458A0..B45DF2, 1,363 bytes | ECX descriptor, RET; children then 13 member cleanups |
| B56FC0 | B56FC0..B56FDD, 30 bytes | ECX sampler, stack flags, EAX old address, RET4 |
| B56EA0 | B56EA0..B56F31, 146 bytes | ECX sampler, RET; holders24/28 then strings18/04 |
| B56DE0 | B56DE0..B56E2C, 77 bytes | ECX holder address, RET; rows free, pool return, holder clear |
| B62280 | B62280..B622D4, 85 bytes | ECX pool, stack slot, RET4; actual locked slab return |
| B623C0 | B623C0..B623CB, 12 bytes | ECX slot, RET; supplies fixed0108FEE4 to B62280 |

B45EE0 allocates 110h at B45F29, calls B43700 at B45F3F, publishes effect+C4
at B45F58, then calls its reader at B45F5E. B43700 stamps D61A44, whose actual slot0
is B46930. The descriptor has no intrusive counter: +4 is PipeID. B41830 calls
B57B50 at B41908 and appends that captured sampler at B4193E. The allocator
fragment B57B80 stamps D621F4; its actual slot0 is B56FC0. These are direct
flags1 owners, never retained-reference decrements.

B41B10 captures a nonnull descriptor and reads its current profile/slot0 at
dispatch. The raw path validates D61A44/B46930 and calls the existing scalar
provider. It clears CURRENT effect+C4 only after successful terminal return,
then preserves the existing fourteen C8, fourteen100 and138 retained-owner
schedule. B41F80 keeps its D61A00 stamp and armed base cleanup before child
validation/cleanup; the raw context must use its same string service.

Descriptor member cleanup remains one shared body. Each C4 child slot address
is captured before its terminal; current array base/count are reloaded for the
next iteration. The raw path validates D621F4/B56FC0 and invokes the shared sampler
body. Only after return is the captured slot cleared. D0/DC children retain the
existing string-first allocation cleanup. The 13 reverse member actions and
failure continuation are shared with the legacy API; no bulk rollback or retry
is introduced. Array counts are cleared while pointer/capacity fields remain
stale. The current data pointer is explicitly captured before the final count0
store, matching the native returning-free schedule.

The sampler captures each nonnull list holder at24 then28. A negative capacity
uses existing B40CF0 reserve(0), minimum ten 8-byte rows, with old-free-before-new
publication. It lowers the current positive count, captures current data, stores
count0, frees rows, returns the same physical slot through B623C0/B62280, and only
then clears the holder. The 10h pool slot's slab ID at+C survives. B62280 shares
its substantive raw-storage implementation with the existing pool companion:
enter actual pool+C section, increment recursion24, read CURRENT slot+C slab ID,
append the signed aligned slot index, update free count/first-free slab, decrement
recursion and leave. No projected `AllocatorListDomain` is constructed by this path.

The complete owned bodies total 2,012 bytes. Live bsp.gpr/PE equality also covers
writer fragments, profile words, the reused reserve, original Win32 IAT imports,
and complete effect/descriptor/sampler FH3 maps/actions. With explicit integrator
authorization, official locked tools repaired the truncated B458A0 returning
tail, recreated its stored extent through B45DF3 exclusive, retained its previous
name/comment, saved, and forced its export. The final listing has 400 instructions
and zero gaps. The call audit also exposed B56DE0's discontiguous stored body:
its listing contained the B56E1F call, but that address belonged to no function.
Separately authorized official recreation restored its complete 77-byte body,
preserved its name/comment, saved and forced its export. No callee-wide free
annotation or unrelated Ghidra write occurs.

Validation includes eight seeds before configure, strict Win32 build and both
existing CTests, native direct/tail call audit, and one manifested /MD fixture.
The fixture compares original/source B62280 entire slab postimages using real
Win32 imports, then executes the nonnull descriptor/sampler chain with two actual
state-list slots, an actual string pool and raw AA0 manager. An explicit fixture
string-service observer delegates real releases while changing effect+C4 and the
descriptor array base; the captured old child slot is cleared, current replacement
array is freed, and effect+C4 is cleared after terminal return. It also verifies
negative-capacity row cleanup, stale physical header fields, pool return order,
string order, and balanced real sections. Product code adds no observer.

Remaining boundaries are deliberate. The existing loader's callable descriptor
and sampler bindings remain unchanged; those producers keep the legacy null
context until separately migrated or detached by their owner. The numeric raw
path requires D61A44/D621F4 and valid nonnegative descriptor extents/aligned pool
slots. The existing NativeStringStorage release interface is noexcept: its actual
pool bridge terminates if a lazy getter throws, rather than reproducing native EH.
Original descriptor/sampler FH3/SEH exceptional execution, arbitrary profile
dispatch, original caller/private-frame ABI, allocator-failure injection and game
or global-shutdown validation are not established. Source/library/tool evidence
is archived separately from measured loaded runtime DLL file evidence; file hashes
are not mapped-image hashes.

## Integrated validation at 56780ba7

Full original/source B62280 pool-return postimages matched. The source nonnull D61A44/D621F4 descriptor/sampler chain passed with actual state-list pool/string pool/AA0 manager, captured child-slot clearing and effect-C4 clearing after provider changes. Whole-chain original execution and legacy callable producer migration remain open.

The combined strict Win32 build, eight seed checks and both CTests passed.
Three final-library probes,136 numeric direct/tail rows,23 saved/read-back
annotations (20 source entries and3 handler contexts), and70 live/PE spans
are retained in `local/checkpoints/56780ba7/native-shader-parent-wave/validation.json`
(SHA256 `2c757704e58574665b12a4cfcc05ea693b880e1923a1e1daf67031202d633065`). Full renderer and gameplay validation remain open.
