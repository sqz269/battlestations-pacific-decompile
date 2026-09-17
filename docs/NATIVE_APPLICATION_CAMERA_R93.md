# Application camera providers and cockpit runtime, R93

Addresses: 00CD7D80, 00CD7DD0, 00B1FF60, 00B71A80, 00B3C800.

## Result

The application renderer now supplies the existing native camera and cockpit
constructors with its canonical camera pool, shared type counter/root/node cells,
raw string pool, node lifetime domain, current renderer parameters, original
read-only constants and one installed viewport registry. This is composition of
existing reconstructed bodies, with no additional unique body credit.

`GameNativeResourcePoolProcess` owns source storage for 0108FFB0 and invokes
CD7DD0 once, preserving its atexit result and CE0E30 registration. GameMain calls
it after the represented Lua pool startup and before CD7E40 mesh startup. The
shared E188B4 allocator owner outlives these callbacks.

`GameNativeTypeStorage` owns camera guard 0108FF9C and descriptor 0108FFA0. The
represented type sequence adds CD7D80 before the later resource type entries.
Every camera bootstrap borrows the existing counter and root/node descriptors.
This preserves represented relative type order, not full original CRT timing or
absolute numeric IDs. The observed application camera/node/root IDs were 6/5/0.

The private camera graph shares `DestructionGraph` node/scene/attachment owners.
Its raw viewport adapter checks the captured renderer's current D5F0A8 profile
and current slot30 B1FF60 each call, then borrows exact DWORD references at
renderer+1A20/+1A24. B1FF60 itself returns renderer+1A14. The old typed interface
carries the raw address here; semantic renderer methods are never invoked on it.

The cockpit decrement provider is a retained source equivalent of CE2220,
resolved from Windows' actual `InterlockedDecrement` export. Original numeric
IAT contents are neither called nor rewritten. D5E5F8, D62CF0, D62C88 and scalar
cells are borrowed from the application's verified original read-only mapping.

## Evidence and validation

- Strict MSVC Win32 build and all three existing CTests pass; no new repository
  test suite was added.
- Five original bodies total 1,051 bytes, plus 16 bytes of CRT entries, viewport
  dispatch and camera far data, match both live Ghidra and the installed PE.
  All 25 direct call rows are checked; four indirect calls are separate.
- One focused startup probe links 51 current production application objects and
  three current libraries. It uses the actual application renderer/device, pool,
  types, strings and scene lifetimes; only the forwarding main supplies calls.
- Two B3C800 cockpit constructions complete. The helper/camera/current viewport
  reference counts are 1/1/1. Near/far and clear setters, four unwritten helper
  fields, canonical identities and viewport registration are checked.
- First construction reads 800x600. The probe temporarily changes actual renderer
  parameter width/height to 640x360, and the next constructor reads those current
  values. It restores 800x600 before entering the normal application loop.
- Both helpers retire through B3C6C0/B3C5C0 and the canonical camera zero callback.
  Camera bindings disappear, viewport records become quiescent, and the second
  camera reuses the first returned pool slot. No live slab preimage is seeded.
- Probe and unmodified application each exit 0 after two ticks/one Present. The
  native renderer worker joins, canonical render owners are empty in the probe,
  and final device/API COM release counts are both zero.

The first runtime assertion exposed an adapter bug: MSVC's `uint32_t` is
`unsigned int`, while `DWORD` is `unsigned long`. Returning a converted temporary
through `const DWORD&` produced dangling parameter references despite a clean
build. The adapter now borrows the exact DWORD type. Failed executables/logs and
the pre-fix adapter/object are retained beside the successful evidence. The
initial failed probe also unwound a live construction block and triggered its
lifetime guard; the diagnostic probe retains incomplete state until process exit.

The report `reports/native_application_camera_r93.json` records source/object
hashes, native call rows, runtime receipts, preserved annotations and immutable
tested/integrated archives. All five descriptive names are preserved; appended
Ghidra evidence retains prior comments, is saved, read back and re-exported.

## Boundaries and next work

Normal application startup instantiates these providers but still does not call
B14A10. The two cockpit calls belong to the focused probe. All native camera
consumers must retire before shared singleton drain; construction records must
be forgotten only after host quiescence, before destroying the camera graph.

Next compose B14A10 using this camera graph and the R92 actual texture cache,
establish full render-resource parent teardown, then advance B107F0. Full material
compiler/post-effect integration is still open. These runs do not establish
failure/reentry/concurrency behavior, original ABI/FH3/SEH, pixels or gameplay.
