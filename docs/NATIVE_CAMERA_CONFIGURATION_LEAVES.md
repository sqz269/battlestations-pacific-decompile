# Raw camera configuration leaves

Four complete normal bodies, 84 original bytes, expose the current camera argument
word without a typed CameraProjection/CameraFrameState owner. Near, far and clear
flags extend existing typed records; clear depth is one new reconstructed body.
The existing typed implementations and interfaces remain present. Names are
descriptive hypotheses, not recovered symbols.

| Body | Coverage | Original effect | Existing record |
| --- | --- | --- | --- |
| B6FBF0..B6FC0A, 27 bytes | complete | Capture MOVSS input; AND +2F0 with FFFFFF41; store +1D4. | set_camera_near |
| B6FC10..B6FC2A, 27 bytes | complete | Capture MOVSS input; AND +2F0 with FFFFFF41; store +1D8. | set_camera_far |
| B6FE10..B6FE1C, 13 bytes | complete | Capture complete DWORD, store +188 and return it in EAX. | set_camera_clear_flags_00b6fe10 |
| B6FE20..B6FE30, 17 bytes | complete | MOVSS input bits to +18C, with no cache change. | new body |

Every original entry receives ECX=actual camera and one stacked four-byte argument,
then RET4. Scalar leaves have no semantic return value. The new MSVC Win32 fastcall
interfaces keep ECX as camera and use EDX as the address of the raw argument word,
so their RET cleans no stack arguments. They are not original-call-site binary
replacements. Naked inline assembly preserves the complete memory access order.
There is no floating conversion, equality skip, clamp, NaN normalization or profile
check. The caller still performs its own x87 argument preparation.

Near/far must capture their input before invalidating the cache word. The explicit
borrowed argument can alias camera+2F0: reloading after the AND would incorrectly
publish the masked bits as the new plane value. All four setters first read their
current argument before writing any camera field. Current pointer values and
private ABI storage remain stable; no concurrent-access or private-stack alias
guarantee is introduced.

## Actual camera producer

B71930..B71939 binds the canonical 108FFB0 camera pool and jumps to B71770. Its
incoming ECX=458h size is ignored. The pool supplies a 45Ch slot: the actual camera
is the first 458h bytes and the slab identifier remains at +458. B71A80's complete
body constructs that camera in place and returns the same address. It writes
far+1D8 at B71C09, near+1D4 at B71C29, valid-flags+2F0 at B71C39, clear-flags+188
at B71C4B and clear-depth+18C at B71C75. These writes corroborate the leaf offsets.
The slab constructor B6FED0 writes each metadata DWORD at slot+458 and advances
by 45C at B6FF02; allocation B71770 multiplies the selected slot index by 45C
at B71846. Both supporting listings are preserved in the external evidence.

NativeCameraTailStorage and its offset assertions already locate these exact
fields in the same pool allocation. NativeCameraOwner, CameraProjection and
CameraFrameState are separate companions/views; their own addresses must not be
passed to the raw setters. The new leaves do not start an object lifetime,
allocate a camera, retain a viewport, or alter pool metadata.

## All caller preparations

Live xrefs found 66 call sites: 15 near, 10 far, 40 flags and one depth. The saved
external evidence includes each preparation and containing-body result, complete
listings for the initial 24 defined caller functions and an explicit raw
preparation for the initially unattributed call at 5CC5D8. After that capture was
sealed, primary analysis-only commit 4ae022a5 established and saved FUN_005cc590,
body 5CC590..5CC5DD. Fresh worker readback preserved all 20 instructions and
confirmed the same 78 bytes. The numeric verifier now passes all 66 rows across
25 containing functions. The parent is not renamed or counted as reconstructed.
Its near preparation is FLD [CE3958], fresh EAX=[E188A8], PUSH ECX,
ECX=[EAX+19FC], FSTP [ESP], CALL B6FBF0, then RET at 5CC5DD.

All scalar calls consume one float32 word. Their callers obtain it from current
constant cells, object fields, saved words or a callee's ST0 result, then commonly
use FLD/FSTP32 before the call. Near restore paths include AE4DA8 and B160CB;
far restore paths include AE4D99 and AE22A2. B9F48D explicitly multiplies and rounds
the far value in its caller. Those computations are not folded into a setter.
4B4952 forwards its original stacked float word after loading camera+19FC and
guards a null camera before calling; the leaf itself adds no null guard.

Clear-flags callers provide constants 0, 1, 2, 6 and 7, a current value OR 6
(AC5EB7), and saved full flags (B160B5, B1649C, B10591 and B84766). It is a DWORD
setter, not a Boolean or a three-bit mask. BA7646/BA88CC push the zero EBX value
established at BA754C; all EBX writes in that function were checked. B841DE's
PUSH7 is at B84192, before intervening local x87 work; its full preparation was
retained rather than attributing a nearby floating spill as its argument.
The B1BF40 and B4CBB0 getters used between argument push and setter CALL are
complete plain-RET leaves and consume none of that pending argument.

For the full-service dependency B3C800, near reads current D7A2F0 and far reads
current CE38B8, with a fresh holder+C load at each call. Clear flags receives 6.
Clear depth receives the result of **FLD1/FSTP32**, not a borrowed D7A24C constant.
These setter primitives do not complete the holder's camera-name/current-node-
constant composition, viewport replacement, FH3 cleanup or published-camera lifetime.

## Validation boundaries

The complete four live Ghidra byte spans match the installed original PE, and all
four native listings were read. Strict Win32 `/W4 /WX /O2 /MD /fp:strict` source
compilation passed; emitted COFF confirms input read, flags AND, final store and
return ordering with only the declared source ABI substitutions.

One focused external fixture addresses the concrete borrowed-argument/flags-alias
risk. It compares the four original byte bodies, unchanged and with no relocated
operands, against the reconstructed functions using one distinctive signaling-NaN
bit pattern. Near/far each run with separate input and input aliasing +2F0; flags
and depth each run once. The complete 45Ch backing, MXCSR and flags return value
are compared. On the original side the same selected DWORD is supplied through
the real stack-argument ABI; this does not pretend to exercise original private
caller-stack aliasing. There is no broad test suite or new repository test.

Strict source compilation, the baseline Win32 build, both existing CTests and
eight seed checks passed. The focused fixture passed six paired comparisons,
6,744 compared bytes, with all eight input/library hashes equal before and after
the run. Its 84 original code bytes remained unchanged. The refreshed numeric
caller check passes all 66 rows; the earlier attribution gap remains documented
in the initial capture. The baseline does not
register this new module. Its initial fixture
therefore uses explicit WorkerSource compilation; the integrator must register the
source and replay against the newly built current library before claiming that
integration has passed.

The immutable capture in `C:/Users/sqz269/bsp-bc-camera-configuration` preserves
the source/header, original spans, caller/producer evidence, three libraries,
probe/include/recipe, objects/executable, input hashes and logs. Default `run.ps1
-Root <root> -LibraryRoot <root>` compiles only external probe.cpp and links those
three selected current libraries. WorkerSource is only for the initial unregistered
stage. The executable embeds a manifest. No fixed code/data mapping is required.
The original 166-member `worker_capture.zip` remains immutable. The separately
hashed `attribution_refresh.zip` preserves the later live caller readback,
primary analysis record and passing numeric check. All eight active fixture
inputs were rehashed unchanged; source, recipe and runtime evidence are retained.

No Ghidra mutation, source registration, shared test change, original ABI
compatibility, full camera lifetime or gameplay claim belongs to this worker packet.

## BC integration checkpoint

The integrator reviewed the complete native body and actual producer/provider
evidence, saved its original analysis signature and full stored range in the
existing BSP project, and registered the source where needed. Exact combined
validation follows separately from worker checks. No original binary entry,
unrestricted FH3/SEH, whole owner lifetime or gameplay claim follows.

## BC exact merged validation

Exact combined source `d6cd5f085cc8d521c31e5bf327daa10b47710e9a` passed the strict Win32 build
and both existing CTests. Camera, CString and texture probes compile only
external probe.cpp against all three current libraries; depth has a complete
seven-byte comparison in its exact library member. Coverage is bounded;
texture is hot-cache only. See `reports/native_render_service_construction_bc_validation.json`
for immutable captures, hashes, adapter scope and remaining limits. Earlier
pending statements describe worker stages. Full service/camera construction,
native ABI/FH3 compatibility and gameplay remain unproved.
