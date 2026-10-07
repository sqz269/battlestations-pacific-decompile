# Ordinary CEnum symbol-map lifetime (CC11)

The complete ordinary normal bodies are reconstructed through new MSVC Win32
interfaces. Symbol-map clear releases owning keys and returns actual14h slots;
the ordinary CEnum destructor additionally releases its owning type header.
Neither operation deletes mappedword+8 or frees the19Ch owner allocation.
The semantic PropertyLibrary, traffic loader, declaration identity and global
owner wiring are unchanged.

## Original bodies and bounds

All endings below are exclusive. Live saved-program bytes equal the installed
PE image. `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was verified
by each `bsp.py ghidra` batch. Whole assembly includes prologs and terminal RETs.

| Entry | End exclusive | Bytes | Decoded instructions | SHA-256 |
|---|---|---:|---:|---|
|008F4B60|008F4C49|233|67|1079eaa4aead384715ddc2f8e81f2cc70e85614b13be0e567951e3f11d2fc1fd|
|008F4E70|008F4EF4|132|37|26fd655fa6a6954eed562a9cbef47c154fa39b6749834a0ae5d7acfaa529715e|

`008F4B8D..008F4B8F` inclusive is an unreachable3B alignment instruction,
bypassed by `JMP008F4B90` at4B8B. The complete byte decoder counts it; Ghidra's
stored listing has66 instructions. No flow repair or Ghidra mutation is needed.
Observed ECX is the nested-map receiver for clear and the19Ch owner for the
ordinary destructor. Both end with RET, without an established semantic return.
These register/tail observations do not establish a drop-in class ABI.

## Clear and ordinary destructor order

Clear captures the receiver and walks64 heads at receiver+8. For each node it
reads current keydata+4 and captures next+C at4B95 before any release. A nonnull
key captures current length+1 and calls00419CC0/00BD1510 at4BA1/4BA8. Null data
skips key-length/storage reads. The slot then returns through the native inline
critical-section/depth/page/WORD-stack schedule: sectionE17584, depthE1759C,
node+10 page ID, tableE175A0, page+500 free stack/count580, earliestE175AC.
The next captured node is processed; each head is setNULL only after its entire
chain. Count receiver+4 is zeroed at4C3D only after all64 heads.

Source composes genuine `destroy_native_string_header_0041dd20` with complete
`return_native_enum_node_0043b0a0`, whose actual14h return schedule matches the
inline body. Header destruction captures current data and length+1, calls the
real getter and pool return, and leaves header bytes stale. `0041DD60` is an
interior address of resize0041DD40 and is not used as a destructor. Node mapped
word+8, header, chain and allocator-owned+10 remain stale and untouched.
Clear does not trim/free pages, delete mapped payloads or free the receiver.

Ordinary destruction stamps owner0=D16508, captures owner+4, calls first clear
at4EA1, then reads current data owner+118 and length owner+114+1. If nonnull it
calls00419CC0/00BD1510 at4EC2/4EC9. It stamps nested receiver0=D162C0 at4ED8 and
calls a second empty clear at4EDE. Statistics10C/110, inline name11C..19B and
stale owning114/118 bytes survive; the owner allocation is not freed. The full
native FS/unwind prolog and normal cleanup tail were inspected, but exceptional
cleanup and native EH/SEH are external. Source uses a new void interface.

There are six direct CALL rows, with path/containing-function attribution in
the report. Imported Enter/LeaveCriticalSection calls at4BB2/4C0B are separate.
Known ordinary caller008F59C3 lies inside scalar-deleting008F59C0; that wrapper
and its root-free/flags contract are not reconstructed here. Nested-map
adjustor/deleting callers at4D56/4D79 are also outside this packet.

## Connected Source fixture

`local/cc11_scene_enum_symbol_lifetime/run_probe.py` prepares a unique manifested
`lifetime_probe.exe`; all prior lookup, word, membership, pool and insertion
artifacts remain immutable and are hash-checked without rerunning them.
The fixture constructs a genuine raw8AD4A0 string pool and real raw38h node pool,
actual E188B4-compatible list with exactCE37A4→complete trim binding, and two
actual19Ch owners through008F4DD0. Installed global.enums supplies22
LandVehicleClasses and6 SoldierTypes symbols, inserted with genuine owning
NativeString headers and008F2850. Two additional empty keys have null data.
No hand-built dictionary nodes, fake profiles, enum ordinals or callbacks are
used. Checked free image-cell regions provide the verified distinct empty
cells and D162D4='E', read-only throughout; these are image-initial fixture
bindings, not captured live-game globals.

Standalone clear of the first owner returns23 slots without trimming, preserves
the second owner/keys, leaves all stale14h slot bytes and unrelated owner bytes,
and makes real membership missing. A repeated empty clear leaves the full string
pool prefix, node pool fields and page unchanged. Reinsertion uses a real returned
slot and is read by genuine case-insensitive membership/known-found word getters.
Each ordinary Source destructor runs once; real string-ring count increases
confirm nonnull key/type returns while empty keys skip. All64 page slots are
free before actual trim, shutdown and list unlink. Explicit fixture root frees
occur only after destructor checks; they do not bind native scalar008F59C0.

Fresh strict compile/link/run, current support provenance, whole COFF/manifest,
actual included compiler/project headers, live PE/code/provider bytes, installed
pre/copy/post hashes, fixture counts and immutable historical checks are pinned
in the report and unique run manifest. Primary owns the integrated CMake build.

## Admission and remaining contracts

Admit successful nonoverflowing coherent finite chains, unique valid aligned
allocated14h slots, stable disjoint owners/query storage, genuine current raw
string and node-pool providers, externally synchronized lifecycle with no alias,
reentry or concurrent mutation. Ordinary destruction is once per live owning
header; repeat empty map clear is admitted. No default, rollback, safe malformed
chain handling or double ordinary destructor policy is added.

Source/compiler/lifecycle fixture proof is separate from original execution.
The original bodies were not executed: private EH/global startup/cleanup,
scalar008F59C0, full original allocator/global/class ABI, mapped virtual payload
destruction, enum namespace/declaration identity, traffic and game remain
unbound. Existing providers use current CRT/OS contracts, without historical
CRT failure, fault, concurrency, or native ABI parity claims.
