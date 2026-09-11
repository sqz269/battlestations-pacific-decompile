# Sound sample construction and lifetime

Addresses: 00A84D70, 00A83850, 00A828B0, 00A82C60, 00A82E80,
00A82B70, 0093F6E0, 0093F950, 00A7A9E0, 00A7B3D0,
00BEF100, 00BEF170, 00BEF220, 008D4390, 008D5B20, 00426520.

Names are hypotheses. `reports/sound_sample.json` records original ABIs,
full byte-span hashes, stored-body limits, annotations and validation provenance.

## Storage and concrete composition

`SoundSampleStorage` contains the actual 7Ch sample allocation. The vtable word
and intrusive count occupy 0/4; load options start at8. Parameter records use
the actual data/count/capacity header44/48/4C. Pooled names occupy50/58/60;
the event group is68, a raw pointer array is6C/70/74, and resource is78.
Parameter records contain index0, minimum4, maximum8 and borrowed nameC.
Resource pointers refer to the existing `SoundOwnedResource` projection.
These C++ interfaces are not original binary-call replacements.

`SoundSampleRuntime` implements the previously required sample constructor
service in `SoundSampleCacheHost` and supplies concrete final release. It
combines the recovered sample code, canonical VFS, resource loader, installed
FMOD entry points and current owner/cache publication slots. It reads the
current owner's event-system handle at the parameter-query call site.
Unknown cache/sample profiles fail explicitly. Application composition still
has to supply its real owners, VFS, pooled storage and resource-load event policy.

The resource-loader boundary projects current actual options into its existing
typed record. It leaves native-unwritten timing explicitly unknown at these
constructor call sites. That adapter does not reproduce aliasing into options
from arbitrary resource-provider callbacks. Scanner/VFS and resource owners
remain their established projections; sample fields and retained references
are not replaced by shadow sample objects.

## Constructor and settings

00A84D70 is ECX=this, one name argument, EAX=this, RET4. It initializes only
the observed fields: mode0, volume/pitch1, distances10/1e9, flags and vectors0,
active limit-1. Padding and timing30/34 remain unwritten until the bank branch
publishes timing. Name58 copies the caller's original string.

The first colon selects an event. Name50 retains the complete name. The
prefix is resolved through the current VFS; suffix60 is the event name.
Load00A84740 uses clone=false and load-if-missing=true, then00A83850 loads
the group. Mode8 becomes1 before00A828B0 reads parameters.

Without a colon, resolve58, construct50 by removing four trailing bytes and
appending `.def`, then resolve50. If found, construct the scanner with the
copied definition name and mode32h, parse, destroy it, then load resource58.
If no definition exists, load the original caller's name, even when58 was
rewritten. Publish PCM length30 and x87 FLD/FSTP duration34 from the resource.
Short-string substring arithmetic delegates the already recovered helper.

Settings, in native comparison order:

| Token | Sample write |
| --- | --- |
| 3D / 2D | DWORD8 = 1 / 0 |
| Vol / Volume | floatC |
| Pitch | float10 |
| FreqRndHz / VolRnd | float38 / float3C |
| Linear / Loop | byte28 / byte1D = 1 |
| LoopPointsMillisec | bytes1D/1E=1, floats20/24 |
| StopAtLoopEnd / StopAtSampleEnd | byte29 / byte2A = 1 |
| MinDistance / MaxDistance | float14 / float18 |
| Decompress | byte1C = 1 |
| mActiveLimit | int2C |
| Ambient | bytes1D/40=1, DWORD8=0 |

00BEF100 and00BEF170 use CRT `%d`/`%f`, including accepted numeric prefixes.
They write success, accept only successful conversions, and return zero after
whitespace handling on failure. Failed tokens remain cached. Float conversion
first rounds to32bits; the native result is returned onST0. 008D4390 accepts
only a case-insensitive match. 008D5B20 has no normal-path effect: unknown
settings loop forever on the same token. The parser preserves that behavior.
Valid input and the existing1023-byte tokenizer bound remain explicit domains.

## Events, arrays and destruction

00A83850 splits60 on `/`, discarding empty spans. Segment1 selects the project
group, and segments2 through count-2 descend into groups. Outputs target actual68.
LoadEventData receives0/0. Before/after FMOD memory delta updates the CURRENT
resource78's size28. Pooled segment names are destroyed in reverse order.
00426520, 00BD20A0 and00427110 retain an explicit vector/string contract here;
native container allocation and arbitrary allocator reentry are not reproduced.

00A828B0 queries the current event system with mode4, reserves the actual
parameter array, then resizes before every parameter query. Range, borrowed
name and index outputs use current array data after each callback. No event
reference release is added. The DLL adapter uses all nine verified stdcall
exports for group/event/parameter operations. Required output pointers must be
available; host guards reject native indeterminate-output cases. Only error2B
triggers the recovered memory-statistics callback.

0093F6E0/0093F950 manage16-byte parameter records; 00A7A9E0/00A7B3D0 manage
four-byte pointers. Reserve clamps capacity to1 and publishes data/capacity
after freeing old storage. Resize zeroes growth and changes count on shrink;
neither array owns referenced names/pointers. Valid signed counts, live storage
and serialized access are required; native allocation-failure ABI is not claimed.

00A82C60 writesD5B074. For a nonnull group, it calls FreeEventData(null,1)
twice, reloading68 for the second call, then accounts the memory delta against
current78. It removes the first cache entry whose weak pointer equals this
sample through current slot8 (00A82B70), releases resource78 and clears it only
after release returns. Cleanup proceeds through pointer array6C, names60/58/50,
parameter array44 and refcounted base. Dead name/array headers remain dangling.
00A82E80 frees the allocation only for flagsbit0 and returns the original pointer.

Constructor unwind mapDEBF50 states5..0 identifies exactly those member
cleanups, excluding resource78. The C++ exception path follows that member
order; it does not invent a resource release absent from the unwind map.
Original SEH frame/exception ABI is not implemented. 00BEF220 also has a
separate actual828h scanner destructor: free/clear nondefault separators,
decrement/release/clear stream824, then destroy pooled filename81C/820.
The immutable-byte runtime scanner retains its VFS stream until destruction
and then releases its pooled filename, without claiming the native scanner ABI.

## Evidence and validation

Six internal free-call gaps were repaired and refreshed. Tails of00A83850 and
00A82C60 were decoded, but their stored bodies still endA839AB andA82D71.
Full spans through RETsA839BF/A82E16 match disk and analysis bytes. The latter
tail includes a second free and base destruction, verified from raw assembly.
No callee no-return flag or shared server setting was changed. A82B70's
STL-probable tag was retired with its prior record retained.

One focused local fixture checks three constructors, sidecar/original-name
selection, numeric prefixes and failed-token retention, padding/unwritten
fields, cache hits, event callback array growth, two group frees with pointer
replacement, memory accounting, scanner cleanup and constructor unwind.
It also uses the installed DLLs and real VFS callbacks to load/release
`sound/splash/splash_medium.fsb` and the event name from `effects.lua:112`:
`planes.fev:planes/muzzle/muzzle5inch/muzzle_5_inch`.
The bank supplied154624 PCM frames/3.50621seconds; the event supplied2 parameters.
Both sample and resource caches were empty after release, with pooled strings
balanced. The output device was explicitly no-sound. This proves installed
resource/library execution, not audible playback or game validation.

Win32 Release and both existing tests passed. Exact source, artifact and fixture
hashes are in the report. Follow-ups include effect playback/update ownership,
remaining effect families, complete auxiliary-owner teardown and application
composition. The continuing runnable-game objective remains open.

## Follow-up from docs/SOUND_INSTANCE.md

The non-spatial manager factory and FMOD channel lifecycle now consume these
actual samples. Installed-bank channel creation, pause, natural completion,
deferred/immediate stop and retained sample cleanup are runtime-tested with
FMOD's no-sound output. See `docs/SOUND_INSTANCE.md` and
`reports/sound_instance.json`; audible output, spatial/event instances and
application-wide voice/effect composition remain open.

## Playback evidence from docs/SOUND_EVENT_INSTANCE.md

The earlier FEV acquisition fixture established metadata and lifetime behavior,
but did not require LoadEventData to succeed. The first event playback fixture
showed result23 for the missing basename muzzle_5_inch.fsb, followed by GetEvent
mode2 result24. Mounting the installed sound/muzzle directory supplies that
basename and permits actual parameter/update/playback/retirement checks. Keep
acquisition proof separate from successful event-data loading and playback.
