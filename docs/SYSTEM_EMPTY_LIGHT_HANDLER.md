# Empty first-light handler and continuation

The empty list at `00B46EEF..00B46EF3` calls the CRT helper once. A returning
installed handler continues at `00B46EF8`; an unconditional host failure result
does not reproduce that path. The integrator's corrected lighting writer was
compiled directly and passed the focused comparison described below.

This is a read-only review of the configured `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Each live query used
`python tools/bsp.py ghidra`, whose client verifies the project, program,
language and image base before querying. The configured project file exists.
Eleven bounded ranges match the installed PE byte-for-byte. Exact ranges,
hashes, bytes, and nine function name/comment preimages are recorded in
`reports/system_empty_light_handler_review.json`. No Ghidra annotations or
shared ledgers were changed.

## Handler dispatch and initialization

`00BF6713..00BF6722` has no incoming arguments. It clears EAX, pushes EAX five
times, calls `00BF66EF`, adds `14h` to ESP and executes `RET`. Its behavior is
consistent with a CRT invalid-parameter-no-information wrapper; this descriptive
identification is a hypothesis, not a recovered symbol.

`00BF66EF..00BF6712` is a five-stack-argument cdecl dispatcher. It decodes the
global slot at `0109DD64` through `00C04FDE`. If nonnull, it restores EBP and
tail-jumps to the decoded pointer at `00BF6703`. The callback therefore receives
the original five arguments; a normal cdecl return reaches the wrapper's
cleanup and return. No result is tested by the wrapper or lighting caller.

The global's bounded live xref query reports one reader (`00BF66F2`) and one
writer (`00BF65B5`). The writer is the three-instruction function `00BF65B1`;
its only reported caller is `__init_pointers` at `00BFBDFB`. That initializer
calls `__encoded_null` at `00C04FD5`, retains its result in ESI, and passes the
same encoded null to `00BF65B1` at `00BFBE10`. The encoding helper `00C04F67`
uses an available `EncodePointer` function, otherwise returns its argument.
The matching decoding helper `00C04FDE` uses `DecodePointer`, otherwise returns
its argument. Both first consult cached TLS state, then a KERNEL32 lookup.

Thus startup supplies a decoded-null handler slot. Static xrefs do not prove
that an external component can never modify it. This review does not claim
that a nonnull handler is installed during ordinary gameplay.

## Default termination path

For a decoded null, `00BF66EF` calls `00C04EF3` with stack value `2`, then
tail-jumps to `__invoke_watson` at `00BF65BB`. The actual `00C04EF3` body is
`AND dword ptr [0109EEA8],0; RET`; it ignores the pushed argument.

Watson builds an exception record with code `C000000D`, queries
`IsDebuggerPresent`, clears the unhandled exception filter, invokes
`UnhandledExceptionFilter`, conditionally calls `00C04EF3(2)` again, and then
calls `TerminateProcess(GetCurrentProcess(), C000000D)` at `00BF669B`.
Successful current-process termination prevents continuation. The physical
cookie-check epilogue and `RET` at `00BF66B6` still exist if that termination
API returns. Ghidra marks Watson `noreturn`; that annotation cannot justify
marking the dispatcher or wrapper unconditionally non-returning.

This review did not execute Watson, modify an installed handler slot, or call
any native process-termination path.

## Caller capture and reload order

The native sequence is:

| Address | Effect |
| --- | --- |
| `00B46EDB` | Get lighting from the original scene; retain result in ESI. |
| `00B46EEA` | Read sentinel from retained lighting `+1C` into EAX. |
| `00B46EED` | Read first from sentinel `+00` into EBX. |
| `00B46EEF..00B46EF3` | If first equals sentinel, call `00BF6713` once. |
| `00B46EF8` | Read retained camera `+198` and compare with mode 3. |
| `00B46EFF` | Read retained lighting `+10` into ECX (environment). |
| `00B46F02` | Read retained first node `+08` into EBX (light). |
| `00B46F05` | Save the environment for later ambient-cube calls. |

On the empty path the retained first node is the original sentinel. A normal
callback preserves native ESI, EBX and EDI. It can change the fields loaded
afterward, but the caller does not get the scene lighting again, read the
sentinel slot again, follow `next` again, or recheck emptiness. Appending a
node does not select that node. Replacing scene lighting or the sentinel slot
also does not change the retained owners. The retained sentinel's `+08`
field must provide the light used by the continuation; native code has no
null-light fallback. Retained objects must survive a returning callback.

## Minimal C++ correction and existing infrastructure

Before proposing an adapter, the review searched the current reconstructed
`src/` and `include/` for CRT invalid-parameter handling and callbacks. No
general implementation of this dispatcher, encoded slot or Watson was found.
`TextInputQueue` explicitly returns false on an empty queue. `front_state_request`
relies on a documented nonempty precondition; its header's blanket claim that
`00BF6713` is non-returning is too strong. Existing uses of `std::abort` implement
other host preconditions and do not establish this native behavior.

The reviewed integrator correction is the small
`SystemInvalidParameterRuntime::invalid_parameter_noinfo_00bf6713(std::string&)`
interface in `include/bsp/system_lighting_constants.hpp`. The lighting writer
accepts an optional runtime pointer and requires it only for `first==sentinel`.
An unbound runtime returns the explicit host status
`invalid_parameter_runtime_unbound`. A false adapter result means the host
could not perform the actual runtime operation and returns `callback_failed`.
Neither status describes a native empty-list outcome. A successful returned
operation continues with captured lighting and first/sentinel pointers and
reads camera mode, environment and light afterward.

The eventual concrete adapter must execute the actual configured runtime
behavior: an installed callback receives the five zeros and may return;
the default path retains its termination behavior. Do not insert an always
successful callback, a no-op handler, an unconditional abort, an empty-list
skip, a list retry, or a fabricated sentinel/light owner. The small interface
does not reconstruct the complete CRT or native object lifecycle.

## Focused verification

One ignored local fixture, `local/system_empty_light_compare.cpp`, executes
the installed bytes for selection `00B46ED1..00B46F08`, scene getter `00B72110`,
wrapper `00BF6713`, and dispatcher `00BF66EF`. It adds only a return at the
capture boundary, relocates the calls, binds the dispatcher slot to a local
encoded callback, and adapts the pointer decoder through Win32 `DecodePointer`.
The null/default branch is not entered. The report lists every patch location.

The one callback changes mode 1 to 3, the retained lighting's environment,
and the original sentinel's light; it also appends a distinct node, replaces
the sentinel slot and replaces the scene's lighting slot. Distinct marker
objects make every selection distinguishable. The native run reports exactly
one callback with five zero arguments, retains original lighting, reloads the
updated environment and original sentinel's light, observes mode 3, and
selects neither appended nor replacement nodes.

The same fixture directly compiles the integrator's current
`src/system_lighting_constants.cpp`, constructs equivalent live field-reference
projections, and performs the same callback mutations. The native captured
selection chooses expected raw words. The corrected writer matches `c43`,
`c44`, `c45`, and `c52.xyz` byte-for-byte, calls its runtime once, and returns
the legitimate `shadow_absent` result after writing lighting. Both compilation
and execution pass with MSVC Win32 `/std:c++17 /W4 /WX /O2 /fp:strict`.

This proves the selected continuation and the corrected projection behavior
for this focused fixture. It is not execution of the entire native lighting
prefix, default Watson, the original game, or a drop-in ABI replacement. The
runtime adapter, real owner bindings and normal game validation remain outside
this review.
