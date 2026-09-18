# Native game profile destruction (R124)

Addresses: `007FD8A0`, `007FB340`, `0061F630`, `007FF9F0`; consumed library
full-range branch `007FB230`; parent binding `004DCF90`.

R124 closes the game destructor's profile, race-record and direct string-pool
cleanup bindings. Four complete normal bodies total **889 bytes**. They compose
the existing raw mission-progress, tree, list and string services over the same
actual pool publication, shutdown gate and singleton manager. Ordinary raw-game
admission, remaining parent services, native exception handling and gameplay
remain open.

## Bodies and native interfaces

All four original entries take the actual owner in ECX, no stack arguments,
and return with RET. New C++ context/progress arguments change that interface;
these are source reconstructions, not drop-in binary replacements. Names are
descriptive hypotheses unless an existing library name is retained.

| Entry | Bytes | Established behavior |
| --- | ---: | --- |
| `7FD8A0` | 631 | Complete normal destruction of the actual F8h profile at game+650. |
| `7FB340` | 70 | Capture bonus-vector begin/end; destroy ascending 1Ch records; free current begin; zero begin/end/capacity, including on null begin. |
| `61F630` | 152 | Release three actual string headers at 10h, 8h, then 0; preserve all record bytes. |
| `7FF9F0` | 36 | Stamp D08D20; capture and release the actual name at 8/C without clearing it. The earlier typed RaceRecord interface remains separate. |

The record producer `7F8B40` confirms three strings at 0/8/10 and a final integer
at 18; `7FC940` appends these records to profile+BC at stride1Ch. Its established
role is a **bonus record**, as documented in `SCORING_BODIES.md`. R124 adds no
new semantic record projection or field ownership beyond that evidence.

## Profile schedule

`7FD8A0` stamps D08D1C and then:

1. Captures optional mission-progress owner64, invokes its actual destructor,
   frees the captured owner, and clears current64 after the callback.
2. Clears alias listCC; frees its current sentinel and clears headD0.
3. Destroys bonus vectorBC, then string vectorAC. Both capture traversal endpoints
   and reload the begin pointer before freeing it.
4. Clears full current tree ranges A0,94,88,7C,70, in that order; each frees the
   current sentinel and clears head/count afterward.
5. Releases strings68,50,3C,34, each capturing data and length+1 before the getter.
6. Destroys the actual plain list14; clears alias list8; frees its current sentinel
   and clears headC. It does not free the enclosing profile.

Every nonnull direct string release repeats the real419CC0 getter before the
BD1510 return. This remains true for large allocations and disabled small returns.
Captured data/size survive getter callbacks; later headers are read when reached.
The bonus-vector final integer and all string header bytes remain unchanged.

## Consumed collection contracts

The source reuses full raw4D1A50,4D05E0,432050,7F8310,7FD780 and their existing
dependencies. The counter-range58D860 and transient-range7FB230 adapters expose
only the actual full begin/end pairs produced by this parent. They reject other
ranges before cleanup. General partial7FB230 erase is **not reconstructed here**.

For7FB230, the original201-byte body is preserved in the fixture as reference.
Its full-range branch calls existing7FA880, then reloads the current head before
each root/count/left/right reset and writes the private output iterator. The
source adapter preserves those stores. The unused partial iterator/increment
branches have rejecting fixture hooks; no successful substitute is supplied.

## Parent binding and failure ownership

Parent4DCF90 now passes an explicit profile context to its profile/race and
direct419CC0/BD1510 calls. Missing context or a foreign call service fails at the
reached binding. The context owns a retained `ActualNativeStringPoolStorage`
bridge over the actual raw cells, and must outlive operations: the nested mission
operation retains a pointer to that bridge for diagnostics.

The profile operation retains owner/context, current native site, parent unwind
state, separate bonus-element unwind state and a nested mission operation. Failure
retains the partial graph and rejects replay. Caller cleanup precedes diagnostic
acknowledgement; acknowledgement frees nothing and refuses a failed/running mission
child. The game parent likewise refuses acknowledgement while its profile child
remains failed/running. This is not original FH3/SEH unwinding.

## Ghidra repair and evidence

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, is verified.
All **1,090 reference bytes** match the installed original PE:889 reconstructed
normal-body bytes plus the201-byte consumed-library reference.

The saved profile function incorrectly ended at7FD905. A returning-free gap and
the missing tail cover535 bytes; the exact RET is at7FDB16. The function was
recreated through exclusive7FDB17 under the shared write lock, preserving prior
name/comment evidence. The bonus vector had another four-byte returning-free gap.
Final audits report no remaining call gaps. No callee-wide no-return flag changed.

Four body names/evidence, the consumed7FB230 branch description and parent binding
evidence are saved with prior annotations, readback and refreshed exports. The
report enumerates every direct CALL in the reference bodies and affected parent
bindings. The producer was read live; its implementation is not added by R124.

## Validation and limits

- Strict MSVC Win32 build and all three existing CTests pass.
- **56 paired cases**, **427 observations**, **1,553,004 matching normalized bytes**:
  original copied bodies versus source, with real raw pool/manager, reconstructed
  profile construction, actual mission owner and shared populated collection services.
- Profiles have0/1/3 added rows; all constructor-created sentinels, nine gate nodes
  and rank-map storage are retained. Added cases populate alias lists, three string
  trees, the transient pair tree, a mission counter tree, and string/bonus vectors.
  The mission score tree stays empty; its existing score-record destructor is a
  shared dependency, not newly exercised with populated score records here.
- All eight null/non-null combinations of the three string headers are checked,
  together with149/150-byte allocation boundaries, both small-return gate states,
  getter callbacks changing a captured header, post-mission publication replacement,
  and a free callback changing vector fields before their final clears.
- Normal full-profile cases drain all tracked owned nodes and strings. Every run
  retires its raw pool/manager; leaf-only cases explicitly drain their retained
  fixture graph afterward. Freed bytes and OS lock internals are not compared.
- Concrete parent pool/race defaults pass with real storage; four missing/foreign
  context checks reject before child entry. A source failure after completed mission
  cleanup retains the child, its live context bridge and remaining graph, rejects
  replay, and requires explicit diagnostic cleanup.
- The separate parent comparison passes **52 paired cases**, **3,537 snapshots**,
  **141,402,076 matching bytes** and four source failure/replay cases. Its profile,
  embedded and array call boundaries remain controlled.

The fixture uses explicit constructor literals and the unselected online-user gate;
it does not test SDK profile import or a real game session. Known allocation pointers
and arena-relative ring entries are normalized. Private stack aliases, arbitrary
invalid ranges, concurrency, hardware faults, native exception identity and gameplay
are not established. Existing shared callees are not newly differential-tested
bodies. No ordinary application run is attributed to this unreachable raw-game
cleanup packet, and no permanent test suite was added.

## Follow-up packets

Close the remaining actual game member containers, scene-record disposal, global
shutdown services, resource-manager/DYN cleanup and required virtual payloads. Then
compose raw allocation/construction and teardown with retained ownership before
ordinary application admission and gameplay validation.
