# Sound manager updates and active entry lifetime

Addresses: 00A7E630, 00A87BF0, 00A7C1C0, 00A7D640, 00A7D680,
00A79990, 00BEE050. Existing bindings reviewed: 00A7C080, 00A7D5C0,
00A7E490.

Seven routines now connect the reconstructed sound channels to the manager's
update and removal loop. `SoundChannelRuntime` binds the D5ABF8 virtual calls and
projected intrusive references, using the canonical `SoundLevelEntry` identities
and `SoundManagerLevels::entries_8c`. There is no second active list. Descriptive
names are hypotheses, and `reports/sound_system_update.json` records the native
ABI, verified byte spans, saved annotations and validation provenance.

## Update sequence

D5B000 slot4 is A7E630. D5B44C slot4 is A87BF0, a wrapper that copies listener
selection10C to110 only when different, then calls the base update. Their native
inputs are ECX manager and stack matrix pointer plus a by-value three-float
velocity, RET10. The apparent unaff_EBX dt in old pseudocode was a decoding
artifact; dt comes from the clock.

The base routine reads current global01090AB0 virtual14 and immediately copies
the timestamp. Verified D68D50 slot14 is BEE050, a getter for timer+20 current
timestamp. It calls existing00530890 to subtract manager118, then computes the
x87 signed64-bit ratio. Low timestamp word publication occurs before FDIVP;
remaining words precede FSTP float. A nonnull current alternate owner F8BBCC is
updated through slot4 with that dt. The FMOD channels-playing query follows,
with output initialized0. Only then does flag69 cause an early return. Neither
timestamp progression nor alternate update is skipped by that flag.

Velocity filtering retains the unusual original arithmetic. It computes a
float-rounded `abs(x)+abs(y)+abs(z)`, then a float-rounded
`sum-abs(previous_x)+abs(previous_y)+abs(previous_z)`, clears that result's sign
bit and compares it with the double constant50 divided by dt. A second x87
sum is compared with double600. Both CF branches accept unordered comparisons.
Failure of either condition sets all three velocity values to positive zero.
The historical `listener.position_b8` field is actually submitted as FMOD
velocity; FMOD position is the matrix translation at ownerF4/F8/FC.

The routine copies the input matrix through existing004134F0, repeats x87
load/store on the forward row E4/E8/EC, then snapshots position, velocity,
forward and up before invoking any sound. The first pass captures the array
begin/end and calls each current entry's slot30(dt, listenerA4), then reloads
that slot and calls24. A79990 implements D5ABF8 slot24 as byte58 != byte59;
a true result increments manager168. This counts virtual-state transitions.

After the first pass, the captured listener values go to EventSystem slot7C,
listener index0. EventSystem slot8 updates the mixer. Both calls reload the
current owner48 handle; neither return code changes control flow. Concrete
bindings use the installed C exports, verified by independently querying the
FMOD listener afterward.

The second pass calls current entry slot10 to refresh ended state, then0C to
query completion, then14 to query nonvirtual state **even when completed**.
It removes completed entries and virtual entries whose flag50 is zero and
signed update count18 exceeds5. Kept entries advance the iterator. Removed
entries receive stop8(0), swap-last erasure and manager164 increment. The same
iterator is reused to inspect the moved entry, and the current end is recomputed
after each iteration. Creation160, removal164 and transition168 counters are
incremented here or by the factory; this routine does not reset them.

## Arrays and shared interfaces

A7D680 compares the supplied iterator with the current last slot. When different
and the pointer values differ, it publishes the last value in the target,
retains the replacement and releases the previous entry. It then reloads the
**current** last slot after that callback, releases its value, clears the captured
slot and decrements current count. The iterator argument itself is unchanged.
Aliased duplicate values skip replacement retain/release but still drop the
last-slot reference.

A7D640 is empty-safe pop: release and clear the captured last slot before count
decrement. A7C1C0 reserves as needed, grows null entries and shrinks in reverse
order, decrementing count before each release and clearing the captured dead
slot afterward. Its count visibility intentionally differs from pop. The C++
projection recreates a trivial null pointer in allocated storage beyond the
reduced vector size for that final dead-slot write. Iterator storage must remain
valid, and structural callback mutation retains the existing array contract;
slot replacement and the required post-callback reloads are supported.

Existing reserve, append and tracked creation retain their behavior. Their C++
dependencies now accept `VoiceReferenceHost` and `SoundFactoryHost`, interfaces
already inherited by the complete voice hosts. This lets manager playback use
the same routines without implementing unrelated subtitle/UI services. Native
append receives the address of a source reference, while the existing C++
interface projects its value. Tracked creation's native hidden output reference
is followed by sample/class/type/flag on the stack, RET14/EAX output address.
Those three previously argumentless Ghidra prototypes are now corrected.

`SoundChannelRuntime` operates only on canonical pointers to projected D5ABF8
instances. It increments/decrements the actual projected reference field using
Interlocked operations and calls the recovered channel scalar destructor at
zero. It rejects other instance profiles. Banks and samples must use their own
lifetime bindings; raw native object+4 operations cannot accept this projection.

## Validation and limits

Win32 Release and both existing CTests passed. No permanent test was added.
One local fixture includes focused callback/order checks and actual installed
FMOD execution. Controlled checks prove first-pass ordering, snapshots surviving
listener mutation, current event-handle reload, retirement on update6, revisiting
the swapped entry, reload after a release replaces the last slot, array count
visibility and balanced references. The velocity test exercises the original
mixed subtraction/addition expression rather than a conventional distance rule.

The installed case uses the real splash_medium.fsb, existing VFS/resource/sample
adapters and explicit FMOD no-sound output. Existing A7E490 tracked creation gives
one manager and one returned reference. The manager starts playback, FMOD
listener queries match position/velocity/orientation, playback naturally ends,
and the second pass removes the manager reference automatically. Final caller
release destroys the instance; sample/class/name/resource/cache lifetimes balance.
Flag69 is tested to preserve clock/listener-selection updates and the FMOD query
while skipping channel creation. No manual completion poll substitutes for the
manager loop in this installed case.

Three missing functions were defined from matching disk and analysis bytes;
ten prototypes/names/comments are saved with prior values retained. No flow
repairs or no-return flag changes were required. This is build and FMOD runtime
evidence, not native object ABI, audible-device or gameplay validation.

## Follow-up packets

- A7F710 with A89DC0/A8A2E0: spatial/event constructors and their virtual
  profiles. Extend instance lifetime/dispatch when their layouts are established.
- Bind the complete voice host and application Phase5 sound initialization to
  the concrete owners, resource/sample/channel runtimes and manager update.
  `src/game_hosts.cpp` still reports unimplemented sound initialization; this
  packet does not claim full application composition.
- Recover alternate-engine slot4 and the remaining manager teardown paths.
- Audible output and gameplay validation remain open.

## Extension from docs/SOUND_SPATIAL_CHANNEL.md

The manager runtime now also accepts the D5B510 spatial bank profile when a
SoundSpatialChannelContext and BankSoundChannelVirtuals are bound. The installed
spatial fixture verifies distance groups, position/velocity and automatic retirement.
A7F710 remains a bank-branch fragment; the event profile and full application
composition are still open. See the new report for separate source/build provenance.

## Event extension from docs/SOUND_EVENT_INSTANCE.md

The active-entry runtime now supports D5B4C8 event instances and the complete
A7F710 bank/event factory. Event slot14 and slot24 are native constant false;
ended refresh uses FMOD state bit8. The existing protected/unprotected retirement
policy is preserved, and a protected installed event reaches natural completion.
Application globals, VFS sound directories and full game validation remain open.
