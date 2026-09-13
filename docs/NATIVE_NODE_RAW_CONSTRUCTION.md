# Native node raw construction

Addresses: `00B6F5A0..00B6F8CD` (814 bytes).

This extends existing `construct_native_node_00b6f5a0` with actual name-header,
raw string-pool context, and current constant-cell inputs. It adds **zero native
bodies**. The previous typed overloads and their established callers remain
unchanged. This is future BD work, separate from the initial BC publication.

| Routine | Coverage | Original ABI | New interface |
|---|---|---|---|
| B6F5A0..B6F8CD | Complete schedule within the explicit lifetime/provider domain below | ECX actual slot; stack actual8h name header; EAX original slot; RET4 at B6F8CB | slot, byte extent, actual header, NativeStringRawPoolContext&, NativeNodeRawConstants const& |

The three constants bind actual volatile DWORD cells CE4970, D7A24C, CE4ADC.
Their reference bindings and the raw pool context must remain stable and
accessible outside the bytes construction writes. There is no constant snapshot,
semantic pool adapter, new singleton, or floating conversion before MOVSS stores.

## Native schedule and actual storage

B6F5A0 itself is the producer of the existing 174h prefix. Store profile CEB130,
count +4=1, then capture CE4970 at B6F5D5. Write profile D62C88, clear hierarchy
+30/+34/+38/+3C/+40 and mask+48, and store captured XMM0 at +50. Header +54/+58
initialization follows at B6F606/B6F608. Clear +A0/+A4/+A8/+130 and descriptor
+164/+168/+16C, then +170 before entering the name operation. No field at +174
or later is written. Bytes 08..2F, 45..47, 135..137 retain their allocation
preimages except for explicit modifications by an aliased external name service.

The B6ED70 allocator replaces its incoming ECX=174 with actual pool0108FF58 and
tail-forwards to B6EB00. Its existing producer supplies payload174h plus trailing
slot ID, stride178h. Direct derived constructors forward actual ECX; in the
camera case the prefix belongs to the actual458h object inside its45Ch slot.
The host NativeCameraOwner is an external owner/binding, never a substitute raw
458h object. B90800 callers use their actual010903F0 owner; their larger derived
storage is outside this prefix's claimed writes.

The C++ aggregate contains atomic and NativeString subobjects. Raw byte stores
through B6F5FA run before beginning NativeNodeStorage lifetime. At the native
name-header initialization boundary, isolated placement default construction
starts lawful aggregate/member lifetimes. Strict MSVC14.51 Win32 COFF inspection
shows exactly a26-byte helper: load slot; zero DWORD+4,+54,+58; RET. The final baseline /Oy- object is29 bytes with an EBP frame and the same three
slot stores. No matrix, padding, scalar or other native-unwritten byte is initialized. The caller restores
+4=1 before any external operation or next current-constant read. The intended
+54/+58 zeros remain. No whole-slot snapshot/restoration is used.

This interface excludes observation of the transient +4 zero/restore, additional
materialization store count, asynchronous/fault-time observations and private
frame identity. It requires accessible aligned storage and does not claim native
binary object lifetime, access-violation timing or identical native store counts.

## Name assignment and current reads

Compare the actual header address against destination+54. Self-alias skips name
allocation after the native header zeros. Otherwise capture source.length and
call current raw41DD40 with preserve byte1. After return, reread source.length;
if nonzero, separately read destination.length, source.data, destination.data
in that order, and copy exactly destination.length bytes, excluding terminator.
The real raw resize writes the terminator. Current source length is a branch
predicate; it is not substituted for the destination copy count. A zero-byte
CRT copy alone is omitted after all current reads, matching the existing raw
string interface's zero-copy boundary. There is no length clamp or rollback.

The existing41DD40 raw overload resolves actual419CC0 before allocation and
nonnull return, then composes BD1120/BD1510. The getter's publication and raw
14h lifetime manager are the supplied actual cells. Getter exceptions propagate.
BF7680's reviewed body includes forward/backward overlap handling, a zero-count
return, and a separate large aligned SSE route at C0C82B. Production uses the
current CRT memmove byte-result contract; original CRT instruction/exception
identity and its private state are not newly reconstructed.

| Native site | Callee | Preparation and cleanup |
|---|---|---|
| B6F656 | 41DD40 | ECX actual+54; current source.length then preserve1; RET8 |
| B6F66B | BF7680 | destination, current source pointer, current destination length; ADD ESP,0C atB6F670 |
| B6F6F9 | 4134F0 | ECX actual+B0; first fresh stack matrix; RET4 at413554 |
| B6F783 | 4134F0 | ECX actual+F0; second fresh stack matrix; RET4 |
| B6F82E | 4134F0 | ECX actual+60; third fresh stack matrix; RET4 |

## Six current constant reads

| Read site | Actual cell | Capture and consumers |
|---|---|---|
| B6F5D5 | CE4970 | XMM0 to+50 before header initialization |
| B6F676 | D7A24C | XMM1 to+4C,+AC, first fresh identity; +5C zero follows scalar stores |
| B6F701 | D7A24C | New XMM1 capture after first x87 copy; second identity |
| B6F78B | D7A24C | New XMM1 capture after second x87 copy; third identity |
| B6F836 | CE4ADC | XMM1 retained for+14C/+150/+154 before sphere zero stores |
| B6F860 | CE4970 | New XMM0 after+13C/+140/+144 zeros; reuse for+148/+158/+15C/+160 |

Each identity writes sixteen DWORDs in native row order, with captured diagonal
bits and XORPS zero off-diagonal bits. Existing4134F0 performs sixteen sequential
x87 FLD/FSTP pairs. The three copies remain distinct and ordered+B0,+F0,+60;
MOVSS preserves raw signaling-NaN words while x87 may quiet them. No cached one,
float assignment, bulk matrix copy, or cross-call constant reuse replaces this.

After the third copy, capture the negative cell, set+138=40, zero the three
sphere coordinates, then read positive again. Finish +44=0, +134=1, mask+48=FFFFF.
Native profiles, constants and fields are hypotheses tied to numeric producers,
not recovered C++ type names.

## All 17 incoming preparations

Full live containing bodies and extended preparations are retained externally.
Every row calls B6F5A0, whose RET4 supplies the one-stack-argument cleanup.
Register provenance was checked from complete containing listings, including
EBX=0 at8680D2 and the72E954/72E96D edge, and EBP=0 atADD57C/B91E50.

| Containing function | Call | Actual slot producer | Actual header producer |
|---|---|---|---|
| 00b75030 | 00b75038 | incoming ECX actual derived slot | original stack header forwarded by B75030 |
| 00b71a80 | 00b71aa7 | incoming ECX actual camera slot saved ESI | original stack header captured at B71A98 then pushed |
| 00b8f5e0 | 00b8f5e8 | incoming ECX actual derived slot | original stack header forwarded by B8F5E0 |
| 00b91d70 | 00b91e9e | B91E45 calls B90800 with ECX=010903F0; EAX->ESI; null guard | actual stack header +18, zero then resize12/preserve1 and copy D636C0 including NUL |
| 00b7c4c0 | 00b7c4c8 | incoming ECX actual derived slot | original stack header forwarded by B7C4C0 |
| 008680b0 | 008681b5 | 86819D calls B6ED70 with ignored ECX=174; EAX slot; EBX=0 from8680D2 | ESI original stack argument captured86811F; actual template+1C header pushed; ESI untouched until call |
| 006ecbe0 | 006ece6c | 6ECE31 calls B6ED70; EAX->EDI; null guard | 41E870 constructs actual stack+1C from current [[ESI+3F4]+54] C string |
| 0072e6d0 | 0072ec68 | 72EBEE calls B6ED70; EAX->EBP; EBX=0 on72E96D jump from72E954 | actual stack+34 header; current [[ESI+3F4]+54] captured EDI; byte scan72EC20..27, resize/preserve1, length+1 copy |
| 00ade820 | 00ade8bf | ADE885 calls B6ED70; EAX->EDI; null guard | 41E870 CF00D8 into actual stack+14 header |
| 00add3f0 | 00add5e4 | ADD583 calls B6ED70; EAX->ESI; EBP=0 atADD57C; null guard | actual stack+28 header; clear, resize24/preserve1, length+1 copy CF00D8 |
| 00ade0e0 | 00ade182 | ADE141 calls B6ED70; EAX->ESI; EDI=0 atADE14F; null guard | 41E870 CF00D8 into actual stack+20 header |
| 00b6f8f0 | 00b6f928 | B6F90F calls B6EB00 with ECX=0108FF58; EAX slot; null guard | incoming ECX source saved EDI; actual source+54 header |
| 00b71d10 | 00b71d3e | incoming ECX destination saved ESI | original stack source saved EDI; B6D800 atB71D36 returns source+54 actual header |
| 00b866c0 | 00b866f6 | B866DB calls B6ED70; EAX slot; null guard | actual header from current stack+14 atB866EF, pushed unchanged |
| 00b743c0 | 00b743c8 | incoming ECX actual derived slot | original stack header forwarded by B743C0 |
| 00b8f960 | 00b8f99e | B8F97F calls B6ED70; EAX->ESI; null guard | incoming ECX source saved EDI; B6D800 atB8F996 returns actual source+54 header |
| 00b91c40 | 00b91c80 | B91C5F calls B90800 with ECX=010903F0; EAX->ESI; null guard | incoming ECX source saved EDI; B6D800 atB91C78 returns actual source+54 header |

B6D800 is the verified four-byte LEA EAX,[ECX+54]; RET leaf. These source headers
are borrowed actual storage; no temporary NativeString owner is introduced.
The raw C-string41E870 body is an existing separate BC provider and is not copied
or counted in this packet.

## Consumed cleanup and remaining composition

Handler CC1A31 references FuncInfo DFA93C and unwind map DFA924. State2->1 invokes
CC1A23 on+164: B6F3E0 performs B6EC70(0), then BF6989(begin). This constructor's
supported fresh descriptor is exactly null/0/0 and must remain so throughout the
name service. Its failure path consumes that state and calls the current shared
free(nullptr). A foreign or corrupted descriptor is rejected; no arbitrary
pointer free, invented point-light retain, or new B6F3E0 body is claimed.

State1->0 invokes CC1A18 on+54 through raw41DD20 once. That captures nonnull data,
then current length+1 with DWORD wrap, resolves the current getter and returns
through BD1510 without clearing the header. State0->-1 invokes CC1A10 ->AA6E10
->BD30F0, publishing D5C104 then CEB130. The source ends the typed lifetime after
these stores. If raw name cleanup itself throws, its state is already consumed;
base cleanup still runs once and the replacement C++ exception escapes.

No physical slot return belongs to B6F5A0. Native FH3/CRT/nested-exception identity,
unmasked FP traps, transient fault states and malformed descriptor recovery
remain outside this interface. Full raw destructor, camera forwarding/current
constants, camera-name construction and persistent environment lifetime require
their separate packets. The same raw context must outlive later required name
cleanup; this change does not publish a complete cockpit camera holder/service.

## Verification and external capture

The native capture compares14 live/disk spans totaling2094 bytes, including the
complete814-byte parent,103-byte x87 copy, raw resize/release, CRT copy body,
three constant cells, handler, cleanup thunks and FH3 metadata. There are17
incoming preparations,5 outgoing call rows and6 consumed-cleanup call/tail
rows; the final live mechanical report check passes28 rows. No Ghidra mutation, ledger update, CMake change or shared test was
made. Accounting remains one existing typed body extension, zero new bodies.

Strict Win32 /W4 /WX /O2 /MD /fp:strict compilation passed, including the exact
materializer store check. Baseline build, existing two CTests, eight seed checks,
and the focused external fixture outcomes are recorded in the JSON report.
The fixture compares original814-byte parent plus unchanged original103-byte
matrix copy with current rebuilt providers. Its only parent relocations are
five call operands and six constant operands; original native FH3 is unexecuted.
The returning real raw-manager validation callback and alias-bound constants
check callback-visible preimages, separate current reads and name header reloads.
A source-only throw from the same real validation path checks consumed cleanup.
These checks are fixture evidence, not a game run, complete native EH or binary ABI.

The immutable capture lives under
`C:/Users/sqz269/bsp-bd-node-construction/worker_capture.zip`; the JSON records its
hash and complete manifest. The default `run.ps1` compiles only external probe.cpp
and links the selected root's three current libraries. It cannot silently compile
the worker implementation or use cached libraries. The archive includes its
original spans, frozen sources/includes, three libraries, probe/recipe, strict
object/disassembly, native/caller evidence, state bytes and logs. /MANIFEST:EMBED
is explicit. Archived inputs permit historical inspection; current-library replay
still requires a root whose build contains this overload.

A separately hashed `final_verification.zip` records the six additional cleanup
rows and the final baseline29-byte materializer observation. It changes no sealed
source, library, probe, recipe, or runtime input. Both archive hashes are in the report.

## BD integration checkpoint

The integrator reviewed the complete native body and actual producer/provider
evidence, saved its original analysis signature and full stored range in the
existing BSP project, and registered the source where needed. Exact combined
validation follows separately from worker checks. No original binary entry,
unrestricted FH3/SEH, whole owner lifetime or gameplay claim follows.

## BD exact merged validation

Exact combined source `4548c163d4f97d227fdd7a2e411df1f9fd81ded7` passed the strict Win32 build and both
existing CTests. Both probes compile only external probe.cpp against the three
current libraries. Constructor includes one original/source pair plus a separate
source-only throw; destructor checks current providers on a source exception path.
The final constructor object is checked inside the built library for its three
materialization stores and immediate reference-count restoration. See `reports/native_node_lifetime_bd_validation.json`
for immutable captures, hashes and explicit limits. Earlier pending statements
describe worker stages. Raw-only runtime construction, camera context forwarding,
native FH3/ABI compatibility and gameplay remain open.
