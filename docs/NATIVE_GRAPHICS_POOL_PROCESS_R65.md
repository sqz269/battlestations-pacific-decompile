# Native graphics-pool process ownership (R65)

Addresses: `00CD7B80..00CD7CB5`, `00CE0CB0..00CE0D49`; pool algorithms
`00B3EC60`, `00B3E2B0`, `00B3E390`, `00B4AB80`, `00B4A190`, `00B4A270`.

## Result

Production startup now initializes the ten contiguous native CRT entries at
`00CE3518..00CE353C` against distinct process-owned 38h pool storage, sharing
the existing `00E188B4` allocator list. `game_main` calls them in table order:

| Storage | Pool | Initializer | Exit |
|---|---|---|---|
| 0108DB70 | Cube texture | CD7B80 | CE0CB0 |
| 0108DBA8 | Volume texture | CD7BA0 | CE0CC0 |
| 0108FBF8 | Material pass | CD7BC0 | CE0CD0 |
| 0108FD38 | Vertex declaration | CD7BE0 | CE0CE0 |
| 0108FD70 | Layout record, 44h slots | CD7C00 | CE0CF0 |
| 0108FDA8 | Physical index | CD7C20 | CE0D00 |
| 0108FDE0 | Physical vertex | CD7C40 | CE0D10 |
| 0108FE18 | Logical vertex | CD7C60 | CE0D20 |
| 0108FE50 | Logical index | CD7C80 | CE0D30 |
| 0108FE9C | Hardware layout, 48h slots | CD7CA0 | CE0D40 |

The previously absent full volume initializer (22 bytes, cdecl EAX atexit
status) and exit callback (10 bytes) use the existing surface-pool algorithm
against **distinct volume storage**. Other native wrappers already existed;
this batch supplies their application process ownership. Descriptive names
are hypotheses. These source C++ interfaces are not binary replacements.

Initialization is attempted once per global. A completed attempt retains its
real atexit result; nonzero registration does not roll back the native pool.
A thrown attempt is not retried. The shared allocator process is constructed
first, and the graphics bookkeeping object completes static construction
before registering native callbacks. Native CRT cleanup therefore runs while
both owners remain alive. Payload destruction must precede pool teardown.

## Production bootstrap correction

The first actual application run rejected its own RO mapping request: the
canonical CRT bootstrap still required four bands (`018A`), while current
`game_main` required seven. Both parent admission and child ownership transfer
now use the exact `038F` set: CE, CF, D0, D1, D5, D6, D7. The existing checks for
the inherited capability, supported PE, reservation ownership and read-only
protections remain; mutable CRT ownership/cookie initialization is unchanged.

The application then logged all ten initializer calls with atexit result zero,
and progressed through VFS, renderer capability gathering and settings loading.
It exited with `C0000005` before a verified frame. A bounded debug run located
the fault in the installed `xlive.dll`, RVA `319049`: `MOV [EDI],EDX` attempted
to write `10640F5E`. Its live bytes match that module's disk bytes. This locates
the fault; its caller and cause remain unresolved. Debugger-specific invalid
handle exceptions were recorded and continued; the AV was observed at both
first and second chance and matches the undebugged process exit. Raw stack
words are candidate addresses, not an unwound stack. No game-install DLL was
changed. Earlier R64 device-availability failures do not describe this run.

## Evidence and verification

- Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three existing CTests
  pass. No permanent tests were added.
- The focused source process fixture checks all ten distinct storages, profile
  words, independent 32-pointer tables, shared-list order and repeated startup.
  Separate physical index/vertex pools grow through 33 slots each. Cube,
  volume, pass and hardware slots allocate/return; shared-list trim runs.
  Real CRT callbacks unlink all ten and drain a retained critical-section
  depth of two while bookkeeping remains alive.
- A separate native fixture executes the complete copied CD7BA0/CE0CC0 bodies
  with relocated storage/call edges to genuine pool providers and real atexit.
  Native initialization and the copied exit callback both execute successfully;
  the latter runs once on the same volume pool during actual CRT shutdown.
- Fresh live Ghidra and installed PE agree on 51 spans, 1,626 bytes. The native
  table, nine profiles, constructor metadata and ABI edges are retained in the
  report. Linked source intervals and physically resolved I386 modules are
  recorded with hashes; map intervals are not native function boundaries.
- Four incorrect CALL_RETURN overrides were repaired across B3EC60/B3E2B0/
  B3E390, restoring 72 bytes of fallthrough. No callee no-return flags changed.
  Six remaining missing instruction starts are unreferenced alignment islands
  skipped by unconditional jumps. The new volume function was defined, and
  names/comments preserve previous values in the annotation journal.

Evidence: [report](../reports/native_graphics_pool_process_r65.json). Exact
source/probe/compiler/build closure is sealed locally before integration;
combined integration-build evidence is recorded separately.

## Follow-up packets and limits

1. Diagnose the installed xlive fault against its actual caller and library
   contract without modifying the original installation or replacing calls
   with success stubs.
2. Compose the full reconstructed renderer/device contexts using these
   canonical pools. The current application renderer remains a milestone host.
3. Validate full device/resource shutdown, original failure/EH paths and active
   frames before claiming ABI or gameplay parity. The focused fixtures do not
   establish any of those properties or complete CRT-table coverage.
