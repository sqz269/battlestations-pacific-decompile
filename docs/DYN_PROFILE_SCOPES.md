# Dyn profile nodes and scopes

`00C50390` creates and appends a child profile node. `00C57020` selects a cached
node and enters a timed scope. The inline range `00C5BCD7..00C5BD03` completes
that scope. The new APIs use the existing actual 9Ch profile and 48h node
storage, their allocator/string implementation, and the processor's real
timestamp counter. No substitute profile owner or clock is supplied.

| Native range | Contract | Coverage |
| --- | --- | --- |
| C50390..C50469 | ESI parent node; stack name/id; EAX appended child; RET8 | Complete normal valid-storage allocation/construction/append |
| C57020..C57063 | EDI scope; EAX id; stack name; EAX scope; RET4 | Complete normal cache selection, active-parent publication and RDTSC |
| C5BCD7..C5BD03 | Inline in C5BB30; native scope at ESP+4C | Complete 14-instruction leave fragment; the surrounding solver remains separate |

Names are descriptive hypotheses. The earlier Ghidra names are
`DYN_physics_00c50390` and `DYN_physics_00c57020`. The third range is not a
standalone function and must not be defined or renamed as one.

## Child ownership and cache selection

C50390 allocates 48h bytes and calls the existing complete C44000 node
constructor with name in EDX and the node/id on the stack. It then appends to
the parent's child vector at +4/+8/+C. When count equals capacity, capacity
becomes `2*capacity+2` before allocation; existing pointers are copied in
order, the old buffer is freed, and the replacement is published. The final
return reloads the appended entry from the vector. Arithmetic uses native
unsigned 32-bit wrapping; malformed storage and wrapped allocation extents are
outside the supported domain. Explicit null-child/destination tests remain.

This routine neither searches existing names nor activates a node or writes a
profile cache. In particular, the new child's +0 remains untouched until an
actual scope entry writes it. The producer establishes these distinct fields:

| Owner | Offsets | Meaning established here |
| --- | --- | --- |
| Profile 9Ch | +0 / +4 | Root / current active node; initialized by C50310 |
| Profile 9Ch | +C + 4*id | Cached node pointer selected by C57020; storage indices 0..35 fit the owner |
| Node 48h | +0 | Previous active node, rewritten on entry |
| Node 48h | +4 / +8 / +C | Owning child pointer vector / count / capacity |
| Node 48h | +10 / +2C | Existing legacy SBO name / supplied profile id |
| Node 48h | +30/+34 / +38/+3C / +40 | Last 64-bit elapsed ticks / accumulated 64-bit ticks / 32-bit call count |
| Scope 14h | +0/+4 / +10 | Start timestamp / selected node; +8/+C untouched |

C57020 reads the real global 0109E9F8, selects the cache by **numeric id**, and
creates a child of current only when that cache is null. After creation it
reloads the global profile before publishing into the originally selected
cache slot. On every entry it writes the scope's node, links node+0 to current,
publishes current=node, then executes RDTSC and stores EAX/EDX in the scope.

Reusing an id ignores a changed name and may change the active parent without
moving the node between owning child vectors. Ownership stays with the parent
that first created it. There is no per-scope saved parent pointer or recursion
isolation for an id already active, and no synchronization is added.

## Callers and timing completion

All 18 C50390 call sites in seven containing functions were inspected, including
their parent-node provenance and name/id pushes. The two C57020 calls are
C5BC1E and C5C045 inside C5BB30, both `Solve`, id18h. Thus its actual cache
offset is **6Ch**, resolving the old unknown offset in the settings-facing
`DynProfilerScopeSlot` entry. Existing runtime-host modules are unchanged.
The report records every site, decoded PE label and containing function.

The selected matching exit starts with actual RDTSC, subtracts scope+0/+4,
adds elapsed low bits to node+38, stores last low/high at +30/+34, adds high
bits plus carry to +3C, and increments +40. It then reloads the global profile
and pops **its current node** through current+0. It does not replace current
with scope.node first. The C++ arithmetic preserves unsigned 64/32-bit wrap
and this field-store sequence. Its timestamp helper uses `__rdtsc` with
compiler barriers; it adds no CPU serialization or alternative clock.

The owned routines require initialized storage and valid borrowed global and
allocator lifetimes. The scope path assumes appropriate balanced use. Native
SEH/unwind, actual OOM/failed allocation and concurrent profile mutation are
outside the reconstructed normal-path interface.

## Verification and remaining analysis repair

The complete 218/68/45-byte spans match the installed executable and live
Ghidra bytes. One ignored original-byte fixture constructs both actual profile
owners and Root nodes, then runs eight entries/exits per side. It covers
initial/growing/reused child vectors, cache reuse under another active parent,
direct append without activation/cache mutation, untouched scope bytes, and
64-bit total/32-bit count wrap. The original exit executes its complete inline
bytes through a private stack bridge; a return is placed immediately after the
fragment, leaving all 45 fragment bytes unchanged.

The fixture compares 18 snapshots and 265 buffers. It performs 32 checks that
start timestamps and start-plus-recorded-elapsed values lie within unsigned
hardware brackets and 16 checks of elapsed/total/count relationships. Only after those
checks does it normalize scope start words and timed-node last/total words.
All other bytes, including call counts, actual pointers after canonicalization,
and A5 unused storage, compare directly. These checks establish structural and
counter arithmetic behavior; they do not establish equal cycle durations or
instruction-level timestamp equivalence.

Both normalized 14,764-byte streams share SHA256
`bc2be02d68b7fa7f08621df5c44c71ce0ee4b3033215fc99ff2a5c1c27dc62be`.
The raw hardware brackets and counter values are retained in
`local/dyn_profile_timing.jsonl`; the ignored audit script checks them again.

Nine allocations and one old-vector free per side agree. All fixture labels
that create nodes fit the existing inline string storage; long-name allocation
uses the existing string implementation and is not exercised here. Quiescent
node/vector allocations are freed by the fixture, leaving zero tracked
allocations; this is not native profile/tree destruction validation. No physics
work, scheduler tasks, game frame or exceptional unwind is executed.

Win32 Release and both existing CTests pass. No tracked tests were added.
Exact source/image/probe/record hashes and the call gate are in the report.

Ghidra still has a false-free continuation gap at inclusive C50434..C50436,
bytes `83 C4 04` (`ADD ESP,4`), after `_free` at C5042F. Actual original-byte
execution includes this returning continuation and passes. The worker remained
read-only; **root flow repair and annotations remain pending**. The source
covers these bytes, while the current saved listing omits that one instruction.
