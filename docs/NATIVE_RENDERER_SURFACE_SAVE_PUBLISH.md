# Native renderer surface-save publication

Full source now covers publisher B5E490[107], capture caller B23C50[235], and
raw field getter B24DC0[9], 351 original bytes. Strict MSVC Win32 build, both
existing CTests, all eight original-byte seeds, and one nonempty actual-library
capture/publication/save fixture pass. Original bodies were not executed.
Names describe observed behavior; they are not recovered symbols.

## Original and reconstructed interface

The original parents receive ECX raw owner and a stack pointer to an actual
eight-byte string header, return with RET4, and expose no semantic result.
The new fastcall interface adds a fixed EDX context, preserved in an additional
saved EBP. Only the caller-input stack offsets shift; the capture's internal
scratch output offsets and raw instruction schedule remain intact. B24DC0
is the exact nine-byte MOV EAX,[ECX+197C]/RET4 body, ignoring its stack scalar
and the new reserved EDX argument. No ownership meaning is inferred for197C.

The context borrows ActualNativeStringPoolStorage, a concrete named D3DX load
import, and the complete required profile window. The actual renderer profile
token must be D5F0A8. The minimum/full required window is exactly12Ch bytes,
[D5F0A8,D5F1D4); this does not establish the complete vtable allocation extent.
Its reached current +128 DWORD is B24DC0. Capture saves the current profile
identity before surface publication1D24, reads the mapped selector afterward,
then invokes that complete raw getter. Foreign profiles are outside the fixed
source domain; there is no arbitrary selector callback or original-code call.
The accessed renderer prefix extends through1D80, including its actual54h-byte
worker at1D2C. This is not a complete allocation or renderer-lifetime claim.

ActualNativeStringPoolStorage must use the application's canonical pool
publication, live gate, and NativeStringPoolLifetimeBinding domain. It remains
alive through each synchronous call. NativeD3dx9SurfaceLoadImport borrows the
caller-owned actual d3dx9_40 HMODULE and resolves exactly
D3DXLoadSurfaceFromSurface; no DLL loading, version fallback or function-pointer
setter exists. Resolution errors are host construction boundaries. The separate
existing save application binding and module remain fixed and installed until
EVERY launched worker returns, including when this capture causes the launch.

## Current data and cleanup schedule

Zero source length returns before any renderer/context memory read. Nonzero
captures current device1A10, calls full B5E380 on worker1D2C, publishes the
acquired surface and invokes the current mapped raw getter in the order above.
It reads wrapper+2C before current destination1D24 and calls the concrete import
with `(dst,0,0,src,0,0,1,0)`.

Each GetDesc, LockRect, UnlockRect and the final extra GetDesc reloads current
surface/table at the original point. Scratch begins uninitialized. With S the
stack base after saved EBX/ESI/EDI, description is S+14h, WidthS+2Ch, HeightS+30h,
locked PitchS+Ch and pBitsS+10h. The alpha loop captures pBits once, ignores
Pitch, and ORs FF000000 into contiguous DWORDs. It reloads lowDWORD Height*Width
after every store and compares an unsigned index. Initial TEST/JBE skips zero
only. The final GetDesc is retained before full publisher invocation.

Publisher enters captured lock48 and increments that captured lock's depth,
then captures a destination header selected by current producer40. Exact header
identity skips context, resize and copy. Otherwise full41DD40 preserve=true
receives captured source length. Afterward CURRENT source length gates a copy
whose size is CURRENT DESTINATION length; current source and destination data
are read in that order. The destination header stays captured across calls.
Final producer is reread, incremented with DWORD wrap, reduced by signed
CDQ/IDIV5, and published. Current lock48 is reread for decrement/leave.

The persistent destination can overlap the source, as original BF7680 backward
copy proves. The source uses the existing string-family host boundary: memmove
for valid nonwrapping ranges and omission of zero-byte copies. The original
zero path accesses neither buffer. Generic CRT speed-global/ISA dispatch,
partial hardware faults and incidental registers are not reconstructed.

Neither original parent has an EH registration or cleanup. There is no added
unlock, release, rollback, null/HRESULT recovery, or queue policy. Exceptions
may retain entered lock/depth and already-published raw changes; those failure
paths were not runtime-tested. Added context/EBP stack and C++ bridges do not
claim original caller/unwind/SEH ABI. Native acquire's unreleased temporary
GetBackBuffer reference is preserved, not repaired by inference.

## Evidence and verification

Discovery f8d08cb4 pins21 fresh guarded PE/Ghidra-matching spans:2,187 captured
bytes including overlaps,1,978 unique bytes and363 decoded code instructions.
The full869-byte CRT body includes inline tables, so separate executable spans
prove overlap/zero behavior. C2DFD4 is the exact6-byte CE240C forwarder; the
installed x86 d3dx9_40 export is ordinal191/RVA28C044. The audit embeds the
original bytes, target identity, complete source boundaries and artifact hashes.

The actual strict library was frozen BEFORE linking the fixture, along with
all seven needed archive objects and23 source/header files. Full original
107/235/9-byte bodies match compiled113/243/9 after only documented EBP/context,
selector/import bridge, branch-displacement and relocation changes. All266
mapped COFF sections (168 from the library) and746 relocations match the linked
image. The complete owned object separately pins71 sections,130 relocations and
26 functions, including every import/resize/copy/profile bridge and unlinked
COMDATs. The parser consumes all map f/i flags. All16,643 runtime code bytes
match the linked postimage;81 actual imports and eight COM/D3DX observations
match recorded module identity and loader relocation evidence.

The one successful nonempty fixture creates a private hidden8x8 HAL D3D9
surface, runs full capture, actual pooled filename publication and real worker
save, then waits for the real thread to return. Pitch32 is recorded; all64
readback pixels equalFF336699 and the saved BMP has RGB51/102/153. Full raw
renderer/header/wrapper/name postimages are preserved without pointer
normalization. A zero header also bypasses inaccessible renderer/context.
The fixture preserves actual driver tables. Load/save HRESULTs are ignored by
source and were not separately observed. Other pitch/format/index domains,
callback mutations, exceptions, original execution, caller ABI, renderer
recreation and gameplay remain outside the result.

Frozen proof and the unchanged focused replay harness are under ignored
`local/renderer_surface_save_publish/`. Run its `verify.py` to recheck the seal's
actual archive, original spans, complete COFF/link/runtime code and raw outputs.
No permanent tests, shared CMake/ledger edits, original game changes or Ghidra
mutations belong to this source-only handoff.
