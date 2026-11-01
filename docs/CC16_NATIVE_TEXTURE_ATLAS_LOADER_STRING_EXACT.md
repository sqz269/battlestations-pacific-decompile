# Exact raw atlas-loader string closure

This source-only proposal reconstructs the whole ordinary **normal** bodies of
00469840 (substring), 00425F40 (header assignment), and 004CAD40 (substring
replacement). Names are descriptive hypotheses. It preserves their real raw8
headers, reached DWORD argument homes, allocating-helper order and current
field epochs. It does not edit or replace the older adapters.

| Original entry | Native interface | Original listing |
| --- | --- | --- |
| 00469840 | ECX source; output/start/count stack; EAX output; RET0C | 95 instructions, 272 bytes |
| 00425F40 | ECX destination; source stack; EAX destination; RET4 | 24 instructions, 53 bytes |
| 004CAD40 | ECX value; search/replacement/count stack; RET0C | 165 instructions, 463 bytes |

The three complete listings were read through `bsp.py show --asm`. Their complete
live Ghidra bytes were captured through `bsp.py ghidra bytes`, then compared to
the installed PE by the ignored seal script. Project defaults and the live
program were rechecked as `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
No analysis, label, comment, flow or saved-project mutation was made.

`substring_native_atlas_string_header_00469840_exact` consumes the initial
start/count homes in original order. Negative start adds to count with DWORD
wrap and clamps start to zero; nonnegative start clamps to the then-current
source length. Source data is captured before temporary allocation. A positive
count uses the **existing whole concrete** BF9280 Source provider. This preserves
its reached BYTE/DWORD windows, padding, overlap and self-induced source-change
domain. No new vendor body or vendor reconstruction credit is claimed. The
output HOME is read after that operation, as at 004698D2. Exact426060 constructs
that reached output; only its normal return arms the output flag. A separately
captured temporary data pointer is returned with the current temporary length
plus one. The empty arm clears the reached output and performs its actual
zero-length exact resize, unless the original header-identity branch skips it.

`assign_native_atlas_string_header_00425f40_exact` preserves header identity.
For different headers, the captured requested length reaches exact41DD40 with
an initialized real preserve BYTE HOME. Afterward, current source length gates
the current destination length, source data and destination data loads. Genuine
BF7680 is invoked even when the reached copied count is zero. The canonical
live feature-word reference is transported without a preload or bool facade.

`replace_native_atlas_string_substrings_004cad40_exact` reads initial search
data before value data and calls the existing whole BF9440 Source provider.
Only a first match reads/captures the replacement argument HOME. Each iteration
reloads the search HOME, constructs suffix then prefix, performs two exact
concats, and assigns the joined result with the original **inline** load order:
destination length, destination data, joined data. This differs from425F40 and
is deliberately not replaced by it. The joined, first-concat, prefix and suffix
buffers are returned in original order. The next search uses captured
replacement's current length, value's current data, then current search HOME
and its current data. It clamps a negative wrapped start or checks the current
value length. It performs this search **before** decrementing the live count
HOME. Initial count zero therefore still replaces and then wraps on decrement.

All frames are fresh, nonmoving and retained. Per-iteration source metadata and
child frames are adopted before the first child effect. Native local header
storage is reused exactly as a loop-local native region; the metadata list is
history, not a projected string vector. The actual output/start/count and
search/replacement/count HOMEs are initialized ordinary DWORD backing borrowed
through volatile references. Original output-home rereads and count writes
remain visible. Source controls, helper backing, code and active private frames
must be disjoint from native payload except the explicit supported raw homes.

The proposal is normal-flow only. Escaping C++ exceptions record failure and
bare-throw; no selected FH3 cleanup, free, rollback, retry or output recovery is
invented. Retain every partial frame, allocation, header, pool/canonical owner
and provider graph through external disposition. The existing BF7680 CPU/DF,
feature-cell, lifetime, overlap and reached-access domain and BF9280/BF9440
domains remain requirements. New Source interfaces and private histories do not
prove native registers, ABI, private-stack aliases, FH3/SEH, fault-PC, NoThrow,
Origin, binary drop-in behavior or gameplay.

No optional tests, compilation, build, runtime, game, install, CMake, ledger,
Ghidra annotation, tracked source mutation, commit or push was performed by this
packet. Root owns independent review, exact joined source admission and builds.

