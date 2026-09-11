# Spatial event instances and the complete spatial factory

The sound runtime now supports D5B4C8 spatial FMOD event instances alongside D5B510
spatial bank channels and D5ABF8 ordinary channels. Nineteen routines are reconstructed,
and 00A7F710 is upgraded from its recorded bank fragment to a complete bank/event factory.
Names are hypotheses, not recovered symbols. This is a C++ projection, not the native ABI.

| Address | Behavior | Original calling contract |
|---|---|---|
| 00A89DC0 | Spatial event constructor | ECX object; sample/class-slot/type/flag + position and velocity float3 values; RET28; EAX object |
| 00A89C80 | Event destructor | ECX object; RET; tail decoded separately |
| 00A89EE0 | Spatial deleting destructor | ECX object; flags; RET4; EAX old pointer |
| 00A894E0 | Get and start event | ECX object; RET |
| 00A89560 | Submit dirty parameters | ECX object; RET |
| 00A89170 | Stop/key-off policy | ECX object; immediate byte in stack word; RET4 |
| 00A88BA0 | Completion | ECX object; RET; EAX0/1 |
| 00A89340 | Refresh ended | ECX object; RET; AL result |
| 00A89C40 / 00A89C70 | Slots14 /24, constant false | ECX unused object; RET; AL0 |
| 00A88FF0 | Normalize velocity | ECX source; hidden output pointer; RET4; EAX output |
| 00A893D0 / 00A899E0 | Parameter reserve/resize | ECX actual12h header; signed count; RET4 |
| 00A89BE0 | Parameter array destructor | ECX actual12h header; RET; tail decoded separately |
| 00A896E0 | Find parameter descriptor | ECX object; hidden output/name; RET8; EAX output |
| 00A89B80 | Set parameter value | ECX object; index/value; RET8 |
| 00A89120 / 00A89E80 | Set attributes / get position | ECX object; six float words, RET18 / hidden output, RET4 |
| 00A89770 | Spatial event update | ECX object; dt/listener; RET8 |
| 00A7F710 | Complete spatial factory | ECX manager; sample/class/type/flag; RET10; EAX object |

## Storage and lifetime

The native event is90h bytes. Its common instance ends at52h; event54 is borrowed
from FMOD, parameters58 are an actual12h array header, dirty64 gates parameter submission,
group68 records routing, position6C and velocity78 are float3 values, dirty84 gates spatial
updates, band88 records distance and ended8C records completion. C++ reuses the canonical
SoundInstance base fields and projects these members. It never reads channel fields from
an event object.

Parameter records are12h: float value0, dirty byte4, untouched padding5..7, and borrowed
FMOD parameter8. Reserve copies all three words. Resize clears only dirty/handle for new
records, leaving value and padding bytes intact. The setter compares the existing value
before deciding to mark it dirty; it does not initialize that value or clamp it to the
descriptor range. The defined-input comparison requires initialized backing value bytes.
The installed fixture explicitly seeds those bytes before resize/set and verifies padding
preservation. Production allocation is not silently zero-filled. Negative counts and
overflowing allocation extents remain outside the valid-storage domain.

Descriptors come from the existing sample producer 00A828B0:16h records of index, min, max
and borrowed name. Lookup is case-insensitive and returns the first descriptor on a miss,
including the native empty-count path if first-record storage exists. It does not invent
a not-found index. Submission clears global and per-record dirty flags before callbacks,
lazily resolves a parameter handle by index, and reloads current array fields before
setting its value. It samples FMOD memory on2B, otherwise ignores results.

Factory 00A7F710 counts once before testing resource78->FMOD sound10. The bank branch
allocates84h and calls 00A8A2E0; the event branch allocates90h and calls 00A89DC0. Both grow the
signed class table and supply zero vectors. DEB7E8 unwind states free only the allocated
object, preserving the already documented base-constructor ownership behavior. The older
bank-fragment entry remains as an explicitly bank-only compatibility entry; the runtime's
complete `create_spatial` uses the full factory.

Destruction installs D5B480, calls EventStop(false) only for a nonnull event, destroys
the parameter array, then runs 00A7BD90 sample/class/name cleanup. There is no invented
Event::release. DEC50C unwind state1 destroys the parameter array and state0 destroys the
base. Parameter pointers/capacity are left as dead header values after disposal.

## Update, stop and floating-point behavior

Event creation captures the current EventSystem, uses sample name60 or supplied live
F8BBEF bytes, and calls GetEvent with mode2 (ERROR_ON_DISKACCESS), then Start. Current
event/master-group fields are reloaded at the native points. The update advances fade and
delay, routes distance bands through the shared functions from SOUND_SPATIAL_CHANNEL.md,
and moves the event's FMOD channel group beneath the configured group. Null mapping mutes
scale28 without a group move. The GetChannelGroup output aliases the original dt stack slot;
its initial bits are the remaining delay when countdown ran. Those bits survive an unwritten
FMOD output. A required successful data load is not replaced with a fabricated handle.

Pitch is frequency_scale20 minus1 with RAW units0. Mixed volume optionally uses the player
boost when pan40 differs from1. Spatial submission snapshots position, velocity and an
orientation computed by 00A88FF0. That helper preserves the native unspilled x87 square/sum,
float spills, existing CRT sqrt binding, double threshold D7A268 and current-component
reloads after output stores. Unlike a snapshot-based normalization, partial source/output
aliasing can affect later components. Zero and alias cases are covered by the fixture.

Slot14 is literally false, and slot24 never reports a transition. The manager therefore
retains its existing policy of removing an unprotected event after six updates. Ended
refresh tests FMOD state bit8 (PLAYING), with a zero-initialized output and ignored result.
Completion refreshes through virtual10 only when stop_requested51 is set. Stop always
queries parameter `Stop` first, even for immediate requests. Immediate or Stop-present
uses EventStop(true); otherwise it queries `Loop` and key-offs that parameter if present.
Only the no-event path sets stopped15 directly. The Loop key-off branch is assembly-grounded;
the selected installed one-shot event does not exercise that branch.

## Evidence and validation

Win32 Release and both existing tests passed. The installed fixture exercises both
branches through the same tracked factory and manager. For
`planes.fev:planes/muzzle/muzzle5inch/muzzle_5_inch`, independent FMOD queries verify
parameter values and cached handle reuse, position/velocity/orientation, pitch, volume and
group parent. Null routing mutes, near routing restores volume, natural completion retires
the protected event, immediate stop follows its parameter-query order, and resource/class/
string cleanup balances. The bank regression still passes. No permanent tests were added.

The first playback attempt exposed missing fixture VFS coverage: FMOD requested
`muzzle_5_inch.fsb` by basename, but only game root and sound/events were mounted. LoadEventData
returned23, GetEvent(mode2) returned24 and Start returned37; acquisition-only checks from the
earlier sample packet did not prove playable event data. Mounting the existing sound/muzzle
directory into the fixture resolves the request. The original installation was not changed.
Application composition must provide its actual sound directory registrations.

Eight absent functions were defined from verified disk/Ghidra bytes. Four free-call
fallthrough repairs were applied without changing callee no-return flags. 00A89C80 and
00A89BE0 still have short stored Ghidra bodies ending atA89CD3 andA89BF1. Their decoded
tails end atA89CF6 andA89BF6 and are recorded separately; the base call atA89CE1 is a
disk/live-byte verified tail call, not a call inside the currently stored Ghidra body.
The project was not restarted and script execution was not enabled to change those bounds.

The report records all function spans, raw tails, original prototypes, prior annotations,
call-site checks, installed artifact hashes and separate source/combined-build revisions.
Execution used FMOD's no-sound device and an explicit finite-fixture CRT binding. Native
ABI replacement, full allocator/SEH equivalence, audible output and game validation remain open.

## Follow-up packets

- Reconstruct spatial bank slot3C 00A8A700 and event profile slots18/1C/20/3C/40 before
  exposing either as a complete gameplay-facing object.
- Compose application sound initialization, current globals/clock/CRT, voice ownership and
  VFS directory registration so the startup path can own and update these real instances.
- Extend stored Ghidra body bounds for 00A89C80/00A89BE0 when a supported locked operation is
  available; preserve the verified tail distinction until then.

## Correction from docs/SOUND_GAMEPLAY_METHODS.md

The remaining event slots18/1C/20/3C/40 and spatial bank slot3C are now reconstructed
and exposed through the runtime. Installed fixtures cover their FMOD behavior, including
playing-event audibility0.5 and query-singleton cleanup. Application sound/voice
composition, native ABI and audible/gameplay validation remain open.
