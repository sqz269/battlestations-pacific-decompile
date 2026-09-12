# Native VFS mount priority-tree insertion

Addresses: `00BE1330`, `00BE0DD0`, `00BE05C0`, `00BDEE60`, `00BE063E`.

The actual-storage insertion closure is reconstructed in
`src/native_vfs_mount_insert.cpp`. It reuses established string, allocator,
owning exception and mount rotation services. Names remain descriptive
hypotheses; no generic library implementation or provider owner is added.
Detailed bytes and CALL evidence are in
[the report](../reports/native_vfs_mount_insert.json).

| Entry / inclusive end | Bytes | Coverage | Original ABI |
| --- | ---: | --- | --- |
| BE1330..BE139A | 107 | complete in stated source domain | ECX tree; stack output, record; RET8; EAX output |
| BE0DD0..BE0FBB | 492 | complete in stated source domain | ECX tree; stack output, insert-left byte, parent, record; RET10; EAX output |
| BE05C0..BE063D | 126 | complete normal allocation and construction | no consumed ECX; stack left, parent, right, record, color byte; RET14; EAX node |
| BDEE60..BDEEBB | 92 | complete in stated string-service domain | ECX destination record; stack source record; RET4; EAX captured destination |
| BE063E..BE0652 | 21 | complete catch action within source allocator | captured node at EBP-14; free, then rethrow; no normal return |

Four bodies total817 bytes; the catch action adds21. All838 bytes and ten
supporting EH/data spans match current verified Ghidra memory and the installed
PE. Every live batch verifies `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Worker access was read-only.

## Actual producers and record copies

The mount tree is manager+3C, with head/count at tree+4/+8. BE1DC0 at
BE1E34..BE1E51 obtains its actual24h sentinel through BDABF0, sets nil+21=1,
self-links left/root/right and zeroes count. BDABF0 produces links+0/+4/+8,
color+20=1, nil+21=0. Inserted nodes use the same layout and allocation size.

BDEE60 produces the actual14h-byte record at node+C: signed priority+0,
string length/data+4/+8, provider+C and flag byte+10. Record padding+11..13,
node padding+22..23 and tree+0 are untouched. This agrees with the independently
owned BDCE40/BDEEC0/BDCFC0 record packet. No duplicate record type is introduced.

The copy captures priority and whether the destination/source string headers
alias, then writes priority and zeroes the destination string. A self-copy
therefore empties its string without releasing the previous pointer. For a
distinct source, 41DD40 resizes from the current source length with preserve1;
the copy then rereads source length, and if nonzero captures destination length,
source data and destination data in that order. BF7680 receives those three
words; ADD ESP,0C confirms the cdecl cleanup. Its backward-overlap branch at
BF769A..BF7844 justifies the existing source use of `memmove`.

Provider is read and stored after string copying; the flag is separately read
and stored afterward. No AddRef, provider destructor or ownership policy exists
in this closure. As in the established actual string API, a zero-byte native
copy is omitted after preserving its preceding reads.

## Search, linking and fixup

BE1330 reads the initial head/root before inspecting the supplied record. If
the root is nonnil, it captures the new signed priority once in EDI at BE134D.
`CMP EDI,[EAX+C]; SETG DL` chooses left only for a strictly greater signed
priority. Equality always descends right. The result is descending signed
priority with stable insertion order among equal priorities; there is no
duplicate rejection or same-prefix replacement.

The captured parent and last direction byte go to BE0DD0. It checks current
unsigned count against0CCCCCCBh, captures the current head once for both nil
children, and calls BE05C0 with five stack arguments and color0. Allocation
and record/string copying finish before any count or tree-link publication.
After success, it captures the **current** head, then increments the **current**
unsigned count with DWORD wrap. A callback can therefore change either header
between allocation and publication. No second capacity check or rollback exists.

The root path reloads head separately for root/minimum/maximum stores. The
nonroot paths install left/right and compare the captured parent against the
current minimum/maximum before updating that extremum. Red-parent fixup covers
both sides, red-uncle recoloring, near-child rotation and far-child rotation.
BE0F46..BE0F84 inlines the same current-child/current-parent/current-head
sequence independently verified in the existing BDA0E0 helper. The source
shares that established helper; it does not infer an additional native CALL.

After blackening the current root, BE0DD0 publishes output **node before owner**.
BE1330 copies that local iterator to the caller in **owner, node, success-byte**
order. The success byte at output+8 is always1, and output+9..11 remains
untouched. The outer record priority capture and the producer's separate
priority capture must not be collapsed into a general comparator/record copy.

## Allocation and exception evidence

BE05C0 calls shared BF681B with24h (ADD ESP,4), retains the allocation at
EBP-14 and EBP-18, arms state0 then1, and for nonnull storage writes links,
calls BDEE60 at node+C, then writes color+20 and nil+21=0. It neither clears
payload/padding nor translates a hypothetical returning-null allocation into
a different result. The established source allocator normally throws instead.

FuncInfo E00D50 has unwind map E00D38: state0 -> -1/no action;
state1 ->0/CC6700; state2 ->-1/no action. Its try map E00D24 covers states0..1,
catch state2, with catch-all handler E00D14 pointing to BE063E. CC6700 passes
captured allocation words to401130 (ADD ESP,8); the full401130 body is RET.
Thus there is no partially constructed payload destructor. BE063E frees the
captured allocation and BE064E rethrows through BF6885 with two zero arguments.
The source catch frees exactly that raw allocation and rethrows its source
exception. A failure in BF681B itself occurs before the guarded region.

BE0DD0's length-error route assigns19 bytes of the diagnostic, arms state0
only at BE0E21, calls411700, publishes native D69260 and throws with D83F98.
FuncInfo E00E10 uses E00E08 `{previous=-1, action=CC6780}`; the action destroys
the completed message at EBP-50 through4072D0. Source uses the existing
`NativeHardwareLayoutTreeLengthError` transport and completed-message guard.
Original FH3/SEH and host exception identity remain distinct.

## Stored-body gaps and verification

Catch_All@BE063E currently ends atBE0646 inclusive. BE0647..BE0652 is a
verified raw continuation containing ADD ESP,4 and the zero/zero rethrow CALL.
It has no containing Ghidra function. Handlers CC6711..CC671A and
CC6788..CC6791 likewise have no stored function; they are static FH3 metadata
evidence, not additional reconstructed source functions. The report labels
all three raw spans with inclusive ends. The primary integrator will repair
the existing catch body under its write lock after lease release, refresh its
export and rerun the strict report checker. The pre-integration checker is
expected to reject only the BE064E call-site ownership.

`local/mount_insert_aw/build01.log` records the strict MSVC Win32 build with
both existing CTests passing after seed verification. No repository tests were
added. The ignored reusable driver accepts `--repo <built-checkout>` and a fresh
`--attempt <name>`; every prior attempt, including failures, remains read-only.

The focused fixture compares original insertion/record/node bytes with linked
source across signed extrema, both rotation directions, zigzags, repeated
priorities, self-alias record construction, callback-mutated provider/flag,
current-count wrap, current-head replacement, the accepted maximum count,
and raw allocation color/padding. Failure and length-error cases are source-only.
Its first attempt agreed on native/source states but failed an incorrect
fixture expectation that priority would reload after string allocation.
Both implementations actually retain the earlier priority, while provider/flag
reload; attempt02 corrects only that expectation. Final counts and artifact
hashes are recorded in the report rather than inferred from compilation.

Before each execution the driver physically copies the actual linked objects,
checks them against current objects and archive members, freezes their sources,
BSP headers, library, executable/map/object, commands, build log and original
bytes, then seals everything read-only. Post-execution hashes must remain
unchanged. Only disassembled direct CALL displacements are relocated. Source
bridges provide existing string resize, mount rotations and real CRT allocation/
free; original branches are unchanged. Original length/catch EH routes are
fatal excluded-path boundaries in the native fixture, and are not executed.

Evidence establishes bounded insertion/record behavior and source cleanup.
It does not independently revalidate original dependency bodies, original CRT/
FH3/SEH, throwing lazy pool release, arbitrary stack aliases, concurrent changes,
binary replacement or gameplay. `NativeStringStorage::release` retains its
existing noexcept boundary. Current-head mutation comparisons verify resulting
bytes only; they do not assert that a callback-corrupted tree remains valid.
