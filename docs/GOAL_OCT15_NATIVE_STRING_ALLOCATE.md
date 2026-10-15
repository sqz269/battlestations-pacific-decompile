# Native string storage allocation helper

The ordinary Source body for `0041CB50` directly composes the existing actual
native pool getter and allocator. It uses `NativeStringRawPoolContext`'s shared
publication cells, without a private pool, semantic allocator or host callback.
The descriptive native name remains a hypothesis rather than a recovered symbol.

| Routine | Native range | Coverage | Original ABI | C++ source |
| --- | --- | --- | --- | --- |
| `BSP_NativeString_AllocateStorage_0041CB50` | `[0041CB50,0041CB67)` | complete | ECX length, EAX allocator block, RET | `src/native_string_allocate_storage.cpp` |

The listing captures ECX in callee-saved ESI, calls the getter, pushes the unread
word `1`, adds one to ESI as a DWORD, passes that size and the returned pool to
the allocator, restores ESI and returns. The allocator result stays in EAX.
The decompiler's `void` result and live undefined/no-argument prototype do not
represent this observed register contract. No original callers or references
were found by the current exported graph and live xref query; caller intent is
therefore not independently confirmed.

| Call site | Native callee | Established contract |
| --- | --- | --- |
| `0041CB53` | `00419CC0` | No native arguments; returns the shared actual pool. Existing implementation performs the native lazy publication/lifetime schedule. |
| `0041CB60` | `00BD1120` | ECX actual pool, stack `(size, unread word)`, RET8; returns block in EAX. Existing implementation uses exact small classes or nullable CRT allocation at size >=150. |

Both callees' bodies and their actual C++ provider contracts were inspected.
The source obtains the pool before evaluating `length+1u`, preserving the native
order and DWORD wrap. In particular, `UINT32_MAX` requests native class zero,
and a large allocation still resolves the getter first. This helper writes no
header or terminator and adds no validation, cleanup, allocation-failure
translation or fallback buffer. Exceptions from the existing getter escape.
The pool's existing real publication/lifetime preconditions remain required.

Live analysis used the configured `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe`; the CLI verifies project name, program path,
language and image base before queries. Full original bytes were checked
against the installed PE. Existing Ghidra name/comment values are preserved in
the report before appending the new evidence through the annotation write lock.

Validation and limits are recorded in
`reports/goal_oct15_native_string_allocate.json`. The C++ API adds a raw context
argument and is not a drop-in native ABI replacement. This packet establishes
ordinary Source reconstruction; required Win32 build checking is pending in
the initial revision and will be recorded in the report. It claims no original
byte differential fixture, game execution or gameplay validation.
