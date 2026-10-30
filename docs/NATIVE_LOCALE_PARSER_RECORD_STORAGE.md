# AA0130 completed-record storage

This uncalled ordinary C++ fragment covers [AA0130,AA0257), not full AA0020.
Its genuine caller has already reached mode3's completed-record branch after
storing0D00 and observing preceding0A00. No branch-ready flag, stream, parser,
semantic LocaleRecord or empty/success-only substitute is created here.

Immutable Context fixes actual manager, parser-mode/index DWORDs, token1/2,
rawWORD buffer/current delimiter slot, mapped CE3A70 dot and SAME AA8/AA4/AA0
cells/canonical pool. Retain the SAME read-only/native/provider graph. The pure
caller-owned nonmoving Frame reserves uninitialized key8 scratch BEFORE its
nested actual map Context/Frame; no key/header payload is read in preparation.
Keep the entire context/frame/key/nested attempts alive on unresolved failure.
Its default destructor performs no native cleanup, recovery or process stop.

Native effects retained:

- Mode1, current WORD0, preceding WORD0, index0, using the supplied current
  slot without recomputing it after index reset. Eligibility is caller proof.
- Scan token1 to NUL; store actual dot/NUL WORD; capture second-token count
  INCLUDING NUL; rescan token1 end; copy forwards in DWORD groups then BYTE
  remainder. Do not replace overlapping REP semantics with memmove/strings.
- Zero key length/data; scan merged length; actual raw41DD40(preserve1).
  Capture key.data then key.length immediately at the AA01B7/1BD epoch, before
  copy/map/wide effects. Key copy uses capturedlength+1. This phase is state1:
  resize/copy escape does not select a made-up caller key destructor.
- State2 only immediately before real A9FC30, whose preinstalled exact nested
  frame supplies a private normal-result borrow of actual node+8. Metadata
  fences remain OUTSIDE selected-action catches; failure retains, not rollback.
- AFTER map, scan CURRENT rawWORD buffer to NUL with native length arithmetic,
  call genuine raw4C53E0(preserve1), then capture CURRENT value.data once and
  stream-copy CURRENT input WORDs through AND INCLUDING NUL. Value is map-owned;
  caller never destroys/releases it. It may be a duplicate or new empty value.
- Consume state2→1 before normal key return. Use EARLY captured data/length,
  not current header. Null skips getter; otherwise actual419CC0/BD1510 sees
  actual AA4 after getter. Do not reread ended key data or retry failed return.

ONLY an ordinary C++ exception escaping actual map or wide invocation selects
CURRENT key41DD20. Consume edge first; a second cleanup failure terminates;
rethrow the original after normal cleanup. No generic catch cleanup exists.
Source preflight/reentry/post-normal guards instead latch failure and retain
all partial native debt. Caught failure cannot overwrite a healthy completion.
Actual historical returns are not retracted or manufactured from diagnostics.
Unpublished allocations, map count/node/value and key header are never repaired,
cleared, freed or retried. Startup health is not safe trim/exit/quiescence proof.

The fragment performs no input/vector/stream/lanx retirement. Full AA0020's
selected map has input/vector/key/key/concat actions and NO stream action;
normal converted/original streams release through captured CE2220 only in the
parent. Full raw scanner/input provenance, aggregate lifetime and raw16 growth
remain missing caller dependencies. Existing providers keep their own current
CRT/copy/zero-call/unfenced/exception limits; local byte fences are not thread
synchronization or transitive proof. Native ABI/FH3/SEH/hardware/private-stack/
double-failure, full loading/startup/Font, runtime, Page/P4 and production claims
remain false. Raw WORDs are not projected to std::u16string or transcoded.

Evidence: saved AA0130..0256, contract0a342da6, exact provider pin and strict
Win32 object compile. Four new files only; existing providers remain unchanged.
No tests, probe executable, game, Ghidra mutation or new Native body credit.
