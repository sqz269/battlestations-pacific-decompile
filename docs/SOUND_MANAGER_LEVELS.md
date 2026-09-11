# Sound manager levels and class invalidation

Addresses: 00a7a440 00a7a3f0 008d5430 00a7b230 00a7abf0 00a7acf0 00a7f8e0 008d41f0

Packet `orch3_sound_manager_levels`, branch `agent/orch3-sound-levels-20260910`.
All descriptive names are hypotheses, not recovered symbols. Analysis used the
read-only `bsp.py ghidra` client, which checks project `bsp`, configured project file
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language and
image base before each query. No Ghidra mutations or game installation changes.

## Established boundary

The four original anchors establish storing a global level, marking live entries
dirty by class, applying settings, and unregistering the singleton. They do not
establish the final gain computation or consume `manager+6Ch`. Three small direct
settings callees were added to distinguish the two arrays and complete the class
volume rules. The existing audio-default reset was inspected only as writer evidence.

| Native field | Proven use |
| --- | --- |
| manager `+4C` | Raw float stored by `00A7A440`; settings `masterVolume` is an input |
| manager `+6C` | No read or write in these eight bodies; final influence remains open |
| manager `+70` | Byte copied from settings `+24` by `008D5455`; distinct from the level |
| manager `+8C/+90` | Pointer array and count of live entries walked by `00A7A3F0` |
| live entry `+14` | Dirty byte: selected entries receive 1 |
| live entry `+44` | Descriptor pointer; descriptor `+8` supplies the class index |
| manager `+98/+9C` | Separate descriptor-pointer array and count |
| descriptor `+10` | Float volume, written by both class setters |
| descriptor `+18/+1C` | Stored string length / data, searched by `00A7ACF0` |

Calling the `+8C` elements buses is provisional. The named values Warnings,
GUITestSpeech and GUIMusic are found in the `+98` descriptor table. These bodies
make no FMOD volume call. They cannot establish a multiplication formula, the role
of `+6C`, or whether all live entries correspond to FMOD channel groups.

## Global level and dirty class mask

`00A7A440`: native `__thiscall(manager, float)`. `MOVSS` at `00A7A446` stores the
argument directly into `+4C`; `00A7A44B` replaces the stack argument with `FFFF`;
`00A7A453` tail-jumps to `00A7A3F0`, whose `RET 4` completes the call. There is no
clamp, enable check, comparison with the previous level, or immediate gain update.

`00A7A3F0`: native `__thiscall(manager, uint32 mask)`, `RET 4`. It snapshots the
array begin and end, then loads `[entry+44]+8`, executes `SHL EBP,CL`, tests the mask,
and writes entry `+14=1` only on a match. x86 masks the shift count to five bits.
Thus `FFFF` selects classes whose index modulo 32 is 0 through 15. It is not proof
that every possible entry is selected. Unselected dirty bytes are preserved.

## Descriptor setters and lookup

`00A7ABF0`: native `__thiscall(manager, uint32 mask, float)`, `RET 8`; EAX after the
call is incidental and is not consumed by the inspected settings caller. Walk the
`+98` table in order. Its array ordinal supplies `1 << (ordinal & 31)`, independently
of the descriptor's `+8` field. When that bit remains selected, compare descriptor
`+10` with the argument. Ordered equality removes the bit from the pending mask
using XOR; otherwise store the argument. After the loop, call `00A7A3F0` only if
the remaining mask is nonzero. There is no clamp. Bits without corresponding table
entries remain pending and still participate in invalidation.

The x87 sequence `00A7AC2D..00A7AC45` uses `FUCOMIP`, `LAHF`, `TEST AH,44h`, `JNP`.
Equal finite values and either sign of zero remove the bit; unordered comparisons
write the value and retain the bit. If the table has more than 32 entries, repeated
bit positions interact with the progressively reduced mask. This code neither
proves a 16-entry limit nor establishes equality of descriptor indices and ordinals.

`00A7ACF0`: native `__thiscall(manager, native_string*)`, `RET 4`, EAX ordinal or -1.
It first requires equal stored string lengths. Two zero lengths match without
reading string data. Otherwise `00A7AD3B` calls CRT `__stricmp`, and the first zero
comparison result wins. A miss returns -1. The reconstruction reuses `NativeString`
and injects the CRT comparison; it does not reproduce CRT or locale internals.

`00A7F8E0`: native `__thiscall(manager, signed index, float)`, `RET 8`. Signed
`index >= count` calls `00A7C2C0(index+1)` on `manager+98` at `00A7F8FD`. Compare
the selected descriptor's `+10`; ordered equality returns. Otherwise repeat the
growth guard/call at `00A7F923`, store the argument, and dirty `1 << (index & 31)`.
The x87 comparison at `00A7F904..00A7F916` treats NaN as changed. Both allocation
call sites are separate injected host methods. No allocation or descriptor
construction behavior is guessed.

There is no negative-index guard. Passing the lookup's -1 result causes a read
before the table, because the signed comparison skips growth. The three settings
calls immediately feed lookup results to this setter, with no failure branch.
The C++ projection documents a valid nonnegative index and valid descriptors after
growth as preconditions; it does not invent missing-name recovery.

## Settings application and corrections

The existing `apply_audio_settings_008d5430` is reused rather than duplicated:

1. `008D5455` copies settings `+24` into sound manager `+70`.
2. `008D546E` applies settings `+20` through `00A7A440` without clamping it.
3. `008D5483/90` store music/speech into dialog manager `+218/+21C`.
4. `008D54BA` calls the external music player's setter only if both owner and
   owner `+50` are nonnull. The existing host contract owns these pointer guards.
5. `008D54D9` applies effects volume to class mask `FFFF`.
6. The lookup/setter pairs apply speech to Warnings and GUITestSpeech, then music
   to GUIMusic. The named overrides occur after the bulk effects call.
7. Only the UI sound singleton `+10` receives the clamped master value.

The UI clamp is instruction-checked at `008D568B..008D56B0`: compare zero with
input and use `JA` to select zero; otherwise compare input with one and use `JBE`
to retain input. NaN and negative zero are retained under the normal masked FP
exception environment. The previous `std::max(0, std::min(input,1))` projection
returned positive zero for both cases. `src/game_settings.cpp` now uses ordered
`<0` and `>1` branches. FP exception/status emulation is outside the C++ interface.

**Correction to MISSION_STATE_ENTRY:** `00F889A0 = 00F88980 + 20h`, where
`00F88980` is the settings object. The fixed singleton load at `0073E47C` and call
at `0073E485` are recorded in `APP_INIT_GAME_ENTRY.md`; its `+20` field is the
existing `AudioSettings::master_20`. `008D41F8` was independently checked live and
writes `0.5f` through `[ECX+20]`; the settings key table records `masterVolume` at
that same offset. Therefore the direct load at `004DA724` is a read of the current
master setting. A lone absolute-address xref and zero bytes in the disk image do
not make it a constant. The old `kMissionEntryAudioLevel=0.0f` assumption requires
the parent integrator's separate mission-entry correction. The same aliasing
reasoning makes `00F889A4` the settings enable byte (`+24`), not an unrelated flag.

## The fourth original anchor is destruction

`00A7B230` is a native `__thiscall` destructor body with no stack arguments, `RET`.
It writes vtable `00D5ABAC`, gets the singleton lifetime manager (`00415350`),
optionally enters its `+10` critical section and increments the lock object's
`+18` counter, gets the manager again, unregisters `[00F8BBD8]` through `00BCFCA0`,
then clears that global. It decrements/leaves the captured lock when present and
writes base vtable `00CE3818`. It does not release FMOD objects or read volume
fields. This body is analyzed/named only; existing lifetime and Win32 contracts
remain external. No no-function address or truncated body was encountered.

## Reconstruction and validation

`sound_manager_levels.hpp/.cpp` reconstruct five bodies: `00A7A3F0`, `00A7A440`,
`00A7ABF0`, `00A7ACF0`, `00A7F8E0`. Existing settings types, `NativeString`, and
`kAllSoundGroups` are reused. Existing settings application receives only the
proven clamp correction; reset and singleton destruction are analyzed only.

Validation status is recorded in `reports/sound_manager_levels.json`. These are
new C++ interfaces, not ABI-compatible replacements. No sound playback, FMOD
runtime behavior, final gain computation, or game validation is claimed.

Follow-up: identify the reader of entry `+14` and its descriptor `+10`, then trace
the final gain formula and `manager+6C`. Inspect `00A7C2C0` and descriptor creation
before supplying a native growth host; verify the descriptor count and the mapping
from ordinal to `+8` class index during initialization (`00A7FF80` is a candidate).
