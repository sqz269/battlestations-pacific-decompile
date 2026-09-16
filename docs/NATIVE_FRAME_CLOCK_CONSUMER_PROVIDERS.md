# Sound and input clock providers

R39 adds raw-publication paths to the existing sound and input adapters without
binding an application clock. `SoundClockBinding` accepts an existing semantic
`FrameClock` or the R38 `NativeFrameClockPublicationContext`.
`NativeInputClockBinding` accepts the existing semantic publication cell or that
same raw context. Neither type has a default constructor. Each constructor
selects exactly one borrowed representation; neither creates a clock.

`GameSoundRuntimeServices.clock` and
`NativeInputDeviceRuntimeServices.clock_01090ab0` keep their names and positions.
Their member types now use these bindings. Existing aggregate initializers
continue to accept the old arguments through converting constructors. The
source aggregates grow: they are C++ composition types, not original game
layouts. No original storage field or native device offset changes.

## Raw behavior

- Sound startup uses current slot +14, copying its four timestamp words into
  the existing array result. It does not sample QPC or call slot +20.
- Sound update reloads the raw publication, copies the current 16 bytes into
  `FrameClockSoundStartupHost` result storage and returns that result pointer.
  `GameSoundRuntime::Impl` delegates to this provider; the existing update
  consumer immediately copies the timestamp before subtraction and services.
- Input reloads the raw publication and copies current into the runtime's
  16-byte result. Its existing `const ClockTimestamp&` interface is unchanged.

All raw reads use `current_published_native_frame_clock`, preserving R38 profile
and target admission. Context, publication and live clock storage must outlive
every adapter call. Result references/pointers last only until another call to
that provider or destruction. No `FrameClock` reinterpretation, dummy semantic
owner, second advancing clock or process binding is introduced.

The semantic paths remain unchanged: input reloads its original semantic cell,
sound update returns the semantic current member, and legacy sound startup
continues sampling. That last legacy behavior is a known difference from the
original current-slot startup call and is not promoted to native proof.

## Original read schedule and source boundary

| Consumer | Current call | Subsequent original reads |
| --- | --- | --- |
| Sound startup A88770 | A887F5, slot +14 | timestamp DWORDs +0,+4,+8,+C copied into owner+118..124 |
| Sound update A7E630 | A7E644, slot +14 | timestamp DWORDs +8,+0,+4,+C, then subtraction530890 |
| Joystick force A98CC0 | A98CE6, slot +14 | device+ B38 deadline is loaded **before** timestamp qwords |
| Joystick poll A98E30 | A990E0, slot +14 | timestamp qwords, x87 division/float spill, +60 seconds, store deadline |

The sound-update entry is **A7E630**; A7E590 in the R37 narrative was an
incorrect label. The audited load A7E639 and current-slot semantics were correct.

The new raw provider snapshots before returning. For joystick force this moves
the raw timestamp read ahead of the caller's deadline read. The supported
domain is serialized calls with live, disjoint, stable storage. It does not
claim concurrent mutation, original fault scheduling, original DWORD load order
or binary ABI parity. Existing consumers and their arithmetic remain unchanged.

The focused fixture and archived evidence are recorded in
`reports/native_frame_clock_consumer_providers_r39.json`. Passing these adapters
does not establish full FMOD/DirectInput runtime composition, application
startup/drain binding, worker shutdown or gameplay.
