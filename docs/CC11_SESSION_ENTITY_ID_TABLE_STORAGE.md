# Complete raw entity-ID table storage and resolver

Packet `cc11_health_session_entity_id_table_storage` adds opt-in raw producers for
the real table fields read by `00521E30`, including both resolver calls from
`00780670`. The existing conceptual `EntityIdTable` implementation, historical
backfilled records, and its callers remain unchanged.

The new code is in `native_entity_id_tables.hpp/.cpp`. Names describe recovered
behavior; they are hypotheses, not recovered game symbols. Target: MSVC Win32.

| Original body, end exclusive | Bytes | Original ABI and result |
| --- | ---: | --- |
| `00521E30..00521E63` | 51 | `CX` ID, payload in `EAX`, `RET` |
| `00951560..00951615` | 181 | `ECX` owner, `RET` |
| `00951660..009516A6` | 70 | `ECX` owner; full DWORD first/count; owner in `EAX`; `RET8` |
| `009516D0..00951711` | 65 | `ECX` owner; low-WORD ID; `RET4` |
| `00951720..00951787` | 103 | `ECX` owner; `RET` |
| `00951790..009517BB` | 43 | `ECX` owner; flags; identity in `EAX`; `RET4` |
| `009517C0..0095181E` | 94 | `ECX` owner; requested ID/payload; `EAX` result; `RET8` |

All **607 bytes / 218 decoded instructions** were compared between live Ghidra,
the installed PE, and the frozen PE. The six alignment bytes in reset are included.
The scalar's old listing ended after its first free call because of no-return
metadata. Complete live/PE bytes include the second free and final `RET4`.
The integrator owns listing repair; this worker did not mutate Ghidra. Preserve
`CG_scalar_deleting_dtor_00951790` and its compiler-generated tag.

## Actual owner and slots

The two global owners are at `00F89A08` and `00F89A5C`, separated by `54h` bytes.
Each owner has its profile at `+0`, **full DWORD** first ID at `+4`, count at `+8`,
slot-buffer pointer at `+4C`, and free count at `+50`. Free sentinels are embedded
at `+C/+1C`; live sentinels at `+2C/+3C`. A `10h`-byte slot contains next/previous
pointers at `+0/+4`, WORD ID at `+8`, untouched padding at `+A`, and payload at `+C`.

`00951660` stores first/count/free-count, saturates an overflowing unsigned
`count*16` allocation request to `FFFFFFFF`, stamps its profile, allocates real
storage, publishes `+4C`, and invokes the complete reset behavior. Reset writes
literal ID zero and null payload to slot zero without initializing its links.
It rebuilds the remaining free slots and owner sentinels, preserves padding and
sentinel payload bytes, and **does not reset `+50`**.

Allocation invokes the full sweep only when free count is zero. A zero requested
low WORD chooses the current free-tail slot and returns its zero-extended ID.
An explicit nonzero low WORD selects unchecked wrapping `(uint16(id)-first)*16`
but returns the original full requested value in `EAX`; known callers consume
`AX`. Allocation stores payload before unlinking, inserts into the live list,
and decrements free count. Release clears payload before unlinking, inserts into
the free list, and increments free count.

The sweep deliberately preserves the native discrepancy: it tests payload at
slot `i`, then relinks slot `uint16(i)-first_id`. It does not repair nonzero-first
tables. Both selected slots and all links must be valid. Runtime sweep comparison
uses ordinary primary `first_id=0`; nonzero-first discrepancy is structural proof.

`00521E30` truncates `CX`, compares it **signed** against the current primary count,
then reads current first ID and slot-buffer fields. It has no zero, null, or range
guard. `ObjectHandleTables` references directly identify genuine owner fields.
The existing `006AD080` resolver has an extra full-word-zero early return and is
therefore not used as a substitute.

## Genuine profiles and service boundaries

Actual `00D19B88` contains one scalar DWORD, `00951790`; the following bytes begin
the string `used`. The Source counterpart is one concrete typed scalar pointer,
verified in COFF to select the complete new scalar. There are no invented slots.
Its C++ calling convention differs from the original thiscall profile.

The scalar captures `+4C` before stamping the profile, always calls real free for
slots, optionally frees the owner when flags bit zero is set, then returns identity
bits. Freed fields remain unchanged. Allocator/free operations use the existing
fixed `singleton_lifetime` provider freshly compiled from its actual Source TU.
That provider reaches real host CRT malloc/new-handler/free services; no recording
hook or fabricated allocator policy is introduced.

## One focused differential fixture

The ignored, manifested `probe.exe` in
`local/cc11_session_entity_id_table_storage_20261007_a` passed one complete sequence:

1. Construct primary `(0,4)` and secondary `(4,3)` raw owners in Source and original.
2. Allocate non-null opaque data pointers automatically and by explicit ID; compare
   full EAX bits, actual slot links, counts, and both resolver branches.
3. Release a low-WORD ID, reallocate in native free-tail order, and consume the final
   slot with a real null-payload allocation.
4. Invoke allocation again at free count zero, executing the complete original and
   Source primary sweep before publishing a non-null payload.
5. Reset with free count one and verify it remains one.
6. Exercise real scalar cleanup with flags `0`, `2`, and `1`; dynamically constructed
   cleanup owners also verify full DWORD `first_id=12340000` storage.

Borrowed payloads are three ordinary data-word addresses. They are never fake
entities, worlds, callbacks, or dispatched objects. Native scalar calls are direct;
the copied native profile slot is verified but native virtual class dispatch is
not claimed. Internal assertion count 1,588 includes byte-identity checks and is
not a count of tests or reconstruction progress.

The original comparison retains every complete body. It changes twelve explicitly
recorded operand ranges: five direct CALL operands, five absolute resolver operands
to genuine constructed owners' actual fields, and two profile-address immediates.
Those ranges occupy **48 bytes**, leaving **559 bytes outside relocations**. In the
final run, **37 body byte values changed and 570 stayed identical**. Internal calls
select complete retained bodies; external calls select the fixed real CRT services.

The actual native profile plus adjacent context is copied as 16 bytes. Only its
sole scalar DWORD is rebased to the complete retained 43-byte body. Final accounting
is **4 changed / 12 unchanged** profile-context bytes. All body/profile bytes and
relocated operands were checked before and after the lifecycle. Runtime addresses
and each original/rebased DWORD are in the report and transcript; ASLR may change
actual byte-value counts on another run.

## Validation and limits

The final strict `/O2 /W4 /WX /fp:strict` Win32 build compiles all three actual TUs
and all fixture includes from a fresh read-only snapshot: 18 repository/fixture
inputs, with the 189 actual host headers pinned separately. Current Source and
snapshot inputs match. The new seven Source bodies total **981 bytes / 341 decoded
instructions**, plus a six-byte profile getter. Complete COFF bodies, profile data,
aliases, native invokers, fixed bridges, and allocator/free providers were reviewed.

Three current support libraries were frozen with source-before/copy/source-after
hash equality before integration builds. The final report pins those libraries,
actual inputs, compiler/linker, installed/frozen PE, binaries, and artifacts. All
231 prior broadcast/append/deferred artifact hashes still match. The primary owns
CMake registration and the full `scripts/build.ps1` integration build.

This proves the bounded constructed-table behavior and complete code paths. It
does not prove host exception/native CRT ABI equivalence, overflow/OOM runtime,
invalid counts or pointers, asynchronous mutation, native class replacement ABI,
global publication, or game behavior. The pair constructor `00927940` and full
entity constructor/destructor `00928630/009287B0` remain outside scope.

The real entity constructor calls allocation at `009286E4`; the destructor chooses
the actual global owner and calls release at `00928810`. The drain gate calls the
resolver at `007806FC/0078077E`. Closing this table provider does not close
`00780670`: synchronization `0077C710`, dispatch `00780120`, and property updates
`00805C60` remain. `0076C600` and `0076C500` retain their qualified consumer status.

Machine-readable evidence: `reports/cc11_session_entity_id_table_storage.json`.

Primary integration at `b728a6343161f29f251a29d40c17ceffc1590584` passed the complete MSVC Win32 build and all three existing CTests. The independent current-library fixture freshly compiled 3 actual TUs, with 16 Source/header/fixture pins and 15 actual compiler project includes. Source, generated recipe inputs, original PE and three current libraries remained unchanged; COFF matched the worker output and native direct calls/tail transfers passed. The report records the complete receipts. One focused lifecycle fixture passed1588 internal byte/range/lifetime assertions; this is not1588 independent tests. All607 original code bytes are accounted with48 designated operand bytes/559 retained outside. Current Ghidra scalar membership includes the full43B body and optional free, preserving its CG name/high tag; old partial-flow evidence and conceptual Source records remain historical. Original ABI, full remaining constructors/world/consumer binding and gameplay remain qualified.
