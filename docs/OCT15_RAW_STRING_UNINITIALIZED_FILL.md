# Native pooled-string uninitialized fill

This packet reconstructs the complete ordinary Source loop at `0044CE20`,
its catch body at `0044CE6D`, and the `fill_n` dispatcher at `0044DB60`.
It uses the existing actual8h header constructor, stacked-header destructor
and concrete raw pool context. The older qualified checked-storage fragments
remain separate provenance; they are not replaced or promoted by this packet.
Descriptive Source names are hypotheses. Existing Ghidra STL names are retained.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete cached native listings were inspected;
live `bsp.py ghidra` queries verified the program, incoming call instructions,
handler and exception metadata. Source is
`src/native_string_uninitialized_fill.cpp`; its contract is
`include/bsp/native_string_uninitialized_fill.hpp`.

| Entry | Original interface and behavior |
|---|---|
| `0044CE20` | ECX destination, EDX unsigned count; stacked value-header, owner, unused value duplicate, unused tag. RET10h, no semantic return. Capture the initial destination and value-header address; construct count consecutive8h headers. |
| `0044CE6D` | Catch funclet using the parent's EBP frame. Read captured begin and completed end; destroy that prefix ascending via `00431F40`, then call `__CxxThrowException(0,0)`. This is not an independently callable Source entry. |
| `0044DB60` | ECX owner; stacked destination/count/value; RET0Ch. Preserve original destination/count, call the core, then return original destination+count*8 using DWORD arithmetic. |

The core captures value-header address in EBX and unsigned remaining count
in EDI. ESI is the current output. Parent private slot EBP-18h captures the
initial destination; EBP-14h records the completed end. After `0044BCB0`
returns, native code decrements the remaining count, advances ESI by8, then
publishes the completed end. A throwing constructor has not advanced that
end. Count zero skips source reads, construction, pool lookup and cleanup.

The value header is never snapshotted, converted into a host string or copied
to a temporary. The constructor rereads its fields on every call. For example,
if value equals the first destination header, the existing constructor zeros
that header and takes its self-construction path; later elements observe the
resulting header. Other aliases or mutations at real allocation/return
boundaries remain visible through the existing actual-header services.
Captured addresses advance as unsigned DWORD values, including wrap; no
C++ array pointer subtraction or host container model is introduced.

The dependency `construct_native_render_alias_string_0044bcb0` uses the
existing `NativeStringRawPoolContext` overload, its actual `00419CC0`
publication/gate/manager bindings, and its verified no-op placement-delete
unwind leaf. The caller does not initialize or destroy the failed current
slot. Prefix cleanup calls complete
`destroy_native_string_storage_argument_00431f40`; that service ignores
owner ECX and delegates to the actual raw-pool header destructor. No fake
allocator, backing buffer release, vector publication or rollback is added.
The caller provides storage and lifetimes accepted by those services.

The dispatcher retains native consumed argument values. Its low scratch-tag
byte is zeroed; the other scratch bytes and duplicated value word are not
read by the core. The owner is only consumed in the catch and is ignored by
the destructor. Those dead words are not new Source parameters or allocator
callbacks. Parent-stack addresses and register/spill identity are outside
the new explicit-context C++ interface.

Handler `00C60100` loads FuncInfo `00D86F80` and jumps to the existing
`__CxxFrameHandler3` implementation at `00BF6B43`. Live metadata has magic
`19930522`, maxState2 and EHFlags1. The two unwind records at `00D86F5C`
have state-1 and zero actions. The single try block at `00D86F6C` covers
state0, catchHigh1, one handler. The record at `00D86F4C` has
adjectives40h/type0/disp0 and catch address `0044CE6D`.

Source expresses this with `catch (...)` and `throw;` in the current MSVC
`/EHsc` C++ exception domain. It destroys only completed elements, ascending;
the failed current slot is excluded. If cleanup throws in Source, that new
exception escapes immediately and later elements remain untouched. This is
not proof of original FH3 catchability, SEH handling, nested/double-exception
identity, binary unwind tables or native calling convention compatibility.

Incoming calls were also inspected: append `00450589` supplies count1,
while insert-count has dispatcher calls at `0044F555` and `0044F62C`.
These are call-interface evidence, not completion claims for their callers.
All seven reported native CALL instructions are checked by
`tools/verify_report_calls.py`.

The Win32 Release build and all three existing test suites pass; CTest took
7.21 seconds. All seven native CALL rows pass the live call-site verifier.
Validation and limits are recorded in
`reports/oct15_raw_string_uninitialized_fill.json`. No new native
differential or exception-injection fixture is added. Existing native math
seeds exercise their existing routines, not this packet. Source/build checks
do not establish original ABI, original-game runtime or gameplay behavior.
