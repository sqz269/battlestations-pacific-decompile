# Raw stream text scanner (R161)

## Result and scope

`native_stream_text_scanner.hpp/.cpp` reconstruct the normal bodies of eight
entries over the actual 828h scanner storage. This scanner reads one byte at a
time from the current stream. Its layout and ownership differ from the 838h
scene/options tokenizer recovered in R158/R159.

| Entry | Bytes | Native contract |
| --- | ---: | --- |
| 00BEE800 | 36 | ECX owner; copy current token to previous; clear token cache |
| 00BEE840 | 106 | ECX owner; return current/next byte in AL |
| 00BEE8C0 | 24 | ECX owner; move byte to previous, clear cache, tail-call byte peek |
| 00BEE8E0 | 860 | ECX owner; return token pointer in EAX |
| 00BEEDB0 | 345 | ECX owner; skip whitespace and publish token EOF |
| 00BEF020 | 133 | ECX owner, stack success-byte pointer; EAX token; RET 4 |
| 00BEF220 | 177 | ECX owner; release delimiters, stream and pooled filename |
| 00BEF2E0 | 410 | ECX owner, by-value 8h filename and extra delimiters; EAX owner; RET 0Ch |

The existing `sound_sample.cpp` destructor interface remains available. This
packet adds its composition with the actual raw string pool and VFS dispatch.
The source interfaces explicitly carry contexts and retained operations; they
are not binary-compatible replacements for the native entries.

## Storage and ownership

Quoted state is at +0. Current/previous 400h token buffers start at +1/+401.
Token cache is +801, previous/current bytes are +802/+803, byte cache is +804,
byte EOF is +805, and token-start EOF is +806. The delimiter pointer is +814,
zero-based LF count +818, actual pooled filename header +81C/+820, and stream
pointer +824. There is no whole-file allocation or read cursor in this owner.

Construction zeroes the owner, clones the argument filename through the actual
pool, then opens the current VFS manager's current slot +4 with mode 32h.
Only after that callback does it read and release the old +824 stream, allowing
the callback's publication to be observed. The returned stream is adopted
without another retain. A null extra-delimiter argument borrows the current
default; a nonnull argument allocates and appends to the reloaded default.
Finally, construction consumes the separately owned argument filename header.
That header is left dangling and must not be destroyed again by the caller.

Destruction compares against the current default delimiter, frees an owned
delimiter, atomically decrements the actual stream count, dispatches the current
slot +0 at zero, clears +824, and returns the filename to the actual raw pool.
The filename header is left dead. Escaping source callbacks retain the owner,
argument identity and progress in an operation that rejects replay and requires
explicit diagnostic cleanup. This is not the native FH3 unwind implementation.

## Byte and token schedule

Each uncached read reloads the current +824 stream and its current slot +24.
It requests one byte directly into +803. A zero actual count marks byte EOF;
otherwise the byte is cached and a LF increments +818. Byte peek initializes
the actual-count local with incoming owner bits, matching `PUSH ECX`.

Assembly shows four uninitialized private count slots in token peek and three
in whitespace recovery. `NativeStreamTextStackPreimages` supplies each initial
value explicitly. The source retains each slot's reuse within its invocation.
Normal admitted VFS reads overwrite the count; the fixture also covers a
callback that leaves it untouched. This models the value dependency, not native
private-stack placement or aliasing.

Whitespace includes comma and NUL through `strchr`. Comments are recognized at
token boundaries only; line comments, block comments and overlapping `/*/`
retain the observed read schedule. Quotes are stripped; backslashes remain
literal. Accept copies the token and clears only token cache. Token EOF follows
the native delayed snapshot of byte EOF. String read rejects an empty quoted
token and preserves the success-output alias behavior and recovery schedule.
As in the native body, inputs exceeding the 1023-byte token payload are outside
the valid buffer domain.

`NativeStreamTextScannerVfsCalls` delegates to the existing
`NativeVfsRuntimeBindings` raw manager, read and zero-reference entry dispatch.
It uses the same physical, retained-memory, adopted and inflated stream services
as the application. The ordinary language-catalog loader is not yet bound to
this source scanner. The process owner for E15334/E15338 remains a future
integration dependency; this interface borrows explicit pointer cells.

## Evidence and validation

The configured `C:/Users/sqz269/bsp.gpr` / `/battlestationspacific.exe` was
verified. Thirteen live spans match the installed PE: 2091 code bytes and
79 data bytes. The report records 34 direct CALLs and one external tail JMP.
The flow audit found five unreachable alignment gaps after JMPs and no missing
call continuation. No listing repair was needed.

Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three existing CTests
pass. A focused fixture compares copied native bodies with the new source:
nine paired cases, 195 observations and 849864 matching observed bytes. It
compares the full owner image with pointer identities normalized, live pooled
filename/delimiter contents, and service traces. Both lanes use the actual raw
pool and retained-memory stream services; the open and selected read callbacks
are controlled adapters. The cases include comments, empty quotes, replacement
during open/read, embedded NUL/high bytes, the 1023-byte boundary, unterminated
comments, EOF, aliased success output, null open and delimiter cleanup. A
source-only escaping open verifies retained ownership and replay rejection.

The copied bodies' seven private count preimages are seeded by narrow entry
wrappers; source calls receive the same values. Relocated direct edges use
explicit helper adapters and the host CRT. This does not prove original CRT
internals, original private stack/FH3/SEH, asynchronous store visibility,
malformed/overflowing tokens, whole-application admission or gameplay parity.
The source's two EOF byte stores are not claimed to preserve hardware fault
boundaries. See `reports/native_stream_text_scanner_r161.json` for exact hashes,
saved Ghidra annotations, archives and any mounted-file diagnostic result.
