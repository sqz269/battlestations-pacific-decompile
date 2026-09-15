# Vehicle-class model binding: complete static schedule, source still open

Addresses: 0095F500

`0095F500` first calls the canonical damageable binder, then derives camera and
`slot` frames from the actual resource at class+50h. This packet reviews its
entire 2518-byte body, including all 633 listed instructions and both repaired
returning-free continuations. It adds no C++ implementation. The camera-container
destructor and its cleanup family remain dependencies of a complete source port.

| Routine | Coverage | Reconstruction |
| --- | --- | --- |
| 0095F500–0095FED5 | Complete static listing and pseudocode review | None; 0 source bytes |

`BSP_VehicleClass_BindModelData` is a descriptive hypothesis, not a recovered
symbol. Original entry: ECX class, no stacked arguments, plain `RET`. The body
saves EBX/EBP/ESI/EDI and uses an MSVC exception frame. EBP holds the class until
the slot pass temporarily changes it to a captured pointer-array element address.
There is no observed useful return-value contract.

## Input storage and evidence boundaries

The binder borrows live native storage. It does not clone a resource, acquire a
class reference or build a replacement registry.

| Storage | Observed use | Producer/contract evidence |
| --- | --- | --- |
| class+50h | Resource pointer, read after 00879AD0 and again during scans | Canonical `native_damageable_class_binding.hpp`; producer 00879590 is documented in `NATIVE_DAMAGEABLE_CLASS_MODEL_BE.md` |
| resource+64h | Checked vector owner; +68h begin and +6Ch end | Existing `native_game_resource_named_groups.hpp`; BF establishes item producer 0071ABB0/0071B3E0 |
| point item+8h | SBO name; buffer +Ch, length +1Ch, capacity +20h | Existing named-group contract; no new layout declaration |
| item+24h | Identifier index | Existing named-group contract |
| item+48h/+4Ch | Vec3 begin/end, signed wrapped byte distance divided by 12 | Existing named-group contract |
| class+BCh | Optional camera container; begin/count at container+0/+4 | `VEHICLE_CLASS_FIELDS.md`: MovieCameraPositions producer 009624A8 onward, 5Ch row stride; binder writes row+8h |
| class+94h/+98h | Pointer-array begin/signed count, passed as header to 005471B0 | Consumer access and 005471B0 body checked here; full element producer is not recovered in this packet |
| pointed slot object+4Ch | 16-word frame destination | Consumer offset only; no new owning-object type or inferred field name |

## Complete body schedule

All sites below belong to `0095F500`. The JSON report contains every direct call
as a separate `address`/`native`/`function` row, including all validation calls.

1. **0095F500–0095F54F:** establish EH state -1; call 00879AD0 with the incoming
   ECX class at 0095F525. Capture resource+64h as the iterator owner and its begin
   as the cursor. Initialize greatest camera index to signed -1 and EBX to zero.
2. **0095F550–0095F60A:** reload the current class+50h resource and current end on
   each iteration; validate unsigned begin/end order, captured/current owner
   equality, and cursor bounds. Compare the counted name against six bytes of
   `camera`, requiring exact length six. Require a nonzero point begin and signed
   point count greater than two. Comparison at 0095F5B9 has three arguments and
   caller cleanup `ADD ESP,0Ch` at 0095F5BE.
3. **0095F60B–0095F9D7:** form signed/wrapping `index = item.index - 1`, retain the
   greatest index, and initialize a 16-word frame with the actual one cell at
   D7A24C. Its translation is raw point 0. Row 2 is point 2 minus point 0. Row 1
   becomes `(point2-point0) cross (point1-point0)` through 004F9B30, followed by
   in-place 0085DC80. If the current camera container exists and signed index is
   less than its current count, call 004134F0 on `begin + index*5Ch + 8h`.
   **There is no nonnegative-index guard.** Do not silently add one or use an
   unsigned comparison in a source port.
4. **0095F9D8–0095FA32:** validate the captured iterator again and advance it by
   four. On exhaustion clamp the greatest index to at most container.count-1
   using signed comparisons. If the resulting value is <= -1 and a container
   exists, call 0095DB40, free the captured container, then zero class+BCh.
   Caller cleanup for this free is `ADD ESP,4` at 0095FA2A. This is conditional
   destruction, not a count resize.
5. **0095FA33–0095FB4D:** iterate signed EBX from zero while EBX < the current
   class+98h count. At 0095FA54 and 0095FA80, call 005471B0 with ECX class+94h and
   stacked EBX+1 when the current count is too small. The `NEG; SBB; TEST F8A0BCh`
   sequence at 0095FA65 only tests whether the selected pointer is nonzero; it
   does not read F8A0BC or inspect an object type. Capture the address of that
   pointer-array element in EBP. Construct a counted SBO string from the four
   bytes `slot` at CEB728, then find the **last** matching group with index EBX
   via 00718000. Arm EH state zero only after string construction returns; disarm
   before its conditional heap free. Reset SBO capacity/length/first byte. Skip
   absent groups or groups with fewer than three points.
6. **0095FB4E–0095FEA9:** initialize another identity-like frame from D7A24C,
   copy raw point 0 to translation, derive row 2 and row 1 with the distinct
   slot-pass schedule below, then call 0085DC80 at 0095FDDE. Reload the object
   through captured `[EBP]` and copy all 16 output words to object+4Ch using
   sequential `MOVSS` loads/stores. This output copy does **not** call the x87
   matrix-copy helper.
7. **0095FEAA–0095FED5:** restore EBP from the captured class, increment EBX,
   compare against the current count, restore exception state and registers,
   and return without stack arguments.

## Floating-point and returning-validation details

Use the canonical **native** interfaces when resuming: the pure arithmetic
matrix interface cannot stand in for the full x87/SSE schedule.

| Work | Camera pass | Slot pass |
| --- | --- | --- |
| Point-0 origin | Three MOVSS values captured in stack temporaries before the next validation | A point-begin pointer is captured in EDI; subtraction reads through it after the next validation |
| Delta | Point 1/2 coordinates pass through individual FLD/FSTP temporaries before subtraction | Subtract directly from the reloaded point 1/2 coordinates |
| Row-2 transfer | Delta is spilled, then copied by three more FLD/FSTP pairs | Same additional row-2 FLD/FSTP pairs |
| Cross | 0095F979 invokes 004F9B30; EDX is point2-point0, stacked operand is point1-point0, ECX output | 0095FD72–0095FDDE computes the cross inline, with retained x87 intermediates and MOVSS publication of its three spills |
| Final publication | 004134F0 performs sixteen ordered x87 pairs | Sixteen sequential raw MOVSS transfers |

The camera cross argument is pushed at 0095F932. EDX then becomes the temporary
at steadyESP+118h and ECX the output at steadyESP+130h; the helper's `RET 4` at
004F9BA6 restores steady ESP. The frame at steadyESP+BCh is passed to 0085DC80
at 0095F9AE. The slot frame is steadyESP+58h. The complete binder listing was
reviewed for EBX/EBP/ESI/EDI writes; no provenance is inferred from a single zeroing
instruction. EBX remains zero throughout the camera pass and changes only at
0095FEAE in the slot loop. EBP's temporary element-address role is established
at 0095FA89/0095FA91 and restored at 0095FEAA.

Each validation may return. The implementation must retain captured owners,
indices, pointers, and scalar origins while reloading the actual storage at
the native sites. A generic `get_point()` that always snapshots or always reloads
the origin would merge observably different camera and slot contracts. Negative
indices, signed overflow, NaN quieting, ambient x87 state and intermediate spills
remain source-port concerns; this packet executes none of those paths.

The entry pushes handler CA95A8. Live listing selects FuncInfo DDD650, whose
bytes identify one unwind row at DDD648: state 0 -> -1 through CA95A0. The listed
funclet is `LEA ECX,[EBP-4Ch]; JMP 004072D0`, matching the SBO temporary's cleanup.
These handlers are not defined as functions here and no native EH runtime claim
is made.

## Dependencies and continuation packet

| Native entry | Existing composition or remaining work |
| --- | --- |
| 00879AD0 | Canonical complete `bind_native_damageable_class_model_00879ad0` |
| 00718000 | Canonical complete `find_native_game_group_00718000`; counted exact name/index, last match |
| 00408720 / 004072D0 | Canonical raw SBO assign/destruct interfaces; throwing cleanup remains to compose |
| 004F9B30 | Canonical `camera_vector_cross_004f9b30`; camera pass only |
| 0085DC80 | Canonical `orthonormalize_native_pose_matrix_0085dc80`, with actual CRT/constants |
| 004134F0 | Canonical `copy_native_camera_matrix_004134f0`; camera publication only |
| 005471B0 | Read-only body review confirms pointer-array grow/zero/shrink-release contract; parent owns source recovery, no implementation added here |
| 0095DB40 | Required camera-container cleanup closure; incomplete listing when first inspected |

Initial live 0095DB40 body ended at 0095DB88, after `_free` at 0095DB84.
Disk continuation 0095DB89–0095DBC6 restores caller ESP, clears tree+4/+8,
calls 005CD640 with stacked zero at 0095DBA6, releases container[0] through
00BF6989 at 0095DBAE, then restores EH/registers and returns. Padding begins at
0095DBC7. These disk continuations are candidate body bytes until the integrator
repairs and verifies their live ownership. They must not be attributed to the
old truncated Ghidra body. During this packet the parent cleared both returning
call overrides and decoded that tail, but the repair tool did not extend stored
function-body metadata: the reported body remains 0095DB40–0095DB88. Ownership
repair remains open. The parent's `vehicle_binding_dependency_flow_repairs_orch4`
report retains those events.

The already listed cleanup path calls 0095D2C0, whose full-range branch calls
0095B860. Its returning-free gap at 0095B88C–0095B896 after 0095B887 was repaired
by the parent under a separate lease, saved and exported; this worker did not
make that mutation. Its source remains open. 0095B860 references 0079D220;
the non-full-range path of 0095D2C0
references 007931C0 and 0095B5A0. The existing `STL_xlen_throw_0095b5a0` name
is not accepted as a contract from that call alone. Do not close this family by
inventing a no-op destructor or assuming the full-range path is the only possible
path when validation callbacks can mutate its storage. The camera-row lifetime
producer, 005CD640, and the cleanup/EH dependencies need a bounded follow-up.

## Verification and limits

Live queries use `config/target.json`: project `bsp` at
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. All queries were
read-only; this worker made no Ghidra mutations. The report records the full
binder byte comparison against the configured installed PE, listing/call counts,
and the exact verifier result: 34 call rows checked, zero failures. All 2518
live binder bytes matched the configured installed PE; their SHA-256 is
`e5f9b9fb5e78051d3476e9887f257cd909dcabca80c6407d37c8145aa09d2a29`.
No build or CTest was run because no source or
build files changed. No differential, throwing-EH, executable, or gameplay test
was performed.

Two live, function-owned callers are 007D3E81 in 007D3E60 and 0082FE51 in
0082FE30, both preserving incoming ECX. A third listed `CALL 0095F500` at
0074DA2E follows `MOV EDI,ECX`, but live `proto` reports no containing function.
It is retained as unresolved boundary evidence, not attributed to a guessed
function or counted as a function-owned verified caller. Six data references
at CFF7B0, CFF7EC, D1A558, D1A9C0, D1A9FC and D1AA78 are not direct calls.
