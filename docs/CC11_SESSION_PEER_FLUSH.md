# Complete session peer-list flush

Packet `cc11_health_peer_flush_all` reconstructs the complete function
`007848F0..007849BA` (exclusive end `007849BB`, 203 bytes, 72 instructions).
`BSP_SessionTransport_FlushPeerBuffers` is a descriptive hypothesis, not a
recovered symbol. Native ABI: ECX is actual transport, no stack arguments,
plain RET. The new Source member takes ECX adapter plus a stack transport and
returns with RET4; it is not a binary replacement.

The new adapter derives from the completed target-flush adapter and borrows
the existing pure aliases to transport+0C and real 0Ch peer nodes. No new
list owner, queue provider, clock service or arbitrary flush callback is added.

## Native order and Source composition

1. Read transport+0C, then that sentinel's next+0 as the initial node.
2. Read transport+0C again for the loop-end comparison. The native preceding
   CMP ESI,ESI is always equal; its unreachable CRT call is omitted in Source.
3. Compare the captured node with a second fresh head. On equality invoke
   the fixed current SDK `_invalid_parameter_noinfo` service. Preserve the
   continuation if it returns; only then read captured-node+8 as the target.
4. Perform the complete target-flush operation: captured target+4 Enter/depth
   increment, three fresh D40/D44/D48 cursor cells in order, current transport
   +20 dispatch for each nonzero modulo32 byte count, post-call captured-cursor
   base/current/bit/first-byte reset, then fresh target+4 depth decrement/Leave.
5. Compare the captured node against another fresh head after the target
   operation. Preserve the returning CRT continuation, then read that node's
   fresh next+0 and repeat the loop-end comparison.

Source composes completed `flush_session_target_00783490`. The original
`007848F0` **inlines** that three-cursor operation; it does not call `00783490`.
Neither peer_count+10 nor a local-peer exclusion participates in this body.
There is no new guard, clamp, allocation, exception cleanup or success policy.
The original stale/captured node and fresh field-read choices are retained.

The CRT boundary follows the existing concrete current-SDK Source wrappers.
It is not the original `00BF6713`/`00BF66EF` encoded handler globals, Watson,
private EH or invalid-storage fault contract. No arbitrary adapter method is
used to implement the validation service. Ordinary fixtures do not trigger it.

## Original bytes and call evidence

Live read-only Ghidra queries verified the existing `C:/Users/sqz269/bsp.gpr`
project and `/battlestationspacific.exe` program. All 203 live bytes equal the
frozen installed PE bytes. SHA-256:
`cd87fa4ef3a418a9ad911054af628066b6d37adbcb7abef381046ea2fd9c1848`.

The original fixture copies the complete 203-byte body, including the dead
self-comparison branch and the inlined three-cursor loop. It relocates only
five four-byte operands: three CRT REL32 operands at copy offsets 1F/31/AC
and real Enter/Leave IAT operands at 3E/97. All other 183 bytes are asserted
unchanged. The copy is writable only during setup, then sealed executable.

The actual host/client D243EC/D24448 profile words are copied; only +20 is
replaced on the original-copy side by inspected ECX/three-stack-argument/RET0C
thunks that call the complete corresponding Source host/client flush through
the stable actual adapter. Source-side +20 and both sides' +24 retain original
PE tokens. These are explicit operation relocations, not an all-original
downstream transport stack. The downstream owned-enqueue chain is complete
Source shared by both sides. The three CRT relocations target a fixed cdecl
real SDK service, never a no-op or simulated validation callback.

Eleven static inbound edges are checked: ten CALLs and the tail JMP at
`0076D217` inside `0076D1B0`. The live xref service labels the latter as
UNCONDITIONAL_CALL; the live listing and disk opcode establish JMP. Three
native CRT callsites are checked, as are the two qualified indirect +20
profile resolutions. The call verifier checks 16 rows, zero failures; the
two import rows are qualified and skipped. Static edge checks do not prove
whole-caller integration or runtime indirect target selection.

## Fresh fixture and emitted code

Fresh MSVC 19.51.36244 Win32 compilation includes 30 repository translation
units, one probe TU and 86 immutable source/header/probe inputs. Flags include
`/O2 /Gy /W4 /WX /fp:strict /MD /EHsc /FAs`, plus `/MANIFEST:EMBED /OPT:REF`.
The manifest requests asInvoker. Three support libraries were copied read-only
with source-before/source-after/copy hash equality before permitting a main
rebuild; source inputs, snapshots, copied libraries and original PE match their
captured hashes after the run. Build and probe both exit zero.

The new Source COFF body is 107 bytes, 42 instructions, with two fixed-CRT
REL32 relocations and one direct completed-target-operation relocation.
Review proves initial head/next reads at 1B/1D; end-head at 23 and 5C; fresh
first guard at 34 then CRT at 38; target+8 at 3D after validation; complete
target call at 44; fresh second guard at 4D then CRT at 51; fresh node.next
at 56. No peer-count value is read. The SDK wrapper is an actual six-byte
IAT tail jump; both +20 forwarding thunks return RET0C. The freshly compiled
target loop remains 150 bytes/63 instructions with its captured-cursor reset.

Six complete-original-loop cases pass **592 checks, zero failures**:

- Empty sentinel with stored peer_count 77 and seeded nonempty targets.
- One client peer with a partial direct payload; second target stays unvisited.
- Two host peers with six distinct payloads despite stored peer_count zero.
- Two client peers sharing a destination with both actual locks held recursively.
- Two host peers with suppression and pool exhaustion after the first peer.
- Two client peers with an empty pool and an all-empty second peer, including bit8.

Each world genuinely constructs both D74 targets, separate locks/cursors,
50+3 sample histories and actual slots 0/1. One genuine constructed/published
clock uses original D68D50 method words, real fixed-mode enable16 and advance
once. Both worlds borrow that same clock publication/context; timestamps are
not manually seeded. Genuine unconnected UDP sockets, owner lock, 16 channels
and six complete 82Ch queue records support actual returning enqueue paths.
Reached owner/transport/list/sockaddr backing is explicit; full factories are
not claimed. No live network sends occur.

Comparisons cover both nodes/head/count unchanged, each target's cursor and
whole backing, histories, actual OS recursion/depth, per-peer counters/D68,
game217C, ordered payload/address/type/socket fields, direct indices and pool
consumption. Entire 4MB owner state compares after only named, range-checked
pointer normalization. Queue payloads survive overwriting both peers' buffers
and fully destroying both targets. Empty-list owner/clock state is unchanged.

The fixture uses finite quiescent valid storage. Read-order and CRT-continuation
evidence is native/emitted code, not fake SDK mutation hooks or invalid-memory
execution. No full owner/session/transport factory, socket send thread, gameplay,
concurrency, private faults or native ABI compatibility is established. Caller
`00784B10` remains open at its unresolved profile+2C operation. The worker
changed no shared metadata, CMake, ledgers, tracked tests or Ghidra state.

Machine-readable evidence: `reports/cc11_session_peer_flush.json`. Ignored
receipt: `local/cc11_peer_flush_20261007_a/`. Main integration and full main
build belong to the primary integrator.

## Primary integration

Main `55afb410e3cb242ff5c60fcabef8502962a92d3a` passed the full Win32 build and all three existing CTests. Root independently reviewed the Source, native body and entire fixture, rebuilt31fresh TUs with86current Source/header/fixture inputs (55compiler includes),3current libraries and the installed PE pinned before/after, and reproduced592checks across6whole-original-loop cases. All203disk/live bytes match and the PE32 asInvoker manifest was verified. Native rows are13direct CALLs,1tail JMP and2qualified indirect profiles; all16checks passed. The diskE9 operand at0076D217 independently confirms the tail transfer. Fresh emitted guard/target/next order and real SDK IAT tail wrapper were inspected. All ordinary quiescent backing/CRT/ABI/runtime/network/game limits above remain; no tracked tests were added.
