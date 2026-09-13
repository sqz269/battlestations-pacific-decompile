# BA discovery: the singleton previously named FileStoreService

This is read-only discovery, not a reconstruction or a startup integration. The
existing `BSP_FileStoreService_GetSingleton` name at **0051F460** is provisional
and is not supported as an archive-service identity by the inspected bodies.
The strongest current interpretation is a **debug-feature configuration owner**:
0051FA50 displays `No DebugFeatures` using BE8200/owner+1C and `AI Debug Render`
using owner byte+2C. World-render callers also gate debug drawing on that byte.
No Ghidra name or shared ledger was changed by this packet.

The machine-readable evidence is
[native_filestore_service_ba_discovery.json](../reports/native_filestore_service_ba_discovery.json).
It records original ABI, complete byte spans/hashes, every direct/import call
in the proposed bodies, source binding pins, cleanup maps and uncertainty.
All eight proposed body spans match the installed PE and live Ghidra bytes.

Implementation follow-up corrected the mechanical call schema: all26 original
rows now also carry `address/native/function`, so the verifier consumes24 direct
rows and reports2 explicit Win32 imports. Primary subsequently repaired the
BE9560 returning-free flow and recreated its complete156-byte body. Both
reports now pass all24 direct rows; the old site/operand fields alone were
not a successful mechanical call audit. Prior failures remain recorded.

## Target and evidence boundary

Worktree `agent/orch2-filestore-service-ba`; packet
`orch2_filestore_service_ba_discovery`. Analysis used the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32, image base
00400000. Each `bsp.py ghidra` command verifies the configured project and
program before reading. The installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Only this document and its report are deliverables. Original installation,
Ghidra analysis, shared exports, C++, CMake and ledgers remain unmodified.

## The bounded owner

0051F460 returns publication **0109DB70**, allocating **34h bytes** when absent.
It captures the first lifetime manager's critical section at +10, enters it
and increments its physical +18 counter, then rechecks publication. Allocation
BF681B and constructor BE94F0 precede publication. It fetches the manager again,
reloads the published owner, and calls BD0C30 even if the allocation returned
null. It releases the originally captured section and returns the current
publication. The fast path returns its first captured nonnull pointer.

Constructor BE94F0 makes exactly one direct call: **41E870**, on owner+10 with
the nonnull empty C string **CE3A0C**. The current name `NativeString_Assign`
at that address describes a constructor; the existing actual source entry is
`construct_native_string_cstring_0041e870`.

| Offset | Bytes | Supported interpretation |
| --- | ---: | --- |
| 00 | 4 | Numeric profile **D68B94** |
| 04 | 12 | Native-string array `{data, signed count, signed capacity}`, all zero |
| 10 | 8 | Native string constructed empty |
| 18 | 12 | 88h-record array `{data, signed count, signed capacity}`, all zero |
| 24 | 1 | Active-feature aggregate byte hypothesis, zero |
| 25 | 3 | Untouched by constructor |
| 28 | 4 | Selected record index, initially zero |
| 2C | 1 | AI debug-render flag, zero |
| 2D | 3 | Untouched by constructor |
| 30 | 4 | Selected record pointer, zero |

Only **D68B94 +00 -> BE9600** is established as this owner's virtual slot.
The adjacent DWORD D68B98 contains BE9730, but that function clears a different
publication, **0109DB78**. It is not evidence of a +04 method on this owner.
The following `fonts/arial19.dat` literal is adjacency, not owner identity.

## Destruction and the missing tail

Ghidra currently ends BE9560 at **BE9599**, the first BF6989 free. Disk assembly
continues through **BE95FB**. The complete sequence is:

1. Resize the 88h-record array at owner+18 to zero through BE8F30.
2. Free that array's current backing pointer.
3. Return owner+10's captured nonnull native-string allocation to the shared
   sized pool, using its current length+1 captured before the getter call.
4. Resize owner+04's native-string array to zero through existing 427110.
5. Free that array's current backing pointer.
6. Clear publication 0109DB70 and install base profile **CE3818**.

There is no early profile reset or publication clear. BE9600 calls this body,
tests bit0 of the low flags byte, conditionally calls BF65AC on the captured
owner, and returns that original owner in EAX with RET4. Ghidra's apparent
incidental free return is disproved by the final `MOV EAX,ESI`.
The separate base cleanup BE8210 clears DB70 and stores CE3818.

Primary completed the locked BE9560 returning-flow/body repair and refreshed
all eight exports. BE8DF0 already spanned its full308-byte body; primary restored
its returning-free tail, which publishes the new backing and capacity. It has
no remaining call gaps. Six alignment bytes after an unconditional jump were
left untouched. This discovery packet itself performed neither repair.

The report records all cleanup-map entries. Getter state1 deletes the captured
allocation before state0 releases the captured guard. Registration failure
retains the published owner. Constructor cleanup runs owner+04 array cleanup
and base reset. Destructor unwind proceeds through owner+10 string cleanup,
owner+04 array cleanup, then base reset. These are source-behavior obligations,
not proof that a future C++ interface reproduces the original FH3/SEH ABI.

## Next implementation packet

Six owner bodies are unconditional candidates, **502 bytes** total. Two
container bodies add **462 bytes** only if the integrator accepts the game
container classification below: **8 bodies / 964 bytes** combined.

| Complete inclusive span | Bytes | Original ABI | Role |
| --- | ---: | --- | --- |
| 0051F460–0051F51C | 189 | cdecl, EAX owner, RET | Lazy singleton |
| 00BE94F0–00BE955C | 109 | ECX owner, EAX owner, RET | Empty construction |
| 00BE8210–00BE8220 | 17 | ECX owner, RET | Base cleanup |
| 00BE8350–00BE8350 | 1 | ECX unused, EAX untouched, RET | Compiled empty shutdown hook |
| 00BE9560–00BE95FB | 156 | ECX owner, RET | Full destruction |
| 00BE9600–00BE961D | 30 | ECX owner, flags on stack, EAX owner, RET4 | Scalar deletion |
| 00BE8DF0–00BE8F23 | 308 | ECX array, signed capacity on stack, RET4 | Conditional array reserve |
| 00BE8F30–00BE8FC9 | 154 | ECX array, signed count on stack, RET4 | Conditional array resize |

Suggested packet `native_debug_feature_owner_ba` owns new
`include/bsp/native_debug_feature_owner.hpp`, `src/native_debug_feature_owner.cpp`,
`docs/NATIVE_DEBUG_FEATURE_OWNER.md` and its report. These names are hypotheses,
not recovered symbols. The primary integrator owns CMake registration, shard
updates and any central profile-dispatch binding. Leases must be acquired for
the complete accepted address set; this discovery lease is not implementation
authorization for other workers.

BE8DF0/BE8F30 are **provisionally game-container template specializations**,
not identified STL functions. Their physical header stores signed element
count/capacity, unlike the adjacent MSVC STL begin/end/capacity pointers. Each
88h record is a game pooled-string header followed by 80h flag bytes. Reserve
deep-copies those fields and releases old game strings via 419CC0/BD1510,
matching the already reconstructed game string-array mechanics at 426520.
Its only unwind action CC6F70 calls **bare RET 401130**; it supplies no copied
record or backing rollback. Original template identity remains unknown. If
further evidence establishes library provenance, preserve the algorithms as
library contracts and isolate only the game element lifecycle; do not port
library code merely to close this owner.

Conditional reserve/resize must preserve signed comparisons, 32-bit wrapped
stride arithmetic, live count/backing reloads around callbacks, zeroing exactly
88h per new record, backward shrink with the count decremented before release,
and the reserve exception path's lack of rollback. A zero-count destruction
fragment is not a complete BE8F30 reconstruction.

Reuse existing actual raw **415350** publication, **BD0C30** registration and
**411EE0** guard destruction with caller-owned DB70/1090AA0 publication cells.
Do not substitute a new private singleton domain. Reuse actual 41E870, 41DD20,
427110 and the shared `ActualNativeStringPoolStorage` bindings. Existing 427880
has a mesh-specific source interface, so raw storage compatibility needs review
before directly reusing it for unwind. Preserve native padding and field timing;
do not place a callable C++ vtable over the numeric profile DWORD.

## Startup reachability and deferred work

The confirmed application edge is **shutdown**:
00738008 -> 0051F460, then 0073800F -> BE8350. It may construct this owner late
if nobody used it before shutdown. Existing source exposes only
`ApplicationShutdownHost::file_store_service_hook_00be8350`; that old hook name
does not establish archive behavior.

Live caller evidence also identifies debug-menu calls 0051FCAA/0051FCD1/0051FE51
and render checks 006F3CC9/009AC7C6/009ACD96. Disk call 0074B54B follows a test
of the game-global owner's +19C4 byte. Its body begins at currently undefined
**0074AF50**, not the preceding defined 0074AF20 thunk. No initialization chain
or parser bootstrap is inferred from that conditional call. The snapshot's
007CD5E0 caller is absent from live xrefs; that discrepancy remains unresolved.

**BE9100** is a separate, unreconstructed debug-profile text loader, with
`Profile`, `Flag` and `End` references and no live caller found. It uses named
scanner operations but also incomplete BEED60/BEEF60/BEF0B0 operations, record
append/copy BE8FD0/BE8D90, feature lookup BE9060, profile selection BE8820 and
active-state refresh BE85C0. BE85C0 obtains a separate lock singleton through
BE8360/publication109DB74. Those named dependencies do not become complete
because some neighboring string/scanner routines have been reconstructed.

No file/archive open, mount, load or startup bootstrap occurs inside the
bounded getter/constructor. Central mixed-owner deletion dispatch and executable
shutdown wiring are distinct integration work. This packet adds no C++, tests
or build claims; byte agreement is neither fixture behavior nor game validation.
