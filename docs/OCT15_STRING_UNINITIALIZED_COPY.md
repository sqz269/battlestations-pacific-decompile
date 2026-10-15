# Native pooled-string uninitialized copy

This packet reconstructs the complete ordinary Source loop at `0044D0C0`, its
catch body at `0044D10A`, the stacked-header storage destructor `00431F40`,
and dispatcher `00505AD0`. It composes the existing actual pooled-string
services. The older `Seg2Host` destructor remains a separate historical record.
The descriptive Source names are hypotheses; existing Ghidra library names
are retained. Original binary and original FH3 exception compatibility are
not established by these new C++ interfaces.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete cached listings were read, and live
`bsp.py ghidra` queries verified the program, incoming edges, catch body and
exception metadata. Source is `src/native_string_uninitialized_copy.cpp`;
the public contract is `include/bsp/native_string_uninitialized_copy.hpp`.

| Entry | Original interface and recovered behavior |
|---|---|
| `00431F40` | ECX unused, stacked actual8h string header, RET4. Capture data first; null skips length/getter/return. Otherwise capture length+1 with DWORD wrap, call real pool getter, then return captured block. Header remains untouched. |
| `0044D0C0` | ECX first, EDX last, stacked destination/owner/dead duplicate/dead tag, RET10h; EAX completed destination. Capture endpoints, construct ascending8h headers and advance completed end only after each successful constructor. |
| `0044D10A` | Catch funclet using parent EBP frame; captured destination begin and completed end. Destroy completed headers ascending, excluding failed current slot, then rethrow. Not an independently callable ordinary Source entry. |
| `00505AD0` | ECX owner, stacked first/last/destination, RET0Ch. Forward exact consumed range/output/owner values and retain core EAX. Low tag byte is zeroed; high bytes and duplicate metadata are ignored by the core. |

The core uses `construct_native_render_alias_string_0044bcb0` with its existing
`NativeStringRawPoolContext` overload. That service initializes the actual
destination header itself, preserves native self-construction/null behavior,
and has a verified no-op placement-delete leaf. No caller initialization,
failed-slot destruction or invented rollback is added here. Captured range
addresses advance with unsigned DWORD arithmetic, avoiding C++ cross-object
pointer subtraction or iteration. Memory accesses must satisfy the existing
constructor/destructor contracts; no general allocation or validation layer
is introduced. Observable source/destination aliases are not rejected.

The new `00431F40` Source wrapper reuses the complete raw-pool `0041DD20`
primitive after verifying its identical capture/getter/return schedule.
Original `00431F40` takes its header on the stack instead of in ECX and ignores
its owner ECX. Both use the current actual `00419CC0` publication cells,
manager and `00BD1510` return service. No host allocator callback is introduced.
The explicit Source context and ordinary C++ signatures differ from native ABI.

Exception evidence was read live: handler `00C60150` loads FuncInfo
`00D87064` (magic `19930522`, maxState2); both unwind actions at `00D87040`
are zero. The single try block at `00D87050` covers state0, catchHigh1, one
handler. The handler record `00D87030` has adjectives40h/type0/disp0 and points
to `0044D10A`. The native catch calls `__CxxThrowException(0,0)` after its
ascending destruction loop. Source uses `catch (...)`/`throw;` in the current
MSVC `/EHsc` domain. A cleanup exception escapes immediately in Source;
remaining elements are not processed. This is an explicit Source-domain
behavior, not proof of original FH3 nested/double-exception or SEH identity.
No new broad exception adapter or synthetic original exception is supplied.

All four incoming core call sites were inspected: two dispatchers and two
insert-count invocations. The latter pass a vector owner plus separate dead
count/tag metadata, demonstrating why these words must not become a fake
allocator or constructor callback. The remaining vector assignment still
needs its other actual-storage dependencies and is not marked complete.

Checked local call sites:

```text
00431F54 CALL 00419CC0
00431F5B CALL 00BD1510
0044D0FA CALL 0044BCB0
0044D11A CALL 00431F40
0044D12A CALL 00BF6885
00505AEC CALL 0044D0C0
0044F18C CALL 0044D0C0
0044F542 CALL 0044D0C0
0044F570 CALL 0044D0C0
```

Validation: Win32 Release build and all three existing suites pass. The native
math differential suite covers its existing seeds, not this string packet.
No new native differential, exception-injection fixture, native ABI or gameplay
proof is claimed. Current-CRT exception behavior is expressed in Source and
supported by original control-flow/metadata evidence, with the limits above.
