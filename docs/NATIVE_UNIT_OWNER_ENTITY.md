# Raw unit-owner construction, peer insertion and withdrawal

`src/native_unit_owner_entity.cpp` supplies concrete normal storage stages of
the real unit-owner constructor and destructor, concrete peer insertion, and
the required raw list/count leaves, recon-record primitives, and scene-base
construction. It does not construct a whole root, publish a unit, provide a callable
profile, or activate the native tick/application path.
Source names and field roles are descriptive hypotheses, not recovered C++
symbols; address and raw storage behavior are the evidence contract.

The `00928630` GameEntity constructor audit below refreshes the current dependency
frontier. It adds no Source body or fragment: the component count remains twenty,
and whole constructor, destructor, receiver activation and game-binding claims
remain false.

| Source entry | Original coverage | Contract |
| --- | --- | --- |
| `construct_native_unit_owner_scene_base_00925ce0` | Complete normal `00925CE0..00925EF3`, 532 bytes | Actual observer/weak prefixes, both x87 matrix copies, mixed-width fields and raw strings; five Source cleanup owners with qualified exception semantics |
| `restore_native_unit_owner_scene_destructor_profiles_009257a7` | Qualified `009257A7..009257B9`, 19 bytes | Three ordered numeric profile writes before life-reference release and child deletion |
| `finish_native_unit_owner_scene_retirement_009257f5` | Qualified `009257F5..009258B0`, 188 bytes | After actual life-reference release and child loop: concrete world/list unlink, pooled names, weak, callback and observed retirement; no root free |
| `construct_native_unit_owner_first_list_0077ef27` | Qualified `0077EF27..0077EFA3` business effects | After actual base and recon-array success: bind three owner/index pairs, install numeric profiles, clear selected fields, allocate the real 0Ch sentinel and publish +29C/+2A0 |
| `construct_native_unit_owner_second_list_and_fields_0077efa9` | Qualified `0077EFA9..0077F053` business effects | After first-list success: allocate the real 18h sentinel, publish +2A8/+2AC, initialize observer/physical fields and capture actual CE4ADC bits once |
| `tidy_native_unit_owner_peer_list_0077b980` | Complete normal `0077B980..0077B9C7`, 72 bytes | Reset head links/count, free each node, free sentinel, zero header+4 |
| `clear_native_unit_owner_holder_list_0077d930` | Complete normal `0077D930..0077D97C`, 77 bytes | Reset head links/count; per node free a non-null +14 payload, zero that field, then free node; retain sentinel |
| `destroy_native_unit_owner_lists_0077e42c` | Qualified `0077E42C..0077E456` business effects | After actual observer destruction: clear +2A4 list, free its sentinel and zero +2A8, then tidy +298 list |
| `buy_native_unit_owner_holder_node_0077ba60` | Complete normal `0077BA60..0077BAA6`, 71 bytes | Allocate 18h; independently guard wrapped +0/+4/+8 destinations; copy the four actual record DWORDs after link stores |
| `buy_native_unit_owner_peer_node_0077bb00` | Complete normal `0077BB00..0077BB32`, 51 bytes | Allocate 0Ch; same guarded links; read actual peer argument after allocation |
| `grow_native_unit_owner_holder_count_0077dd00` | `0077DD00..0077DD92`, 147 bytes | Unsigned wrapped count bound 0FFFFFFF; actual legacy string/logic-error ownership with Source exception transport |
| `grow_native_unit_owner_peer_count_0077de40` | `0077DE40..0077DED2`, 147 bytes | Corresponding bound 3FFFFFFF; no allocation ownership or rollback |
| `release_native_unit_owner_holder_scratch_0077b030` | Complete normal `0077B030..0077B04B`, 28 bytes | Free non-null scratch+0C and zero it; owns no list node |
| `append_native_unit_owner_peer_0077eb50` | Qualified business body of `0077EB50..0077EC10`, 193 bytes | Insert holder, then peer, then set actual F87190; requires actual argument cell and caller/frame scratch preimage |
| `finish_native_unit_owner_peer_tail_0077f054` | Qualified `0077F054..0077F076`, 35 bytes | Fresh actual E188A8/game+1FE4 and conditional E0AF20 read; mode2/byte0 calls concrete insertion with peer0 |
| `restore_native_unit_owner_destructor_profiles_0077e3a0` | Qualified `0077E3A0..0077E3C7`, 40 bytes | Ordered five numeric profile writes before the session-release decision |
| `remove_native_unit_owner_global_matches_0077bea0` | `0077BEA0..0077BF3B`, 156 bytes | Capture actual sentinel and target once, remove every matching node, free then reload/decrement count; accepted Source invalid-parameter boundary |
| `withdraw_native_unit_owner_global_0077e409` | Qualified `0077E409..0077E41B`, 19 bytes | After actual session-release arm completes/skips: materialize root value cell and remove every match from actual F87194 list |
| `construct_native_unit_owner_recon_record_00805a70` | Complete normal `00805A70..00805AE9`, 122 bytes | Actual 34h record writes and fresh target reload through the existing raw observer domain; qualified Source cleanup |
| `destroy_native_unit_owner_recon_record_00804300` | Complete normal `00804300..0080430D`, 14 bytes | Stamp D08E74, then accepted raw 4B7EF0 at record+14; retain record allocation and +28 target bits |

These Source interfaces target MSVC Win32 and use the genuine raw allocation.
They do not reinterpret `GameUnit`, synthesize a replacement header, zero an
entire receiver, or introduce an alternate allocation/process domain. Calls to
the accepted `buy_native_unit_owner_list_head_0077b960/0077b940` retain the
BF681B malloc/new-handler boundary. `singleton_lifetime_free` supplies the same
CRT free domain used by those allocations. Numeric profile words remain
evidence values; they are not host-callable virtual tables.

## Construction and failure ownership

The actual containing allocation and factory preimage must already exist.
Stage one requires successful `00928630(root, 1, unchanged full flag)` and the
actual three 34h recon records at +1E8/+21C/+250, constructed through
BF7CD1 with 805A70/804300. The current Seg60Host uint16 requested-ID interface
is narrower than the formal full DWORD; this remains an interface limitation,
not demonstrated gameplay divergence or authority to change that interface.

The three backrefs target root+1E4, not root: +214/+248/+27C receive that same
address and +218/+24C/+280 receive 0/1/2. Profile writes and scalar stores follow
the Original order. Stage one owns the 0Ch sentinel only after allocation
returns and the +29C store completes. If allocation throws, the enclosing
owner still owes its actual recon/base unwind.

Stage two begins at the second allocation, after the Original state-2
transition. If that allocation throws, the first list still belongs to the
enclosing owner. Neither stage substitutes guessed cleanup or erases evidence
of partial construction. Both successful allocations belong to the real root,
not to a temporary RAII wrapper. The ordinary list tidy leaf is available, but
its existence does not establish the enclosing Native exception frame.

Stage two preserves the BYTE stores at +2C0/+2F0 and WORD store at +2C8.
It reads CE4ADC once after +2C8 and before +2E8, then copies the captured bits
to +2E4 and +300. MOVSS is a bit transfer here, including non-finite patterns.
Untouched factory bytes and padding stay untouched. It stops before the real
E188A8 publication read, game+1FE4 comparison, E0AF20 byte and possible
77EB50 peer-holder construction. The separate tail below supplies those effects
when its actual storage and lifetime prerequisites are met.

## Raw recon-record lifetime primitives

805A70 now operates on the actual 34h allocation and its preimage. It writes
D08E74 at +0; zeroes the embedded callback owner's +4/+8/+C and target +14
(record +18/+1C/+20/+28); stores BYTE1 at record+24; then stamps CE75CC at
record+14. It clears record+4/+8/+C and BYTE+10 in that order. The Original
MOVSS zero at +C is reproduced as the same zero DWORD bits. Padding +11..13
and +25..27 and the actual +2C owner/+30 index remain untouched. The constructor
returns the exact record address and reads the current +28 target after those
writes. A non-null capture invokes the real 6952A0 pair removal with callback
base record+14, then clears +28 after that call returns.

The Source API borrows the existing `NativeObserverLifetime` object. It does
not reinterpret the cc13 semantic recon classes, create another process domain,
install callable numeric tables, or substitute callback-only services. Current
`src/native_game_array_lifetime.cpp` still matches the accepted R122 Source hash
for its concrete `destroy_native_game_participant_observer_004b7ef0` child.
That child stamps CE74FC, captures its +14 target, uses the same raw graph for
6952A0, then executes concrete 695870 callback-owner destruction. Retirement
804300 stamps only record+0 before calling that child at +14. The normal child
retains the target bits and frees its observer-array storage without freeing
the containing record; its final callback profile is CE3CD4.

Fresh FH3 data establishes handler C9053B -> FuncInfo DC1248 -> unwind map
DC1240 -> funclet C90530. The sole state0 action reloads the captured record,
adds 14h and jumps through 4BD350 to 4B7EF0. Source arms a private guard after
the actual target capture and invokes that accepted child on a throwing call;
normal completion disarms it. This is Source exception ownership over the
same record, not Original FH3 stack-slot, hardware-fault or throw ABI. The
accepted child itself covers its normal body, so failure inside that cleanup
and the enclosing vector iterator remain explicit Native lifetime gates.

Current typed metadata recognizes 804300 as its own complete 14-byte function;
its older comment about no function start is historical. No listing repair was
needed. These record primitives do not yet supply BF7CD1/BF7C6E iteration,
the actual GameEntity base, or a completed root/derived factory. The first
owner stage still requires all three records to have genuinely completed in
the actual containing allocation, with the full enclosing failure owner.

## Conditional peer tail and insertion ownership

The tail loads the borrowed actual E188A8 publication once, reads the resulting
game+1FE4 DWORD, and reads the actual E0AF20 byte only when the mode equals 2.
Only a zero byte enters 77EB50 with an explicit peer argument of zero. There is
no added null guard, cached Game object, substituted mode, or callback provider.
The two skipped branches leave scratch and dirty storage untouched.

77EB50 requires both live owning lists and a disjoint aligned 10h scratch
preimage from the actual caller/frame domain. Original scratch+0A/+0B are
unwritten but copied by 77BA60. Source preserves those exact supplied bytes;
it does not invent a zero-filled temporary or claim to recover Native stack
provenance. The actual peer argument cell is borrowed: its first value becomes
holder scratch+0, while the later peer-node payload reads that cell again after
the second allocation. The other scratch writes are DWORD FFF0BDC1 at +4,
WORD zero at +8, and pointer zero at +0C.

The actual holder head/previous are captured before the first allocation.
After allocation, checked count growth precedes holder linking. The next head
and previous come from a fresh +29C load after that insertion. The second
allocation and checked count precede peer linking; only then is the actual
F87190 byte set to 1. Node producers use the same concrete BF681B allocation
domain as the sentinels, with independent wrapped destination guards and
ordered volatile payload reads. No virtual tables or receiver are synthesized.

Fresh live/file EH records prove C8A068 -> FuncInfo DBA454 -> unwind map DBA44C
-> C8A060, which passes EBP-1C to 77B030. State0 is armed at EBA1 before the
holder allocation and disarmed at EBD5 before the peer allocation. Only the
scratch+0C payload is owned by that map. Neither a newly allocated but unlinked
node nor an earlier completed list insertion is rolled back. Source preserves
this cleanup around throwing calls; `/EHsc` may move its host state markers
across nonthrowing memory operations. Native frame layout, hardware-fault
unwind and Original exception ABI are not established by that Source guard.

The two count guards use unsigned `bound - captured < increment`, including
wrapped subtraction. Their real failure path builds the legacy string
`list<T> too long` with count16 and constructs the existing owning
`NativeAliasListLengthError` through the concrete legacy logic-error body.
No shadow process or replacement `std::length_error` is introduced. The
completed temporary message is guarded only after assignment returns, matching
C89F58/DBA324/DBA31C/C89F50 and C89F98/DBA37C/DBA374/C89F90, whose cleanup
targets 4072D0. Transport uses the Source host exception type; Original throw
RTTI, register return residues and exception-frame ABI remain outside coverage.

## Lifetime boundary and current Original evidence

The destructor prefix supplies the five DWORD writes in Original order: +0,
+10, +24, +170, +1E4. It ends before the E0AF20/E188A8/game+1FE4 session-release
decision. Neither the profile writes nor the later withdrawal replace the
00779780 message owner, 0077C2A0 routing, or their actual EH lifetime.

The separate withdrawal stage starts at E409 only after that actual message
arm succeeds or legitimately skips. F87194 is the list object itself, not a
pointer publication or a synthetic membership cache. The stage creates the
real root-value cell and calls concrete 77BEA0. That leaf loads header+4 before
capturing the target DWORD, then loads the first node. It keeps that initial
sentinel as the loop terminator while re-reading the current header+4 for
iterator checks. Every matching 0Ch node is unlinked in the Original order;
next/previous are reloaded before the second link store. Only after real CRT
free returns does it reload header+8 and decrement modulo32. Nonmatches and
duplicate matches retain their Original traversal behavior. The target cell
may be in the first removed node because its value was already captured once.

All five returning invalid-parameter sites call the accepted concrete
`invoke_native_invalid_parameter_00bf6713` entry. Its actual SDK
`_invalid_parameter_noinfo` policy remains qualified: this does not establish
Native BF66EF encoded-handler selection, debugger/Watson behavior, fault or
return addresses. No callback-only service, repaired header, null-list default,
membership set or count clamp is introduced. The caller still supplies actual
live raw allocations, the canonical list, and its synchronization domain.

The list destructor continuation starts only after the real session-message
arm, F87194 withdrawal and +2B0 observer destruction. Withdrawal now has a
separate concrete stage; observer destruction is still required before entering
this list stage. Its end precedes the recon-array destructor iterator and the
GameEntity/SceneNode destructor. Root scalar flags/free, embedded +310 tick
unlink and current-subnode deleting dispatch also remain outside this stage.

Fresh typed metadata verifies `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The complete live/file-backed Original bodies are
77EED0..77F08D (446 bytes), 77E380..77E490 (273), and 77E810..77E82D (30), all
with `no_return=false`. The current complete destructor listing includes the
post-free continuation through E490. Its old E441 truncation comment is
historical; it did not justify a listing repair. No listing repair was made.
The actual recon destructor call at E46C targets **BF7C6E**; BF7D69 is not that
call's target. The scalar wrapper calls E380 at E813 and BF6989 at E820 when
flags bit zero is set. This packet does not replace that wrapper.

805A70/804300 now have the qualified raw Source primitives above. 925CE0 and
928630 were claimed when the existing queue reached its turn. This bounded
packet supplies only 925CE0; 928630 remains the next queued frontier. Its
semantic Source interface is not treated as an actual raw constructor.
Observer destructor 006E0860 remains leased to
argon-v2-seg33 and is separately queued; the accepted raw 6952A0/695870
dependencies alone do not establish that missing owning composition.
Remaining root gates include their concrete owning
composition, actual observer/weak/string/entity-ID/pending domains, World and
hierarchy publication, actual Game/network/dirty bindings and scratch frame
provenance, actual session-release ownership and observer destruction, callable World
and effect teardown, complete scalar/exception ownership, genuine derived
factory, current tick methods and remaining-job dispatcher. Existing canonical
AA0/F878CC process identity is unchanged. Full receiver activation stays false.

## Scene-base construction

`construct_native_unit_owner_scene_base_00925ce0` covers the complete normal
532-byte body on the same actual 170h root prefix. It initializes the observed
endpoint at +0 and callback endpoint at +10, then calls the existing concrete
weak-owner domain on actual +24. The caller must have established the atomic
reference-count lifetime in the same +28 storage; no semantic class overlay,
private handle pool or replacement weak identity is introduced.

After weak construction, the body captures actual D7A24C and installs the
SceneNode profile words. It clears the child fields in original order and
calls the existing exact x87 `004134F0` copy to +74. It clears the two string
headers, captures D7A24C again, writes the physical flags/scalars, and makes
the second distinct x87 copy to the **same** +74. Raw `0041DD40` receives
actual +154, zero length and preserve=false through `NativeStringRawPoolContext`.
The emitted code preserves the following current-data/current-length memcpy
arm. Its null-source CRT/fault behavior is not dynamically qualified. Final
field writes preserve their DWORD/BYTE widths and original order.

No world, parent, sibling links or other untouched fields are defaulted.
In particular +30..+47 retain the caller's preimage. The numeric profile
words do not make a table callable in Source, and successful construction
alone does not make normal SceneNode destruction safe without world ownership.

Native handler CA6CBA selects FuncInfo DD9EE4, whose five entries at DD9F08
unwind in reverse: string +15C (CA6CAC -> 41DD20), string +154 (CA6C9E),
weak +24 (CA6C93 -> 925540), callback +10 (CA6C88 -> 695870), and observed
root (CA6C80 -> 695760). Five Source guards use those real children and
preserve ownership at throwing call boundaries. Their host state scheduling
does not establish Native FH3, asynchronous-fault timing, stack or exception
identity. No failure was injected, and cleanup throwing during unwind remains
unqualified. `928630` registry selection, full requested ID, callable +148
dispatch, pending-init publication and enclosing lifetime remain separate.

## Scene-base retirement continuation

The actual scene-base destructor is `00925780..009258C2` (323 bytes).
`00928630` is the GameEntity **constructor**, not this destructor. It remains
queued independently. This packet supplies the exact 19-byte profile prefix
and 188-byte normal continuation after the child loop; **the whole destructor
and complete scene-base lifetime remain incomplete**.

The intervening call `009257CF -> 00740270` always reaches the actual
E1AEA0 life-reference registry, even when root+168 is null. Both 740270 and
its 7400C0 array-erasure dependency remain held by `argon-v2-seg41` and are
queued as `cc12_unit_receiver_life_ref_retirement`. No null default or no-op
release replaces them. The following loop reloads current child count +50
and head +48 and calls the real child's current scalar slot0 with flag1.
Its concrete child dispatch also remains required; no callback substitutes it.

The admitted continuation unconditionally reads actual root.world +30 and
world+4 before concrete 903F30 unlink. It then reads current parent +3C.
A parent uses actual parent+48 for 924710, then clears +3C. Parentless flow
reloads world, uses the header pointed to by world+8, and only a nonzero +B8
reloads world again to remove root from the distinct embedded world+0C
count/head/tail list through real 4845A0. The parented arm does not remove
from that unit list. No world or hierarchy storage is synthesized.

Normal cleanup then returns name +15C, name +154, destroys actual weak +24,
callback +10, and observed +0, in that order. The raw string helpers capture
the original data/length before the actual pool getter; headers remain
unchanged. The same established atomic +28 and handle identity remain in
use. Root allocation, name header bits and weak-handle field are retained.

Handler CA6C72 selects FuncInfo DD9E98 with five entries at DD9EBC.
Its reverse cleanup owners are string +15C (CA6C64), string +154 (CA6C56),
weak +24 (CA6C2F), callback +10 (CA6C08), and observed root (CA6C00).
Five Source guards enter with those owners and disarm each member before its
normal destructor. The emitted code preserves ownership at throwing calls;
host guard-bit optimization across nonthrowing unlinks is not Native fault
timing. The Native nullable adjustment in two funclets does not relax this
entry's requirement for a real nonnull constructed root. Failure execution,
cleanup throwing and Original FH3/SEH/frame behavior remain unqualified.

## GameEntity constructor dependency frontier

At integrated main `3ad814e99`, the complete `00928630..0092878F` constructor
is 352 bytes and 81 instructions. Fresh live Ghidra and installed-PE bytes agree.
The actual raw scene constructor `00925CE0`, adjacent ID-table owner pair,
`00928240` SceneRegistry lock getter and `00926BE0` pending-list producer now
have concrete Source providers. The older readiness document's statement that
these registry owners were unresolved is historical, not their current status.
No semantic table, deduplicating queue or substitute manager is needed for them.

The original sequence still matters. Successful construction of the same root's
scene prefix precedes the transient `CFF378` and final GameEntity profile stores.
The actual SceneRegistry `+4` section is captured, entered and incremented at
`+18`. Only the **low byte** of the first DWORD selects primary `F89A08` versus
alternate `F89A5C`; `0x100` selects alternate. The full second DWORD reaches
`009517C0`, whose AX is written to root `+174`. The constructor then invokes
the actual `+148` slot with `(1FF,9)`, writes the nine holder DWORDs and current
`F876A4` bits, and enqueues the same root through `00926BE0` while the captured
outer scene lock remains held. It clears `+1D4`, decrements depth and leaves.

Live profile cells establish `D192E0+148 -> 00927D20` and
`D192E0+5C -> 0047F190`. The complete 34-byte class-query leaf already has a
concrete Source provider. The complete 299-byte role method, however, performs
a current `+5C` call for every selected role and ultimately invokes
`00696350(root,event=5,value=0)`. Fixed constructor value nine does not remove
that notification. The existing event-word dispatcher requires an actual
callback-owner `+0C` provider through `ObserverEventWordCallbackAccess`; the
current Source tree contains the interface and consumers, but no concrete
provider that closes this constructor's dispatch domain. A dummy callback,
omitted notification, or assumed permanently empty endpoint list is not an
admitted replacement. The general role method's session-message and `+2C`
branches are also not supplied by this packet.

The exact constructor cleanup map is now recovered, rather than inferred from
its normal state stores. Handler `CA6FAE` loads FuncInfo `DDA3B8`, magic
`19930522`, maximum state three, with unwind map `DDA3A0`:

| State | Next state | Funclet | Actual cleanup |
| --- | --- | --- | --- |
| 0 | -1 | `CA6F90` | Root from `[EBP-18]`, tail to full `00925780` |
| 1 | 0 | `CA6F98` | Same root `+178` name, tail to `0041DD20` |
| 2 | 1 | `CA6FA6` | Actual guard at `[EBP-14]`, tail to `00411EE0` |

The name and captured-guard cleanup providers exist. Complete `00925780`
still requires held `740270/7400C0` and the actual current-child scalar-delete
loop before the admitted retirement continuation. Consequently, successful
normal fragments alone cannot establish constructor failure ownership. Native
FH3/SEH execution, reentrant effects and original frame/entry ABI remain unproven.

This audit verified three full spans (685 bytes, 183 instructions, twelve call
sites), six constructor EH records and both profile cells against live bytes and
the installed PE. It adds no split API exporting a held lock, Source stub, or
reconstruction ledger credit. The twenty existing components and their strict
Win32, 77-check retirement and 3/3 build receipts remain the frozen `deb673fe9`
evidence; this documentation-only audit does not rerun or relabel them as new
constructor execution. `928630` stays queued, as do the separate life-reference
and `6E0860` observer-owner dependencies.

## Validation and integration

Strict MSVC Win32 `/std:c++20 /EHsc /O2 /MD /W4 /WX /fp:strict` compilation
passed. All twenty public Source schedules, the shared count helper and the
recon and scene-base Source unwind guards were
inspected for the ordered
mixed-width writes, initial-word capture, current-head reloads and real free
calls. A manifested isolated comparison links the new production object,
the actual head-producer TU and the existing concrete CRT allocation domain.
The original packet's seven complete live spans match the PE. The peer packet
adds seven freshly compared bodies (one overlaps the constructor) and twelve
matching live/file EH records; those record bytes do not prove Native exception
execution.

The focused comparison executes copied Original normal fragments/leaves with
their real internal calls, redirects only the external allocator/free and
borrowed global address, and supplies private register/EH-state scratch. It
compares a nonzero 320h raw preimage byte-for-byte after both construction
stages and list teardown, normalizing only allocated sentinel identities.
It also checks retained self-links, empty/two-node lists, null/non-null holder
payloads, and initial-word capture before an aliased store: **20 checks, zero
failures**. These are synthetic storage fixtures, not constructed receivers.
This prefix fixture was relinked against the extended Source object and still
passes all 20 checks.

The peer fixture executes copied Original 77EB50 and the normal tail with its
internal calls retained. It redirects only external allocator/free calls and
the three borrowed global operands. A populated first insertion and a second
peer0 tail insertion compare list topology, payloads, counts, untouched root
bytes and dirty state. The opaque two-byte scratch preimage is read from the
Original-created holder node and then supplied to Source, rather than guessed.
Both skipped tail branches, near-bound and wrapped count growth, and null and
non-null scratch cleanup are covered. Source-only owning count exceptions and
one second-count failure check cover retained first-list effects without
rollback. Result: **37 checks, zero failures**. The second-count failure keeps
the unlinked peer allocation, as the Original map prescribes; no Original throw
path was replayed. No allocation-failure injection, Native exception execution,
invalid-list/fault behavior, original register ABI, startup or gameplay is
claimed. Both fixtures operate on synthetic storage, not constructed receivers.

The withdrawal packet additionally compares two complete live/file bodies:
77BEA0 (156 bytes) and 77E380 (273 bytes). Its manifested copied-Original
fixture retains the internal E417 -> 77BEA0 call, redirects the actual global
operand and external free/qualified invalid-parameter boundaries, and stops
each fragment before the next lifecycle dependency. Nonzero root preimage
bytes match after the five profile stores and remain unchanged by withdrawal.
List topology, retained sentinel payload/header bytes, duplicates, all matches,
no matches, an empty list, target capture from a subsequently freed node, and
modulo32 count decrement pass **20 checks, zero failures**. Returning handler
execution, malformed-memory faults, Native exception execution and the actual
global/game receiver remain untested. The previous peer and prefix fixtures
were relinked with this extended object and still pass **37 and 20 checks**.

The recon packet compares complete live/file bodies for 805A70 (122 bytes),
804300 (14), and accepted observer child 4B7EF0 (88), plus five EH/thunk records.
Its copied Original retirement leaf remains connected to the copied observer
child; only the admitted 6952A0/695870 boundaries use the same concrete Source
graph. The fixture uses actual raw manager publication, canonical observer
edges/endpoint arrays, a real Win32 critical section, and canonical manager
teardown. A nonzero 34h preimage checks return identity, exact field widths,
padding and owner/index preservation. Empty and populated retirement check
repeated pair references, a second edge, dispatch invalidation, retained target
bits, endpoint counts/capacities and balanced locking. Only allocated array
pointer identities are normalized. Result: **20 checks, zero failures**.
The constructor normally clears its target before reloading it; this fixture
does not execute the non-null reload arm or its Source unwind guard. Native
exception execution, fault behavior, array iteration and actual game binding
remain untested. Earlier packet results remain in their frozen receipts.

The scene-base packet compares complete live/file bodies for 925CE0 (532
bytes), matrix copy 4134F0 (103), and weak construction 925490 (173), plus
eight EH records. Its copied Original constructor calls the copied matrix
body; weak construction and raw resize bind to the same admitted concrete
Source domains. The weak body is byte-audited, not copied-executed. The fixture
uses actual caller-owned weak-pool storage, a shared allocator list, real
singleton and recursive-lock storage, and the raw string context. Full 170h
bytes are compared with only the allocated weak-handle pointer normalized;
the handle's real target, reference counts, balanced lock depths and retained
external weak reference are separately checked. Actual constant 1.0 and a
masked signaling-NaN stress pattern compare x87 results/status. Result:
**30 checks, zero failures**. Exception execution, the normally skipped memcpy
arm, unmasked x87 faults and Native entry/frame ABI remain untested.

The retirement packet audits all 323 parent bytes and eight EH records against
live Ghidra and the installed PE. Its fixture executes only the copied 19-byte
profile prefix and 188-byte continuation. Real raw unlink/list/pool/weak/
observer Source children are bound; the held release and child loop are not
executed. The same 170h root with established atomic +28 and actual caller-owned
World/parent/neighbor/list storage is reused for both comparisons. Parented,
parentless-listed and parentless-unlisted cases compare complete prefix and
post-retirement bytes, both chain topologies, conditional first-match unit
removal, preserved pooled-name headers and returns, retained external weak
reference, populated cross-endpoint edges and dispatch invalidation, and
balanced recursive locks. Only allocation pointer values are normalized;
root, World, parent, neighbor and endpoint identities are retained. Result:
**77 checks, zero failures**. Full constructor/destructor lifetime and Native
exception or child-dispatch execution are not established by this fixture.

After syncing pushed main 90BD7AFDC, the worker's `scripts/build.ps1` Release
build includes the permanently registered TU and passed all three configured
tests. The new object is also separately strict-compiled and directly linked
to the freshly built core, Lua and zlib libraries and the application's Windows
libraries for the manifested comparison. Earlier fixtures remain preserved in
immutable D318001E7 receipts and were not rerun for these additive entries. No
CMake edit was needed. See `reports/native_unit_owner_entity.json` for exact
validation status and pins.
