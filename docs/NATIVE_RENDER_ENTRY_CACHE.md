# Native render-entry cache in window startup

The 14h allocation following device startup is the application's render-entry
cache at 108FE88. Its array holds 10,000 records of 28h bytes (400,000 requested
bytes), consumed by mesh/instance collection and debug rendering. The counter at
owner+8h is reset by the existing end-frame source. The allocation's purpose is
established by these consumers and the recovered record initializer.

`native_render_entry_cache.cpp` reconstructs the actual owner, its array and
concrete destruction. The production helper for BED1E8..BED222 now performs the
window startup allocation sequence after device startup in the composed fixture.
The existing typed platform projection remains separate. Detailed evidence is in
`reports/native_render_entry_cache.json`.

| Body | Bytes | Recovered operation / original ABI |
| --- | ---: | --- |
| BEBF00..BEBF26 | 39 | Record initializer; ECX record, EAX same record, RET |
| BEC240..BEC249 | 10 | Free array data; ECX header, RET |
| BEC3E0..BEC479 | 154 | Reallocate unless requested equals used count; ECX header, stack DWORD, RET4 |
| BEC590..BEC620 | 145 | Publish/register base; ECX owner, EAX original owner, RET |
| BEC630..BEC6C8 | 153 | Unregister current publication; ECX owner, RET |
| BEC6F0..BEC70D | 30 | Base scalar deletion; ECX owner, stack flags, EAX original owner, RET4 |
| BEC870..BEC8DD | 110 | Construct cache; ECX owner, EAX original owner, RET |
| BEC8E0..BEC904 | 37 | Destroy cache, tail JMP BEC630; ECX owner, RET through base |
| BEC910..BEC949 | 58 | Derived scalar deletion; ECX owner, stack flags, EAX original owner, RET4 |
| BED1E8..BED222 | 59 | Fragment inside cdecl11 BECEE0; allocate14h, construct, resize current publication to10,000 |

The nine bodies total 736 bytes. The fragment is not a newly discovered function
entry. Source context arguments are new interfaces, not drop-in register/stack or
FH3/SEH replacements. Descriptive names are hypotheses, not recovered symbols.

## Raw storage and order

The actual 14h owner contains profile00, data04, used count08, capacity0C and
tracked critical-section pointer10. Its array header is the embedded 0Ch region
at +4h. Record initialization uses SSE stores: +00/+14 receive positive zero,
then the current D7A24C one literal is captured, +08/+0C/+10 are cleared and +18
receives that captured value. DWORDs +04/+1C/+20/+24 remain untouched.

`BEC3E0` compares requested capacity against the **used count**, not the capacity
field. Equality returns without touching any header field. Otherwise it captures
old data, zeroes count/capacity and frees data. A nonzero request allocates
requested×28h bytes; unsigned multiplication overflow yields FFFFFFFF as the
allocation request. Record iteration follows the signed SUB/JS/JNS schedule in
the complete 403560 library helper. Data and capacity are then published; used
count remains zero. A zero request leaves the now-freed data pointer stale. No
buffer-preservation, capacity guard, null-success correction or safe-container
policy has been added.

Base publication/removal uses the application's existing raw manager. The first
getter's section+10h is captured for Enter, the extra section+18h depth and Leave.
Publication precedes the second getter and the current-publication registration
read. Removal rereads publication after the second getter and clears it only
after BCFCA0 returns. The original receiver is distinct from that current value.
Normal Leave stays within state1 cleanup. State1 calls guard411EE0, then state0
resets the original receiver through412430. FH3 maps E01EF8/E01EE8 and
E01F2C/E01F1C establish the schedules, including first-enter failure cleanup.

`BEC870` completes base publication, writes D68CC0, captures old count08, arms
state0 and clears data04. If old count is nonzero it clears count/capacity, frees
the current data (now null in ordinary execution), then repeats capacity=0.
With old count zero, old capacity remains unchanged. It arms array cleanup before
creating the actual tracked section through BD1860. E01F68/E01F50 state2 unwinds
the current header through BEC240 before state0 base removal. The intermediate
state1 has no action. Resize's E01EC4/E01EBC state0 frees its captured allocation
if record construction unwinds.

Both concrete cache destructors restore D68CC0, release the current tracked
section, free current array data, then remove the base publication. The scalar
form additionally frees the original owner when flags&1. There is no added
rollback on an earlier destruction failure, element destruction, header clearing
or array preservation.

The window fragment allocates14h at BED1EA and constructs at BED203 when the
allocation is nonnull. It reloads108FE88 at BED208 and derives its array header,
disarms owner-free cleanup, then calls resize at BED21E with10,000. Constructor
failure frees the captured allocation after constructor cleanup; resize failure
retains the published owner. E01FC0/E01FB8 and CC7480 confirm that distinction.
The source helper preserves the publication reload and null-allocation branch.

## Listing repair and evidence

Erroneous CALL_RETURN overrides on CRT free thunks hid reachable instructions.
Repairs cover BEC240, BEC3E0, BEC6F0, BEC870, BEC8E0 and BEC910. Disassembling
the missing bytes alone left holes in stored function bodies; bounded recreation
was also necessary. The final six listings have zero gaps and both formerly
unowned BEC435/BEC458 calls belong to BEC3E0. The BEC8E0 body ends in a proven
tail jump; a local copy of the repair tool permits only that exact reviewed tail,
retaining byte checks, leases and the Ghidra write lock.

All 736 body bytes and the59-byte window fragment match live Ghidra and the
unchanged installed PE. Additional captures cover FH3 maps/funclets, profile and
one literals, the free/new thunks and the complete library iterator. The call
verifier passes24 direct CALL rows and one tail JMP. Existing library names remain
unchanged. Repair events and prior function metadata are retained in
`reports/native_render_entry_cache_flow.json` and
`reports/native_render_entry_cache_definitions.json`.

## Validation and remaining scope

The final strict Win32 build and both existing CTests pass. No permanent tests
were added. Final fixture compile/link3 and runtime capture2 use the new production
startup helper; 24 consistent explicit source units were compiled and23 linked.

A focused case executes all39 original BEBF00 bytes unchanged, against the mapped
original one literal, and compares all40 output bytes and the return pointer with
source. Dirty unassigned fields are preserved. A zero-count constructor preserves
an incoming capacity; a detached array header verifies equality against used count
and the stale pointer after resize(0), without subsequently freeing that pointer
again. The concrete flag0 destructor releases the temporary owner's section and
base. The production window fragment then creates the actual heap owner and
10,000 records; every initialized field is checked, used count is zero, and the
owner is registered in the same manager as platform/renderer.

The composed source path continues through installed cold DDS/effect admission:
eight exact HAL shader bytecodes, primary4/secondary1, canonical28 and cursor164.
Device startup and the real focused Reset retain the16MiB vertex/1MiB index
buffers and default surfaces. The actual control thread joins exit0, focus is
restored and the timer request is balanced. The cache's concrete flag1 destructor
then releases its section, buffer and owner and removes its registration and
publication. Installed files are hash-checked before/after.

This is source composition and bounded original-leaf differential evidence, not
original whole-cache/FH3/SEH ABI or gameplay validation. Unwind states are not
fault-injected; invalid storage, overflow allocation success and concurrent
mutation are unproved. Full BECEE0/WndProc/power startup, cache-consuming active
frames, drawing and application shutdown remain outstanding. Generic singleton
deletion dispatch for D68CBC/D68CC0 is not integrated; its shared files are leased
to another owner. The tested cache uses its concrete native scalar route. The
fixture still supplies its STATIC window/activity, renderer scratch preimages,
material policy, PC/USA and loose-file mount.

Address index: `00bebf00`, `00bec240`, `00bec3e0`, `00bec590`, `00bec630`, `00bec6f0`, `00bec870`, `00bec8e0`, `00bec910`, `00becee0`.

Immutable closure: `local/checkpoints/0ca920c7/native-render-entry-cache/validation.json`, SHA-256 `1a2fd0c640d113df03ddf7c8c133126809b76cf903c71c07f2a002ea37079895`. It retains 5049 artifacts, 81 physical mapped Win32 modules and 306 selected production source providers, including source/compiler inputs, objects, original bytes, assets, results, prior annotations and repair events. The captured docs/report precede this closure metadata addition.
