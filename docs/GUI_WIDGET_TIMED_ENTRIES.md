# GUI timed-entry allocation and ownership contract

This packet establishes the producer contract for the nonzero timed-entry
boundary in `GuiTextChildDeletion`. It adds evidence, not an allocator, entry
owner or completed destructor. The actual native heap/type association is
missing. No forwarding interface, placeholder vtable, new widget tree or
foreign-pointer free is introduced.

Analyzed producer addresses: `00AA8B00`, `00AA77B0`, `00AA6F30`, `00AD3A80`.
Names below are hypotheses, not recovered symbols. Ghidra stayed read-only in
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, verified by the
repository wrappers for every batch. The existing
`reports/gui_text_child_lifetime.json` supplies the read-only deletion handoff.

| Routine | Native ABI and exact bounds | Coverage |
| --- | --- | --- |
| `BSP_GuiTimedEntry_GetOrCreate` (`00AA8B00`) | ECX widget, signed index stack, EAX entry; final RET4 at `00AA8B7C`, length3, inclusive end `00AA8B7E` | analyzed complete producer; C++ owner implementation absent |
| `BSP_GuiTimedEntryArray_Resize` (`00AA77B0`) | ECX header, signed size stack; RET4 at `00AA77FD`, length3, inclusive end `00AA77FF` | analyzed complete size/initialization behavior; no implementation |
| `BSP_GuiTimedEntryArray_Reserve` (`00AA6F30`) | ECX header, signed requested capacity stack; RET4 at `00AA6F8C`, length3, inclusive end `00AA6F8E` | analyzed complete reserve, including omitted saved-listing bytes `00AA6F81..00AA6F89`; no implementation |
| `BSP_GuiTimedEntry_CreateByIndex` (`00AD3A80`) | ECX index, EDX widget; no stack arguments; final RET at `00AD3B40`, length1 | analyzed complete index selection/allocation/construction; native allocator and concrete type owner remain prerequisites |

The same canonical `GuiWidgetOwnerExtraFields::pointers_88_90` words represent
widget `+88` backing pointer, `+8C` signed count and `+90` signed capacity.
They are not three object pointers. `AA9390` zeros them at `AA9488`, `AA948E`
and `AA9494`, with EBX zero established at `AA941C`. This contract requires a
future owner to bind those SAME live words. A separate vector with mirrored
count/capacity would lose observable state and cannot satisfy deletion.

Reserve clamps the requested capacity to at least1 using signed comparisons.
If current capacity already suffices it returns. Otherwise it requests exactly
`capacity*4` bytes through array-new thunk `BF55BE`, with DWORD multiplication
wrap and no growth factor. It copies current count entries, rereading count
and old backing while iterating, then reloads old backing for `BF6989` free.
Only after free returns does it publish new backing at `AA6F84` and capacity
at `AA6F86`. Count is untouched. The saved listing skipped those stores because
of false no-return analysis on free; exact PE bytes and Capstone agree with
Ghidra's read-only byte query. This is an interior continuation, not a new
function. The free call's ADD ESP4 is at `AA6F81`.

Resize reserves only when requested size exceeds current capacity. On growth
it clears each newly exposed pointer slot. On shrink it decrements count to
the requested value, without deleting or clearing removed elements. It then
stores the requested signed count. It does not own pointed-to entry deletion.
Unusual negative sizes and DWORD overflow are native unchecked states, not a
reason to substitute standard vector semantics or a capacity-doubling policy.

Get-or-create ensures size index+1 when needed, checks current slot, and calls
the factory only for a null entry. It captures the destination slot address
before the factory allocation and publishes the returned pointer to that
same captured slot at `AA8B54`. Afterwards it rechecks the current count and
returns from current backing. Allocation/new-handler reentry must not invalidate
the captured slot. There are three distinct resize sites, not a single reserve
followed by cached header reads.

All eight direct widget call sites were inspected. `64820F`, `64824C` and
`649221` request index0; the first two use EBX proven zero by the complete
`648060` register listing. `64F878`, `64FA30`, `64FAEE`, `64FBA2` and `64FCCF`
request index2 on current widgets at their enclosing HUD object's F0/F0/E8/EC/F4.
No direct caller currently requests index1, but the factory body handles it.
Live xrefs enumerate seven resize call sites and three reserve call sites.
The cached caller graph's apparent resize self-edge is not a CALL in the body.

Factory `AD3A80` allocates exactly14h bytes through scalar new `BF681B`.
Indices0 and1 both run `AC2ED0`: base `AD3970` stores profile `D5D204`, time0
at+4 and borrowed widget at+8; derived construction changes profile to
`D5CA74` and initializes +C/+10 to1. Index2 initializes profile `D5D210`,
time0, the SAME borrowed widget and zero at+C/+10. Other indices return null.
These defaults are not two independent widget/state owners. Full-function
register filtering establishes ESI as captured EDX widget across allocation;
ECX is the index, not the destination widget, at the factory entry.

The consumed deleting-profile contracts are:

| Profile | Current virtual0 | Effect |
| --- | --- | --- |
| `D5CA74` | `AC2FE0` | Calls base `AD3990`, which only stores `D5D204`; then frees through `BF65AC` iff flags bit0; EAX original entry, RET4 |
| `D5D210` | raw `AD3A40..AD3A5E` | Stores `D5D204` inline; frees through `BF65AC` iff flags bit0; EAX original entry, RET4 at `AD3A5C`, length3 |

No child widget, string or material ownership is released by these deleting
bodies. Their stored widget+8 is borrowed. This does not make a destructor-only
placeholder vtable sufficient: the live update loop `AA87B0` calls `AD39A0`
with two stack arguments (RET8); it subtracts delta from +4 and dispatches
current virtual8 when the countdown is negative. `D5CA74+8` is `AC2F40`, which
uses widget virtual54 and virtual4C; `D5D210+8` is `ABE7B0`, a separate widget
operation. Their actual callable type/owner bindings are not provided here.
Numeric native vtable addresses must not be installed as callable C++ tables.

`AA9730` remains root-owned. Its entry drain captures each actual slot,
invokes current entry virtual0(1) at `AA986E`, and clears that slot only after
successful return. It reloads count between iterations. The final array free
is `AA994C`; count becomes0 just before it. Backing `+88` and capacity `+90`
are not cleared by this final free. A lifetime association must therefore
record storage retirement without inventing extra native field stores or
freeing the dangling header twice. The unusual negative-capacity repair path
allocates4 bytes at `AA98A3`, copies entries, frees old backing and publishes
capacity1 before the final free; it is not silently supported by a new vector.

The concrete missing prerequisite is an allocation-domain association.
`BF55BE` jumps to `BF681B`; that routine calls native malloc, retries through
the native new handler and throws its native bad_alloc on exhaustion.
`BF6989` jumps to `BF65AC`, whose actual body is `BF9DC8`: it conditionally
uses small-block heap state `0109ED7C` and otherwise calls HeapFree on the
native CRT heap `0109E1BC`. Existing `singleton_lifetime_allocate/free` are
explicitly matching HOST `std::malloc`/`std::free` services for newly allocated
reconstructed C++ objects. They have no association proving ownership of an
already populated native `+88` header or its entries. Passing such pointers to
them would mix allocation domains. No canonical raw timed-entry owner or
native CRT heap service was found in the current source.

A ready implementation needs to establish all of the following together:

1. One allocation domain for pointer backing and14h entries, with allocation
   identity/provenance and its matching reserve/free paths. Foreign pointers
   cannot be adopted merely because their bytes resemble the header/profile.
2. A live binding to the existing widget header and borrowed widget identity,
   plus concrete timed type dispatch for the verified profiles. No duplicate
   widget tree, copied live array or default update callback.
3. Publication and destruction order above, including allocation reentry,
   captured-slot validity, flags0 versus flags1 and backing retirement after
   final free. Only then may the root's AA97F6 boundary be removed.

The current executable's reconstructed Text deletion continues to reject a
nonzero header. No C++ owner/header files or tests were added, so compilation
is not applicable to this evidence-only packet. The machine-readable report
records exact direct/indirect call rows and verification. No runtime,
native differential, ABI or nonzero-array deletion completion is claimed.
