# Raw camera pose storage (CC10)

Addresses: 00B6DAE0, 00B71400, 00B700E0, 00B63F10.

The complete1336B packet uses actual node/camera words and caller-owned initialized DWORD backing. It reuses the published raw world, matrix and CRT providers through new source interfaces. It does not route raw identities through typed `CameraState`/`CameraTransform` companions or activate B71A80 construction.

| Entry / exclusive end | Bytes | Original ABI | Coverage / source |
| --- | ---: | --- | --- |
| B6DAE0 / B6DB0F | 47 | ECX actual node, stack position; tail current34, downstream RET4 | complete; `set_native_raw_world_position_00b6dae0` |
| B71400 / B71422 | 34 | ECX actual camera, stack position, RET4 | complete; `set_native_raw_camera_position_00b71400` |
| B700E0 / B701F5 | 277 | ECX actual camera, stack eye/target, RET8 | complete; `set_native_camera_look_at_storage_00b700e0` |
| B63F10 / B642E2 | 978 | ECX output64, EDX eye3, stack target/up3, RET10, EAX output | complete; `build_native_camera_look_at_00b63f10` |

## Actual argument cells and dispatch

B6DAE0 captures its incoming pointer once, copies x to actual120, and overwrites the **same actual argument word** with actual+F0 before reading source.y/source.z. While z is live in ST0 it loads current profile/current34; z is stored to128 afterward, then the captured target receives that same overwritten argument cell. `NativeCameraPoseDispatch::resolve_profile` must perform integer-only, x87-neutral lookup without allocation, callback or native mutation. Its current34 invocation executes the genuine exact target, with prepared persistent frames. The source projects the tail dispatch through its explicit interface; it is not a drop-in register/stack ABI.

B71400 captures its original input before clearing camera2F0 with FFFFFE4B, writes the nested argument cell, calls raw B6DAE0 and then raw B70660. D62CF0's actual camera entries are30=B71400,34=B71460,3C=B6DBC0,40=B6DBE0. Concrete bindings can execute those genuine bodies. Other reached profiles require their real provider; no fixed-slot fallback or logical hierarchy is supplied.

B700E0 captures eye once and current30 before its flag clear, writes the actual pushed position word and invokes30. It reloads the **current incoming target word after that call**. Ordered target copies into1A0 and target-minus-live-eye spills precede the genuine419440 length call. Length spills float32, and unordered/nonpositive comparison selects zero reciprocal. Current D7A24C is read at B7016F with a live x87 value; normalization spills/stores and actual target/up argument writes retain their native order.

After B63F10, B700E0 captures the current profile/table **before** B63B30 inverse. It writes the inverse's returned pointer into the actual final world argument, then reads34 from the captured table. It does not capture34 before inverse, nor reread the current profile afterward. Required30/34 providers receive actual receivers and mutable argument cells, using prepared frames without invented zero preimages. These bodies install no EH; a source provider failure retains the completed prefix, with no rollback.

## Lifetime-safe scratch views

All views are disjoint immutable host metadata borrowing already-live initialized volatile DWORD arrays. They introduce no overlapping C++ aggregate objects. Backing and metadata remain stable through callbacks. The original physical correspondence is explicit; saved registers, return addresses and other private gaps are excluded from the source ABI/alias domain.

| Body, original entry ESP=E | Live backing |
| --- | --- |
| B6DAE0 | actual mutable incoming argument E+4 |
| B71400 | nested base argument E-8; direction6 at E-20 |
| B700E0 | own39 at E-9C: reciprocal0, delta1..3, normalized4..6, view7..22 at E-80, inverse23..38 at E-40 |
| B700E0 nested calls | position argument E-AC; builder arguments4 at E-B8; builder locals13 at E-F0; final world argument E-AC, reusing former builder up.z |
| B63F10 | locals13 at E-34; actual target/up4 at E+4 |

The camera position callback's nested base argument also occupies B700E0 E-B8. Nested world-edit frames may overlap regions whose previous contents are dead. Only each callee's live return outputs have a comparison contract; later calls do not promise preserved dead private scratch. The earlier raw-world packet supplies the corresponding128B derive/24B direction views.

## Builder arithmetic and library closure

B63F10 initially passes the actual up-word address to419440. It captures the **current target pointer only after that CRT-capable call**, then captures eye values. Incoming up3 is overwritten by target-minus-eye; its next length call uses that same actual address. The original target-argument word becomes length, reciprocal, dot/branch and translation scratch. Locals hold reciprocal, forward3, normalized-up3, right3 and eye3. Four genuine length calls and two genuine cross calls retain x87 stack values, unordered branches, float32 spills and interleaved output/input/scratch stores.

The original `POP EDI` at B6423A changes ESP-relative offsets by4. The new kernel maps physical backing addresses across that shift, including the same target-argument word used by the three final translation spills. All309 native instructions and122 stack references were audited; the compiled kernel retains the259 floating/SSE mnemonic sequence. Pure binding loads and a new prologue/epilogue replace native private stack management.

Negative zero D7A208, threshold double D62BA0, perturbation double D7A3A0 and one D7A24C are borrowed current cells. The builder reloads one at B64232 after its CRT-capable calls and initial output stores, independently of the pose's earlier B7016F read. No constant or vector snapshot replaces those read points.

Published419440 and4F9B30 already operate on raw scalar pointers with the original arithmetic/store schedules. They are reused directly; no library engine is duplicated. Their own unexposed private frames, saved-register additions, pushed argument temporaries and fault-time observations are outside this packet's alias contract. The genuine `legacy_crt_87except_00c27489` can bind `CameraAxesCrtAccess` with the same owning runtime's live0109DD78/E16BD0 cells and thread errno accessor. Its documented host errno transport and reserved FPIEEE-record initialization limits remain inherited; this packet does not claim original private CRT-record/FH3 identity. Missing CRT binding is rejected by a pure source-interface check before native work, with no success/default seam.

## Verification and limits

Every1336 native byte equals live Ghidra and the original PE. All four existing functions have complete extents; no definition or flow repair is needed. The report includes11 direct call rows, two indirect calls and the indirect tail jump. Worker Ghidra activity was read-only. Existing descriptive names and typed implementations remain recorded separately.

One ignored executable compares five original/source pairs: finite near-parallel builder; builder output overlapping its actual four-word argument block; full pose with current target/profile mutation after genuine world dispatch; NaN pose whose second genuine CRT event changes one after the pose capture; and position input.x aliasing the actual argument word before that word becomes the world pointer. It uses genuine raw B6F5A0 prefixes, real string pool,419440/4F9B30/inverse/world providers and genuine CRT/errno handling. The camera tail is explicit fixture preimage storage, not proof of full B71A80 construction or companion admission.

The copied original covers1336B plus the173B published direction dependency. Only13 direct rel32 operands and7 absolute constant operands are relocated; all indirect instructions remain unchanged. Actual profile words reference fixture-owned tables with exact genuine-provider bridges; source resolution maps the same table identities to native targets. Original indirect world calls switch to a separate live fixture stack while running the genuine source world provider, so its explicit native scratch cannot overwrite an active host adapter frame. All selected native scratch is one live DWORD backing with disjoint pointer views.

Callback changes are **declared harness instrumentation after a genuine world or CRT provider**, not recovered mutations performed by B71460 or C27489. The probe compares normalized actual identities, whole initialized node payloads, each live builder/pose output region, real callback/CRT traces, errno and masked x87 status. It excludes dead nested scratch and normalizes a retained eye-pointer identity across distinct host frames. The late builder-one read is dynamically exercised; native/compiled ordering also establishes the earlier pose read and live-z34 lookup. Near-parallel branches and exceptional cases are bounded examples, not exhaustive geometric coverage.

Strict Win32 build and all three existing CTests pass; the focused five-pair probe passes. Compiled evidence confirms position argument store before y load,34 lookup while z is live, final argument write before the captured-table34 load, and an x87-neutral lookup-helper body. The required resolver must satisfy that neutrality too. Source throw paths, asynchronous mutation, excluded private-stack aliases, unmasked native faults/FH3 transport and game runtime are unexercised. No tracked test suite or constructor/lifetime activation was added.
