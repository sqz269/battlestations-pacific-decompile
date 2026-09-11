# Panel palette lookup and insertion

Packet `orch4_panel_palette_d` and supporting lease
`orch4_panel_palette_helpers_d`, branch `agent/orch4-panel-palette-20260910d`,
2026-09-11 UTC. Live queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; worker Ghidra access was read-only. These names
describe evidenced roles and remain hypotheses rather than recovered symbols.

## Missing keys have no initialized default color

`0044EC00` is signed-int map lookup-or-insert, returning the live node's value
at **node+10h**. Its stack reservation is24h, with no earlier writes to four
scratch words. At the miss branch, after the three saved registers, call the
current stack pointer S:

| Instructions | Observed copy |
| --- | --- |
| `0044EC3C/42` | uninitialized `[S+0C]` to pair word1 at `[S+20]` |
| `0044EC40/4A` | caller key to pair word0 at `[S+1C]` |
| `0044EC46/52` | uninitialized `[S+14]` to pair word3 at `[S+28]` |
| `0044EC4E/5B` | uninitialized `[S+10]` to pair word2 at `[S+24]`, accounting for PUSH |
| `0044EC5F/64` | uninitialized `[S+18]` to pair word4 at `[S+2C]`, accounting for PUSHes |

The five-word pair is passed to hinted insertion. `0044AB90` allocates24h
and copies all five words through **integer MOVs** (`0044ABB6..D4`). No
constructor, zeroing, x87 load/store, floating-point conversion, or clamp is
present. Thus a native miss has residual stack bits, with no deterministic
default color established by these routines. The C++ interface requires four
defined `uint32_t` words as explicit input and reads them only on a miss. It
never evaluates uninitialized C++ data or silently replaces the bits with zero.

Existing keys return their current value identity without allocation or any
use of the supplied miss words. Duplicate explicit insertion likewise keeps
the first node and its existing payload. All ordering comparisons are signed
32-bit comparisons, including negative keys and INT_MIN/INT_MAX.

## Actual storage and integration

The implementation borrows an already initialized actual Win32 tree header:

| Storage | Fields |
| --- | --- |
| Tree12h | opaque allocator/comparator `+0`; head `+4`; unsigned count `+8` |
| Node24h | left `+0`; parent `+4`; right `+8`; signed key `+C`; Float4 `+10`; color `+20`; sentinel flag `+21`; untouched padding `+22..23` |
| Sentinel links | minimum `+0`; root `+4`; maximum `+8` |
| Checked iterator8h | owner `+0`; node `+4` |
| Insertion result0Ch | iterator; inserted byte `+8`; untouched padding `+9..B` |

Red is0, black is1. The existing `NativeHardwareLayoutTreeIterator` and
`NativeHardwareLayoutTreeInsertResult` layouts are reused as aliases; the
hardware map's node operations are not reused because its flag/value offsets
differ. This is actual storage access on MSVC Win32 with new C++ function
interfaces, not a portable owning `std::map` or native calling-convention
replacement. There is no tree constructor, erase, whole-map destructor or
panel-owner reconstruction in this packet.

The integration API is:

```cpp
std::array<float,4>& panel_palette_value_0044ec00(
    void* actual_tree, const std::int32_t* key,
    const PanelPaletteValueWords& missing_stack_words,
    const SingletonLifetimeCallbacks& invalid_parameters);
```

Resolve the current panel owner's map at its native load site and pass actual
storage. Miss words must be explicit defined input, with no invented default.
Supplying them is an input-domain boundary, not a new game callback inside the
lookup. The returned reference aliases actual Float4 storage and remains valid
through insertion rotations; external erase/destruction ends that lifetime.
The existing `VoiceSequenceHost` integration is parent-owned.

Read-only loader context `0044FA30` loads `DialogColors`, obtains an integer
key, calls this lookup at `0044FE64`, then stores four Lua-number values divided
by double255 into that same returned object, at `0044FE9C`, `0044FED4`,
`0044FF0D`, `0044FF45`. Constant `00CE4B48` is verified bytes
`00 00 00 00 00 E0 6F 40`. These sequential stores replace the initial bits;
lookup itself does not initialize or decode a color. The loader, Lua calls,
exceptions between lane stores, and whole panel state machine are not ported.

## Direct helper closure and ABI

| Address | Reconstructed role | Original input / return | Last instruction / bytes |
| --- | --- | --- | --- |
| `0044EC00` | Lower bound, lookup or insert, return Float4 | ECX=tree; key* stack; EAX=value*; RET4 | `0044EC94` /3 |
| `0044E7B0` | Checked hinted insertion with unique fallback | ECX=tree; output*, hint-owner, hint-node, pair* stack; EAX=output*; RET10h | `0044E969` /3 |
| `0044E090` | Unique signed-key insertion / duplicate result | ECX=tree; output*, pair* stack; EAX=output*; RET8 | `0044E146` /3 |
| `0044D2C0` | Count guard, allocate, link, rebalance | ECX=tree; output*, left-byte, parent*, pair* stack; EAX=output*; RET10h | `0044D4A9` /3 |
| `0044AB90` | Allocate24h and copy pair bits | left*, parent*, right*, pair*, color stack; EAX=node*; RET14h | `0044ABDE` /3 |
| `00448BA0` | Right rotation | ECX=tree; node* stack; RET4 | `00448BEF` /3 |
| `00449F00` | Left rotation | ECX=tree; node* stack; RET4 | `00449F4B` /3 |
| `00449160` | Checked predecessor | ECX=iterator; RET | `004491E8` /1 |
| `004491F0` | Checked successor | ECX=iterator; RET | `00449252` /1 |
| `004487D0` | Checked iterator equality | ECX=left; right* stack; bool AL; RET4 | `004487F6` /3 |

Hinted insertion includes empty, before-minimum, after-maximum, before/after
interior hint, and unique-walk fallback. Unique insertion checks predecessor
when descending left to avoid inserting a duplicate. Both preserve current
node identity. The link body increments the current count only after allocation,
updates head extrema, fixes red-black violations, and sets the root black.
It publishes output **node before owner**. Unique insertion publishes **owner,
node, inserted byte**; hinted fallback publishes **owner then node**. Padding
is not initialized. Integer copies preserve all payload bits, including NaN
and signed-zero encodings, without FP exception/quieting operations.

Both rotations reread the transferred child after their initial link store.
The reconstruction reuses the left helper for the equivalent inlined left
rotation inside `0044D2C0`. Volatile actual-field access preserves those
explicit loads/stores. This adds no synchronization or concurrent-tree policy.

Checked iterator helpers use the existing required `invalid_parameter` callback
for `00BF6713`; it is allowed to return. Owner validation then reloads node
fields, and tail validation returns without further advancement. Equality
compares current nodes after a returning owner-validation handler. No implicit
success or no-op invalid-iterator implementation is supplied.

## Allocation, exception reuse, and metadata corrections

Allocation reuses `singleton_lifetime_allocate` with object/native/host sizes
24h. The original allocation helper's null guard is retained; its caller has
no added null recovery. The shared allocator's existing malloc/new-handler
domain remains its documented boundary.

`0044D2DC CMP count,0CCCCCCBh / JC` permits insertion below the threshold and
throws at or above it. The length path constructs the counted message
`map/set<T> too long` with count19, arms cleanup only after successful assignment,
constructs the existing logic-error storage and stamps vtable `00D69260`, with
native throw profile `00D83F98`. This exactly matches the existing owning
`NativeHardwareLayoutTreeLengthError` transport, reused here without a new
CRT/STL exception port. Its host RTTI/catch type and ABI remain different from
native C++ EH; no binary SEH/exception equivalence is claimed.

The old autogenerated name `STL_xlen_throw_0044D2C0` and `stl_throw_site` tag
describe only one branch of a normally returning link/rebalance routine.
The report records that prior name and proposes a descriptive correction,
with tag review pending the integrator. `STL_inst_004487D0` is a valid library
instantiation identity and is retained with added role evidence. Other
`stl_probable` iterator/lookup tags remain consistent with observed behavior.
No prior plate comments were returned for these ten addresses.

## Validation and remaining boundaries

`./scripts/build.ps1` passed MSVC Win32 Release and existing
`reconstructed_math`1/1. One ignored fixture
`local/panel_palette_insert_check.cpp` passed through actual tree storage and
all reconstructed entrypoints. It exercises13 signed keys, hints and fallback,
red-black/parent/extrema invariants, duplicate identity and payload retention,
loader-style overwrite, both iterator directions, untouched result padding,
and the count-boundary exception. Its raw payload includes signed zero, qNaN,
sNaN and arbitrary bits, compared bytewise rather than evaluated as floats.
No shared tests or test targets were added.

All ten complete live instruction listings have zero gaps; counts and
inclusive body ends are recorded in the JSON report. No missing functions or
flow repairs were found. Ghidra annotation/tag correction remains pending.
Validation is host-fixture/build evidence, not native differential, actual
game/Lua loader execution or displayed-color proof. Callers must supply valid
tree storage, defined reached key/value words and live owners across native
service callbacks. Native stack-history acquisition is external input; the
reconstruction does not claim to reproduce accidental stack contents.
