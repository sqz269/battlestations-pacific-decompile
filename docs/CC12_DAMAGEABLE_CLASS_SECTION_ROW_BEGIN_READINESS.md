# DamageableClass section row-begin readiness

Owned parent anchors: 0087CE00 0087CE0D 0087CE48, inside 0087CA80.

This is a two-file readiness packet. The exact initialization/append and
temporary lifetime boundaries are established from the retained complete
parent audit. Genuine current Source implementations exist for memset,
section-vector append, section copy/fill and section destruction. The older
parent report's `source_absent_actual_30_byte_vector_append` classification is
stale: the current vector implementation and its complete 14-entry report
supersede that availability claim. Here 30h means 48 bytes, not decimal 30.
No reader fragment, new function, CMake/ledger/Ghidra change, fixture or build
is added. Current Source availability does not establish caller composition,
actual application bindings, native ABI/FH3 or runtime behavior.

| Boundary | Exact region | Bytes / instructions | State at normal successor |
| --- | --- | --- | --- |
| Temporary initialization only | CE00..CE2E | 47 / 10 | parent state 13; source temporary initialized |
| Append setup/call only | CE2F..CE4C | 30 / 7 | temporary state 14 still armed at CE4D |
| Smallest initialization plus append | CE00..CE4C | 77 / 17 | temporary state 14 still armed |
| Matching normal temporary cleanup | CE4D..CE87 | 59 / 15 | parent state 13 at CE88 |
| Minimum lifetime-closed candidate | CE00..CE87 | 136 / 32 | pair state 13 remains live; no row cursor returned |
| Following back-row selection, context only | CE88..CEBA | 51 / 15 | actual selected row in ESI at CEBB |

The 77-byte prefix hash is
`855a377ad32bae0e0193292ba4e3db2512b8b9041c2477d2912c0edaec40e266`.
The 59-byte normal tail hash is
`02d83f4e096a624ea164554201d560b5d7111434bcb16be4899719aa2e9e97fd`.
Together the 136-byte candidate hashes to
`16c1a68b21959bb3c291bdfc3215e32f9b548345f1ff64e7b5bb25df42b917bd`.
The JSON records every byte and instruction for all six bounded views.

The original parent remains the verified 3238-byte/858-instruction reader with
SHA-256 `00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
Installed PE bytes and current saved instruction starts match the retained
verified live listing. The existing 37-state audit supplies FH3 evidence.
No fresh live Ghidra query, Native child/data/handler opening, mutation or
restart was performed. The integrator's separate tooling rollout does not
change the scope of this retained-evidence packet.

The native parent takes descriptor in ECX and Lua row on the stack, RET4.
With S=ESP after E4h locals and four saved registers, actual descriptor is
stored at S+40h. The successful iterator owner still holds key S+58h and
value S+2Ch under state 13; Sections S+80h, Damage S+44h and Unique S+94h
remain live in enclosing owners. Temporary section storage is S+A8h..S+D7h,
exactly 30h bytes. It is distinct from those live Lua slots and the saved
descriptor. The header address is the actual descriptor+18h, not a copied
model or STL container. Header+0 is preserved; begin/end/capacity-end are
header+4/+8/+Ch, hence descriptor+1Ch/+20h/+24h.

At CE0D, the native cdecl call is BF79F0(temp,0,30h). CE1A removes 12 argument
bytes. CE12 loads CE38B8 and CE1D/CE26 store it to temp+28h/+2Ch. The retained
constant audit records bytes `00 00 20 41`, DWORD 41200000h, float32 **10.0**.
No new data read was made. Later FailureChance/FailureDamageThreshold Lua
defaults are separate operations and must not replace these initial values.

| Actual temporary offset | Initialized bits and eventual role |
| --- | --- |
| +00h | zero before append; actual section vtable written by copy/destruction providers |
| +04h / +08h | zero; later category/index fields |
| +0Ch..+20h | six zero float32 words, later model/position payload |
| +24h | null intrusive effect-owner pointer |
| +28h / +2Ch | 41200000h / 41200000h, both 10.0f |

CE2F reloads the stored descriptor, CE3A forms its actual+18h header, CE3D
passes the actual S+A8h source pointer, and CE40 raises decimal state 14 before
CE48 calls87C870. The append ABI is ECX=header, stacked source, RET4, with
no semantic return value. The input temporary's zero vptr does not supply a
substitute table: the genuine section copy provider writes its caller-borrowed
actual D0DF04-equivalent table identity into constructed destination records.

State 14 has parent state 13 and action C96994, which calls 878EF0 on EBP-58h=S+A8h.
On append failure that temporary cleanup precedes key/value and outer Lua
cleanup. It does not free the stack temporary or roll back the descriptor's
vector. Append's own complete catch paths have separate partial-construction
and allocation rules. A later field-read failure must likewise retain the
already completed append and any committed/partial row writes.

The successful normal tail has an important order distinct from invoking the
existing destructor as a black box:

1. CE4D captures temporary+24h into ESI; CE54 compares that captured owner to 0.
2. CE56 lowers 14->13, then CE5E writes actual D0DF04 to temporary+0.
3. CE69 uses the earlier comparison; null skips all owner/slot cleanup writes.
4. Nonnull decrements the captured owner's+4 through InterlockedDecrement.
5. On zero, load that captured owner's current vptr/vslot0 and call it with
   ECX=owner and no stacked flags.
6. After normal return/nonterminal decrement, CE81 clears original temp+24h.

If the virtual callback changes temp+24h, its replacement is cleared without
being retained or released. If the normal callback throws, state 13 is already
stored; the temporary must not be retried by a fragment guard. Outer pair/Lua
owners remain responsible for their own cleanup. An unwind callback throwing
again remains an ordinary noexcept/termination versus native-FH3 boundary.

`destroy_native_damageable_section_00878ef0` is the genuine **unwind** provider:
it writes vptr before loading the handle, matching the retained state 14 action.
`release_native_ref_counted_handle_0041de40` loads its slot at its own entry.
Neither directly expresses the parent's earlier capture followed by state
lowering/vptr write. The vector TU has a private captured-owner release helper,
but it does not clear the caller's slot. A future bounded fragment must
explicitly compose the observed capture/atomic/current-vslot/conditional-clear
schedule or supply an equally evidenced provider. No such new helper is added
here. The vector insertion routine's private temporary has its own different
normal cleanup; it must not be confused with this parent temporary.

The current providers were inspected as actual implementation bodies:

| Source implementation | Current verified scope and remaining interface boundary |
| --- | --- |
| `native_crt_memset.cpp`, BF79F0 | Complete naked instruction body; new four-argument interface borrows actual stable canonical0109EEA4 DWORD |
| `native_damageable_section_vector.cpp`,87C870/87C2D0/87B950 and helpers | Complete append/checked insert/growth/catch Source over actual 10h headers and 30h records |
| `native_damageable_section_fill.cpp`,8798F0 | Actual copy-construction loop and forward current-vslot cleanup of completed prefix |
| `native_damageable_section.cpp`,878B40/878EF0/41DE40 | Actual ordered x87 copies, borrowed vtable, intrusive atomic retain/release and actual virtual dispatch |
| `singleton_lifetime.cpp` | Existing allocation/new-handler/free boundary; no new allocator invented |

The vector's spare-capacity path constructs one record at captured end before
publishing end+30h. The full path uses the complete checked-insert/growth
implementation and its real section/legacy-string/length-error dependencies.
It preserves header reloads, returning invalid-parameter callbacks, actual
record vslot0(flags 0), owner vslot0(no flags), wrapping 32-bit arithmetic, x87 copy
effects and partial-write behavior. A std::vector push, memcpy record copy,
no-op invalid handler, invented vtable or fake release callback is not an
equivalent provider. Historical vector validation covers eight bounded
original/source scenarios; it is retained evidence, not a new row-reader run.

For this fixed48-byte zero-fill, the real memset body branches around its
feature-word read and vector engine. Its Source API nevertheless requires a
valid alias to the actual canonical cell; a fabricated zero cell is not an
established binding. Original three-argument/12-byte caller cleanup and the
new Source four-argument/16-byte cleanup are distinct. DF=0 and a writable,
nonwrapping extent disjoint from provider frames/arguments remain required.

`NativeDamageableSectionVectorAccess` also needs the actual stable table identity
and actual returning invalid-parameter service/context. Complete scoped Source
searches found its declaration/provider parameter uses, without an instantiated
DamageableClass row-reader binding. This is a bounded search conclusion, not
proof that every possible dynamic binding is absent. The exact argv, complete
stdout/stderr, return codes and base commit are saved and pinned in the report.
Current source/header pins are separate from retained test and ABI receipts.

CE88 begins subsequent row selection. It captures the then-current end only
after temporary cleanup, validates using current header fields and potentially
returning BF6713 calls, derives captured-end minus30h, and stores the captured
end at S+DCh. Header comparisons reload selected fields after callbacks; the
captured end is not replaced with a universally fresh value. Those sites do
not retry validation. A future composition must preserve that distinction;
neither an append return value nor a cached pre-append pointer identifies the
actual continuation row. This 51-byte selection block and all CEBB onward Lua
field/loop work remain outside the proposed 136-byte minimum.

Readiness is therefore: direct provider bodies and native boundaries known;
reader Source still held. A future separately owned packet must choose either
the 77-byte prefix with an explicit state 14 handoff or the 136-byte lifetime-
closed boundary, establish actual receiver/scratch/vtable/CRT/service bindings,
retain the enclosing state 13 owner, preserve normal versus unwind cleanup,
and compile/review the concrete composition. Original ABI/FH3/SEH/longjmp,
fault/double-exception identity, application receiver and gameplay remain held.

Only this document and its JSON report are changed. No new build/test was
needed for readiness; no old test result is counted as fragment execution.
The prior Root Source 603 review remains context: 603 inputs, 75 Core + 3 App whole
objects, 151 positive Core definitions and three existing checks. Its frozen
artifacts and the admitted iterator/entry fragments are unchanged.

Primary review independently froze every reported artifact occurrence, matched all29
selected tracked provider/context blobs against the worker baseline and current
Source, and replayed all six complete region decodes against the unchanged original
PE. All858 full-parent starts match the pinned saved and retained live listings.
This accepts readiness for a separately owned136-byte lifetime-closed Source packet.
No caller binding, new reconstruction, original ABI, runtime or gameplay credit
is added. Normal captured-owner cleanup and unwind cleanup remain distinct.
