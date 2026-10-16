# Native CRT substring search, 00BF9440

The correct Original library names remain `_strstr` and `_strchr`. The new,
default potentially throwing interface is
`void* __cdecl bsp::find_native_crt_substring_00bf9440(const void*, const void*)`.
It covers the whole ordinary contract of the 134-byte, 66-instruction Original
range 00BF9440–00BF94C5, SHA-256
`0aa74e3eadfb78899c55fa44b65c6aa4fd0cc653ad09eff41df06903fb03faa9`.
The C++ name is descriptive. Source credit remains zero pending Root and
independent review and the separate registered Main gates. Existing provider
Source/header and its additional credit remain unchanged/zero.

## Ordered behavior

Read needle BYTE0 once. If it is zero, return the supplied haystack, including
null, without reading needle BYTE1 or haystack payload and without advancing a
payload cursor. Otherwise read and cache needle BYTE1 once.

For a one-byte needle, construct a genuine, initialized, four-aligned eight-byte
private object containing two actual unsigned 32-bit words: the exact haystack
pointer bits followed by the zero-extended first needle byte. Pass that object's
actual address to the unchanged ordinary
`bsp::find_native_crt_character_00bf86f0(const void*)` interface. Keep the object
live, stable and disjoint from all reached payload and private controls until
the ordinary call returns. The provider reads its binding BYTE at +4 before its
binding DWORD at +0. Passing Original homes or the needle pointer would bind the
wrong bytes. This bridge is neither a Native register adapter nor a redirect to
the Native normal entry.

The Original one-byte path restores its saves and entry ESP, prepares full EAX
as the zero-extended first byte and transfers to interior instruction 00BF86F6.
That interior entry consumes EAX and the haystack home; ECX, EDX and the second
home are not service inputs. The preceding six bytes of the normal `_strchr`
entry are skipped. No new function or gap decoding is introduced.

For longer needles, read one source BYTE and advance by one before comparing
the cached first needle byte. Equality precedes the source-NUL test. After a
first-byte match, read the next source BYTE and advance by one before comparing
the cached second byte. A mismatch retries that same captured BYTE against the
first byte; it does not reread or skip the candidate. A zero captured on this
path fails the first-byte comparison and stops at the subsequent NUL test.

When both cached bytes match, preserve the genuine candidate pointer `q` and
the genuine restart pointer `q + 1`. They do not move with extension scanning.
Reset the needle extension cursor to the supplied needle on each attempt:

1. Read needle at current needle cursor +2. Zero succeeds before any paired
   source read and returns `q`.
2. Read source at the current source cursor, then advance that cursor by two
   before comparing, including on mismatch or a following needle NUL.
3. If equal, read needle at the old needle cursor +3. Zero succeeds before the
   partner source read and returns `q`.
4. Otherwise read source at the advanced source cursor -1, then advance the
   needle cursor by two before comparing. Equality repeats extension; either
   mismatch restarts at the fixed `q + 1`.

There is no post-success payload read. An extension mismatch may reread earlier
source bytes when the next candidate is examined, as the Original does.

## Unchanged provider

The accepted provider comprises 00BF86E0–00BF86E4 and 00BF86F0–00BF87AD:
195 bytes and 89 instructions, concatenated SHA-256
`22f27502570ab4aa937590c5aebf8c378db90379f04e561945d1db27bff36e8e`.
Its alignment peel reads a BYTE at the actual cursor, advances one, compares
the target before NUL, and never rounds the address down.

At actual four-byte alignment, read a complete DWORD `W`. In unsigned modulo
32-bit arithmetic, `D = W ^ repeated_target`, `T = W + 0x7efefeff`,
`U = D + 0x7efefeff`, and `C = (~D ^ U) & 0x81010100`. Advance the actual
cursor by four before the stopping decision. Nonzero `C` performs one genuine
same-address DWORD reread, then tests captured lanes low to high, equality
before NUL, without BYTE payload rereads. With zero `C`, use
`Z = (~W ^ T) & 0x81010100`: zero `Z` continues; nonzero `Z & 0x01010100`
returns null; otherwise a set high bit of `T` continues and a clear high bit
returns null. The high-bit compensation, both masks and conditional reread are
retained. A provider DWORD can include later in-range bytes beyond a match or
NUL; those actual full-width windows must exist.

## Ordinary raw-storage domain

Target MSVC Win32, little-endian BYTE8/pointer32/unsigned32 and ordinary flat
DS/SS storage. Every reached BYTE, full DWORD and reread must be in genuine,
live, determinate readable storage. The actual trace must stop finitely.
Raw unaligned and interior addresses, read-only inputs, high-bit bytes,
haystack/needle overlap, identical inputs and ordinary caller-stack storage
elsewhere are admitted. There is no signed/INT_MAX length cap, nonoverlap,
snapshot, full unused terminator extent, blanket readable padding, invented
typed payload object, ownership or writable-input requirement.

Every actual cursor and effective-address step of one, two or four bytes must
stay nonwrapping in 32 bits and within or one past the same genuine allocation.
No one-past address is read. This includes provider terminal +1/+4, extension
source +2 on a failed comparison or following needle NUL, needle +2 and reached
partner offsets. The private terminal geometry is not waived when a compiler
eliminates or reschedules dead arithmetic. Unused partner geometry does not
require unused partner readability. The Original uses modulo arithmetic; this
interface admits its stated nonwrapping ordinary subset.

Exclude aliases between payload/bindings and Original or new callee private
frames, argument homes, saves, return addresses, controls, spills, scratch and
helper backing, as well as self-modifying control. Ordinary caller stack
elsewhere and input/input aliases remain admitted. Faults or partial progress,
MMIO, external/concurrent mutation, asynchronous interruption protocols,
lifetime/mapping changes and exotic segment protocols are outside this claim.
Results are the genuine borrowed matching address or null; returning `void*`
does not transfer ownership, lifetime or permission to modify read-only storage.

## Actual strict objects

One HostX86/x86 MSVC 19.51.36244.0 process compiled the new caller and the
byte-identical existing provider together. The actual invocation used
`/std:c++17 /O2 /W4 /WX /permissive- /fp:strict /EHsc /MD /c /Bv
/showIncludes /sourceDependencies`, separate output directories and distinct
TU basenames. CL and _CL_ were absent. The actual caller closure has three
files; the provider closure has ten, including its existing `<cstdint>` chain.
All actual files were genuinely before-pinned. Stdout is 1,437 bytes; stderr is
1,100 bytes of `/Bv` version information; exit is zero with no diagnostic.

The caller object is 1,127 bytes, SHA-256
`6e3dbd66ed388601d1fd3c8ed26f08761ba7619e5217b1913d654d65946c7ae4`:
five sections, nine primary/five auxiliary symbols, one 207-byte code section
with 88 decoded instructions and no emitted private function. Its one REL32
provider reference names exactly
`?find_native_crt_character_00bf86f0@bsp@@YAPAXPBX@Z`, defined once by the
reviewed companion. Its other relocation is FPO metadata.

For caller entry ESP `S`, the eight-byte backing is at `S-8` and `S-4`.
Instructions 0x28–0x33 copy the full haystack DWORD and store a MOVZX-derived
target DWORD. At 0x37–0x3c the actual address is passed to the ordinary provider;
the block remains live across that call, and 0x41 removes the four-byte
argument. Its layout and four-byte alignment follow the actual DWORD stores,
the eight-byte object assertions and the ordinary four-aligned MSVC stack ABI.
No original home layout or physical tail return is claimed.

The caller's longer path retains cached first/second bytes, same-captured-BYTE
retry at 0x7c–0x80, fixed candidate in a private slot, and fixed restart in ESI.
Source extension BYTE load 0x94 precedes source +2 at 0x96 and comparison 0x99.
The partner read 0xa4 uses the old source cursor +1, equivalent to the advanced
cursor -1, only after the second needle-NUL guard. The compiler hoists its
private needle +2 computation/update to 0x87/0x8e; the reached needle +2 read
already requires that address, and no extra payload or unused partner extent
is introduced. Source retains the explicit post-partner update. This is an
ordinary dataflow proof, not physical private-arithmetic ordering identity.

The companion object is 2,044 bytes, SHA-256
`af009472134141bc9ee8f8b17963a5bb16e87ef907e9945ad83d0b97d7c443f1`:
13 sections, 20 primary/13 auxiliary symbols, five code sections totalling
213 bytes and 89 instructions. The public body is 189 bytes/79 instructions;
the four emitted private helpers total 24 bytes/ten instructions. No code call,
code relocation or undefined external exists in the companion. Its five
relocations are FPO references to its own definitions.

The companion captures the binding pointer, then reads target BYTE+4 and
haystack DWORD+0 in that order. It subsequently reuses its by-value argument
home's low byte as the target spill; that private home is outside the admitted
alias domain. The public body retains peel +1, word +4, same-word reread at
0x62 only on a nonzero candidate, bounded register lane shifts 0/8/16/24 and
equality-before-zero. The emitted `address` and `pointer` helpers return the
full argument bits in EAX; `read_word` returns a fully defined DWORD in EAX.
The emitted `read_byte` helper defines AL only; EAX high bits retain address
bits and are not a widened-byte result. Public inlined sites are reviewed
separately and zero-extend the sought byte where needed.

All code, padding NOPs, section headers/data, symbols and auxiliary slots,
string tables, relocations, C13 records, FPO and raw `.chks64` data were
inspected. Every object byte is accounted for, with no unclaimed gap or trailer.
Each C13 stream contains S_OBJNAME and S_COMPILE3, no file-checksum/line
subsection; opaque compiler checksums are recorded without treating them as
Source authentication. `/MD` contributes MSVCRT/OLDNAMES default-library
directives, not an unresolved runtime service. Neither object contains a GS
cookie, runtime/provider helper, table, global/TLS state, SSE/string instruction,
DF use/change, ES override or segment-based payload operation.

Callee saves and all plain-RET exits restore the actual stack. Caller saves
ESI/EBP only on the longer path; EBP is a general cursor register, not a frame
chain. Actual FPO records and stack/home reuse are retained. These objects use
the ordinary compiler calling environment, not a proof of Native caller entry
conditions; selected code is DF/ES-independent. Future Main `/Oy-`, host,
selected provider object and archive bindings require separate actual review.
No physical Original flags/registers/frame/homes/EH/fault/caller/runtime/game
identity follows from Source or this static compiler evidence.

## Evidence and remaining gates

The authoritative published gate is
`reports/cc12_native_crt_strstr_readiness.json`, identical in all twelve clauses
to the scoped packet and accepted Root proposal. Its author/peer evidence stays
external. The new self-only task family is
`local/WholeNativeCrtStrstrSource01`; its post-commit manifest and authentication
record the actual new execution and outward-coordination pairs, own Original
UUID/full-prefix bounds, exact input stages, compiler receipt, both dependency
closures, complete objects and preservation. Earlier compiler or transport
proof is not adopted. Two genuine schema-display AttributeErrors and capped
preparation displays remain in this task's raw history; corrected bounded
displays supplied the complete data. They were not compiler failures.

The required unmodified report/doc call checker is pending the integrator. Its
built-in Client.verify queries live project/program metadata and its suffix
checks query function metadata even with zero call rows. The Sol integrator
explicitly deferred it to a separately pinned integration phase to retain this
author packet's no-live-Ghidra boundary. No author checker invocation, stdout,
PASS or count is fabricated. Zero call rows would not prove semantics or a
Native tail-entry ABI. No tests, probes, SDK comparison, selected body execution,
live Ghidra, worker baseline, Main build or publication occurred.
Root/independent Source review, actual registered normal Main build, three
existing CTest suites, all actual caller/selected-provider/private-helper
objects, both Core archive indices and unique literal bindings remain pending.
An unchanged Main provider object may be retained from an earlier compile;
its actual before snapshot, current Source identity, complete bytes/ABI and
archive selection must be proved without calling it freshly compiled.
