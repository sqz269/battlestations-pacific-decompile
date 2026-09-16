# Native diagnostic application binding R41

## Scope

R41 binds the already reconstructed native diagnostic owner to the current
`GameStartupHost` application path. It adds no owner, publication, manager, or
deletion rule. Both calls borrow the one loader-zero diagnostic publication in
`GameSingletonHost` and the `SoundLifetimeAccess` view of that host's actual raw
singleton-manager cell.

The implementation changes only `src/game_hosts.cpp`. The raw getter and
shutdown bodies remain in `src/native_diagnostic_sink_lifetime.cpp`; their
component lifecycle, unregister schedule, profile dispatch, and shared-drain
fallback were established before this packet.

## Original application calls

The original initialization window is:

| Site | Operation |
| --- | --- |
| `0073E13C` | call the complete font and GUI startup at `0073BAE0` |
| `0073E141` | load the `After InitGui` trace literal |
| `0073E146` | call the retail-stubbed trace at `004C9C90` |
| `0073E14B` | call the native diagnostic getter `004C14C0`; discard its result |
| `0073E150` | begin the following initialization phase |

The 25-byte installed-PE span at `0073E13C` is:

```text
E8 9F D9 FF FF B9 A8 F1 CF 00 E8 45 BB D8 FF
E8 70 33 D8 FF 68 A0 71 00 00
```

Its SHA-256 is
`5c9036dd29332075c5213de7be410a1c89f135be1dc3e850b91e02419cf53980`.
Fresh guarded Ghidra bytes matched the installed PE completely.

The getter has 24 direct call instructions in 12 original functions.
`0073E14B` is the sole direct application-initialize call. Other sites are
runtime diagnostic statements or buffer-lock precondition paths. An indirect
error path could demand the lazy owner earlier; in that case the explicit
application call is intentionally warm and does not register a duplicate.

The original shutdown neighborhood is:

| Step | Site | Operation |
| --- | --- | --- |
| 18 | `0073830A` | destroy registered singleton `00F8BF44` through `00736300` |
| 19 | `0073830F` | destroy diagnostic publication `0109CF14` through `007363B0` |
| 20 | `00738314` | finalize and free global block `01090900` through `00BBC5D0` |

The 15-byte installed-PE span at `0073830A` is:

```text
E8 F1 DF FF FF E8 9C E0 FF FF E8 B7 42 48 00
```

Its SHA-256 is
`9ff33f4c6e767bc4831674e3b5b530d188a39090bfc1e420c5964f35995d941e`.
Fresh guarded Ghidra bytes matched the installed PE completely. `0073830F` is
the only direct caller of `007363B0`.

## Source binding

`GameStartupHost::run_initialize_phases` now calls
`native_diagnostic_sink_get_or_create_004c14c0` immediately after
`run_font_and_gui_startup_0073bae0` returns and before source-only summary reads
or the following reconstructed phase. The omitted original checkpoint is a
bare retail trace stub, so it carries no state that needs another source
provider. The getter result remains discarded, and source does not assert that
allocation returned a nonnull owner.

`GameStartupHost::application_shutdown` now calls
`destroy_native_diagnostic_sink_007363b0` after its current menu, frontend, and
device teardown, before `application_destruct` and before
`destroy_singleton_lifetime_manager` performs the shared raw drain. The
existing `ApplicationShutdownHost::singleton_teardown` marker remains
unimplemented.

This placement establishes the recovered application-shutdown phase and keeps
the diagnostic after the current GUI/frontend teardown. It does not claim that
the current host implements original steps 1 through 18 or 20 through 42, or
that the source call is adjacent to reconstructed providers for original steps
18 and 20.

The two calls use exactly:

```cpp
singletons_->diagnostic_publication_0109cf14()
singletons_->sound_lifetime()
```

The first accessor returns the canonical stable `0109CF14` cell by reference.
The second borrows the same `01090AA0` cell used by all existing raw singleton
registrations and by the final host drain. No header, `GameSingletonHost`
layout, deletion table, or CMake change is needed.

The behavior-only `DiagnosticSinkSingleton` in `gui_startup` remains separate.
It has no native allocation, raw registration, canonical publication identity,
or drain role, and existing semantic callers and fixtures remain unchanged.

## Lifetime and failure behavior

- A cold application getter allocates and publishes the four-byte `CE752C`
  owner and registers the then-current publication. A warm call returns before
  resolving the manager and cannot register a duplicate.
- Allocation null remains an allowed null result. R41 adds no required-nonnull
  check.
- Registration or second-manager failure propagates. The getter keeps the
  published allocation and releases only its captured guard, matching the
  established source schedule. R41 adds no repair or retry.
- Normal explicit shutdown unregisters and scalar-deletes the owner. The later
  raw drain observes the retained null vector slot and cannot delete it twice.
- The shutdown wrapper's initial-null path creates no manager, so repeating the
  wrapper alone is harmless. The broader application initialization and
  shutdown methods are not claimed to be repeatable.
- If later initialization or earlier application shutdown fails while the
  owner is still registered, the existing `GameSingletonHost` destructor drain
  is the fallback. An owner retained after registration failure has no raw
  vector entry and remains outside that fallback.
- An unsupported current profile throws only after genuine unregister, keeps
  the prior mutation, and releases the captured section. R41 does not invent a
  fallback profile or swallow the exception.

## Validation

The current Win32 Release build regenerated `game_hosts.obj`, `bsp_core.lib`,
and `bsp_game.exe`. The `game_hosts.obj` receipt establishes:

- COFF machine `014C` (`x86`);
- `/DEFAULTLIB:MSVCRT`, the required `/MD` runtime family;
- unresolved typed references to both
  `native_diagnostic_sink_get_or_create_004c14c0` and
  `destroy_native_diagnostic_sink_007363b0`, resolved by the existing core
  library during the game link.

Native seed verification passed. The strict project build and all three current
CTest targets passed:

- `reconstructed_math`
- `native_math_differential`
- `tool_tests`

No new fixture was added. The prior genuine-host fixture already exercises the
getter, explicit shutdown wrapper, vector hole, physical diagnostic demand, and
raw-drain fallback. Repeating those component calls in another fixture would
not execute the new `GameStartupHost` placement. Full application execution is
still required to validate that path dynamically.

## Evidence boundary

R41 is source-compiled and link-tested for MSVC Win32 `/MD`. The exact native
call sites and bytes are PE/Ghidra verified. This does not establish original
no-argument ABI compatibility, FH3/SEH identity, the missing application
shutdown actions, full application execution, renderer/device behavior, or
gameplay.
