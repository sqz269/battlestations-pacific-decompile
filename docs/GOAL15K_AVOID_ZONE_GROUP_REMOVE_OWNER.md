# Complete raw group owner removal

Address `0041D150`, live name `FUN_0041d150`. The existing reviewed descriptive
name `BSP_AvoidZoneGroup_RemoveOwner` remains a hypothesis, not a recovered
symbol. Worker makes no Ghidra mutation. The older typed
`cc13_avoid_group_remove_owner_0041d150` remains unchanged.

| Entry | Physical body | Coverage | Source |
| --- | --- | --- | --- |
| `0041D150` | `0041D150..0041D1D8`, 137 bytes, 54 instructions | complete | `src/native_avoid_zone_group_remove_owner.cpp` |

Original ABI is void thiscall: ECX raw 14h-byte group, one stacked owner word,
`RET 4`, no semantic EAX result. The Source entry
`remove_native_avoid_zone_group_owner_0041d150(group,unused_edx,owner)` preserves
that physical receiver/stack shape; unused Source EDX is not an Original input.
Owner is captured once into EBP from the actual incoming stack word. Existing
`Cc13AvoidZoneNativeGroup` and `AvoidZoneNativeStorage` schemas are reused;
group array/count/capacity are `+4/+8/+C`, zone association is `+10`.

For a null owner, ECX still contains the original group. The final Native path
is ordinary `CALL 0041D0C0` at `0041D1CF`, then POP EDI, POP EBP and `RET 4`.
It does not push a clear argument or tail-jump. Source calls the accepted full
raw group-clear provider, which releases active zones, zeroes active slots and
count, and retains the array, capacity, vtable and layer.

For a nonnull owner, the cursor is the captured actual slot pointer. Each
iteration reads `zone+10` BEFORE its later zone-null test. That fault order is
preserved: there is no defensive null-slot guard. Nonmatches advance the cursor
by four bytes. A matching nonnull zone is cleared through the real raw record
cleanup, its CURRENT array word is captured/freed and then zeroed, its body is
freed, and its current slot is zeroed.

After destruction, CURRENT group count and array are reloaded. The current last
slot is loaded and written over the cursor, then one is subtracted from the
CURRENT group count with the original ADD -1 flags. The cursor is not advanced;
the SAME slot is examined again. At each iteration boundary, current count and
array form the end with modulo-DWORD LEA. No stable compaction, inactive-slot
normalization, capacity reset, callback or rollback is added. When removing the
last active slot, its already-zero value is copied back to that slot.

The original stack, saves/restores, comparison flags, all seven branches and
both `RET 4` epilogs are literal. Native two-byte MOV EDI,EDI at `0041D16E` is
retained with its exact encoding. There is no local Native EH registration or
additional Source guard/catch. Every reached raw address and matching current
heap allocation must satisfy the original contract, including the pre-null
association dereference and unchecked modulo end arithmetic.

Normal transfer sites within the owned body:

| Site | Native callee | Source binding and physical arguments |
| --- | --- | --- |
| `0041D17D` | `004167C0` | concrete raw corner cleanup, ECX zone, no stack words/plain RET |
| `0041D189` | `00BF6989` | matched current free, captured array, PUSH/ADD ESP,4 |
| `0041D198` | `00BF65AC` | matched current free, captured zone, PUSH/ADD ESP,4 |
| `0041D1CF` | `0041D0C0` | complete concrete raw group clear, original ECX, no stack words/plain RET |

Dependencies are the actual accepted `native_avoid_clip_cleanup` and
`native_avoid_group_lifetime` entries and genuine `singleton_lifetime_free`.
BF6989 jumps through BF65AC to BF9DC8. No typed callback facade, allocation
provider, fake success routine or Native library rename is introduced.

The sole live caller is `00423640`, at `00423671`: after a nonnull lookup result,
it pushes captured incoming owner ESI and moves the returned group to ECX.
The Original live prototype is still undefined(void); primary owns later name,
physical thiscall prototype, comment/save/export work under the write lock.

Strict MSVC x86 compilation passed with `/std:c++17 /EHsc /MD /W4 /WX
/permissive- /fp:strict /O2`. Fresh target-verifying bsp.py captures used
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. All 137 captured live
bytes match the installed PE and actual Source COFF body except exactly four
verified zero-addend REL32 CALL operands. That is 121 fixed bytes plus the four
actual provider relocations; no instruction/branch/frame byte is otherwise
masked. All 54 instruction boundaries are contiguous. This proves the complete
owned operand/byte mapping under relocated dependencies, not their private CRT,
exception, deployment or gameplay identities.

One ignored real-heap Source fixture passed. Its five-zone array is
`[keep0,match0,match1,keep1,match2]`. It proves a replacement match is re-examined,
nonmatches advance, and removal of the last active match copies the zeroed
slot. The active result is `[keep0,keep1]`, count2/capacity5 with unchanged array,
vtable and layer. Inactive slots retain the original duplicate/stale words.
Bounded heap observations show all nine matching zone/array/record blocks freed
and six retained blocks still used. Null owner then invokes real group clear:
the six retained blocks are freed, count/active slots zero, inactive words and
array/capacity/vtable/layer retained. Actual Source-vtable scalar deletion then
frees the group array and group. These are 17 tracked fixture blocks, not a
global no-leak or Native differential claim. No invalid-null or SEH fault is
forced, and no new test target/framework is added.

Ignored artifacts under `local/goal15k_avoid_group_remove_owner/` include full
Native captures, `compile.log`, `byte_receipt.json`, `verify_bytes.py`,
`remove_owner_probe.cpp`, `run_probe.cmd` and `probe.log`. The byte verifier takes
an optional registered object path; the fixture script takes `archive` to drop
the overlay object. Primary owns final source registration, build/CTest and
registered archive checks.

Original class/RTTI/owner semantics, private CRT/exception/frame identities,
Native binary deployment, runtime differential and gameplay remain unproved.
Pointer owner matching is the recovered operation; no actual owner producer or
production adoption is implied by this standalone Source entry.
