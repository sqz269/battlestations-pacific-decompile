# Native renderer camera preparation

`prepare_native_renderer_camera_00b285a0` reconstructs the complete 544-byte
`00B285A0..00B287BF` body. The original receives the renderer in ECX, the camera
in its caller's stack argument, and returns with `RET 4`. The new Win32 fastcall
interface adds an immutable context pointer in EDX and exposes no semantic
return value. It is not a drop-in binary replacement or game validation.

Evidence is the freshly guarded `bsp.gpr` / `/battlestationspacific.exe` capture,
installed-PE equality, complete built-object and linked/runtime verification,
and a focused original-parent/full-library composition fixture. The original
PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The parent span SHA256 is
`27a20b51491a9f478eaabd0354c1663f3a2266a46c1ed3c502e10b8e962e72d3`.
Names and field purposes remain descriptive reconstruction names.

## Storage and context

The renderer is actual raw renderer storage. Its relevant fields are the real
tracked critical-section pointer at `+04`, cached states owned by B24460, the
plane set at `+17C0` with count at `+1900`, pending and active plane counts at
`+19F0` and `+19EC`, current D3D9 device at `+1A10`, and support byte at `+1B51`.
Each child state or clip call owns its existing synchronization and exception
behavior. This parent adds no outer guard or exception handler.

The camera and every nonnull ancestor must be actual raw nodes: parent `+30`,
flags `+5C`, local matrix `+B0`, world matrix `+F0`. The camera additionally uses
the completed raw getters' cache/projection layout and its actual fog-owner
pointer at `+184`. `NativeCameraOwner` and `NativeNodeStorage` currently store
`CameraTransform` companion pointers in hierarchy words; they must not be cast
or silently passed as a compatible raw hierarchy. This packet does not migrate
that representation or connect the existing semantic camera-preparation API.
See `NATIVE_CAMERA_WORLD.md` and `NATIVE_CAMERA_CACHE_GETTERS.md` for the concrete
provider and migration boundary.

The 16-byte `NativeRendererCameraPrepareContext` borrows the completed
`NativeCameraFrustumContext`, actual renderer synchronization globals, actual
double scale CE4B48, and current conversion-mode DWORD 109EEA4. The pointer
members are immutable; pointed-to runtime state remains live. All references
must outlive the call. Frustum requires the already-bound complete
`LegacyCrtMathRuntime`, actual dispatch word 109DD78, negative-zero word D7A208,
actual matherr-policy storage E16BD0, and the real CRT errno accessor. No new
allocator, math, camera, projection or state callback is introduced.

## Preserved operation order

1. Set render state **0x98** to zero before loading the camera stack argument.
   Call inverse-view-projection for its cache side effects, then obtain frustum
   storage. Copy all 16 records with the completed x87 plane-set provider and
   only then reread the source count and publish renderer count.
2. Test the support byte before zeroing both plane counters. If supported,
   read the current unsigned count. For each index read flags once to exclude
   bit 4, then read them again to require bit 2. The count and flags are not
   snapshotted or bounded to the nominal 16-record capacity.
3. For each admitted plane obtain current view, perform scaled affine inverse,
   obtain current projection, perform general inverse and multiply in the
   original operand order. Preserve the original 16-DWORD `REP MOVSD` and all
   six ordered `MOVSS` transpose swaps, including changing ESP offsets while
   original arguments are pushed. Transform the selected plane through the
   complete plane wrapper and its vector provider.
4. Load the current pending slot and call the full clip provider with the
   original stack-plane pointer. Only after normal return increment the
   *current* pending count. Reload the camera stack argument and current count
   in their original positions. A provider exception suppresses the increment
   and all later parent work. Returned HRESULT does not control continuation.
5. Form `((1 << (pending & 31)) - 1)` with native DWORD arithmetic, set state
   0x98, then reload pending after the child returns and copy it to active.
6. Finally load the current camera `+184` owner, obtain its `+08` color storage,
   run the complete ARGB conversion with the borrowed scale/current mode, and
   set render state **0x22**. Null owner skips only this final color work.

Frustum extraction writes six records with flags 7 and leaves its count alone;
those six records fail this parent's bit-4 exclusion. The fixture therefore
uses count 8 and two authored additional records with flags 2, which genuinely
exercise the inverse/transpose/transform/device path.

The assembly preserves all 138 original instructions. The built parent has
590 bytes: one saved EDX word, seven context-load instructions, and one final
EDX pop supplement the original body. The original local offsets remain
unchanged. The saved context is at normal ESP+124; the caller camera argument
is at ESP+12C instead of +128. Context loads under two arguments use +12C;
the color load under one argument uses +128. All branch targets are checked
against the mapped original instruction targets. This additional stack word
is an explicit interface difference. Mutation of the original callee camera
argument slot is preserved; its absolute address is naturally process/call
specific. Neither C++ parameter snapshots nor camera-owner field substitutions
represent that slot.

Four private naked adapters (state 23 bytes, clip 23, color 18, plane 16)
translate only calling conventions to completed concrete providers. They do
not substitute arithmetic, results, lifetime or synchronization. Whole COFF,
all relocations, linked bytes and runtime bytes cover these adapters too.

## Validation and limits

The strict MSVC Win32 Release build passed `/W4 /WX /fp:strict`, both existing
CTests passed, and all eight native seed spans matched the installed PE.
Only an ignored extra-source CMake hook includes this packet and the two
permitted dependency cherry-picks in the worker build. Shared CMake, ledgers,
Ghidra metadata and the game installation were not changed.

The ignored fixture runs the **complete original 544-byte parent**, with only
its 20 external CALL displacement operands rebound, against the actual built
library implementation. All descendants execute completed library providers;
the reference side uses five explicit integer ABI/context bridges, including
frustum. This is original-owned-parent/full-library composition, not execution
of original descendant machine-code bodies. The library is frozen byte-for-byte
at SHA256 `c8b3a51fd238f594cac8b69769d42c2b0455dce6b4ea161eb3b40e76bd4d5a33`.

Three paired runs cover a cold three-node raw hierarchy and two user planes;
post-real-COM mutation of the actual camera argument, support byte, copied
flags, pending count, current device, live retained fog owner, conversion mode
and final state callback; and a C++ exception thrown after a real SetClipPlane
return. The two hidden HAL devices, real COM methods, real tracked Win32
critical section, and concrete initialized/retained fog owners are used in all
runs. Fog setup/cleanup providers are separate from parent dependency claims.
Arena owners are logically deleted with flags zero after slot clear, so the
fixture does not free VirtualAlloc storage through the CRT.

All three pairs match 935,408 bytes of complete callback/pre/post storage and
device-query traces without tolerances. Normal runs preserve the stack and
nonvolatile registers, an empty x87 stack, control word 037F and MXCSR 1F80;
the observed x87 status is 0020. The exception pair leaves pending 5, active 0,
one completed clip call, and the real child guard fully drained. All 22
observed COM calls returned actual S_OK before any observer mutation/throw.
No HRESULT-failure or generic allocator-failure runtime claim is made.

Verification covers all 419 mapped local COFF sections and their 1,591
relocations, including 308 fixture sections and inline COMDATs. Of these,
399 immutable code/constant/EH sections match runtime images. The 258 mapped
function entries include 79 functions from 22 exact archive members and 179
fixture functions. These are complete section-byte proofs; compiler section
tails may include padding/data and are not asserted to be instructions.
Seven full postimages preserve the original parent, linked `.text`/`.rdata`,
both constants, and actual/observer COM tables. Loader IAT writes are explicit.
External module identities and function-entry bytes are checked with PE
relocations; two writable CRT data imports have identity/value evidence only.
No whole-system-DLL byte or original SEH ABI equivalence claim is made.

Artifacts remain under the isolated worktree's `local/camera_prepare/`, with
the actual archive, exact objects, source/header inputs, map, probe, traces,
postimages, complete relocation proof and artifact pins sealed there. The
tracked audit links those pins. No permanent tests were added. These focused
cases do not establish every malformed count, alias, floating-point trap,
concurrent mutation, or invalid raw hierarchy. Exact instruction proof retains
unchecked addressing/order, while invalid inherited skipped-guard cleanup
domains remain outside the providers' supported runtime contract.
