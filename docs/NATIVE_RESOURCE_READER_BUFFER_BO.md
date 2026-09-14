# Actual optional reader buffer BO

This packet reconstructs `BF05D0..BF06F9` (298 bytes) and
`BF0700..BF077C` (125 bytes) against actual storage and the existing raw
string-pool context. Both take ECX=header, one signed DWORD stack argument,
and RET4, with no stable result contract. The C++ interfaces are new APIs.
All 423 bytes match fresh live Ghidra bytes and the installed PE. The reserve
body includes a recovered ten-byte gap; no Ghidra repair was performed.

## Established layout and producers

The 10h reader base is stream at +0, then a 0Ch header at +4: data pointer,
signed count and signed capacity. `BF09B0` adds four to its captured reader
before calling `BF0700(0)` at BF0A07, then frees current buffer data. The
independent header destructor `BF0980` also calls `BF0700(0)`, then free.

Each element is exactly 0Ch bytes: native string length/data at +0/+4 and an
opaque DWORD at +8. `BF0810` proves the stride and source-entry construction:
when full, it reserves doubled capacity (minimum one), calls `BF0580` at
the current data+12*count, then increments current count. `BF0580` zeroes
the first two words, copies the native string unless source equals target,
and finally copies current source+8. These immediate producer bodies establish
storage; the opaque word's broader meaning is not inferred. No projected
vector or independently owned element type is introduced.

## Reserve and resize order

`BF05D0` clamps signed requested capacity below one to one and returns if
current signed capacity suffices. Otherwise it allocates the wrapping DWORD
product 12*capacity through `BF55BE -> BF681B`, represented by the shared
singleton allocation service with equal native and host byte counts. It does
not clamp multiplication or reconcile capacity with current count.

For each index below **current** signed count, reserve computes the new entry
with DWORD wrapping. A zero destination skips construction. Otherwise it
captures the source entry from current old data, zeroes destination string
words, resizes through actual `41DD40` with preserve=true, then reloads source
length and both string data headers before copying destination length bytes.
Finally it reads and copies current source+8. The source entry address remains
captured across pool calls; the buffer data/count are re-read for subsequent
iterations. Source equality still zeroes the two string words and copies +8.

The new block is unpublished during copying. Afterward reserve returns old
strings **forward**, consulting current data and count each iteration and
capturing each string's data and wrapping length+1 before the pool getter.
It frees **current** old data through `BF6989 -> BF65AC`, then publishes the
new data pointer and requested capacity. It never writes count.

`BF0700` reserves when requested count exceeds signed capacity. Growth
captures its initial count/difference, reloads data each iteration and zeroes
only entry+0/+4; +8 remains untouched. Shrink proceeds **backward**: decrement
current count in the actual header, compute the last entry from re-read
count/data, and return its string. Each return resolves the current raw pool;
the next loop comparison re-reads count. Final normal completion writes the
requested count. Capacity, removed string headers and opaque words remain.
Negative count is not normalized; callers must provide valid backing for the
same wrapping accesses that the native body performs.

Both functions use `NativeStringRawPoolContext`, including actual publication,
shutdown-gate and raw-manager cells. Every nonnull return resolves the getter,
including disabled small returns and large blocks. Existing actual-header
resize/destruction bodies preserve capture order and let getter failures
escape; a `NativeStringStorage::release noexcept` adapter is not substituted.
The BF7680 memory operation uses overlap-safe copying, like the existing raw
string helper, and omits a zero-byte call whose memory effect is empty.

## Exact EH and listing limits

BF05D0 installs handler `CC77A7 -> BF6B43` with FuncInfo `E023E0` (36 bytes):
magic19930522, maxState1, unwind mapE023D8 and no try blocks. Its only map row
is state0 -> -1, action `CC7790..CC77A6`. Let P be native entry ESP: allocated
base is captured at P-18h, index at P-14h and destination at P-10h. State0 is
armed around each entry construction, then reset to -1. The funclet passes
captured destination and base+12*captured index to `401130`, which is a bare
RET. It destroys no string and frees neither a completed entry nor the new
array. No catch map, partial-copy rollback or replacement-owner guard exists.
Old-string returns/free occur in state -1. BF0700 has no local EH frame.

Allocation failure before copying leaves the header unchanged unless the
allocation service itself changes it. A later raw-pool failure leaves all
completed effects intact: the unpublished new block and copied strings have
no cleanup in this leaf; a failure returning old strings can leave some old
strings already returned. Shrink has already decremented count before a
failing return. The source adds no recovery or native unwind emulation.

Read-only evidence identifies these exact repair candidates for the primary:

- BF06DD..BF06E6: five instructions missing from BF05D0's listing after free
  at BF06D8: `ADD ESP,4; MOV [EBX],EDI; POP EDI; MOV [EBX+8],ESI; POP EBP`.
  Clear the stale call-return override if present, decode these ten bytes and
  verify membership through the existing RET4 at BF06F7..BF06F9.
- CC77A7..CC77B0: decoded from matching bytes as FuncInfo load and EH dispatch
  jump; live proto reports no owning function. CC7790..CC77A6 already has
  correct `Unwind@00cc7790` ownership and needs no ownership repair.
- Dependency only: BF0980 currently ends at BF0991. BF0992..BF0996 contains
  `ADD ESP,4; POP ESI; RET`; BF0997 is padding. Recover that five-byte tail
  when implementing its destructor; this packet does not implement it.

## Validation and boundaries

All analysis/export batches used verified `bsp.py ghidra` access to
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 Win32 and base
00400000. The report separates direct calls, tail jumps, indirect dependency
calls and decoded-only EH dispatch. The latter has no invented function owner.
Whole-report verification passed 20 rows (18 direct calls and two thunk
jumps), with zero failures. It explicitly skipped the two indirect destructor
dependency calls; the unowned EH jump is separately classified.

Strict MSVC Win32 compilation passed with `/std:c++17 /EHsc /MD /W4 /WX
/fp:strict /permissive-`. One ignored fixture uses actual 0Ch headers/entries
and a real constructed 8AD4A0h raw pool. It checks string/opaque-word copying,
forward and reverse return order through the pool's most-recent-first reuse,
growth preserving +8, shrink retaining old string words, signed reserve clamp,
DWORD allocation wrap and deterministic pre-copy allocation failure. Failure
inside entry-copy or old-string pool return was not induced; those cleanup
contracts are established from the listing/FH3 evidence, not fixture execution.

The fixture links a frozen BN library from primary source
`9673354d1a985c46c83a8226caa7a54e0290328b`, SHA256
`d9947ae6d8e55c2903641262080c16b543df87eb1519df37d932b1824a9030f0`.
Source-before/copy/source-after hashes all matched. Its safe-named probe embeds
a manifest. Exact source, evidence and fixture hashes are in the report.
No permanent tests, CMake, shared metadata, full reader/root dispatch or native
CRT/FH3 replacement were added. The primary owns the combined build; original
body differential, hardware-fault, concurrent-mutation and gameplay validation
remain outside this packet.
