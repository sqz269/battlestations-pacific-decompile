# Native unit health and parts initialization

Addresses: 0087BCC0

`initialize_native_unit_health_parts_0087bcc0` reconstructs the complete caller
`0087BCC0..0087BF73` (692 bytes). Original ABI: ECX is the canonical unit,
EBX/EBP/ESI/EDI are preserved, bare RET. The new C++ interface borrows the
existing unit and class fields and requires complete native operation providers.
It does not establish a runnable unit owner, native ABI replacement or gameplay
equivalence. Names are descriptive hypotheses, not recovered symbols.

| Routine | Coverage | Status |
| --- | --- | --- |
| `0087BCC0` new native caller | complete normal body, both model branches and numbering tail | reconstructed, Win32 build and original-byte caller fixtures passed; providers required |
| `unit_initial_condition_0087bcc0` | partial value projection of `0087BCF4..0087BD43` | activation, native field writes, vector fill, parent traversal, model/EH and numbering omitted; reversed-range clamp also differs from native |
| `unit_part_detail_0087bcc0` | partial detail-value choice within `0087BE0D..0087BE8E` | all dispatch, allocation and x87 effects omitted |

The previous `reconstructed_ledger_backfilled_from_source` classification of the
initial-condition helper overstated its coverage. Both old helpers remain
available and are explicitly labelled partial; this caller does not delegate to
their projections. It reuses `unit_parts.hpp`/`unit_hit_path.hpp` constants and
the actual `NativeUnitObserverAlias` and `NativeUnitScenePropertyHolderView`.

## Actual borrowed storage and ordering

`NativeUnitHealthPartsView` binds the same canonical unit alias, model `+360`
and property-holder `+C0` cells that the scene initializer uses. It creates no
unit/model/scene owner, shadow fields, padded native production object or
semantic property bag. Class fields `+48/+4C` are HP/Armour as established by
the class reader. Class `+1C/+20` delimit 48-byte descriptors. The checked vector
at unit `+344` has begin/end at `+4/+8`; it is **not** the render pointer array's
data/count/capacity layout, so the source borrows its actual cells explicitly.
The class `+50` producer and complete provider binding remain unresolved.

After `0077F0E0`, the caller captures the current class, loads armour and global
one, writes armour, loads class HP, writes maximum HP, marker, current HP and
condition byte in that order. Both HP stores use the same XMM0; `+370` is not
set to one. The source object's `/O2 /fp:strict` disassembly preserves this
MOVSS order (`+51..+82`). The vector count uses signed DWORD subtraction/divide
by 48; comparisons use unsigned counts. No reversed-range clamp is introduced.

Resize may rebind the class/vector. Every loop iteration reloads the current
class, but a returning `00BF6713` keeps that captured class-vector identity;
its begin and the destination begin are then reread at the native points.
Index/byte-offset arithmetic wraps at 32 bits. Valid reached storage is required.
The parent chain uses `+3C`; current virtual `+B0` is selected for each call.
The first nonzero result causes a second call on the same parent with a fresh
table lookup, and that second result is discarded.

The optional class `+50` object is captured after the parent loop. Missing it
sets unit `+360` to null and returns without processing properties. Otherwise
kind `1B` selects the detail branch; each branch allocates `1AC` bytes before
the x87 load and model dispatch. The code retains the native null-allocation
branch even though the inspected `00BF681B` allocator normally returns storage
or throws. Unit `+360` is published only after constructor return.

Holder `+04` is a **kind**, not a refcount. Kind 2 reads a saved record's `+58`,
calls `00876EC0`, then reads `+5C` from that same saved object after the call.
The other branch calls `00876EC0(unit,0)`, defaults numbering to 1, then reloads
current holder `+C0` and its `+08` before `Numbering` lookup. This branch does
not add a kind-1 restriction absent from the caller. The final model presence
test precedes the `+35C` store. Kind 6 then kind 1B short-circuit in order; the
numbering call reloads `+360` after those virtual calls.

## The recovered virtual stack contract

The previous notes incorrectly assigned the detail float to unit virtual
`+190`, then only one argument to part-set virtual `+8`. The checked live
tables `00CFC3D0` and `00D0BF80` both select `0080DF80`, whose complete six-byte
body is `MOV EAX,6; RET`. The pre-pushed float survives that bare RET:

| Point | Top of stack before CALL (excluding that CALL's return address) |
| --- | --- |
| `0087BE4B` / `0087BE8E`, unit `+190` | detail float reserved for the later call; **no unit stack argument** |
| return from unit `+190` | same detail float; EAX contains selector |
| `0087BE52` / `0087BE95`, part-set `+8` | selector DWORD, detail float |
| return from part-set `+8` | both arguments removed (required RET8 boundary) |
| `0087BE58` / `0087BE9B`, `007135C0` | canonical unit, selected part set; ECX is allocation; RET8 |

The actual part-set table target could not be established from its unbound
class `+50` producer; `call_part_set_08` remains an explicitly required dynamic
provider with both arguments. The checked selector value 6 is evidence for two
profiles, not a constant substituted for current unit dispatch. Their parent
`+B0` slot selects `006D1DF0..006D1DF6`, `MOV EAX,[ECX+360]; RET`.
Root defined both previously absent leaves during review; the worker did not
mutate Ghidra. Supporting bytes and initial absence receipts are retained.

The source captures the unit table before FLD global/FLD1, then the part-set
table and unit `+190` entry, then FSTP. The pure table binding must preserve
the x87 stack/environment. Compiled source offsets `+219..+245` confirm that
order with explicit FLD/FLD1/FSTP. It calls `+190`, then reloads entry `+8` from
the saved part-set table (`+26C`), even if the object's current table changed.
Saved table entries are provider tokens, never cast to native process code in
production. All reached fields and tables must stay valid.

## Complete required operations

Every direct callee body was read before its contract. These are provider
requirements, not claims that this packet implements the callee or its transitives.
The report records all 12 direct and 9 indirect caller sites individually.

| Native sites | Callee | Required boundary |
| --- | --- | --- |
| `0087BCDC` | `0077F0E0` | whole activation/list registration and kind-dependent property setup; ECX unit, RET; suspect inherited STL pseudocode annotation is not used to invent behavior |
| `0087BD4F` | `0087B460` | whole checked pointer-vector resize; ECX actual `+344`, count and zero fill, RET8; no replacement STL implementation |
| `0087BD95`, `0087BDB4` | `00BF6713` | CRT invalid-parameter path, may return; retain subsequent reloads |
| `0087BE1C`, `0087BE5F` | `00BF681B` | existing malloc/new-handler/bad-allocation boundary; cdecl size, ADD ESP,4 |
| `0087BE58`, `0087BE9B` | `007135C0` | complete part-instance construction and internal EH; ECX allocation, unit and selected part set, RET8; existing `unit_parts` field sequence is not a substitute |
| `0087BECB`, `0087BED9` | `00876EC0` | whole state update, conditional world announcement and model visible-group update; ECX unit, state, RET4 |
| `0087BEFB` | `008F2260` | whole native property-bag lookup including native string/map lifetime; ECX current bag, key, RET4 |
| `0087BF43` | `00711BE0` | whole numbering wrapper: model `+160` then `+0C`, EDX numbering to recursive `00711A20`, RET4; no private material implementation |
| `00C967D4`, `00C967DF` | `00BF65AC` | existing free boundary on saved allocation during unwind; cdecl, POP ECX |

## Allocation cleanup and evidence limits

Handler `00C967E6..00C967EF` selects FuncInfo `00DC8BD4` and jumps to
`00BF6B43`. Max state 2 and unwind map `00DC8BC4` give `(-1,00C967D0)` and
`(-1,00C967DB)`. Each funclet loads `[EBP-10]`, calls `00BF65AC`, pops one
argument and returns: complete spans `00C967D0..00C967DA` and
`00C967DB..00C967E5`. Ghidra currently truncates their last POP/RET; these exact
tails are labelled `no_ghidra_function` in the report. Original bytes, metadata
and disk listings include the inclusive endpoints. Root owns any repairs.

The C++ caller frees the captured allocation when a provider throws during
selection/construction, then rethrows. The focused source constructor-failure
case verifies exactly one free and preserves the previous published model.
Original native FH3/SEH dispatch, faults and constructor-internal cleanup were
**not executed**; source cleanup success does not prove their equivalence.

The Release Win32 build and existing `reconstructed_math` and
`native_math_differential` CTests pass. `verify-seeds` matched all eight seed
spans. Five original 692-byte caller versus separately arranged source-storage
cases pass: no part set; kind-1B saved kind-2 data; ordinary detail/property bag;
null allocation and unchanged numbering; and both returning range traps with
class/vector rebinding. They also cover parent repeated dispatch, saved-table
entry mutation, live numbering and final model reload, with untouched guards.
All direct/indirect callees in the native caller fixture are explicit bridges;
their complete original bodies are retained for evidence but not executed.

The probe has an embedded Win32 manifest. Original bytes, call relocations,
native boundaries, compiler/source disassembly, tool versions, source, headers,
SDK/CRT libraries, objects, executable, inputs/results and SHA-256 manifests are
retained under `local/health_parts_*` for root archival before worktree cleanup.
The production owning runtime still must supply actual class/vector/model and
provider bindings. No frame log, runtime scene admission, drop-in ABI, native
exception equivalence or gameplay validation is claimed.
