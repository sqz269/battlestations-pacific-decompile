# Native capability nested arrays

Four complete original entries now have raw MSVC Win32 source interfaces:
`B23120[96]`, `B25E60[133]`, `B29D40[248]`, and `B2AE20[116]`, totaling593 bytes.
They compose the complete raw primitive reserve and existing shared allocation
service. They do not implement the capability gather or a typed-vector adapter.

Each actual header contains three DWORDs: data, signed count, signed capacity.
Inner elements are12 bytes: format DWORD, four flag bytes, payload DWORD. Outer
elements are the inner headers. Every reached raw access must have a valid
extent/lifetime under the native wrapping arithmetic. Backing allocations use
the existing singleton lifetime shared malloc/free heap; no additional owner,
reference-count, callback, overflow check or malformed-storage repair is added.

The original entries take ECX actual header and one DWORD callee argument and
return with `RET4`. Copy assignment returns the destination in EAX. Public
fastcall declarations add an explicitly ignored EDX argument. These are new
source interfaces, not native caller or SEH replacements. The actual argument
slots remain caller-prepared callee storage rather than owner-field pointers.

## Ordered raw behavior

`B23120` grows capacity through full `B22B30` only when signed target exceeds
current capacity. For each added element it reads current base, writes format
DWORD0, then four individual zero BYTEs at+4..7. It leaves the entire DWORD+8
untouched. Growth's remaining count is captured before the loop; shrink
decrements current count repeatedly and finally publishes requested count.

`B25E60` first resizes destination0, then captures its source argument in EBX
and unconditionally reserves current source count. An initially empty source
therefore still reaches the primitive's minimum-one allocation. Only afterward
does it replace the actual source argument slot with its loop counter. Each
source-row pointer is captured before any further destination reserve and is
retained across that call. The append reads current destination count/base,
copies three DWORDs in order, increments current destination count, and compares
the saved iteration counter with current source count. There is no self-copy
skip or generic vector-assignment rule.

`B29D40` clamps its actual request slot to1 when needed, then captures the
clamped slot again. It reads current outer count after allocation, saves fresh
base/completed index, and deep-copies each current old inner header into a fresh
zeroed header. Each iteration saves the current target and arms state0 before
initialization/copy. After return it increments the index, compares current
outer count, disarms state, then stores completed count. The source keeps this
order rather than introducing rollback for completed inner buffers.

After copying, outer reserve walks current old headers again. It captures each
child address, resizes that child0, frees its current data, and rereads current
outer count. If that destruction loop was entered, it reloads the actual request
argument slot after the loop (`B29E0B`); otherwise the earlier capacity remains
captured. It then frees current outer data, reloads the saved fresh pointer, and
publishes fresh data followed by capacity. Outer count is not written.

`B2AE20` grows through full outer reserve, then initializes each additional
header to three zero DWORDs using current base. Shrink publishes the count
decrement before capturing the child address, resizing it0 and freeing its
current data. It rereads current count before repeating; final count publication
uses the original requested target.

All three literal entries preserve original stack scratch slots and instruction
widths. Fresh raw bytes include the saved-analysis gaps after free calls:
`[B29DFD,B29E0F)`, `[B29E17,B29E25)`, and `[B2AE83,B2AE8B)`. These contain
continuation, request reload and publication instructions. The worker made no
Ghidra flow changes; the primary handles the callsite repairs after lease transfer.

## Outer reserve exception/source boundary

Original `CBD327` loads FuncInfo `DF5AFC`, map `DF5AF4={-1,CBD310}`. The full
FuncInfo retains its final flags DWORD1. Action `CBD310` reads saved completed
count, multiplies by12, adds saved fresh base, pushes that pointer, then reads
and pushes saved current target. Its complete target `401130` is just `C3 RET`.
The action frees neither the fresh outer allocation nor completed inner data.

The source outer entry passes its actual request-slot address to a fixed C++
core. Volatile saved fresh/completed/current/state fields retain the original
read/write schedule. A C++ catch runs the fixed no-op action specialization only
when state0 is armed, then rethrows. It supplies no allocation rollback. The
compiled specialization uses `C2 00 00 RET0`, with the same stack/flags effect
as the original RET; its encoding and address are explicitly different. This
is not a reconstruction claim for a generic CRT vector cleanup or `401130`.

Native private-EH-frame aliases, native CRT exception objects and hardware/SEH
fault dispatch are outside the new source frame ABI. The actual public request
slot remains real writable callee storage. Allocation/free use fixed cdecl
bridges to `singleton_lifetime_allocate({object,bytes,bytes})` and matching free,
preserving the existing current malloc/new-handler/retry/throw source service.
No arbitrary allocator or cleanup callback is introduced.

## Verification and replay

The strict Win32 build and both existing CTests passed; all eight fresh seed
checks matched disk. The starting main commit contained reserve2 source but
had not yet registered it in committed CMake. The first strict library therefore
failed the focused fixture link for the missing primitive symbol. That untouched
library and logs are retained separately. The ignored extra-source hook was
extended with the existing primitive CPP, followed by a successful incremental
strict build. No shared CMake or provider source was changed.

The final actual library was frozen before fixture linking. Three exact archive
members and twelve source/header inputs are pinned. All345 bytes of the three
literal owned entries match original instructions except declared direct calls;
the complete primitive130 bytes are checked the same way. The outer reserve is
a reviewed translation of all248 original bytes, not a literal instruction
match. Its full entry/core, fixed helpers and compiler EH records are covered
by whole COFF and linked-section proof.

One focused ignored fixture compares a nested sequence through all593 original
owned bytes against the full actual library. Real CRT malloc/free forwarding
first performs the real call, then mutates only fixture storage. It changes the
current outer base after allocation, changes an initially empty source during
its unconditional reserve, and changes source again during append reserve to
test the previously captured row. Inner growth checks payload+8 preservation.
Shrink exposes count2 before the real child free, then changes current outer
base/count to1; the loop observes that current state. Each original/source run
performs four real allocations and six real frees.

The two whole344-byte raw traces are retained. Only the twenty explicitly
identified header data fields normalize by their established allocation roles;
raw addresses and all eleven role identities remain recorded, including address
reuse across different lifetimes. Counts, capacities and payload are literal.

The original private page includes the four complete bodies and original EH
handler/map/action/RET target. Eighteen exact operands are checked and rebound:
twelve calls plus six EH/data bindings. The handler points through the concrete
linked `__CxxFrameHandler3` import; that exception path is not exercised. All4096
bytes of the bound page remain unchanged after normal execution. Normal FS-chain
restoration is checked. Original primitive/CRT execution is not claimed: original
owned bodies compose the complete compiled primitive and fixed source providers.

The proof covers all20 owned COFF sections,127 mapped sections,60 actual import
providers, and10,637 whole runtime code bytes. Map parsing strips every leading
`f`/`i` flag before assigning object ownership. No allocation-failure, new-handler,
C++ exception or hardware/SEH injection was added. There is no gather, gamma,
device, gameplay or native caller ABI claim. The audit records exact pins;
`local/capability_nested_arrays/REPLAY.md` gives the unchanged fixture replay order.


## Primary integration

Primary registered all 4 complete source entries and passed the strict Win32 build, both existing CTests and eight fresh seeds. All three packets use the same frozen main library `b1fa83e3959f4db6a0cdeb207054a57057bea9749cd48f8b5d3037b7f09e14c5`. Unchanged original/full-main-library nested grow/copy/shrink sequence matches both344-byte traces under real allocator-induced pointer/count mutation with11 lifetime-specific pointer roles. All593 original bytes executed; three owned345bytes plus complete primitive130 match after declaredCALLs. Outer248 is full reviewed source/EH translation, not literal identity; RET0 specialization explicit. Three exact objects/20wholeownedCOFF/127 mappedsections450relocations,60actual imports and10637 runtimecodebytes verified. Private4096-byte original page unchanged after18 checked operand bindings, including actual linked__CxxFrameHandler3. No failure/new-handler/exception-path/nativeSEH/privateframealias/originalcallerABI, gather/device/gamma or game claim. Reviewed names and evidence are saved with prior comments retained; all affected exports were forcibly refreshed. Immutable primary evidence: `local/capability_nested_arrays_primary/`.
