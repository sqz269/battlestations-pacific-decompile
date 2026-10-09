# Pending registry raw8 constructor: 00874BC0

The 69-byte ordinary constructor is reconstructed in
`src/native_pending_registry_constructor.cpp` as an explicit C++ receiver and
publication-reference interface. It uses the existing real tracked-section
creator and admitted registry base cleanup. The normal MSVC Win32 build and
all three existing checks passed. Native entry/EH compatibility and gameplay
remain unproved.

## Ordered construction and Source failure policy

`construct_native_pending_registry_00874bc0(receiver, actual_publication)`
captures the raw8 receiver, then enters its Source C++ protected region:

1. Store numeric profile `D0DEA0` in the actual receiver's first DWORD.
2. Call `create_native_tracked_critical_section_00bd1860()` with no arguments.
3. After normal return, store its actual returned pointer at receiver +4,
   including a null return; return the captured receiver address.

There is no +4 preclear, generic base initialization, normal publication store,
owner allocation/free or registration. Both DWORDs are written on normal return;
the caller must supply actual writable, aligned raw8 storage. The volatile stores
preserve the observed store order without establishing thread safety.

A Source C++ exception in that protected region invokes the actual admitted
`cleanup_native_pending_registry_base_008748f0` with the receiver and same
borrowed publication reference, then rethrows. That concrete helper clears the
current cell without an identity check before the actual generic helper stamps
`CE3818`. It leaves +4 untouched. Callee mutations are not rolled back, no section
or owner free is added, and no earlier publication is restored. If cleanup itself
throws, ordinary Source C++ nested-exception behavior applies.

The accepted native constructor arms state zero before its profile store and
section call. Its encoded action `C963E0` loads the current receiver spill and
tail-transfers to `8748F0`. The Source catch implements an explicit C++ failure
policy using that established target. It does not reproduce the original
helper-frame relation, mutable spill aliases, native dispatch or fault domain.

## Actual providers and full compiled review

The [primary report](../reports/cc12_native_pending_registry_constructor_primary_review.json)
retains the whole root, handler, catch tail, EH table, providers, physical COFF
relocations, unique positive Core definitions, member identity and source/build
pins. Complete objects and build artifacts are also retained under ignored
`local/cc12_registry_constructor_primary/`.

| Selected Source function | Complete bytes / instructions |
| --- | --- |
| New constructor | 85 / 31 |
| Actual tracked-section creator | 39 / 16 |
| Its same-object private allocator | 58 / 20 |
| Actual singleton allocator | 90 / 37 |
| Actual pending registry base cleanup | 25 / 9 |
| Actual generic base profile helper | 14 / 6 |

The constructor's protected-state store is at offset 44, before profile store
51 and the sole ordinary section call 57, whose REL32 operand is at 58. Returned
EAX is stored to +4 at 62; ESI is copied to EAX at 65. Full review reaches the
plain Source `RET`. No constructor call to a generic base was invented.

The separate Source catch contains 23 code bytes followed by six `CC` padding
bytes: it reloads the actual Source reference/receiver arguments from EBP+0C
and EBP+08, calls the concrete base helper through operand 7, then supplies
zero/zero to the current CRT rethrow through operand 19. The 29-byte compiler
handler and complete 88-byte EH table are reviewed. Physical table links reach
the actual catch; its single try block covers state zero and has an ellipsis
descriptor. This is a compiler catch route, not a replica of the native unwind
action. The generated project uses synchronous C++ exception handling.

The whole section creator obtains 1Ch through its physical same-object private
allocator, calls imported `InitializeCriticalSection` when nonnull, then zeros
depth +18h. Its private helper forwards the real `{critical_section,1Ch,1Ch}`
request to the actual singleton allocator. That allocator uses host `malloc`,
the current `_callnewh`, retry and Source `std::bad_alloc` transport. No cleanup
is added if OS initialization exits nonlocally. The constructor adds no projected
section, default context or fabricated provider.

All five selected complete object payloads occur exactly once in `bsp_core.lib`.
Five selected public definitions are unique and positive; the private allocator
is proved by physical same-object symbol index/section rather than a public
name. All selected REL32 call operands and their actual symbol records are
checked. The six selected functions total 311 bytes / 119 instructions.

## Validation, admission and remaining boundaries

Normal build: `pwsh -NoProfile -File scripts/build.ps1`,
2026-10-09 09:30:49.793613 through 09:31:06.441380 UTC, exit zero.
`reconstructed_math`, `native_math_differential` and `tool_tests` all passed.
No new tests or probes were added. The existing unrelated duplicate Lua
`spawn_request_id_matches` linker warning remains.

The complete accepted 69-byte native span was rechecked against the unchanged
installed PE. Saved Ghidra name/comments are updated through the sanctioned
write-lock annotation tool, preserving old values and comments in its receipt;
the project is saved and the affected export refreshed. No bytes, function
membership, prototype or no-return property is changed. Admission records one
reconstructed Original function / 69 Original bytes, with ABI/gameplay false.

The new ordinary two-argument C++ interface is not the original ECX-only entry.
Original FH3 frame-spill delivery, exception identity, hardware faults/SEH,
runtime dispatch, alias and nested-failure behavior remain qualified. A real
allocator/new-handler C++ exception can enter the Source catch, but no forced
live failure or original execution is demonstrated here.

`D0DEA0` and `CE3818` are numeric Native identities, not callable Source vtables.
This entry produces no canonical publication cell, getter or manager registration.
The borrowed cell must be genuine and survive all users. Process-cell/deletion
binding composition is a separate packet. The game map does not select this new
root, so this build does not establish constructor execution, startup or gameplay.
