# Scene command list push-back

| Native routine | Span | Coverage |
| --- | --- | --- |
| `00468350` | `[00468350,004683A7)`; 87 bytes, 36 instructions | complete ordinary Source |

The final `RET 4` starts at `004683A4` and includes bytes A4 through A6.
The earlier 85-byte/38-instruction estimate omitted part of that instruction
and did not describe the complete body. All 87 bytes match live Ghidra and
the installed PE.

The original ABI has the actual list in ECX and one stacked DWORD at entry
ESP+4. ESI captures the list before allocation. The new ordinary C++ API
takes the actual 12-byte list and explicit backing for that original argument
cell. Source pointer metadata stays stable and disjoint from reached storage;
the actual reached DWORD storage may alias. This exposes the late argument
read instead of eagerly passing a payload value. The body has no semantic
EAX return, local exception handler, catch, rollback or capacity check.

Allocation requests 12 bytes from the unchanged real current-CRT allocation
service. On a nonzero result, native code writes zero to node+0, +4 and +8,
in that order. It then reads the original stacked argument and stores node+8.
The native null-result branch also reaches the store through address 8;
Source retains that path without adding a null guard. The concrete provider
returns nonnull storage or throws, so its admitted returning path does not
supply null.

Next, the body captures current list+8 and writes node+0, then reads current
list+0 for the branch. The nonempty branch reloads list+8, writes that tail's
+4, publishes list+8, zeros node+4, then reads and increments CURRENT list+0.
The empty branch publishes list+4, then list+8, zeros node+4, then reads and
increments CURRENT list+0. The final count is not the earlier branch count.
The raw accesses are ordered volatile DWORD operations; address and count
arithmetic wrap as Win32 unsigned words. No blanket nominal-structure or
nonalias restriction replaces those reads and writes. Reached storage must
support the actual aligned raw DWORD accesses and genuine allocator domain.

| Containing routine | Native listing | Contract |
| --- | --- | --- |
| `00468350` | `00468356 CALL 00BF681B` | size 0Ch; caller ADD ESP,4; allocation precedes original argument read |
| `00469610` | `0046966C CALL 00468350` | ECX is captured database+14Ch; child argument was pushed before state=-1; child RET 4 |

The sole incoming caller constructs a 14h record before pushing its result;
this packet gives no new raw Source credit to that constructor or caller.
Payload ownership and a nominal payload type are not inferred. The existing
semantic `Seg6Host` method remains separate and unchanged.

The full 105-byte native allocator was read. It retries malloc after a
nonzero new-handler result and constructs/throws its native bad_alloc after
handler failure. The existing concrete service uses `std::malloc`, `_callnewh`
and `std::bad_alloc` with equal native/host request sizes here. Its allocation
kind is descriptive metadata and is unused by that service. Original heap,
encoded handler/global identity, errno/fatal paths, exception object/RTTI and
throw transport are not proved by that current-service boundary. No callback,
new heap, synthetic allocator context or private exception frame is added.

Source, header, provider and complete native byte hashes are in the report.
Primary independent review passed the complete Source/header and concrete
provider without changes. Locked annotation retained the existing descriptive
name, preserved the full previous comment, saved the project and refreshed
the affected export. Both typed transfers and both documentation claims pass
verification. The primary registered Win32 build at `efa7febae` passed all three existing
checks in 9.55 seconds. Its immutable receipt and log hashes are pinned in
the report. The retained 137-byte emitted entry preserves the complete raw
access schedule: allocation before node zeros and late payload, captured
tail followed by the nonempty current-tail reload, and final current-count
read after ordered publications. The exact object occurs once in the
archive. Compiler metadata, request and security-cookie instructions are
additional ordinary Source transport; this is not native byte or private-ABI
identity. No worker CMake queue or duplicate registration was taken. No tests are added. Physical stack/register/fault/private ABI,
Original allocator identity and runtime/game behavior remain unproved.
