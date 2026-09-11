# Sound instances and FMOD channel playback

Addresses: 00A79880, 00A799B0, 00A7A2E0, 00A7A4C0, 00A7A570,
00A7A630, 00A7A660, 00A7AAA0, 00A7AF10, 00A7BD90, 00A7BF40,
00A7C480, 00A7C5D0, 00A7D5A0, 00A7F640, 00A81860, 00A81870,
00A818C0, 00A818E0.

## Recovered behavior

The existing sample runtime now feeds reconstructed channel creation, update,
completion and destruction through the installed FMOD library. Nineteen routines
are implemented in `src/sound_instance.cpp`. Descriptive names are hypotheses;
`reports/sound_instance.json` records original ABI, complete byte spans, hashes,
Ghidra changes and the exact validation provenance.

Sample getters return options at actual sample+8, PCM length from resource78+1C,
and the borrowed FMOD sound from resource78+10. A818C0 tests that sound pointer;
it is not an event discriminator. Manager vtables D5B000 and D5B44C both contain
A7F640 at slot0C. The factory increments manager160 before any rejection, accepts
only samples with an FMOD sound, allocates5Ch, grows the signed class table as
needed and constructs base52h plus D5ABF8 channel state. No active-list insertion
occurs in this factory; the existing voice routine performs that separate retain.

The base constructor writes refs1, consumes/increments global F8BBD4, retains the
actual sample, retains the current class slot and copies the sample50 name. It
sets independent frequency, volume and fade factors; options4 supplies volume24.
The destructor releases sample then current class then name and base. It clears
owning pointers only after their callbacks, and preserves the dead name header.
The constructor's DEB100 unwind map is narrower: class44, name0C and base only.
It omits raw sample4C despite the earlier retain. That retained reference on name
allocation failure is preserved, with a focused injected-failure check; the
fixture explicitly removes that extra reference afterward. The factory's DEB7B4
map frees its allocation. Native SEH and allocator failure ABI are not claimed.

A7A4C0 creates an FMOD channel with index-1 and paused=true. Non-looping samples
receive loop count0, then all eight speaker-mix inputs are1. Result2B triggers
the original memory-stat query without aborting the remaining sequence.

A7AF10 consumes dt plus an unused second stack word (RET8). It skips stopped
instances, advances fade through existing0042AC60, decreases a positive delay,
creates a channel if absent, and increments update18. When dirty, it applies
resource frequency times instance20, the combined volume, and conditional3D pan
and speaker mix, then clears dirty14. It updates paused state from the bitwise OR
of delay38 and `(instance1C != 0 || class0C != 0)`, queries discarded audibility,
and updates virtual-channel history. The volume path preserves native x87
grouping and three explicit float stores rather than flattening the factors.
The countdown's FCOMIP/JC skips clearing delay38 for an unordered result, so a
NaN remaining duration preserves the flag. The local fixture checks that branch.

Virtual slot14 (A7A630) returns **not FMOD isVirtual**. It does not call isPlaying.
Slot10 (A7A660) updates ended5A using initialized-false isPlaying output, with
FMOD results24/B also treated as ended. Slot0C (A799B0) refreshes slot10 only
after stop_requested51; without that request it reads cached ended5A. The update
routine itself does not refresh ended5A. These distinctions preserve the
manager/voice caller's responsibility to refresh completion.

Stopping sets request51 first. Immediate stopping, a virtual channel, or absent
deferred-stop policy calls FMOD stop and sets stopped15. For a nonvirtual channel,
StopAtLoopEnd has priority and sets loop count0; StopAtSampleEnd also sets the
loop range to PCM0..resource_length-1 with unit2. Both deferred paths return with
stopped15 unchanged. Channel destruction calls FMOD stop even for a null channel,
then destroys the base. Scalar destructors free only flagsbit0 and return the
original pointer; their two incorrect `_free` fallthrough gaps were repaired.

## Composition and supported domain

`SoundInstance` is a C++ projection. Its inherited `SoundLevelEntry` and
`VoiceSoundStartFields` are the canonical manager and voice fields, as exercised
by existing dirty walkers and the voice delay setter in the fixture. It retains
the actual7Ch sample and existing class descriptor. This is not a52h/5Ch binary
layout: never pass a projected instance to code that reads raw object+4 or uses
native vtable offsets. Scalar free paths require the matching C++ allocation.
An application lifetime host must operate on the projected reference field.

`NonspatialSoundChannelVirtuals` binds D5ABF8 slot10/14 to the recovered routines
and rejects other derived tables. Other virtual calls and spatial/event classes
remain explicit follow-ups. The supplied context reloads the current owner at
native global-read sites; callbacks can replace channel/sample/class fields.
Calls are serialized, sample/resource/class pointers must remain live, and class
indices follow the existing nonnegative/nonoverflowing table contract.

The twelve FMOD channel bindings call actual installed stdcall C exports. C++
bool pointers are translated to four-byte FMOD_BOOL outputs. A7A630 originally
reads an uninitialized bool if FMOD fails without writing it; that undefined
native output is rejected explicitly rather than treated as successful playback.
Resource PCM/frequency queries require fields initialized by a bank load.

## Validation

Win32 Release and both existing CTests passed. No permanent test was added.
`local/sound_instance_fixture.cpp` loads the installed splash_medium.fsb through
the existing VFS, resource and sample runtimes and executes actual channel
creation/update/destruction using FMOD's explicitly selected no-sound device.
Independent FMOD queries verify pause, no position advance while paused,
frequency176400 and mixed volume0.1485. The channel naturally finishes; a second
channel verifies deferred PCM loop points0..154623 and immediate stopping.
Existing manager dirty marking and voice delay fields affect the same instance.
Retained samples, classes, pooled names and resource/cache removal are checked.
An installed planes.fev event is correctly rejected by this factory while its
request counter still advances. This is FMOD runtime evidence, not audible or
gameplay validation, and not differential execution of the original routines.

Three missing functions (A7F640, A7A630, A7A660) were defined from verified disk
and analysis bytes. Nineteen prototypes/names/comments are saved with prior
values retained; correct compiler-generated scalar-destructor names remain.
No imported library was reconstructed, no no-return flag was changed, and the
original installation was used read-only.

## Follow-up packets

- Manager slot10 A7F710 and constructors A89DC0/A8A2E0: spatial/event instance
  construction, their larger layouts and additional register/stack inputs.
- Active instance removal and manager completion traversal: bind projected
  instance lifetime and the manager's refresh/retain/release sequence before
  claiming full voice/effect playback composition. Current append code is
  separate and must not acquire duplicate owning storage.
- A7B520/A7B5D0 and related derived update routines: global time interpolation,
  scale composition and their complete assembly/ABI. They were inspected only
  as candidate dependencies and are not counted among this packet's routines.
- Audible-device execution and gameplay validation remain open.

## Follow-up from docs/SOUND_SYSTEM_UPDATE.md

The manager now calls channel update, transition and ended/completion queries in
the recovered two-pass order and retires entries through swap-last removal.
`SoundChannelRuntime` binds projected D5ABF8 references and virtual calls to the
canonical active array. Installed FMOD testing covers tracked creation through
automatic manager retirement and final sample/cache cleanup. Spatial/event
profiles, complete application/voice composition and audible output remain open.
