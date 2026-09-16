# Renderer destruction and shared shutdown dispatch (R64)

Addresses: `00B32920`, `00B339F0`, `00B32900`, `00B5DF00`, `00B5DF70`,
`00B5BB20`, `00B25290`, `00B27CF0`, `00B5FD40`, `00B5FDA0`, `00B5FE40`,
`00BD30E0`, `00BD0400`.

## Result

The full 4,257-byte renderer destructor, its deleting entries, raw system-registry
terminals and query terminals are recovered into current source. The shared raw
manager now dispatches the renderer secondary and the state/system registry
profiles through their genuine deleting functions and the same application cells.
The renderer binding is appended at offset 128; previous binding offsets remain.

The renderer destructor is statically reviewed and build-tested. Registry/query
terminals and derived registry manager dispatch have focused runtime evidence.
Full renderer shutdown and production startup binding remain unvalidated.

## Native behavior retained

- `B32920` joins the control worker, resets bindings, releases query/cache/surface
  and retained owners, deletes current registries, performs the native COM call
  sequence, then destroys members and both bases. Its 29-state cleanup map and
  original actions were checked against live Ghidra and the installed executable.
- Use the genuine `0041CC80` owned-section release for `+199C`, paired with the
  constructor's actual allocator. Use current `actual_records` effect cleanup.
- Device `+1A10` and default color surface `+197C` must be initialized. Device and
  factory must stay callable across unconditional releases and later reference
  pairs. A constructor-only renderer cannot satisfy this input contract.
- `B339F0` frees only after returning destruction and flags bit zero. `B32900`
  adjusts the registered secondary by minus `0C`. `B32F10` is an interior address
  of `B32920`, not another destructor entry.
- System cleanup uses the current raw string/manager publications. Query cleanup
  releases captured COM, clears the current field, reloads renderer publication,
  removes the raw pointer, and restores the reference base. `B25290` is byte-for-byte
  identical to the existing `B25300` pointer removal body.
- Manager dispatch passes the popped owner even if its publication changed.
  Reverse registration can remove child registries before renderer destruction;
  the renderer observes their current null publications.

Non-null model fields require canonical raw-name model bindings. The writer/type
of `+19E0` remains unproved. Original private frames, FH3/SEH and binary ABI parity
are not established by these new source interfaces.

## Verification

Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three existing CTests pass.
No new repository tests were added. Fresh evidence covers 69 original spans,
6,210 bytes, all 13 complete function listings, and the parent exception map.

Seven focused checks pass in the current terminal probe:

- Full copied native and source query chains: scalar flags zero, current-profile
  flags one, base stamp, null invocation, guarded/unguarded pointer removal,
  untouched fields, stale array slots and balanced actual locks.
- The non-null COM branch uses an owned reference to a real D3D9 factory through
  its `IUnknown::Release` contract. It does not test a created GPU query.
- Native/source raw system destruction of real 52-row registries, including
  derived/base deleting entries, current publication clearing and stale fields.
- Source canonical manager drain of real 62-entry shader-state and 52-entry
  system registries with the application's VFS, Lua, raw strings and type services.

The fixture uses the full source renderer constructor and genuine idle control
worker. It manually tears down the constructor-only renderer with established
providers before the final manager drain. This does not execute `B32920`.
The linked image audit captures 40 source intervals and 29 physically resolved
I386 modules. Map intervals are not exact native function boundaries.

## Device runtime attempt and next step

The current active Windows session is Remote Desktop. D3D9 identifies the RTX
5090, but HAL `GetDeviceCaps` and device creation return `D3DERR_NOTAVAILABLE`.
The earlier successful gather fixture now fails device creation too. The session
and GPU configuration were left unchanged. Failed setup runs are retained apart
from the successful terminal evidence; early constructor-only cleanup attempts
faulted before a valid full-destructor fixture could be established.

When HAL device creation is available, finish the prepared real device/default
surface fixture, validate full renderer shutdown and its manager dispatch, then
connect the complete startup/shutdown graph to the production application. The
native unconditional-release sequence needs explicit external COM references
in the focused fixture. Active frame, model/resource and gameplay proof remain.

Evidence: [report](../reports/native_renderer_destruction_r64.json). Only bounded
files were recovered from the three prior branches; no unrelated ancestry merged.
The report records source hashes, fresh native bytes, prior annotations and
immutable local build/probe archives.
