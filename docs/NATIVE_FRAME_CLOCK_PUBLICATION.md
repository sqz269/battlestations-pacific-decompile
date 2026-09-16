# Borrowed frame-clock publication adapters

R38 packet A adds five source operations over a borrowed
`NativeFrameClockPublicationContext`. It owns no clock, allocation, manager,
registration or executable table. Application, sound, input and renderer
bindings remain unchanged.

Each call loads the current `01090AB0` publication once, requires a live actual
80h clock with profile identity `00D68D50`, checks the corresponding word in the
borrowed profile, then invokes the existing source implementation:

| Operation | Slot | Existing provider |
| --- | --- | --- |
| Advance | +08 | `update_native_frame_clock_00bedc30` |
| Current | +14 | `get_raw_timer_current_00bee050` |
| Interval | +1C | `get_raw_timer_interval_00bee070` |
| Sample | +20 | `sample_native_frame_clock_00bee080` |
| Enable fixed | +24 | `enable_fixed_native_frame_clock_00bedb20` |

Current and interval return the actual 16-byte member pointer, without creating
a semantic `FrameClock`. Sampling forwards the caller's writable timestamp
storage unchanged to the existing raw sampler. Fixed mode forwards signed
32-bit milliseconds. No helper adds a QPC/QPF success branch or performs another
update or sample.

The publication cell, actual storage, profile data and method context must
outlive each call. Returned member pointers remain borrowed; publication reload
does not extend the old owner's lifetime or synchronize destruction. The caller
must supply genuine accessible 80h storage: checking a profile DWORD does not
prove allocation size, lifetime or pointer validity.

Null publication/profile/output and unsupported object or slot identities raise
`std::logic_error`. This is an explicit source contract boundary, not the game's
fault/exception ABI. The R35/R33 internal dispatch guards are unchanged. Original
numeric method words are never called as executable addresses.

These helpers serve current-publication callsites. The application frame loads
AB0 separately at `00737AF6` (update) and `00737B03` (interval); sound startup
loads it at `00A887EA` for current slot +14. A consumer such as camera axes
`00B46C89` that captures the owner earlier must preserve that capture, rather
than replace it with a later helper reload.

The R37 application contract remains the integration plan. This packet creates
no startup producer, deletion binding, worker callback, semantic-clock migration
or active-rendering proof. R35/R36 own method arithmetic and raw lifecycle;
R38 validation is limited to publication dispatch and the focused fixture
recorded in `reports/native_frame_clock_publication_r38.json`.
