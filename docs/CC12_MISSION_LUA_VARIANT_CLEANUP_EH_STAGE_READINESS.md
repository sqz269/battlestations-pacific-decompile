# Mission Lua variant cleanup: four EH action stages

The four selected handler/descriptor/map/action sets are fully byte-accounted:
**75 code bytes / 17 instructions and 176 selected data bytes**. Live Ghidra bytes
equal the original PE, and all 17 code starts are saved instructions. Each handler
is a physical 10-byte thunk without a saved function at its entry. Each action has
a complete saved body. No listing or function repair was performed.

The decisive distinction is the cleanup input: `00C82FE0` loads the current DWORD
at `[EBP-10h]`, while `00C830A0` loads the current DWORD at `[EBP+4]`. The latter
corresponds conditionally to the normal subtree owner's overwritten argument
slot, not to its captured EBP/EDI register values. `00C970D0` loads `[EBP-10h]`
and adds `0Ch`; `00C83080` forms `EBP-50h` without loading a pointer from that
address. A captured-receiver cleanup object would erase these distinctions.

This is an audit of cleanup readiness, not a Source implementation. The selected
tables and actions do not prove exception-dispatch policy. Native `006EE020`,
which two actions tail, remains a separately bounded dependency. Its metadata
only describes a 30-byte saved body; its bytes were not opened here. There is no
new Source, Original ABI, startup, runtime or gameplay credit.

## Exact selected code and data

The baseline is `f3bb494d7f222735cb62e5442345acbdf2df3fdc`. The configured project
is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 little-endian
32-bit at image base `00400000`; the live function count remains 64,729. The JSON
records complete code bytes, independent decoded instructions, saved instruction
contexts, physical offsets, whole-window hashes and the original-image identity.
The configured project file exists; no stronger server-reported absolute project
path assertion is made.

Root Astra subsequently read all 75 code bytes / 17 operations and all 176 data
bytes, independently matched every window and hash to the original PE, decoded
each instruction, and approved this evidence gate. The report pins that approval.

| Accepted normal owner | Handler, 10 bytes | Descriptor, 36 bytes | Map, 8 bytes | Action extent |
| --- | --- | --- | --- | --- |
| `006EE040` | `00C82FE8` | `00DB0F84..00DB0FA7` | `00DB0F7C..00DB0F83` | `00C82FE0..00C82FE7`, 8 bytes |
| `00884AF0` | `00C970DB` | `00DC946C..00DC948F` | `00DC9464..00DC946B` | `00C970D0..00C970DA`, 11 bytes |
| `006EE890` | `00C830A8` | `00DB10C0..00DB10E3` | `00DB10B8..00DB10BF` | `00C830A0..00C830A7`, 8 bytes |
| `006EE5D0` | `00C83088` | `00DB1094..00DB10B7` | `00DB108C..00DB1093` | `00C83080..00C83087`, 8 bytes |

Each handler is exactly `MOV EAX,descriptor; JMP 00BF6B43`. It changes EAX but
neither ESP nor arithmetic flags. Each selected descriptor consists of these
nine DWORDs: `19930522, 1, map, 0, 0, 0, 0, 0, 1`. Each map consists of two
DWORDs: `FFFFFFFF, action`. This fully describes the selected bytes. The usual
interpretation as one state-zero cleanup edge toward state -1 remains conditional
on the unopened interpreter's table and frame-delivery contract.

| Action | Entire physical instruction sequence | Direct memory effect |
| --- | --- | --- |
| `00C82FE0` | `MOV ECX,[EBP-10h]; JMP 006EE020` | One current DWORD read |
| `00C970D0` | `MOV ECX,[EBP-10h]; ADD ECX,0Ch; JMP 0041DD20` | One current DWORD read; 32-bit pointer addition |
| `00C830A0` | `MOV ECX,[EBP+4]; JMP 006EE020` | One current DWORD read |
| `00C83080` | `LEA ECX,[EBP-50h]; JMP 004072D0` | No data-memory read in the action |

None has a physical CALL, RET, PUSH, POP, state store, FS restoration, null test
or guard branch. The existing action return slot and ESP are delivered unchanged
to the target by JMP. EAX, EDX and nonvolatile registers are not rewritten by any
action. MOV/LEA/JMP preserve incoming arithmetic flags; the ADD in `00C970D0`
defines ordinary 32-bit addition flags. Target behavior is a separate contract.
Saved call-count metadata and pseudocode calls do not turn these tail jumps into
physical calls or recovered C++ signatures.

## Frame delivery and current-slot identity

Let `F` mean EBP supplied to an action. Let `S` mean the selected normal owner's
entry ESP from its already accepted audit. This packet **does not prove F=S**.
The accepted 54-byte `00BF6B43` adapter creates its own frame and passes descriptor
and stack words to unopened `00C07991`; that adapter frame cannot be substituted
for F. No interpreter body, action invocation ABI or runtime unwind was examined.

Subject to a future frame-delivery proof that maps F to the owner's S:

| Owner and normal publication | Selected cleanup input | Required distinction |
| --- | --- | --- |
| `006EE040` publishes N at `[S-10h]`; normal code retains N in ESI | Current DWORD `[F-10h]` | Read the slot at cleanup time; do not substitute normal ESI or entry N |
| `00884AF0` publishes P at `[S-10h]`; normal ESI later becomes size+1 on the pool path | Current DWORD `[F-10h]`, then add `0Ch` | The current slot supplies a header base even when normal ESI has another role |
| `006EE890` overwrites its original node argument `[S+4]` with V=`N+0Ch` before state zero and `00884AF0` | Current DWORD `[F+4]` | Use the overwritten, potentially aliased argument word, not initial K or captured N/V |
| `006EE5D0` constructs a temporary string at `S-50h` on its initial nonzero-flag path | Address `F-50h` | Address formation, not a pointer load or a heap-owner snapshot |

The three MOV actions capture their DWORD exactly once. Later target side effects
do not trigger a second frame-slot read in the action. Conversely, mutations
before action entry can change the delivered receiver independently of the normal
register's earlier value. The pointer ADD wraps modulo 2^32 and validates nothing.
LEA alone cannot fault by dereferencing the temporary; the subordinate destructor
can still fault when it reads that address.

## Accepted normal stages and a candidate Source cleanup policy

The following schedules are inherited from accepted normal-body receipts, not
new reads of those bodies. Older receipts' statements that these four handlers
were unopened remain historical. Their unrelated Source-absence statements and
large input-pin arrays are not promoted into current absence or build evidence.

| Owner | Accepted normal state and publication schedule | Cleanup-policy requirement still to implement |
| --- | --- | --- |
| `006EE040` | Full DWORD -1 at frame creation; publish local N; form `N+10h` before full DWORD zero; call `00884AF0`; late load/test `[N+8]`; full DWORD -1 before conditional size/pool work | Separate inner cleanup from outer buffer retirement; cleanup reads the current local slot if the state-zero action is selected |
| `00884AF0` | Publish P; stamp DWORD header while state is -1; full DWORD zero before `00886920`; late load/test `[P+10h]`; full DWORD -1 before size/pool work | If selected, clean current local-slot P's `+0Ch` string header; do not repeat the entire payload destructor |
| `006EE890` | State -1 during recursive `+8` branch; capture next link; publish V into `[S+4]`; full DWORD zero before `00884AF0(V+10h)`; late buffer read/test; full DWORD -1 before pool/free work | Preserve publication into the actual argument word and the late cleanup read from it; avoid captured-node cleanup or an invented recursive rollback |
| `006EE5D0` | Initial zero-flag path retains state -1 absent aliases. Nonzero path initializes and assigns the temporary string while -1; then stores current ESI, normally zero under compatible preservation, before `00411700` and the throw-helper path | Arm temporary cleanup at that actual stage. Assignment failure, constructor failure and throw-helper behavior have distinct boundaries; do not arm at function entry or destroy the temporary on the ordinary zero-flag path |

State stores and guard inputs are ordinary backing memory. Normal child writes,
frame/receiver aliases and exposed stack-slot aliases can change them. Full DWORD
state writes must not become independent Boolean fields unless a Source model
states and validates that separation. `006EE5D0`'s current-ESI state publication
must not be silently strengthened into an unconditional immediate zero store.
The actions themselves do not test state, advance it, disarm a guard or restore FS.

A candidate Source model therefore needs explicit live storage for the published
receiver/argument word, the temporary's actual storage, and the cleanup stage.
For the first three actions, it must load the live word when cleanup is selected.
It may then retain that just-loaded pointer across the target call. The raw string
header path also needs the actual retained pool publication, manager and gate
references. A copied pointer, synthetic pool or a destructor that always replays
normal teardown is not justified. Host C++ cleanup, hardware faults, Native
exception search/unwind, rethrow and a second failure during cleanup remain
separate policy questions; this packet chooses none of those policies.

## Subordinate cleanup and stack boundaries

`006EE020` has only metadata in this packet: saved extent `006EE020..006EE03D`,
30 bytes, 12 instructions, three blocks, three edges and two calls, to pool getter
`00419CC0` and return helper `00BD1510`. The current `undefined(void)` signature
is not an ABI contract. Its receiver fields, capture order, frame effects, return
cleanup and failure policy require a separate full-body gate. The two tail jumps
do not justify replacing it with either `006EE040` or a guessed string destructor.

For `0041DD20`, accepted Source provides two real header-destruction overloads.
Both first capture the current DWORD data pointer at header+4; null skips length
and release. Otherwise they read current DWORD length at header+0, add one with
DWORD wrap and release the captured buffer. They leave the header untouched,
including release-time changes. The `NativeStringStorage&` overload is noexcept;
the `NativeStringRawPoolContext&` overload calls the actual pool getter before
return-block, retains the actual gate reference, and allows getter exceptions to
escape. Those are different Source failure boundaries, not Native EH identity.

For `004072D0`, existing `NativeLegacySboStringStorage` is 28 bytes. The Source
destructor tests current DWORD capacity at +18h; capacity >=16 frees the current
heap pointer at +4. After that call returns, it writes DWORD capacity=15, DWORD
length at +14h=0, then one zero byte at +4. It does not initialize the leading
word or remaining union bytes. Its noexcept and host free service remain Source
policies. Both inherited Native string targets use plain RET; their bodies were
not reopened here. The actions add no arguments and do no cleanup themselves.

The accepted normal graph separately requires: `00884AF0` and `00886920` RET0;
`006EE890` RET4 for recursion and its container caller; `006EE960` RET14h for the
five-word raw-clear call; and the recovered `006EE5D0` continuation RET0Ch for its
three-word loop call. The normal pool pair has three pending words: getter RET0
then return-block RET0Ch in the inherited contract. Getter aliases can alter
those words before the return helper reads them. Native free uses caller cleanup.
No fresh target-body, stack-execution or Original ABI proof is added here.

## Cyclic graph and evidence domains

The accepted graph includes `006EE040 -> 00884AF0 -> 00886920 -> 006EE960`, with
the container selecting `006EE890` or a loop through `006EE5D0`; subtree and loop
paths reach payload cleanup again. This static graph does not establish that
recursive receivers share one identity, that the graph terminates, or that each
allocation has one owner. The new action data narrows cleanup-stage dependencies
but does not close that lifetime cycle.

| Accepted body domain | Boundary retained here |
| --- | --- |
| `006EE040` | Complete saved 98 bytes / 29 instructions |
| `00884AF0` | Complete saved 101 bytes / 29 instructions |
| `00886920` | Complete approved raw 168-byte / 59-instruction span; only 46 saved starts, 13 missing starts; no listing repair |
| `006EE960` | 201 raw bytes / 77 instructions; ordinary entry accounts for 200 bytes / 76 instructions; unlisted post-return NOP remains separate |
| `006EE890` | 150 raw bytes / 50 instructions; 46 saved starts; 11 reachable continuation bytes missing from the listing |
| `006EE5D0` | 637 saved bytes / 215 instructions plus separately approved 54-byte / 19-instruction continuation =691-byte candidate; following 13 INT3 bytes are diagnostic padding, not function credit |

None of these six normal bodies, either string target, `006EE020`, `00BF6B43` or
`00C07991` was reopened. The accepted shared adapter's nested historical capture
gate remains false, while its top-level full Root Astra assembly validation is
true; this report preserves both fields and does not rewrite that receipt.

The four inspected Source string files and bounded supporting receipts are pinned
in the JSON. The prior Source80 build is accepted context only: its 80 inputs and
four artifacts are not copied or replayed here, and no claim is made that those
artifacts remain current after another Root build. This packet runs no build,
test, probe or runtime unwind. It edits only this document and its JSON report;
it changes no Source, CMake, ledger, GPR state, function extent or saved listing.

Admission remains held on the `006EE020` leaf, interpreter/frame delivery and
failure policy, the actual selected object/pool lifetime, cyclic teardown and
Source composition, including the raw-versus-saved distinctions above.
