# Session messages 107 through 112: integrated verification

Addresses: 007618E0, 00761950, 00761960, 00761990, 007619B0, 00761A20, 00761A30, 00761A60, 00761A80, 00761AF0, 00761B00, 00761B30, 00761B50, 00761BC0, 00761BD0, 00761C00, 00761C20, 00761C80, 00761C90, 00761CB0, 00761CD0, 00761D30, 00761D90, 00761DF0, 00761E10, 00761E20, 00761EC0, 00762640.

Three isolated worker packets were reviewed and integrated through `agent/cc10`.
The tested combined source revision is `23612ead4913641371f6222c13a11678e3325498`.
It was published to `main`; subsequent unrelated reconstruction commits may advance
that branch. The [verification receipt](../reports/native_session_messages107112_cc10_integration.json)
records exact source, library, fixture and export hashes.

| Packet | Complete bodies | Native bytes | New original/source pairs | Cumulative fixture pairs |
|---|---:|---:|---:|---:|
| [107/108](NATIVE_SESSION_MESSAGES_107_108_CC10.md) | 8 | 354 | 836 | 50,763 |
| [109/110](NATIVE_SESSION_MESSAGES_109_110_CC10.md) | 8 | 354 | 836 | 50,763 |
| [111/112](NATIVE_SESSION_MESSAGES_111_112_CC10.md) | 12 | 805 | 6,596 | 56,523 |

The batch reconstructs 28 normal routine bodies, 1,513 native bytes and six
five-slot profiles. Types 107 through 110 reuse the established scalar codec and
its exact source adapter addresses. Type111 adds two boolean fields and six
2-bit DWORDs after its type/sender header. Type112 has a WORD at offset1C and
three numeric floats with separately loaded borrowed scales and ordered x87
transfers. Their allocation sizes and accepted type sets are preserved.

## Verification

All three local fixtures were copied without modifying historical evidence and
rerun against the same combined `bsp_core.lib`. Their original/source observation
files are byte-identical. The **8,268 new pairs compare 2,897,740 bytes**; cumulative
counts overlap and must not be summed. The five inherited source-only cleanup
fault checks provide no new native exception evidence.

Strict MSVC Win32 builds passed. Workers initially had two configured CTests.
After eight seed-byte comparisons matched the installed image, the integration
reconfiguration enabled `native_math_differential`; all three CTests passed on
each integrated packet. The final call check passed 32 rows: 30 owned calls and
two analyzed external constructor callers. The independent 111/112 review
confirmed layouts, predicates, codec widths, x87 ordering and fixture hashes.

The primary defined 14 missing routines under temporary address leases and the
Ghidra write lock, with exact live/disk bytes recorded in the three definition
reports. All six scalar cleanup bodies retain their return paths with zero
instruction gaps. Integration saved 28 reviewed names/evidence comments,
preserved prior comments (including the existing engine-jam constructor name),
and refreshed all 28 affected exports. Workers performed no Ghidra mutations.

The receipt also verifies that the source, headers, docs and reports match their
worker Git blobs, allowing checkout newline conversion in JSON reports, and that
each source has exactly one CMake registration. Ignored worker evidence archives
were copied and integrity-checked before worktree retirement.

## Evidence boundary

These are source interfaces with borrowed provider contexts. The fixtures do not
establish full binary ABI compatibility, original CRT/FH3 behavior, malformed
input handling, allocation failure, arbitrary aliasing, concurrency, unmasked FP
exception ordering, networking or gameplay equivalence.

The six factory constructor calls still lack stored Ghidra ownership. The receipt
records their dispatch cells, allocation sizes and raw instruction bytes without
inventing an enclosing function. Full factory reconstruction remains open.
Entries 113 onward require a fresh profile, boundary and lease review before
dispatch.
