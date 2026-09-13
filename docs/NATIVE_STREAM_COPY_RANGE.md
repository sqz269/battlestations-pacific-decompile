# Actual stream range copy

`copy_native_stream_range_00bef840` reconstructs the complete 168-byte
`BSP_Stream_CopyRangeToMemory` body at `00BEF840..00BEF8E7`. The descriptive
name remains a hypothesis. This is an actual raw-storage service; the older
semantic memory-stream helper does not provide this routine's ownership or
dispatch behavior.

The native ABI is ECX = borrowed source, stack = offset low/high then length
low/high, EAX = new memory-stream wrapper, `RET 10h`. The saved Ghidra prototype
does not express these arguments. Assembly establishes them: offset low/high
are loaded at BEF855/BEF85E, source is captured in EDI at BEF85C, and length low
is loaded into EBX at BEF87F after allocation. The high length DWORD is unused.
The C++ context/dispatch arguments are reconstruction services, not native
parameters or a claim of drop-in ABI compatibility.

The routine captures the source's seek entry at vtable +1Ch and seeks using
both offset DWORDs and origin zero. It allocates a raw 10h-byte backing owner
and calls the established 8D43C0 producer. That producer uses the signed low
length: positive requests become the backing length and allocation size;
nonpositive requests store zero and allocate one byte. Its accounting still
adds the original low DWORD with wrapping arithmetic. No range validation is
added here.

After construction, the routine reloads the current source table, captures
backing data at +8, then reads the captured table's +24h entry. It invokes one
read with the original unsigned low length and a null actual-count pointer.
The return status is unused. A short read leaves the remaining allocation
bytes as they were; the destination is not initialized by this routine.

BEF6D0 creates the actual 14h-byte stream wrapper from the backing owner. The
copy routine captures that result, decrements the backing's intrusive count
at +4, and, on zero, captures the current backing table and its slot-zero
target before dispatch. The borrowed source is never retained or released.
No shared slice is returned, and the source position is not restored.

The one native cleanup state covers backing construction. The action
`CC7670..CC767A` frees the captured raw allocation saved in native `[EBP+4]`.
Its stored Ghidra body stops at CC7678; raw CC7679..CC767A contains POP ECX and
RET outside that stored body. This is recorded as an eleven-byte **partial
cleanup fragment**, including the explicitly uncovered two-byte raw tail.
The handler CC767B and FuncInfo/map E0224C..E02277 confirm the single state.
The state is disabled before the read. Read and wrapper-creation exceptions
therefore retain the backing allocation, as the source implementation does.
The C++ catch models the established allocation cleanup; it does not execute
original FH3/SEH or reproduce arbitrary aliases into the original stack frame.

All three native callsites were inspected with their owning bodies. BB5201
uses source `[EBX+14h]`, offset EDI, and the branch's captured length. BB8CCD
uses source `[ESI+Ch]`, local offset EBX and record length `[EDI+1Ch]`. BBA627
uses source `[ESI+14h]`, raw record offset +14h and length +8. Each passes zero
high words. These callsite observations do not extend reconstruction coverage
to their owners.

The source consumes the existing retained-memory producers, allocation
service, borrowed accounting globals, and captured-target stream dispatch.
No library function was renamed or reimplemented. The report records exact
calls, ABI provenance, raw hashes, and validation artifacts. The focused local
fixture compares relocated original copy/backing/wrapper bytes with the linked
source implementation; callbacks and allocator are host bindings. Original
FH3/SEH, invalid pointers, arbitrary concurrent mutation, the exceptional
zero-reference branch, and gameplay remain outside the runtime proof.

Validation: the strict MSVC Win32 build and both existing CTests passed after
all eight native seeds matched disk. Retained `local/stream_copy_ay/attempt02`
passed four native/source states and 122 checks, including one source-only
read-failure observation. Its 67 frozen inputs include the four actual linked
archive members, their current source files, and only compiler-observed
dependency headers. Every input remained unchanged; current source, header,
object, and archive parity was rechecked. The first attempt is preserved with
its probe-only assembly-identifier compilation error, before native execution.
Seven numeric CALL rows pass the report verifier; the four captured indirect
sites are established by assembly and the stated dispatch contracts.
