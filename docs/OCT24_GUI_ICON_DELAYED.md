# Raw delayed Icon singleton prerequisite

`NativeGuiIconDelayedLoads` composes the real F8BD98 producer, raw cancellation
and normal drain over the application's existing actual string/01090AA0
singleton-manager domain. This closes one prerequisite from
`OCT24_GUI_ICON_LIFETIME.md`; it does not supply the authoritative Icon
allocation/derived-state/scalar provider or activate a page, widget or manager.

The component introduces no process publication itself. Its constructor borrows
the SAME externally owned F8BD98 cell, requires its fresh null preimage and
performs no native work. The caller must install the real shared deletion arm
below BEFORE any getter call. No current caller or runtime switch is added.

## Actual storage and bodies

The singleton is exactly10h: D5C4B8 at0, raw entry pointer4, signed count8 and
capacityC. Entries are actual0Ch records containing an8h native string header
and opaque raw Icon owner at8. There is no std::vector/std::string replacement,
texture wrapper, native reference count or second manager registration.

* AB5700's fast path returns its first F8BD98 capture. The slow path uses an
  actual-only `SoundLifetimeAccess` constructed from the raw string context's
  SAME manager-publication reference. `CapturedSoundLifetimeSection` performs
  the first manager lookup, captures section10 once, enters it and increments
  its real recursion18. The getter rechecks F8BD98, allocates exactly10h through
  the existing source CRT allocator, writes D5C4B8 and three zero fields, then
  publishes. A second manager lookup precedes the CURRENT publication read for
  BD0C30 registration. The slow return reloads F8BD98 after captured-section
  leave. Native null allocation behavior is retained, including the attempted
  manager registration of the null publication; no fallback object is supplied.
* AB50E0 always reads the actual count gate. Empty cancellation is that native
  branch, not an empty callback. For valid nonempty arrays it scans in order,
  copies the last matching replacement through actual419CC0/41DD40 string
  services, copies the raw owner, releases the reloaded tail name and only then
  decrements CURRENT count. A removed slot is examined again without advancing.
  The assembly's fresh source-length test and destination-length memcpy are
  preserved. No Icon/texture retain or release belongs to this registry record.
* The reached AB4850(0) shrink arm decrements CURRENT count BEFORE each tail
  name release and reloads backing/count afterward, then stores0. AB5560 frees
  the actual backing pointer without clearing its consumed pointer/capacity,
  clears the SAME current F8BD98 cell, then stamps CE3818. AB57C0 frees the
  object iff flags bit0 is set, and returns its original address. There is no
  unregister call: the ordinary process drain already popped the registration.
  For flags0 the caller owns the completed physical allocation thereafter.

Allocation and backing/object free use the existing
`singleton_lifetime_allocate/free` Source CRT domain. Raw names use
`NativeStringRawPoolContext`, including its actual string-pool publications,
manager cell and small-return flag. Abstract semantic/CRT-fallback string
services cannot be substituted through this API.

## Required shared dispatch and terminal borrower

The packet does not edit shared singleton or application files. Primary
integration must append a borrowed service pointer to
`NativeSingletonDeletionBindings`, preserving previous member offsets, and
route the actual popped D5C4B8 object to it:

```cpp
// Forward declaration and appended default-null binding:
class NativeGuiIconDelayedLoads;
NativeGuiIconDelayedLoads* gui_icon_delayed_loads{};

// delete_current_profile's current-profile switch in
// src/native_singleton_destruction.cpp (include the new service header):
case 0x00d5c4b8:
    if (bindings.gui_icon_delayed_loads) {
        bindings.gui_icon_delayed_loads->delete_registered_00ab57c0(owner, flags);
        return;
    }
    break; // Preserve the existing missing-binding failure.
```

D5C4B8 slot0 is AB57C0 directly; this singleton has no refcounted BD30E0
prefix. Update the binding-size assertion for the appended pointer while
preserving all existing member-offset assertions. Keep the service, its one
publication cell, string context and process
manager providers alive through the whole native drain. The popped pointer is
the registered creator identity, independent of the current F8BD98 value;
destruction clears that current cell even if another value was published.
The service's saved creator pointer is metadata only, never another owning
registry, count or native retain.

`requires_process_retention()` remains true while its actual registered owner
exists, while a call is active, or after an entered failure. Destroying metadata
in those states terminates; no destructor silently drains or repairs native
state. `last_operation()` retains native-site/phase/acquisition/guard/free
observations; `failure()` retains the C++ exception. A failed call cannot be
replayed. The existing captured guard does its established C++ unwind cleanup
on a failing getter; allocation/publication/registration effects are preserved.
Failures during name shrink/cancel retain the native partial count/headers and
prohibit normal teardown, retry or implicit cleanup.

## Supported route and evidence

The intended reached path is initially-empty, non-delayed Icon destruction:
get/register the real object, run real count-gated cancellation, and later
perform its genuine flags1 process drain. Raw valid nonempty cancellation and
shrink are implemented, but AB5F00 queue insertion, AB51B0 delivery and AB58D0
file callback are not supplied. `DelayedTextureLoad=true` must remain rejected
by the caller; the existing Icon runtime already rejects that property route.
No raw/semantic entry conversion or prefilled test array is used in production.

Source/export/listing evidence: AB5700 through AB57B1 (inline allocation/stores
at AB5756..5775, BD0C30 at AB5788), AB50E0 through AB51AA (copy/release/count
phases), AB4850's reached shrink-to0 arm, AB5560 through AB55BD, and AB57C0
through AB57DB. Live D5C4B8 bytes are `C0 57 AB 00 00 00 00 00`. Ghidra was
verified on the configured bsp.gpr `/battlestationspacific.exe` domain before
the analysis batch. Existing descriptive names remain hypotheses; no saved
analysis mutation or new reconstructed-function credit is claimed.

This is a new MSVC Win32 C++ service. Native valid extents are explicit:
nonnegative count/capacity, count<=capacity and sufficient actual0Ch backing.
Corrupt/overflowed arrays, destructive callback reentry, original CRT failure
identity, private FH3/SEH stacks and arbitrary hardware faults are outside the
supported API. Calls are serialized with producers and process drain.

Temporary `bsp_core` source admission built through `scripts/build.ps1`:
`native_gui_icon_delayed_loads.cpp` compiled, the executable targets linked and
both existing CTest targets passed. No new tests or game runs were added. The
shared D5C4B8 arm is still a prerequisite: source/build proof does not establish
a live registered drain, native draw, page admission or OFF/ON identity.
