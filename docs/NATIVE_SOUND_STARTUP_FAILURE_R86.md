# Sound startup failure investigation and current application runs

Addresses: 00a823f0

## Result

R85's application startup failure remains unexplained. Its current and sealed
R84 executables both returned1 at the host guard `FMOD bank raw-length output
unavailable`, before window creation. Their logs did not preserve the preceding
FMOD result codes. R86 does not claim to have fixed that failure.

A local instrumented application subsequently loaded the same fallback bank
successfully. The unchanged R85 executable also succeeded before any tracked
source edit. Six further instrumented runs succeeded. These results establish
that failure is not deterministic under the inspected runs; they do not identify
whether device state, timing, memory layout or another input caused it.

The tracked change adds context to the existing host guard: resource path,
byte length, requested mode, CreateSound result, GetLength result and whether a
bank pointer was returned. On startup exceptions the application now logs the
existing journal's nonzero FMOD results, including earlier initialization calls
and any recorded recovery calls, then rethrows. Reporting performs no additional
FMOD calls. It adds no retry, output substitution, sound-disable fallback or
new native branch.

## Native and runtime evidence

The complete A823F0 body spans [A823F0,A82718), 808 bytes. The collector verifies
that body and the six-byte C2DEC0/C2DEBA import thunks against live Ghidra and
the installed PE: 820 bytes total. The create/length prefix was reviewed in the
listing; the unchanged arithmetic/setter tail was byte-checked, not newly
reconstructed or machine-executed by this packet.

A823FA..A82424 zeros6Ch create-info bytes, writes cbsize6Ch and gives its+4
length field to the VFS whole-file call. The default2D mode is A4Ah. A82492
calls CreateSound; A824E5 asks GetLength for RAWBYTES=8. After each, native code
checks only result2Bh for the memory-statistics path. A824FF reads the stack
output and stores resource+28 even after other errors. The source's existing
host guard rejects an unavailable output instead of reading an indeterminate
C++ local; the new message preserves that already documented boundary.

The instrumented source object was linked with current game objects and core
libraries into an isolated local executable with an embedded manifest. It
records the same source FMOD calls and dumps the input bank. It is diagnostic
source execution, not an original A823F0 differential test. Captured input:

- Resource `sound/gui/error.fsb`, 2,688 bytes, FSB4 signature.
- Create-info size108 bytes, mode A4Ah, nonnull system and returned bank.
- CreateSound and GetLength return0 on all seven captured bank traces.
- Installed fmodex.dll and fmod_event.dll report file version4.18.4; their
  hashes and the bank hash are recorded. The original installation was read
  only; traces and settings are under this worktree's ignored local directory.

The first instrumented run also passed, but its relative diagnostic path did
not create a bank log after the application changed directory. The rebuilt
diagnostic uses absolute paths. Only the seven captured traces support the
input/result claims above. The diagnostic initially writes the source guard's
generic message; it was built before the tracked reporting change.

## Current build verification

The final strict MSVC Win32 build and all three existing CTests pass. No new
repository tests were added. After the reporting change, the actual bsp_game
executable ran both existing finite scenarios with isolated settings:

| Run | Exit | Window/device | Presents | Native startup skips |
| --- | --- | --- | --- | --- |
| 2 frames | 0 | 800x600, success | 1 | 1 |
| 8 frames, press start on1 | 0 | 800x600, success | 7 | 1 |

Both report131 FMOD calls and zero FMOD errors at sound startup. The shared raw
singleton drain completes, renderer publications/queue are null, and its worker
is joined. Source hashes and exact executable/library artifacts are sealed
before integration. Prior failed R85/R84 receipts remain preserved separately.

The eight-frame screenshot was inspected: the main-menu frame and header are
visible, while the central panel is empty. This is not a complete interactive
menu or gameplay validation. Native frame commands remain empty; full B107F0
resource startup, post/material construction and the camera tail remain open.
No audible playback, error recurrence after the new diagnostics, native entry
ABI/FH3/SEH, or complete game parity is established by these finite runs.

## Follow-up

Use the retained result codes if the sound failure recurs. Proceed with the
full post/material/camera runtime and B107F0 startup binding; keep the earlier
failure and incomplete visible menu as open evidence limits.
