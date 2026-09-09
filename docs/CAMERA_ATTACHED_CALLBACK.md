# Attached group notification from camera transforms

Bounded read-only analysis, 2026-09-09. Every live batch used Client.verify()
from tools/ghidra_export.py to verify project `bsp`, program
`/battlestationspacific.exe`. No Ghidra functions/names, shared metadata or C++
were changed. Raw exports are ignored. This is evidence inspection, not a
runtime test or proof of complete hierarchy behavior.

## Concrete attached type

Transform +A0h has a concrete group-owner route. Constructor `00b8f5e0`
calls transform constructor `00b6f5a0`, installs vtable `00d634f8`, initializes
pointer/count/capacity at +178h/+17Ch/+180h to zero, and sets +138h bit2.
The native type-registration helper at `00b8f590` stores the string address
`00d634f0`, whose bytes decode to **cGroup**. Adjacent `00d634d8` contains
**cGroupParamsResource**. These are observed native strings, unlike the
proposed descriptive function names below.

Vtable +8h points at `00b8e620`, returning type ID global `0109032c`.
Vtable +Ch points at `00b8f650`, which compares its stack type ID against the
three entries at 0109032Ch, 01090330h and 01090334h. The reparent path
`00b6e680` uses `00b8e600`, another getter of 0109032Ch, with a candidate
parent's virtual+Ch. A non-group parent substitutes its +A0h owner; a group
parent remains the group candidate. This ties the observed type directly to
the attached-owner route, rather than inferring it solely from nearby strings.

This investigation establishes one concrete installed vtable. It does not
prove an exhaustive list of derived group types or overrides.

## Attachment and removal ABI

| Address / proposed name | Checked interface and behavior |
|---|---|
| `00b6d7b0` / `BSP_Transform_SetAttachedGroup` | ECX=transform, stack group pointer, RET4. Detach old group through 00b8f4c0 if present; assign +A0h=new group; if nonnull call 00b8f460(new group, this), then dispatch virtual+1Ch on each immediate child with that group |
| `00b8f460` / `BSP_Group_RegisterTransform` | ECX=group, stack transform pointer, RET4. Only if transform+A0h differs from this group, append transform to group +178h vector, growing capacity as needed, increment +17Ch, then assign transform+A0h=this |
| `00b8f4c0` / `BSP_Group_UnregisterTransform` | ECX=group, stack transform pointer, RET4. Only if transform+A0h equals group, remove pointer through 00b7bed0 on group+178h and then clear transform+A0h |
| `00b7bed0` / `BSP_PointerArray_RemoveBySwap` | ECX=array header, stack pointer to searched pointer, RET4; bool AL result. Search array; replace found slot with last entry unless already last; decrement count; no stable ordering |

The assignment-before-registration sequence in 00b6d7b0 is present in assembly:
00b6d7ca writes +A0h and 00b6d7d5 calls registration. Thus registration's
unequal-owner guard can skip insertion on this path. Do not silently reorder
these operations to fit a conventional ownership model. Additional group
attachment override `00b8f4f0` also exists; its entire ownership behavior is
outside this bounded reconstruction.

## Actual +3Ch callback

The installed cGroup table entry at `00d63534` (00d634f8+3Ch) is
`00b6dbc0`. Its entire 25-byte body is:

```asm
00b6dbc0 and dword ptr [ecx+138h], FFFFFFC3h
00b6dbc7 mov ecx, dword ptr [ecx+A0h]
00b6dbcd test ecx, ecx
00b6dbcf je 00b6dbd8
00b6dbd1 mov eax, dword ptr [ecx]
00b6dbd3 mov edx, dword ptr [eax+3Ch]
00b6dbd6 jmp edx
00b6dbd8 ret
```

Proposed name: `BSP_Transform_InvalidateBoundsAndNotifyGroup` (bounds is an
interpretation of the flags, not an independently established full flag map).
ECX is the receiver; there are no stack arguments and no meaningful return
contract established. It clears receiver +138h bits4h/8h/10h/20h, then tail
calls the attached group on that group's ECX. It does not pass the initiating
transform pointer. With no attached group it returns immediately. With nested
groups this follows the owner chain upward; it does not iterate +178h members,
+34h children, or +3Ch siblings. No cycle detection is present in the body.

SHA-256 for 25 bytes through RET, excluding INT3 padding:
`ff03f5e2a5c5c4450e8385ba9b6c671844c50e3c6de7f712550e6988134727f7`.

The group's virtual+40h entry is instead `00b8e6b0`: it only ANDs +138h
with FFFFFFCFh and RETs. The camera table's +40h entry is `00b6dbe0`, which
clears +138h bits10h/20h then follows its +A0h virtual+3Ch chain. These two
+40h targets must not be conflated.

## Camera cache consequence and implementation boundary

The established group notification body has **no write to camera +2F0h**.
It also does not clear a member's world/view flags at +5Ch. It cannot itself
repair combined camera caches after an ancestor changes: notification travels
up through group owners and carries no initiating node argument. Separately,
the transform descendant invalidation walker 00b6da30 clears child +5Ch
bits2h/8h but does not invoke these callbacks or write camera +2F0h.

Direct camera setters still use their confirmed FFFFFE4Bh mask on +2F0h
before the base setter and direction/target refresh; see
CAMERA_DIRTY_PROPAGATION.md. That direct-setter behavior does not establish
ancestor-driven combined-cache invalidation.

An explicit optional notification callback is an appropriate adapter boundary
for the reconstructed local-matrix setter. It should preserve notification
order and the existing validity gate, and should not claim to be native cGroup
ownership or infer camera cache writes absent from the evidence. Implementing
the observed group +3Ch callback itself requires only the +138h mask and
optional enclosing-owner dispatch; claiming full group registration, lifetime,
scene updates or derived-type behavior would require further evidence.

Remaining uncertainty: another derived owner override, scene update phase, or
camera-specific hierarchy restriction may account for ancestor-driven camera
cache behavior. None is established by this bounded trace; no such behavior
was invented or added to the repository.
