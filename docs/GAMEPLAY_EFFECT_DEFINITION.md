# Gameplay effect definition ownership

Addresses: 008700e0, 0086b870, 0086e770, 0086eb60, 0086edd0, 00870d00,
00871440, 0086b650, 0086e8a0, 00869a20.

Packet `orch4_effect_definition_l` reconstructs the definition allocation fragment,
identity assignment, component-reference array and full normal destruction. These
are concrete dependencies of acquisition/loading; acquisition `008700E0` and its
13 component-loader families are not claimed complete. Names are hypotheses.

Current follow-up: `orch3_gameplay_definition_unwind_ac` adds the recovered
three-state definition cleanup and complete `0086FC30` array destructor. See
`GAMEPLAY_DEFINITION_UNWIND.md` for the map, partial-state behavior and validation.
The original native exception-dispatch ABI remains unvalidated.

## Actual storage

The allocation at `00870240..00870279` requests 24h bytes. The inline constructor
writes base vtable `00CEB130`, reference count1, then derived vtable `00D0DA58`.
It zeros component pointer +8, **signed element count +C**, signed capacity +10,
and NativeString words +1C/+20. Words +14 and +18 retain their allocation bytes;
+18 becomes the ID later. This is a pointer/count/capacity array, not an STL
begin/end/capacity-pointer triple.

`GameplayEffectDefinition` keeps exactly those 24h bytes. The allocation helper
preserves their representation before beginning its C++ shell lifetime; no shadow
vector or implicit string/reference destructor exists. `0086B870` writes ID +18
before resizing/copying the name. Its self-name guard skips the string copy but
still writes the ID. Copying uses the current source length after resize and the
current destination length for memcpy, through existing actual-header operations.

## Component reference array

The three complete routines operate directly on a 12h native header, including
the one embedded at definition+8. Real Interlocked operations act on each actual
component's reference count at +4. Only a transition to zero invokes the supplied
runtime binding for that component's current virtual slot0.

`0086E770` reserves at least one slot. Signed requested capacity is compared to
the current capacity. On growth it allocates `requested * 4` bytes with native
DWORD wrap, copies slots in ascending order while retaining nonnull references,
then releases old slots in ascending order. Both loops reload the current count.
Old-slot release captures its slot address and object, decrements the reference,
calls the zero-reference virtual when needed, and clears that captured slot after
the callback. The routine reloads the current data pointer for free, then publishes
the newly allocated pointer and requested capacity. It does not overwrite count.
Unused capacity remains uninitialized.

`0086EB60` grows only when count **equals** capacity, choosing wrapped doubled
capacity or1 when the signed doubled value is <=1. It computes the destination,
zeros it, then reads the caller's pointer slot. Thus a source aliasing that exact
destination appends null. A nonnull source is published and retained; current
count is incremented afterward. Do not pre-capture the source before growth or
before destination initialization.

`0086EDD0` reserves when requested size exceeds capacity, then initializes any
new slots to null. Shrinking decrements the actual count before reading the slot,
captures the current buffer/slot, releases it and clears the captured slot after
the callback, then tests the current count again. At return it unconditionally
stores the requested size. It does not capture the whole buffer or loop bound
once: a callback may transfer remaining references into another valid buffer.

## Definition destruction

`00870D00` performs this normal path:

1. Restore derived vtable `00D0DA58` and call the concrete manager getter004C1650.
2. Read the current definition ID, find it with0086B650, then erase that iterator
   with0086E8A0. There is no comparison against the map's value pointer and no
   payload release. An absent ID reaches stock `out_of_range` with
   `"invalid map/set<T> iterator"`; it is not quietly ignored.
3. Destroy the current pooled name, leaving its header unchanged.
4. Resize the current component array to zero, then reload and free its data.
5. Apply the existing00BD30F0 base-vtable store (`00CEB130`).

After normal destruction the data/name pointers remain dangling representation
values, capacity is unchanged, count is zero, and the intrusive reference word,
ID and word14 are not reset. `00871440` calls this destructor, frees the owner only
when flags bit0 is set, and returns the original address even after freeing it.
The manager map remains non-owning; definitions remove their entry themselves.

On manager/map failure, the recovered cleanup destroys name, component array,
then base. Normal name destruction first disarms name cleanup; normal array
destruction first disarms array cleanup. A component callback that throws during
the normal array stage therefore runs only base cleanup: partial count/slot state
and the backing allocation remain. There is no retry, missing-ID suppression or
cache rollback. The scalar never frees raw owner storage after a throwing destructor.

The exact24h owner, component buffers, existing shared lifetime domain and
NativeStringStorage are used directly. Actual component virtual dispatch is a
required runtime binding until the component classes are reconstructed.

## Original ABI

| Address | Inputs / return | Classification |
| --- | --- | --- |
| 00870240..00870279 in008700E0 | ESI=0 before fragment; allocate/initialize ESI definition | reconstructed allocation/constructor fragment only |
| 0086B870 | ECX definition; ID and name-header pointer stack; RET8 | reconstructed identity assignment |
| 0086E770 | ECX array header; capacity stack; RET4 | reconstructed reference-array reserve |
| 0086EB60 | ECX array header; source pointer-slot address stack; RET4 | reconstructed reference-array append |
| 0086EDD0 | ECX array header; size stack; RET4 | reconstructed reference-array resize |
| 00870D00 | ECX definition; RET at00870DC1 | normal destructor and three-state C++ unwind |
| 0086FC30 | ECX actual12h array; RET at0086FC46 | resize0, reload/free buffer; no internal EH |
| 00871440 | ECX definition; flags stack; EAX original address; RET4 | reconstructed scalar deletion |
| 0086B650 | ECX map; output iterator and signed-ID pointer stack; EAX output; RET8 | analyzed stock find contract |
| 0086E8A0 | ECX map; output iterator plus owner/node iterator stack; EAX output; RETCh | analyzed stock checked erase contract |
| 00869A20 | ECX mutable owner/node iterator; RET | existing stock helper; explicit register prototype restored |

STL tree algorithms are reused through the canonical manager map. The checked
missing-ID condition uses the matching host standard exception; native exception
objects, invalid-owner validation, allocator failure/unwind, original function ABI
and concurrent invalid storage are not established by this C++ interface. The raw
array routines retain native signed comparisons and wrap without adding recovery
for invalid sizes or freed source slots. Zero-byte memcpy follows the existing
string service convention. No placeholder component payload is constructed.

## Ghidra corrections and validation

Assembly established the normal path beyond false `_free` returns. Two internal
gaps are repaired: 19 bytes in0086E770 and3 bytes in00871440. The 35-byte00870D00
tail and54-byte0086E8A0 tail were decoded and their bad free-call overrides cleared,
but stored bodies still end at00870D9E and0086EB10 respectively. Complete semantics
come from the verified native bytes; these metadata extents remain open.

0086E8A0's former whole-function `STL_xlen_throw` label is wrong. It is a returning
tree erasure routine with an invalid-iterator exception branch. Prior inventory
tag/bookmark state is preserved when retiring that label. Its pseudocode also
discarded the two-child transplant branch because00869A20's stored `void(void)`
prototype hid mutation of the caller's stack iterator. An explicit pointer in ECX
was applied using a one-argument `__fastcall` model; the source-language convention
is not recovered by that model. Live signature readback and a refreshed caller
export verify that the false unreachable-block warnings disappeared. The initial
semicolon form was rejected without changing the signature; the report preserves
both the old signature and the accepted form.

Win32 Release and both existing CTest targets passed. One focused ignored fixture
checks the actual24h fields, untouched words, ID-before-allocation and self-name
assignment, reference-preserving growth, destination/source alias append, ID-only
weak-map erasure, name-before-components cleanup and descending component release.
Its last-slot callback transfers the remaining references to another live buffer;
later callbacks verify the old captured slot was cleared and the current buffer
and decremented count were used. Scalar flags2 leaves storage available for field
inspection. The actual component dispatch remains fixture-supplied.

Reports distinguish source fixture validation from subsequent combined builds.
Full analyzed spans have live-Ghidra/disk byte hashes after project verification.
The existing native differential target still covers only its math seeds; no
native-definition ABI or gameplay validation is claimed.

## Follow-up packets

- Acquisition00871BA0/00871B50/008700E0 and cache insertion0086F930, using the
  concrete manager, name index and actual24h definition. Preserve fresh refs1,
  cache-hit retain and final cache-cell reload after temporary cleanup.
- Component loading00870400 and the 13 constructors/readers. Sound's D0DA18
  virtual+14 reader is0086EF60, which first invokes00868BF0 and resolves samples
  through00A83FD0. Tracer returns a secondary pointer at+80; follow its native ABI.
- Supported stored-body extent repairs through00870DC2 and0086EB47, plus the
  earlier manager/name-index tail extents documented in their packets.

## Correction from docs/GAMEPLAY_EFFECT_ACQUISITION.md

The previously open008700E0 acquisition sequence and00871B50/00871BA0 wrappers are
now reconstructed around this allocation/identity code. Unique weak insertion,
cache-hit retain and final cache-value reload after temporary cleanup are concrete.
The00870400 component virtual remains a required service. Stored cleanup-body
extents remain incomplete: the Java bridge execution gate rejected the read-only
capability probe; see docs/GHIDRA_BODY_EXTENTS.md for the saved response.
