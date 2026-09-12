# Native physical pending-record lifetime

Addresses: `00BF3880`, `00BF3C10`, `00BF3DA0`, `00BF3ED0`, `00BF4B80`.

This packet reconstructs the five complete normal-storage bodies against actual
Win32 record/queue bytes. The supplied `NativeStringStorage` and current CRT
allocation/free services remain explicit boundaries; these C++ interfaces are
not native ABI entry replacements. Descriptive names are hypotheses.

| Routine | Bytes; inclusive last byte | Original ABI | Coverage |
|---|---|---|---|
| BF3880 names destruction | 119; BF38F6 | ECX record, RET0 | complete normal storage; throwing-getter EH excluded by existing noexcept boundary |
| BF3C10 copy construction | 225; BF3CF0 | ECX destination, stack source, EAX destination, RET4 at BF3CEE | complete, including source C++ second-name failure cleanup |
| BF3DA0 reserve | 237; BF3E8C | ECX queue, signed capacity stack, RET4 at BF3E8A | complete; native reserve rollback action is empty |
| BF3ED0 resize | 151; BF3F66 | ECX queue, signed count stack, RET4 at BF3F64 | complete signed branches and DWORD arithmetic |
| BF4B80 vector destruction | 23; BF4B96 | ECX queue, RET0 | complete, including raw returning tail |

The saved BF4B80 body ends at BF4B91, the last byte of its free call. Disk and
live bytes match through BF4B96. Raw BF4B92 is `ADD ESP,4`, BF4B95 `POP ESI`,
BF4B96 `RET`; exclusive end BF4B97. No owned function needs a new definition,
but this existing body needs its missing tail recovered before relying on its
stored decompile as the entire body. The other four stored bodies are complete
at this audit. Nothing was changed in Ghidra or the installed executable.

## Actual storage and producer evidence

The queue's three DWORDs are backing at +0, signed count at +4 and signed capacity
at +8. BF4D30 initializes this header at physical-provider +14/+18/+1C.
BF3ED0's producer writes establish record stride38h and its initialization mask.
BF43B0 produces the submitted stack record at stable ESP+24h, then BF4665/6D
passes that address to BF41C0 with provider+14h. Full-listing register filtering
establishes EBX zeroed at BF4423 and EDI as the CreateFile result at BF4446.

| Record bytes | Producer and retained meaning |
|---|---|
| +0 | BF44E2, with temporary pushed argument, stores open handle |
| +4 | BF4593, with outstanding pushed allocation size, stores heap OVERLAPPED pointer |
| +8 | BF44E6, with temporary pushed argument, stores operation kind1 |
| +C/+10 | BF4551/55/62 stores original staging block and aligned read address |
| +18/+1C | BF44F6/FC passes ESP+3Ch to GetFileSizeEx for low/high size |
| +20/+24 | BF4649..4E constructs first owned name |
| +28/+2C | BF4653..5C constructs second owned name |
| +30 | BF45B7/C0 stores callback while two caller arguments remain outstanding |
| +14/+34 | no write in default/copy construction; meaning unknown and left untouched |

This agrees with docs/VFS_PENDING_DISPATCH.md and VFS_PENDING_LIFETIME.md.
There is no new typed object overlay. Reads/writes use the existing actual bytes.

## Lifetime and observation order

BF3880 captures the second name data, skips its length read for null data, then
releases captured data with current length+1 and DWORD wrap. It subsequently
reloads the first name data and length. Headers remain intact, including service
callback changes. Handles, staging blocks and OVERLAPPED storage are untouched.

BF3C10 copies scalar words in native order, skips +14, zeroes first name before
its identity guard, resizes/copies first then second name, and only then reloads
and copies callback+30. Source/destination identity still zeroes both headers.
After each resize it rereads source length, destination length, source data and
destination data in that order. Native BF7680 supports overlapping byte buffers;
the source uses `memmove` and omits zero-byte CRT calls after the header reads.
The underlying existing resize helper retains its own CRT/pool limits.

Reserve clamps signed requested capacity to1 and grows only if it exceeds current
signed capacity. Allocation size is wrapping DWORD capacity*38h. Each copied and
destroyed element rereads queue backing/count. Old names are destroyed forwards,
current backing is freed, then replacement backing and capacity are published;
count is never reset by reserve. No vector rollback or value reset is added.

Resize rereads count after reserve, zero-initializes all new words except14/34,
then checks current count for shrinking. It decrements the live count before
each trailing name destruction and reloads backing and count for each iteration.
Final requested count is written after the loop. Signed negatives and wrapping
address calculations are preserved without adding guards; callers remain
responsible for readable/writable memory. BF4B80 calls resize0 and frees the
current backing without clearing pointer/capacity, even for an initially empty
queue. It is not an empty-queue no-op.

## Exception maps and actual services

| Owner | Handler / FuncInfo / map | Native action |
|---|---|---|
| BF3880 | CC7ABB / E027FC / E027F4 | state0 -> -1, CC7AB0 tail-jumps41DD20 on captured record+20 while second name release is active |
| BF3C10 | CC7B1B / E02880 / E02878 | state0 -> -1, CC7B10 tail-jumps41DD20 on destination+20 after first name completes |
| BF3DA0 | CC7B47 / E028AC / E028A4 | state0 -> -1, CC7B30 calls401130 with two stack words; caller ADD ESP,8; callee is a single RET |

Thus failure during first-name construction has no record owner cleanup. Failure
during second-name construction releases only the current first name. Reserve
leaves replacement allocation and earlier completed records undisposed when
copying throws. The source deliberately preserves this schedule; a cleanup guard
for those allocations would change behavior.

BF3880's native getter may throw while releasing second name; its native map
then destroys first name. Existing `NativeStringStorage::release` is noexcept:
that getter-failure domain terminates in `ActualNativeStringPoolStorage` and is
excluded here, as in the integrated VFS pair/base implementations. Bind that
actual bridge to the application's current publication/gate/lifetime domain;
there is no local pool singleton substitute. `singleton_lifetime_allocate/free`
reuse current CRT malloc/new-handler/free semantics, not the game's CRT globals
or FH3 runtime. BF3880's getters consume no pushed release arguments; BD1510
cleans12 bytes. String resize cleans8; BF7680 copies clean12; vector allocation
and free each clean4 at their caller. Raw JMP rows are distinct from CALL rows
in the report.

## Checks and limits

The strict Win32 build and both existing CTests passed after seed verification.
All seven original/source comparisons and the source exception case passed,
along with the current CRT-service composition smoke. All 28 report CALL/JMP
rows passed the live verifier. No permanent tests were added.

The report records live/disk-matching original spans, exact build/probe commands,
frozen source/object/core-library/probe hashes before execution, and results.
The ignored native fixture maps a private PE copy, relocates only exercised
instruction/table references and intercepts allocator services in that process.
It compares full record/arena bytes and ordered service traces for native versus
source cleanup, copy, reserve, grow, destruction and wrapping allocation size.
The source exception case is checked against the verified native map; original
FH3 exception execution is not claimed. A separate unmodified CRT-service smoke
checks allocation/copy/destruction composition before interception.

This satisfies the pending-vector dependency of BF4D30 constructor cleanup.
Complete provider destruction, pending submission/completion, cancellation, tree
cleanup and FileStore manager lifetime remain outside this packet. There is no
frame-causality claim, original ABI replacement claim or game validation.

## AR parent integration, 2026-09-12

All five bodies (755 bytes) are integrated and independently reviewed. The original seven native/source whole-arena and ordered-trace comparisons, one source exception case, and58-input pre-execution proof remain unchanged. Names/comments are saved and exports refreshed; all28 report call/transfer rows pass. Standard locked repair decoded BF4B92..BF4B96, but the stored BF4B80 body still ends BF4B91: full stored-tail repair is not claimed. Original FH3 and game behavior remain unvalidated.
