# Raw GUI model clone: B752B0

`clone_native_gui_widget_model_00b752b0` composes the complete normal 322-byte
body `[00B752B0,00B753F2)` for flags `26h` and `3Eh`, with parent fixed to zero.
The final instruction is `RET 8` at B753EF (3 bytes). Original ECX is the
source Model; stacked arguments are flags,parent; EAX is the destination.
This is a new MSVC Win32 source interface, not a binary entry replacement.

The destination is an actual 188h slot in the same canonical 01090054 pool.
The native Model occupies its first 184h bytes; the trailing pool index is
preserved. Existing `NativeModelOwner` and `NativeModelReference` companions
borrow that storage and its actual +04 count. The source is resolved to its
existing canonical companion, and its current D62DE8+10 must be B752B0.
No `GuiWidgetOwnerRuntime` destination factory or semantic Model constructor
is called. The older Text clone wrappers remain separate interfaces.

## Construction and ownership

B752D1 calls B74D00 on 01090054. After allocation, B752EA borrows the CURRENT
source+54 header through B6D800; B752F2 forwards its actual address to raw
B75030. `NativeInstanceGeometryAccess` supplies only the existing transactional
host preparation, failed-companion retirement and completed-reference binding
callbacks. They must preserve native bytes and may not add a native retain.
The raw node name pool and the same borrowed CE4970/CE4ADC constant cells are
mandatory. No temporary name, second native owner, or semantic pool is made.

The native handler is CC1C78, descriptor DFAC7C and unwind map DFAC74:
state0 -> -1 invokes CC1C70, an 8-byte `MOV ECX,[EBP-10]; JMP B748C0` cleanup.
B748C0 selects the same pool and reaches B74750. This source uses those
existing pool methods; it does not port the funclet/handler. Only preparation
and raw construction are inside the slot-return scope. Native B7530A disarms
state0 before B6F150. The source also disarms before completed host registration,
so a registration exception exposes the live owner/raw slot without returning
it. Later failures retain the model creator and all nested acquisitions.

`NativeGuiWidgetModelCloneAcquired` is a one-shot diagnostic and ownership
publication, not a second reference owner or a resumable transaction. Success
returns one `NativeModelReference&`, also in `creators.model`. A caller that
publishes that creator into its actual owner must clear this field to record
transfer; otherwise it must release through the same canonical domain.

## Current data and callbacks

The existing actual B6F150 fragment performs the complete supported base copy.
Both flag values include 20h and skip source child cloning. Actual point-light
links, current destination50/38, scene/root/hierarchy, retained130 and matrix
notifications retain that provider's established ordering and dispatch limits.

Only after those callbacks does B75317 read current source+180. A nonnull
identity must resolve to the same actual Mesh companion. Its current D62D60+10
must be B742A0, checked by the existing actual mesh clone provider. Flags26
always passes null stream services and shares streams. Flags3E requires the
same actual geometry/vertex domain's `NativeStreamCloneServices`, and invokes
the owned stream-copy route. There is no unknown-profile or flags fallback.

The existing B75331..64 association reads current source17C then178 using
x87, calls B75170, and consumes the captured mesh creator. Its caller-owned
mesh slot clears immediately before release so terminal failure cannot repeat
that release. Native EBP captures imported InterlockedDecrement at B7531F;
source providers retain their established atomic/current virtual-zero terminal
bindings, not arbitrary runtime replacement of that import.

The existing tail reads current source174 then destination174, publishes new
before increment, then releases captured old through its current zero callback.
Ten individual current source+08..2C x87 FLD/FSTP pairs follow those effects.
No source snapshot crosses callbacks. Scratch/opaque bytes left unwritten by
native construction retain their caller/pool preimages.

## Evidence

Strict `scripts/build.ps1` passed with both existing CTests. Live Ghidra and
installed PE agree on the full322B body and the8B cleanup boundary; hashes and
all eleven native call rows are recorded in
`reports/native_gui_widget_model_clone_cc10.json`. Ghidra was read-only.

One ignored probe, `local/output/cc10_model_clone_probe.cpp`, runs four full
original/source composition pairs: flags26/3E, each with null geometry or an
actual empty Mesh published by a scripted scene callback during base copy.
The original body retains its current180 load, branches, arguments, association
and x87 tail. Its direct callees and known current geometry10 are rebound to
the same existing actual-source providers; IAT increments/decrements use current
Windows atomics. Host companion construction/registration is explicit in the
original constructor bridge. The known geometry10 load pair is bound to its
source implementation; arbitrary profile dispatch is not differentially tested.

All184 destination bytes match after normalizing only independently allocated
name-data and geometry identities. The probe also checks x87 status, signaling
NaN quieting after a callback, name contents/borrowed header identity, actual
creator counts, preserved opaque bytes and pool+184. Additional source-only
checks verify constructor-mode rejection returns the same slot, completed host
registration failure preserves a live creator, and a later missing geometry
binding preserves the model creator. Native exception transport is not tested.
The exact command and artifacts are in the report; `/MD`, `/fp:strict`,
`/MANIFEST:EMBED` are used and `NDEBUG` causes a compile-time error.

## Boundaries

Valid retained actual storage, initialized raw pools/profile cells, matching
canonical owner domains and genuine lifecycle callbacks are required. Reached
deep mesh/material/stream providers keep their existing limits; this probe has
no mesh sections, materials, or positive vertex/index streams. Raw node
materialization, constructor failure, canonical terminal callback and allocator
exception projections remain explicit. Null allocation is diagnosed before the
native later null access. Other flags/parents, unknown virtual targets, original
CRT/FH3/SEH identity, outer register ABI, asynchronous/fault-time observations,
application adoption and game behavior are not claimed. AA9520/AC6040 and all
GUI orchestration are outside this packet.
