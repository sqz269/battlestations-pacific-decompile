# Complete raw render-command queue destruction (R71)

Addresses: 00b1f330, 00b1f6b0, 00bd0400, 00b1ebe0, 00b1d950,
00b1e7f0, 00b1cc80, 00b1f150, 00b1d590, 00b1c3c0.

The actual `34h` queue now has its complete ordinary destructor and scalar
deleter. The shared raw singleton dispatcher admits profile `00D5E5F4`, whose
slot zero is `00B1F6B0`, using a borrowed destruction context. This closes the
destruction dependency of the existing queue getter and constructor. The
application has not yet bound this queue context or native frame methods.

## Recovered schedule

`00B1F330..00B1F3B9` is 138 bytes: native ECX is the queue and the routine
returns with plain RET. It writes `00D5E5F4`, arms cleanup state 2, and calls
the full `00B1EBE0` executor. That executor can run queued commands and retire
their current owners. Replacing this call with a command deletion loop would
change native behavior.

After execution returns, the destructor sets state 1 **before** resizing the
current row array to zero and freeing its current data pointer. It then sets
state 0 **before** resizing the command-pointer array to zero and freeing its
current data pointer. It unconditionally clears `00F8D440`, even if that cell
does not identify the destroyed owner, then writes base profile `00CE3818`.
Array data pointers and capacities remain stale. There is no separate release
of queue context `+30`, manager unregister, or worker stop in this body.

The unwind map at `00DF5108` has three states and actions at `00CBCC20`,
`00CBCC28` and `00CBCC33`: base cleanup, command-header cleanup, then row-header
cleanup. Ordinary source C++ unwind follows the same armed states and invokes
the existing complete helpers. It does not retry execution or separately
destroy pointed commands. A second escaping C++ cleanup exception terminates.

`00B1F6B0..00B1F6CD` is 30 bytes: ECX queue, one flags stack word, EAX original
owner, RET4. It calls the ordinary destructor, frees the owner only for flags
bit zero, and returns the original pointer. An escaping destructor exception
does not reach that free. The source APIs add explicit context/frame bindings
and are not original ABI replacements.

## Ghidra repair and evidence

The saved ordinary body initially ended at `00B1F379`. Its first array free
hid the following 64 bytes. The locked flow repair decoded the tail and cleared
both free-call overrides, but did not extend the stored function body. After
archiving full documentation, the supported locked recreation tool restored
the complete `00B1F330..00B1F3B9` body. The name, formal prototype, parameters,
comments and labels matched before/after recreation. The decompiler changed
the generated stack-local name `pvStack_c` to `local_c`; both records survive.

A second repair restored the three-byte continuation after the scalar free
at `00B1F6C0`. Current listings contain 39 and 11 instructions, respectively,
with zero gaps. Both repair histories are committed separately. Fresh live
Ghidra bytes match the installed PE for 1,293 bytes covering the two bodies,
reached queue/command/member helpers, manager dispatch, profile and EH metadata.
The unrelated command-body padding at `00B1DA6D` is `LEA ECX,[ECX]`, skipped by
the preceding jump; it required no mutation.

## Validation

The strict MSVC Win32 build and all three existing CTests pass. The initial
source-registration line was corrected to use this repository's deferred CMake
registration before the successful build. No repository test was added.

A focused local differential probe executes all 138 original ordinary-body
bytes and all 30 scalar-body bytes, with the queue-publication operand relocated
and direct callees adapted to the existing complete source providers. It
compares those normal paths with the new source implementation using a
nonempty command queue, two actual row strings, and real raw pool storage.

The probe covers control 0 and 1, flags 0 and 1, old/incoming context references,
nonterminal batch references, scalar pointer return, and unconditional clearing
of a deliberately different publication. Flags-zero cases compare the entire
final `34h` owner, including preserved configuration bytes and stale array
fields. A separate actual raw manager drain exercises the new
`D5E5F4 -> B1F6B0` profile route with flags one and a nonempty queue. All pass.

The executor reaches the real `00B1D950` readiness-false branch because the
fixture renderer is inhibited. Batch/context references stay above zero, and
model/group terminal paths are not exercised; fixture resolvers reject an
unexpected terminal lookup. The retained command in control 1 is cleaned up
separately after observing the native destructor's retention behavior.

These are bounded original normal-body and source lifetime results. Original
FH3 handlers are not executed; source exception cleanup is assembly-reviewed,
not runtime-proved. No active rendering, material pass, application shutdown,
concurrent mutation, full renderer frame, original CRT/SEH ABI or gameplay
claim is made. Source interfaces require the same actual ownership domains
and prepared persistent frames whenever those native stages are reached.

## Next dependency

Compose the actual queue getter, stop pipeline, destructor and frame contexts
with the application's existing renderer and singleton manager. Active native
EndFrame also requires its real XLive import and valid online lifecycle, plus
substantive command/debug providers for nonempty work. The current application
still uses its D3D frame bridge. See
`reports/native_render_queue_destruction_r71.json` for call-site evidence,
native bytes, probe logs and scoped artifact receipts.
