# Native render batch keys and hardware comparison

This closes two small original bodies and the enabled index-zero key loop on
actual native storage. The queue singleton acquisition and final sort dispatch
in `B51DF0` remain separate. No game installation or live game was modified.

| Original | Reconstructed scope | Original ABI |
| --- | --- | --- |
| `BF7456..BF74CB` | Complete 117-byte x87 conversion helper | Input ST0; consumes one value; EDX:EAX result; RET |
| `B51AB0..B51AF5` | Complete 69-byte material/depth predicate | ECX/EDX entry pointers; AL predicate; RET |
| `B51E2E..B51ED4` within `B51DF0` | Enabled index-zero key loop | Original ESI batch, EDI zero; private parent frame; new C++ batch-reference interface |

Implementation: `native_render_batch_keys.hpp/.cpp`. The descriptive C++ names
are hypotheses. Existing semantic sorting and binary32 conversion interfaces
remain available, with their narrower domains. The previous complete-function
ledger entry for the semantic material predicate is preserved as a previous
interface rather than counted as another original function. The CRT helper's
existing `LIBCRT_unmatched_00bf7456` identity is retained.

## Hardware leaves

Both leaves use MSVC Win32 naked assembly. The fixture verifies all 186 emitted
bytes against the installed executable; there are no external calls in either
body. They preserve native x87 instructions and do not add a control-word reset,
finite-value rejection, SSE comparison, or a numeric conversion substitute.

`BF7456` duplicates ST0, stores a binary32 sign observation, converts under the
current rounding mode with FISTP, reloads the integer and corrects toward zero.
The zero-low-word path checks the integer high word; zero and integer-indefinite
take the original two-pop path. The public hardware entry requires an assembly
caller supplying ST0, even though its integer return uses the usual EDX:EAX.

`B51AB0` captures the left effect pointer, reads the right effect DWORD at B0,
then reads the left DWORD. Unequal values compare as signed32; only AL is
replaced, retaining the right DWORD's upper 24 bits. Equal values load right
depth followed by left depth, use FCOMIP, pop the remaining value and return
exactly zero or one. Under masked exceptions an unordered comparison returns
false. Original x87 status and exception behavior remain in the instruction
stream; unmasked exception delivery was not exercised by the fixture.

## Actual key loop

The new interface borrows the existing actual 18h batch header. Its pointer
cells and pointees are read as raw native storage, never as semantic entry,
section, material or effect objects. Required minimum touched extents are entry
28h, section 24h, material 80h, effect C1h and conditional texture 24h. These
minimum extents do not establish complete native class sizes.

Signed count <= 0 returns before touching the array. Each iteration reloads
the array, then reads entry+4 -> section+20 -> material, signed16 material+34,
material+7C -> effect and effect+B0. Only a positive material count reads the
texture slot at material+10, and only a nonnull slot reads texture+20. The
unsigned byte at effect+C0 is read after the first prefix multiplication.

```text
texture = signed16(material+34) > 0 && slot0 != null ? texture+20 : 0
prefix = (((effectB0 & 63) * 4096 + (texture & 4095)) * 256) + uint8(effectC0)
key = (uint64(prefix) << 37) + uint64(conversion_EAX)
```

The original second `__allmul` constant is high DWORD 20h, low DWORD zero,
which is 2^37. Depth is loaded late from entry+14 and passed to the real hardware
helper. Its EDX is discarded. The low key word is the helper EAX; the high word
is `prefix << 5`. The loop stores low at entry+20 before high at entry+24,
then increments its index and rereads signed batch count. Duplicate entry
pointers execute the reads and stores again. There is no allocation, reference
operation or rollback.

## Validation and limits

One private original-code fixture verified 132 extended-input conversion pairs
across three precision settings and four rounding modes, 60 comparator pairs,
and 12 actual-storage key-loop pairs: 1,236 normalized result/state words agree.
It checks EAX/EDX, x87 control/status/tag, signed and unordered comparisons,
unsigned effect bytes, negative depth zero extension, conditional unusable
texture slots, duplicate entry pointers and nonpositive count with an unusable
array. All runtime floating exceptions were masked. Both complete leaves match
the original machine bytes exactly, including their register/stack operations.

The original key-loop fixture enters at B51E2E and exits at B51ED4 with a private
frame trampoline. It runs the original BF7456 and `__allmul` bodies. It does not
stub or claim to execute queue acquisition, configuration or subsequent sorting.
The C++ loop has a new interface; the two hardware leaves expose their original
register contracts. No full batch virtual dispatch or gameplay validation is
claimed.

Strict MSVC Win32 build and both existing CTests pass. No tracked tests were
added. Primary review also reverified the discovery report's 17 code spans and
one table span against live Ghidra and the installed PE, and its 20 source blobs
at their recorded Git commit. Detailed hashes, saved annotations and closure
state are in `reports/native_render_batch_keys_audit.json`.
