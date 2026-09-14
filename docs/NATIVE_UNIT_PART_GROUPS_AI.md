# Unit-part damage-group production

Addresses: 00713380, 00713270, 00711C30, 004CDBE0, 004B3FC0,
00506DD0, 00505D50, 005058E0, 004F9AD0, 004FC4D0, 004FC4F0,
004FE2C0, 00501770, 00504F10, 00711E60, 00712FF0, 00712A10,
00711F70, 00712970, 00712B80, 00712900, 00712040, 0070F3D0,
00712F20, 00712F60, 00712F90, 00712940.

The complete `00713380..007135BD` caller now builds damage-group rows in the
existing unit-part allocation. `NativeUnitPartConstructionBindings::call_00713380`
has a concrete default that invokes it. This removes a required abstract
provider from the existing constructor; the other constructor providers and
full game resource loading remain separate dependencies.

## Caller and native contracts

The caller takes ECX=part, no stack arguments, and returns with bare RET. It
borrows the selected set at part+160 and the checked vector at set+7C, whose
begin/end at+80/+84 delimit eight-byte records. Record+0 points to a named
record embedding a 1Ch legacy SBO string at+8. Record+4 is the same raw node
identity appended to the existing group rows and consumed by collision
construction and group selection. No replacement owner or shadow vector is
created. Names are descriptive hypotheses, not recovered symbols.

`00711C30` copies the record name into a fresh existing SBO destination (ECX
record; stack destination; RET4). `004CDBE0` constructs a substring (ECX source
SBO; stack destination,offset,count; RET0C). Both preserve destination+0 and
reuse the canonical `00408120` implementation after the original initialization
stores. Their C++ interfaces have a new ABI.

The complete name is copied before taking a six-byte prefix. Comparison is
case-sensitive against the verified literal `damage` at CFD7C8, and requires
the prefix length to equal six. The suffix starts at six and is passed to
the actual source CRT `atol`: whitespace, signs and numeric prefixes retain
that contract. The parsed DWORD minus one is the unsigned row index. The
caller grows to index+1 when needed, then appends record+4. It neither clamps
numbers nor removes duplicate nodes. Zero and negative numbers retain their
validation/unsigned-error paths. Original static-CRT locale and overflow state
are not proved equivalent to the source CRT.

Source/set identities and endpoints are reloaded at the native continuation
sites. The full copied name remains owned across allocation and append failures;
its destruction precedes the final iterator validation. The prefix and suffix
temporaries retain their explicit free/reinitialization schedule. Source C++
cleanup is exercised through the owning vector-length error for a long negative
suffix. Native FH3, hardware faults and private native stack aliases remain
outside this source interface.

## Reused pointer-vector implementation

The raw DWORD-vector operations have identical full instruction streams after
call relocation. Their leaf dependency mappings close recursively; no alternative
pointer-vector implementation is introduced.

| Native address | Existing canonical source address | Operation |
| --- | --- | --- |
| 00506DD0 | 00BD0BC0 | Append actual value slot |
| 00505D50 | 00BD08D0 | Checked single insertion and iterator output |
| 005058E0 | 00BD0700 | Counted insertion and growth |
| 004F9AD0 | 00BCFEB0 | Checked DWORD allocation |
| 004FC4D0 | 00BD0160 | Assign value slots |
| 004FC4F0 | 00BD0180 | Copy backward through memmove_s |
| 004FE2C0 | 00BD0500 | Copy range through memmove_s |
| 00501770 | 00BD0560 | Construct a repeated value range |
| 00504F10 | 00BD0590 | Owning vector length error |

The row-vector length error `00711E60` also matches `00BD0590`. The only
non-call byte differences in the two throw entries are their handler addresses.
All three FuncInfo records, unwind-state maps and temporary-string cleanup
instructions agree after address normalization. Correct existing library names
are retained. The source CRT owns allocation/free, memmove_s, invalid-handler
state and exception transport; this is not original static-CRT/FH3 identity.

## Outer row storage

`resize_native_part_group_rows_empty_00713270` implements the **empty by-value
fill invocation made by the recovered caller**. The original native entry takes
ECX=header, stack(requested count,16-byte row value), RET14h. The source API
exposes that exact empty-fill/end-resize use, not a general replacement for the
nonempty-fill and arbitrary-position `00712FF0`/`00712B80` templates.

The actual header and each row use opaque+0, begin+4, end+8, capacity-end+0C;
outer stride is 10h and inner stride is four. Growth uses the native unsigned
0FFFFFFF bound and 1.5-times capacity rule. In the replacement path, `00712A10`
initializes each destination row to empty and swaps its three pointer cells
with the old row. Existing inner allocations therefore keep their addresses;
their elements are not deep-copied. New rows initialize only those three cells,
leaving the opaque word untouched. After old-row destruction/free, publication
orders capacity, end, then begin. Shrink frees removed rows and retains outer
capacity. In-place end growth initializes only new rows.

This specialization covers all row operations issued by `00713380`, including
its zero-count shrink path. It requires well-formed storage owned by the same
CRT and allocation/free callbacks that do not structurally mutate the vectors.
General insertion positions, nonempty fill values, native exceptional transfer
cleanup and arbitrary malformed-vector continuations are not claimed.

## Evidence and validation

Nine false fall-through gaps after `free` were repaired in five routines under
the Ghidra write lock and saved. Original bytes and refreshed listings are
retained. The audit distinguishes physical byte extents from embedded catch
function ownership: eight call rows were attributed to their actual catch
entries. All **133 owned call rows** pass the live checker. One additional
rethrow call at `0071317B` is decoded and byte-verified but remains outside a
Ghidra function body: catch `0071315E` ends at `00713173`. The formal tail-repair
tool refuses this noreturn-call tail because it requires a terminal RET. No
function was recreated, no synthetic entry was added, and no callee no-return
flag was changed. This unexecuted exception tail is recorded separately.

One ignored fixture compares six paired scenarios, executing nine group callers,
six complete AH collision callers and two explicit empty-fill resizes per side.
It covers ignored/case-mismatched names, base groups, gaps in group numbers,
duplicate names, signs/whitespace/trailing text, heap-backed names/suffixes,
replacement growth, in-place growth and shrink/regrow. Group memberships,
capacities, node identities, source names, collision list links/values,
published pointers and local bounds agree. Three preservation checks per side
confirm that outer growth retains an existing inner allocation. The rebuilt
caller also raises the owning length error for a long negative group number.

The oracle retains 35 physical original body spans with 141 declared operand
relocations, including the seven AH references. This is not a claim that every
instruction in every retained span executed. Unimplemented template-assignment
and native-exception guards were never reached. Both sides use the canonical
SBO substring implementation and actual source CRT boundaries. Eight existing
native seeds match. The Win32 build and both existing tests pass; no repository
test cases or workers were added.

`reports/native_unit_part_groups_ai.json` and `local/unit_part_groups_ai/` retain
the exact byte, call, ownership, alias and runtime evidence. Upstream model/set
loading, other unit-part constructor dependencies, game admission and gameplay
parity remain unproved.
