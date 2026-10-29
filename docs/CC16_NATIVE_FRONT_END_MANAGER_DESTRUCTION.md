# Raw front-end manager teardown parent

`destroy_native_front_end_managers_004da650` supplies the complete normal parent schedule using actual Game storage, four borrowed volatile publication cells and the same real Game lifetime scalar-dispatch service. The whole pause helper remains an explicit required binding. The existing semantic `frontend_managers.cpp` API and callers are unchanged and are not promoted to raw storage.

Base: `e64249b004a989f162a1b7b9cda3493606535e88`. Only the new header/source and these evidence files are changed. No CMake, caller wiring, storage layouts, guards, ledgers or Ghidra annotations are edited.

## Recovered receiver and body size

Independent retail-PE disassembly and the saved listing count **107 bytes /38 instructions** at `004DA650..004DA6BA`, correcting the preliminary 39-instruction count. Body SHA-256 is `de86dcf6b8c97dce481c962b085cefd813f36b124f81bc03fc449c5e563d6070`.

The original begins `PUSH ESI; XOR ESI,ESI; PUSH ESI; CALL004D95F0`. None of those preceding instructions changes incoming `ECX`. Callee `004D95F0` explicitly copies `ECX` to `ESI`, reads Game `+61Fh/+620h/+1FE4h` and consumes the stacked argument with `RET4`. Therefore the saved no-input annotation is incomplete: incoming `ECX` is the actual Game receiver. The new C++ parameter is `NativeGameStorage&`, the existing actual `71A0h` storage type. Raw OnDestroy already supplies that same Game to its required `call_004da650(game)` binding.

This is an explicit C++ interface, not original register/stack ABI, a recovered EAX-result contract, FH/SEH or game validation. The original parent has no stack inputs of its own and returns with plain `RET`.

## Required real suppliers

| Input | Required authority |
| --- | --- |
| Actual Game | Same live `NativeGameStorage` passed by raw OnDestroy, valid throughout the pause call. No `GamePauseState` reinterpretation. |
| `required_calls.call_004d95f0(game,0)` | Whole actual raw 140-instruction pause helper, including its real Game fields/dependencies. Pure virtual; no default success, reset-stack approximation or implementation is supplied here. |
| `E198AC/E198B4/E198B8/E198C4` references | Same genuine writable volatile cells used by the actual owners. No private globals, copied publication snapshots or semantic-manager fields are manufactured. |
| `lifetime_calls` | Same actual `NativeGameLifetimeContext::calls` instance used by Game lifetime/OnDestroy. Its real `virtual_scalar(captured,0,1)` must dispatch the captured receiver's current native-table byte slot zero with flags one, using matching tables/heaps. |

The context borrows all six references and owns nothing. Caller storage, references, Game, tables, heaps and providers must remain valid through the entire serialized call. These are genuine-provider preconditions, not authentication of arbitrary manually assembled contexts. Required bindings remain separate root admission work.

## Native publication epochs

The parent first invokes the whole pause helper with actual Game and stacked zero, before reading any manager publication. It then runs these rows in order:

| Cell | Capture | Scalar call | Post-return clear |
| --- | --- | --- | --- |
| `00E198AC` | `004DA659` | `004DA669` | `004DA66B` |
| `00E198B4` | `004DA671` | `004DA681` | `004DA683` |
| `00E198B8` | `004DA689` | `004DA699` | `004DA69B` |
| `00E198C4` | `004DA6A1` | `004DA6B1` | `004DA6B3` |

Each row reads its volatile cell once. A null capture skips both call and clear. A nonnull capture is passed to the real scalar service with native byte offset `0` and flags `1`; after normal return, the **current cell** is cleared unconditionally, even if the callback replaced its pointer. The incoming captured object is not dereferenced again after it may have been deleted. Later rows are freshly read after preceding callbacks.

A pause exception naturally stops before any parent manager read. A scalar exception leaves its reached effects, skips that row's post-call clear and skips later rows. No catch/finalizer clear, pointer-equality repair, rollback, replay/terminal protocol or fallback callback is added. Callback publication replacement is preserved; arbitrary concurrent mutation or parent reentrancy is not supported.

## Existing projection and verification boundary

The old semantic routine invokes projected main/multi/options destructors and merely clears `set.in_mission`; its reset-interface-stack host call is not the whole raw pause routine. That code, its API and its call sites are preserved byte-for-byte in source. It does not supply this raw parent or the missing whole `004D95F0` binding.

Strict MSVC Win32 object compilation passed `/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX /c`. Emitted inspection confirms pause first, all four fresh captures, zero/one scalar arguments, and four success-path-only zero stores. The C++ `NativeGameLifetimeCalls` dispatch uses a compiler base adjustment/host vtable slot; that is distinct from the **payload slot-zero argument**, which remains zero. There is no numeric native-code pointer call in the source.

The direct pause target and four indirect transfer sites are checked against original evidence; indirect targets still depend on real runtime tables/providers. No new test, fixture, probe, game, DLL call or full build was run. The root owns CMake/shared OnDestroy binding, actual cell/provider admission and runtime validation after independent review. The pause child, actual dispatch/deletion bodies, enclosing lifecycle/failure handling, original ABI/FH/SEH and activation remain unclosed. No duplicate native/vendor-body, ABI, activation or gameplay credit is added.

[Implementation report](../reports/cc16_native_front_end_manager_destruction.json).
