# CC12 Mission Lua payload destructor physical body

Baseline: published Main `365083d632ee8aaff7095a02292766b1f5271051`.
Evidence: [independent capture and decode](../reports/cc12_mission_lua_payload_body_extent_readiness.json).
This is a read-only body audit. It admits no Source, ABI, runtime or gameplay credit.

## Complete ordinary body

The live GPR and installed PE agree on all 160 captured bytes at
`008876B0..0088774F`. The contiguous ordinary body is **148 bytes / 49
instructions**, ending at plain `RET 00887743`. Twelve `CC` padding bytes follow;
the next saved function starts at `00887750`. Both internal branches target
decoded instruction starts inside the 148-byte body.

The saved Ghidra function still ends at `008876E9`: its 58-byte/18-instruction
prefix ends in an ordinary CALL to `BF6989`. The newly decoded continuation is
physical evidence, not a repaired saved function or decompiler result.

| Call site | Actual direct target | Ordinary role |
| --- | --- | --- |
| `8876DD` | `6B88E0` | Raw vector operation, receiver payload `+10h`, request zero |
| `8876E5` | `BF6989` | Free current post-vector-call data |
| `887703` | `419CC0` | Obtain current pool for the payload `+0Ch` buffer |
| `88770A` | `BD1510` | Return that buffer with wrapped length `+08h` plus one |
| `887727` | `419CC0` | Obtain current pool for the payload `+04h` buffer |
| `88772E` | `BD1510` | Return that buffer with wrapped length `+00h` plus one |

Each later operation requires the preceding child to return normally. There is
no added guard, rollback, failed-call fallback or successful-cleanup guarantee.

## Current operands and state

Incoming ECX is captured as payload in ESI and spilled locally. EDI first holds
the raw vector header address, then its **current data pointer after** the vector
child returns. That pointer is passed to fixed free. The next buffer read from
payload `+0Ch` occurs before reclaiming the stacked free argument.

The first buffer is tested and only the state's low byte becomes zero. Its
nonnull path reads the current length at `+08h`, wraps DWORD `length + 1`, pushes
`unused=1`, size and buffer, calls `419CC0`, puts the returned pool in ECX and
calls `BD1510`. The pool getter itself has **no Native arguments**: the three
already stacked words belong to the subsequent pool-return call.

After that conditional path, the second buffer is freshly read at `+04h` and
tested. The full state DWORD becomes `-1` before its conditional release. Its
nonnull path reads length `+00h` and uses the same getter/return sequence. The
body contains no payload/string clear store or post-free vector-data access.
The two null branches target `0088770F` and `00887733` respectively.

Let S be entry ESP. The state is at `S-04h`, handler `C97543` at `S-08h`, old
FS head at `S-0Ch`, payload spill at `S-10h`, saved ESI at `S-14h` and saved EDI
at `S-18h`. Normal completion restores saved EDI/ESI, old `FS:[0]` and ESP, then
uses plain RET. No semantic EAX result is promised. The handler/helper frame
relation and exceptional behavior were not expanded by this packet.

## Available Source and remaining boundaries

Existing concrete pool Source exposes the actual mutable `01090AA8` publication,
canonical `01090AA0` manager/domain and `01090AA4` small-return gate. The process
owner already supplies Source cells and raw contexts. Pool return uses its
qualified Source CRT/Win32 domain; this is not original heap, CRT or fault proof.
The Native return interface is ECX pool, stacked `(block,size,unused)`, RET `0Ch`.
Native vector-buffer free has its existing explicit Source allocation domain.

The raw vector child remains a separate dependency. This audit supplies no Lua
payload/vector construction, genuine Lua owner integration, Source destructor
composition or callable Native entry. Arbitrary alias, concurrency and fault
outcomes cannot be inferred from normal operand order.

The complete ordinary body conditionally returns **both string buffers**. The
58-byte prefix cannot support an assertion that neither is ever released.
Actual return behavior remains subject to the pool shutdown gate and successful
calls. The caller's later outer queue-node free is a distinct lifetime from the
vector data and the two string buffers.

## Ghidra and validation

The handler `C97543`, shared dispatcher and unwind actions were not queried.
State stores alone do not establish exception dispatch, state-update order,
fault filtering or nested failure. A missing continuation does not reveal the
current FlowOverride, separate fallthrough override or callee no-return flags.
The current repair tool is conditional/read-only; exact metadata preimages and
an attested atomic repair route remain unavailable. No mutation was attempted.

Guarded live queries verified existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and matching total count 64,729. The installed PE
hash remains `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Twelve canonical repository pins match the baseline Git blobs. The captured
prefix exactly matches the independently accepted worker evidence. Native
descendants were queried for metadata only; no descendant body, new build,
test, probe, execution, ledger edit or Ghidra edit occurred.
