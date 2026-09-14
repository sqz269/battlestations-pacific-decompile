# Record-valued sampler parameter B1B890

`set_native_sampler_parameter_record_00b1b890` covers the complete normal body
`00B1B890..00B1B97B` (236 bytes), with an explicit source C++ projection of its
cleanup states. Original ECX is the complete receiver; its actual Lua owner
starts at receiver+4. The two public stack words are the actual NativeString
key-header pointer and a pointer to four adjacent binary32 values; `RET8`
consumes them. The source adds caller-retained operation diagnostics in EDX
and places the two actual14h locals in that operation. It is not a drop-in ABI.
The descriptive sampler-parameter name is a caller-based hypothesis.

The globals and result locals retain their caller-supplied preimages until the
existing providers initialize them. Result precedes globals by exactly14h;
the same globals storage is reused after the first explicit destruction.
B67980's returned object reaches B67580, and the second B67980's returned
object reaches B68100. The latter's returned pointer is ignored: subsequent
numeric writes and cleanup use the actual result local.

A naked entry adapter passes addresses of the original public argument slots.
The key pointer is captured once after the first globals construction. The
record pointer is captured once after the second globals destructor returns.
Each component is read only after the preceding write returns. A small source
helper performs actual `FLD32/FSTP32` before each concrete B66580 call with
index1,2,3,4. No four-value snapshot, guessed record class, semantic table,
virtual callback, or synthetic tracking update is introduced. The one observed
caller, `004DF5B7` in `004DE610`, constructs `WaterTracerColor` through0041E870
and passes current `game+5FC` plusC44 and the singleton at00F8D434.

The native state sequence is -1,0,-1,1,3,-1. State0 covers new-table creation;
state1 covers tracked result acquisition; state3 covers the second globals
destructor and all four numeric writes. The original map also contains state2,
which would clean result and then globals, but the normal body never publishes
it. The source does not invent that transition.

| Failure site | Source cleanup matching the armed native state |
| --- | --- |
| New-table writer | Current globals once |
| Result acquisition | Current globals once; no invented partial-result rollback |
| Second globals destructor | Current result once; globals destruction is not repeated |
| Numeric writes | Current result once |
| First globals destructor or final result destructor | Already disarmed; no repeated destruction |

On a source C++ exception that reaches the parent catch, the matching current
local is destroyed through B67700. A second exception during this noexcept
unwind cleanup terminates. Explicit-destructor failures retain their unresolved
diagnostic obligations. Failed operations must be acknowledged only after
obligations are resolved; acknowledgement performs no resource cleanup. Replay,
destruction of a failed unacknowledged frame, and unresolved obligations are
rejected. Valid fresh operation storage must remain stable throughout the call
and diagnostic lifetime; callers must not alter its bookkeeping during a call.

All providers are existing actual-storage implementations: B67980/B67700 in
`native_lua_objects`, B67580 in `native_lua_field_setters`, B68100 through the
concrete B67800 tracked-object path, and B66580 in `native_lua_numeric_element`.
The source requires a valid actual NativeString object and a source-compatible
Lua5.1.1 owner/VM. B68100 uses C-string semantics, ignoring header length;
B67580's null-data fallback has its documented restricted header domain. This
packet does not expand those provider contracts or construct an owner.

The retained DM evidence pins the installed/live parent, three cleanup
funclets, handler/table, complete incoming argument preparation and literal.
The machine-readable report contains every original call and all four cleanup
or handler tail transfers. The handler definition at `CBC808..CBC811` was
missing during DM and was subsequently created and saved centrally by the
primary agent; its record is pinned separately. No analysis metadata was changed
by this worker.

One full strict MSVC Win32 Release build passed, with its one configured
`reconstructed_math` CTest. This fresh worktree lacked `local/seed_reference.hpp`,
so the native differential target was not configured. All eight seed spans
verified; the primary agent owns the planned seeded combined build and its two
CTest cases. All17 original call/tail rows passed the read-only verifier.

Generated code retains the late key and record-slot dereferences and all four
ordered component calls. The helper emits one `FLD32/FSTP32`, then raw32-bit
`MOVSS` transfers into the child's stack argument, with no second conversion.
Exact object, listing, source and log hashes are retained in the report and
`local/do-generated-order.json`. No new tests or parent fixture are added.
Compilation, generated-order inspection and existing checks do not establish
native FH3/SEH, hardware-fault handling, Lua C longjmp,
full private Lua register ABI, unrestricted exceptions/aliasing, or gameplay.
The parent source does not convert longjmp into C++ exceptions or undo earlier
Lua table writes. The numeric child's prior focused x87 fixture is dependency
evidence, not a runtime test of this whole parent.

Primary integration validated exact source `1532eca3eec2255741bc3deea207bd6cb38a496b` with Win32 Release and both existing CTests (2621 unchanged tracked inputs). All six full generated sections and normalized relocations match the reviewed worker;32 specific ordering bytes and15 concrete source-call relocations are checked. DQ source/provider cleanup pins remain unchanged. One local source-Lua fixture constructs the actual owner, uses an explicit linked VM and actual NativeString with CRT fixture storage, and writes one four-component record (negative zero, min-subnormal, signalling NaN,1.25). Exact stored doubles, x87 flags0003/CW037F, current result cleanup/tracking, retained high-water/stale reference cell, caller key, record, stack sentinel, receiver prefix and local padding all pass. No original parent/FH3/SEH, source failure/longjmp, native string pool or game runtime is executed. The existing worker1-CTest limitation is resolved by this exact root2-CTest build.
