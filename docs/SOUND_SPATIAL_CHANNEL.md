# Spatial bank sound instances

The projected sound manager can now create, update and retire an FMOD bank sound through
the native D5B510 spatial-channel profile. Ten routines are reconstructed, and the bank
branch of factory `00A7F710` is a separately recorded fragment. The event branch remains
unbound. Names below are descriptive hypotheses, not recovered symbols.

| Address | Behavior | Original ABI |
|---|---|---|
| 00A7D560 | Shared channel constructor | ECX object; sample/class-slot/type/flag; RET10; EAX object |
| 00A8A2E0 | Spatial channel constructor | ECX object; same four words plus position and velocity by-value float3; RET28; EAX object |
| 00A8A3D0 | Spatial destructor | ECX object; RET |
| 00A8A460 | Scalar deleting destructor | ECX object; flags; RET4; EAX former object |
| 00A8A290 | Set position and velocity, slot34 | ECX object; six float words; RET18 |
| 00A8A3A0 | Get position, slot38 | ECX object; hidden output float3 pointer; RET4; EAX output |
| 00A79A40 | Select distance band | ECX manager; float distance; RET4; EAX band |
| 00A7EC60 | Find distance variant of type | ECX manager; type/band; RET8; EAX index |
| 00A7BA40 | Get type's selected-listener group | ECX manager; type; RET4; EAX pointer |
| 00A8A480 | Update spatial channel, slot30 | ECX object; dt/listener pointer; RET8 |
| 00A7F710 | Spatial factory, bank branch only | ECX manager; sample/class/type/flag; RET10; EAX object |

## Recovered behavior

The native spatial channel is 84h bytes: the shared 5Ch channel, position at5C,
velocity at68, dirty byte74, distance band78, channel group7C and DSP80. The C++
projection inherits the canonical manager entry and voice-start fields; it is not an
84h native object. The constructor clears vectors and band before the shared constructor,
then installs D5B510 and copies the two vectors. Slot38 is a position getter with a hidden
output pointer. It snapshots all three floats before writing, including an aliased output.

Factory A7F710 increments attempted creation before its resource gate. A818C0 tests
resource78->FMOD sound10. The implemented true branch allocates the spatial channel,
grows the class table using the signed class index, and supplies zero vectors. Its
DEB7E8 unwind state0 frees the allocation; the existing A7C480 sample-retain exception
behavior is preserved. A false gate reaches the separate 90h event constructor A89DC0;
the fragment reports that boundary with an exception. It does not return a fabricated
failure or publish an event as a channel.

Update A8A480 advances fade and positive delay, creates the channel paused if absent,
and initializes group7C from the manager captured before creation. It computes the
listener's world position (listener+50, projected transform[12..14]) minus position5C,
spills each component, then calls the existing 00419440 vector-length kernel with an
explicit borrowed CRT binding. Routing reloads the current manager between calls.

Distance thresholds are manager20/24/28/2C. Equal thresholds enter the next band.
The assembly's final JA means an unordered comparison selects band4; the old pseudocode
suggested band3. A7EC60 strips only a recognized case-insensitive dist1..dist4 suffix,
then appends the requested suffix and calls the existing zero-on-miss type lookup.
The actual 00467CF0 reverse-find helper never examines position0, so `_dist1` is not
stripped. Its pooled substring, comparison, resize and append primitives are reused.
Configuration names and group arrays retain their existing C++ container projections;
stable storage callbacks and bounded valid indices are required.

The group lookup uses selected ordinal108 and null-fills missing entries. A new null
group sets scale28 to zero without changing FMOD's actual channel group; a later nonnull
mapping moves the channel and restores scale28 to one. Dirty state updates frequency,
volume, 3D pan and optional speaker mix. Dirty spatial state submits position/velocity
snapshots. Pause bits and virtual58/59 history follow the base-channel rules. This update
ignores its FMOD results and does not query audibility or refresh ended state; the manager
performs completion checks afterward.

Destructor A8A3D0 removes a nonnull DSP, reloads DSP80 for release, samples memory on2B,
then clears80. It stops the channel and invokes A7BF40, which stops the channel again
before releasing sample/class/name. DEC564 state0 calls A7BF40 on unwind. The scalar
destructor's free fallthrough at A8A475 was repaired without changing no-return flags.

## Validation and integration

MSVC Win32 Release and both existing tests passed. The local fixture extends the prior
installed-manager fixture using `sound/splash/splash_medium.fsb`, explicitly setting the
borrowed sound's 3D mode in the fixture. Actual FMOD queries confirmed channel position
and velocity, distance group selection, null-mapping mute without a group move, restoration
to the master group, natural completion and automatic manager retirement. Actual DSP
remove/release and both destructor stop calls ran; sample, class, cache, resource and
string references balanced. The fixture uses the no-sound device. It adds no permanent tests.

The companion report records disk/Ghidra byte hashes, original signatures, prior annotation
values, function definitions, call sites, fixture/library hashes and separate source and
combined-build revisions. Two missing accessor functions were defined from verified bytes.
The saved names/comments and refreshed exports retain previous evidence.

`SoundChannelRuntime` accepts D5B510 when supplied `SoundSpatialChannelContext`, using
`BankSoundChannelVirtuals` for the shared slots10/14. Existing D5ABF8 callers retain their
prior constructor. Four concrete FMOD C exports supply group, attributes and DSP operations.
No native object layout, drop-in ABI, audible output or gameplay validation is claimed.

## Follow-up packets

- Reconstruct event constructor A89DC0, destructor A89C80/A89EE0, update A89770,
  creation A894E0, parameters A89560 and completion/stop virtuals; complete A7F710.
- Reconstruct spatial slot3C A8A700 and its DSP/doppler behavior before exposing that
  profile as a complete game object. Other non-manager virtual slots remain outside this adapter.
- Compose the application sound/voice hosts with recovered current-manager, time and CRT
  services; validate actual application/gameplay execution separately from this installed fixture.
