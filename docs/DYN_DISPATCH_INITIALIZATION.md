# Dyn dispatch initialization

The GeneralConvexIntersect global now has a concrete owner, exact support-direction
initialization and a real Windows critical-section lifecycle. Its seven companion
scene dispatchers are the actual one-word objects produced by initialized PE data.
The API requires their complete callable vtables; it generates no collision tables
or methods. Descriptive routine names are hypotheses; the class names below are
proved by native RTTI.

| Entry | Inclusive body | Original ABI | Coverage |
| --- | --- | --- | --- |
| 00C48FD0 | 00C48FD0..00C49222 | cdecl, no arguments, EAX global owner, RET | Complete constructor |
| 00CC8950 | 00CC8950..00CC8960 | cdecl, no arguments, EAX atexit status, RET | Complete CRT initializer |
| 00CD91D0 | 00CD91D0..00CD91DB | cdecl, no arguments, RET | Complete teardown |

Live bodies have 122/5/3 instructions and no flow gaps. All three final RETs
are one byte. 00CC8950 was initially undefined despite the call at its first
instruction; the integrator defined its exact 17 bytes under the write lock,
saved and exported before this worker claimed it. The old function snapshot
may still attribute that address to an earlier candidate; the live definition
is authoritative. No worker Ghidra mutations were performed.

## Owner and producer

`DynGeneralConvexIntersectStorage` is the 290h-byte owner at 0109EA48. Its
vtable is at +0, untouched alignment word at +4, 26 triples of doubles at
+8..+277h, and a 24-byte Win32 CRITICAL_SECTION at +278h (0109ECC0).
Native initialized-image bytes at the general owner begin with zeroes; the
new globals aggregate starts from the same zero state. Its aggregate layout
does not claim the eight original globals were contiguous.

00C48FE2 publishes vtable 00D7A1A8 **before** 00C48FEC calls
InitializeCriticalSectionAndSpinCount(owner+278h,10000), ignoring its BOOL.
00C48FF2..00C491D1 then writes all 78 double words in the original store order.
The only loaded constant is exact double -1 at 00D7A250; FLDZ and FLD1 supply
zero and one. The ordered triples before normalization are:

```text
(0,1,0)    (0,-1,0)   (0,1,1)    (0,0,1)    (0,-1,1)
(0,-1,-1)  (0,0,-1)   (0,1,-1)   (1,0,0)    (1,1,0)
(1,1,1)    (1,0,1)    (1,-1,1)   (1,-1,0)   (1,-1,-1)
(1,0,-1)   (1,1,-1)   (-1,0,0)   (-1,1,0)   (-1,1,1)
(-1,0,1)   (-1,-1,1)  (-1,-1,0)  (-1,-1,-1) (-1,0,-1)
(-1,1,-1)
```

These are all 26 nonzero ternary directions, in native order. There is no
synthetic tuning input, random direction, alternate ordering or zero-vector
fallback. ESI is assigned 0109EA60 once at 00C491B4, advances by18h at
00C491F8, and is tested against 0109ECD0: exactly 26 iterations. Its last
component pointer is only advanced into the lock for the terminating comparison;
that lock address is never read as a direction.

The x87 loop loads y,x,z, computes `(y*y + x*x) + z*z`, then calls the actual
ST0 CRT sqrt entry at 00C491EF. FLD1/FDIVRP retain the reciprocal in extended
precision, followed by x, y and z double stores with the original pops. The
implementation forwards to existing `native_crt_sqrt_st0_00bf7030` using a
required `CameraAxesCrtAccess`: real dispatch-bypass storage and the actual
00C27489 exception service. It adds no float/double spill or replacement sqrt.
The OS call, all ternary writes, x87 sequence and comparison/store order remain
explicit in the source. New borrowed arguments use a different C++ ABI.

## Static objects and execution boundary

Each table has one virtual entry and its preceding CompleteObjectLocator.
Live table bytes and type-descriptor strings establish this mapping:

| PE object | Vtable | Actual RTTI class in Dyn | Virtual entry |
| --- | --- | --- | --- |
| 00E17434 | 00D7A184 | BoxBoxIntersect | 00C49A30 |
| 00E17438 | 00D7A18C | TerrainConvexMeshIntersect | 00C53630 |
| 00E17440 | 00D7A1CC | SphereSphereIntersect | 00C518D0 |
| 00E17444 | 00D7A1D4 | BoxSphereIntersect | 00C48330 |
| 0109EA48 | 00D7A1A8 | GeneralConvexIntersect | 00C535E0 |
| 00E17448 | 00D7A1F4 | ConvexRayIntersection | 00C44780 |
| 00E174E8 | 00D7A1FC | BoxRayIntersection | 00C50DD0 |
| 00E174EC | 00D7A204 | SphereRayIntersection | 00C50740 |

The seven small objects have no constructor writers: their vtable words are
present in the installed PE and their live incoming references are scene
constructor reads. `bind_dyn_dispatch_static_objects` expresses this data
producer using the caller's actual relocated callable tables. It does not
construct general-convex storage. `dyn_scene_dispatch_objects` returns existing
`DynSceneDispatchObjects` pointers into the stable owning aggregate.

GeneralConvexIntersect's virtual entry 00C535E0..00C53623 takes five stack
arguments (RET14h), passes owner+8 directions and owner+278h lock to a temporary
work record, and calls 00C53010 at 00C53619. That execution method and the other
seven virtual targets remain external. Their table identities are established;
this packet does not implement their collision algorithms. Scene constructor
00C38070..00C38461 borrows these objects and publishes general-convex in
0109E9F4. Tables, objects and CRT services must outlive every borrowing scene.

## CRT registration and lifetime

The sole static-initializer array reference is 00CE36B0 -> 00CC8950.
00CC8950 calls the full constructor, pushes CD91D0 at CC8955, calls genuine
CRT `_atexit` BF6FF5 at CC895A, pops its one argument with POP ECX, then returns
EAX unchanged. The `_atexit` body calls `__onexit` and converts its pointer to
0/-1; no library implementation is duplicated here. Registration failure
does not roll back the already constructed object or lock.

The existing single-bound-owner pattern supplies this callback's actual global
storage. `bind_static_dyn_dispatch_globals_0109ea48` rejects a different owner.
`initialize_static_dyn_dispatch_00cc8950` requires real atexit registration;
it does not bind PE objects or silently acquire a replacement owner. The
registered `destroy_static_dyn_dispatch_00cd91d0` calls the contextual teardown,
which only calls DeleteCriticalSection on +278h. There is no vtable reset,
direction clear, deallocation or automatic destructor. Construct once; delete
once after all scenes/tasks have stopped using the object. Invalid double
initialization, concurrent startup/destruction and native EH/fault ABI are outside
the callable domain.

## Verification and limits

Win32 build and existing CTest **2/2 passed** after seed verification. No tracked
tests were added. The report verifies all direct call rows and lists the two
WinAPI IAT calls separately with their exact native instruction bytes.

One ignored `/MANIFEST:EMBED` probe reuses the preserved scene native image
SHA256 `86f0f456d654ba64cf421f073ba2aa95d7dbef99fa324b42c7e29a6ad3f1728e`.
Original C48FD0 was already relocated there. Three recorded operand relocations
make the original CC8950/CD91D0 callable in that same private mapping. Original
installed/live body bytes match for all three entries. Neither installation nor
Ghidra was modified by the probe.

The fixture compares all 78 direction doubles, vtable, untouched poisoned
padding and x87 control/status over **12 constructor cases**: six masked x87
control words and both explicit CRT bypass values. Both sides share the recovered
CRT numerical service. Its exception boundary leaves produced result/control
unchanged and compares **120 callback records**; unary uninitialized argument2
is never read. This establishes operation/service-input agreement, not independent
CRT-handler fidelity.

**26 real lock lifecycle checks** exercise recursion, exclusion of another
thread and admission after release. Four registration observations cover original
and reconstructed startup with success and failure, including no rollback. Seven
PE static words match, and a different live owner is rejected. Collision methods
are not executed. Reports include exact input generation, code/image/artifact
hashes and logs. No mission, gameplay, full-task execution or native binary/EH
compatibility claim follows from these checks.
