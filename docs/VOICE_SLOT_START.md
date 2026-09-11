# Voice slot construction, assignment and start

Packet `orch4_voice_slot_start_b`, with separately leased direct helpers. All
Ghidra calls verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. This worker made no Ghidra mutations. Names are
descriptive hypotheses. Raw exports remain under ignored `exports/bsp`.

The recovered sequence now exists in `src/voice_slot_start.cpp`. The integrator
must switch `construct_voice_line_005babb0` to the direct
`start_voice_clip_005b9050` call using `host.slot_start_context()`, retaining the
bank once beforehand but removing its old outer `playback_argument` guard:
the new routine consumes that reference itself. The old abstract start method
is temporarily retained solely so this isolated worker branch builds against
the unchanged `src/voice_playback.cpp`.

## Native records and ownership

The native 18h slot contains state +00, intrusive sound +04, intrusive auxiliary
sound +08, **owned NativeString +0C/+10**, and float timestamp +14. Slot assignment
is not a raw copy or a stop operation. It writes state, then for each reference
publishes the replacement, retains it, and releases the captured old value.
Identical pointers skip both operations. It reads source auxiliary, string and
timestamp only after preceding releases, preserving callback/reentry effects.
The native string copy helper already reconstructed in `NativeString` matches
the resize/current-length/memcpy sequence, including self-copy suppression.

Cleanup first returns the native string buffer through its supplied storage,
leaving the header unchanged, then releases/clears auxiliary, then reloads and
releases/clears sound. It does not stop playback, reset state or clear the string
header. Cleanup must be invoked once on a completed slot whose owned storage
will subsequently be discarded. Reusing its stale string after destruction is
invalid. The implementation reuses `destroy_native_string_header_0041dd20`.

The clip record adds byte +14, alternate NativeString +18/+1C, and inline resource
handles beginning at +20. `VoiceClip.word_08` is the index into that handle array
on the ordinary branch; its extent is not established here. The category-bank
argument and selected handle are resource **wrappers**, not the previously
reconstructed `SoundOwnedResource`: `00A81860` returns wrapper+8, whose first word
is a kind discriminator. Casting it to the asset class would read the wrong
layout. These resources stay opaque until their owner/constructor is recovered.

## Constructor and orchestration

00702CC0 constructs an empty slot, caches class `Warnings` using existing
`find_sound_class_00a7acf0`, captures mission clock, and sets state zero. Global
guard bit0 is set before string allocation/lookup and stays set even if lookup
returns -1. It is process state, not a per-slot cache or thread-safe C++ static.
Reentry sees the old cached index until the first lookup completes.

A negative record sound ID skips both playback branches but still consumes the
owned bank argument. Otherwise 00A7D120(1) runs first. A nonnull bank is played
into auxiliary using type `Normal`, cached `Warnings`, and flag1. The returned
temporary reference is assigned into the slot and released before temporary
`Normal` string storage is returned.

If record+14 is nonzero, 00449AF0 compares record+18 with the literal empty string
at 00CE3A0C; this reduces exactly to stored length nonzero. A nonempty name is
copied into slot+0C, passed to alternate sound engine 00A78CE0 with float0.1, and
then state becomes2. Assembly of 00A78CE0 shows its second stack argument is
unused; its real queue path is still a required service, not assumed playback.

Otherwise the indexed resource is retained locally. If nonnull it is played
into slot+4, its *current* pointer is reread after temporary destruction, then
00A798C0 writes +3C=0.1f and +38=1. State is written1 afterward. There is no
factory-null recovery: the native timer setter would fault on null. Missing
resource skips this branch; it does not undo the earlier warning-sound flag.
The selected resource reference and by-value bank reference are then consumed.

005B9050 retains a second bank reference for constructing a temporary slot,
calculates destination `manager+8+24*index`, assigns the temporary, destroys it,
polls the destination and conditionally logs `NOT FOUND`. The `played` log is
unconditional, even after that failure log. Both logs use clip record text and
the resource index, not signed record sound ID. Its incoming bank reference is
released only after logging. Only slot0 is established by existing callers'
scan; the native accepts -1 without checking. The projection directly selects
slot0 and requires the host to map any other native index/address or raise the
corresponding fault. It never silently substitutes a valid slot.

## Direct sound-manager dependencies

00A7B0A0 scans canonical `SoundConfigurationState.types_38`, checking stored
length before CRT case-insensitive comparison. It returns the first ordinal
or **zero** on miss. Existing class lookup instead returns -1 on miss.

00A7E490 uses kind0 -> manager virtual+0C and kind1 -> virtual+10; other kinds
return null. The kind read is repeated for the second test. A successful virtual
factory result owns one reference. Its pointer is appended to canonical
`SoundManagerLevels.entries_8c`, retaining a manager reference; the hidden-return
reference is then retained and the factory temporary released. Factories remain
required native/FMOD services. They must produce actual canonical entries or
native null, never invented successful objects.

00A7C080 reserves at least one entry, never shrinks, retains all copied slots
before releasing old slots in forward order, then installs pointer/capacity.
00A7D5C0 grows only when count equals capacity, doubles capacity with a minimum
of one, then appends and retains one pointer. Standard vector storage replaces
the native allocator; the references and manager fields are shared with existing
sound-level code. The projection requires stable array structure during releases,
valid counts/capacities <=INT32_MAX/2, and the real nonthrowing atomic retain
primitive. It does not model integer-overflow allocation or allocator/SEH ABI.
00A798C0 uses ordered `duration>0`: positive stores duration/enabled1; zero,
negative and NaN store +0/enabled0. The caller's 0.1f bytes are CD CC CC 3D.

## Address, ABI and instruction evidence

| Address | Native ABI | Inclusive final instruction (bytes) |
| --- | --- | --- |
| 00702CC0 | ECX=fresh slot; Clip12*, owned resource by value; EAX=this; RET8 | 00703016 (3) |
| 005B82C0 | ECX=destination slot; source slot*; EAX=this; RET4 | 005B8371 (3) |
| 005B7FC0 | ECX=slot; RET | 005B8068 (1) |
| 005B9050 | ECX=manager; index, Clip12 by value, owned resource; RET14 | 005B9154 (3) |
| 0054D4C0 | ECX=reference field; source reference*; EAX=this; RET4 | 0054D4FE (3) |
| 00A7B0A0 | ECX=sound manager; NativeString*; EAX=type ordinal; RET4 | 00A7B10F (3) |
| 00A7E490 | ECX=sound manager; hidden return*, resource*, class, type, flag; RET14 | 00A7E61E (3) |
| 00A798C0 | ECX=sound; float duration; RET4 | 00A798E3 (3) |
| 00A7C080 | ECX=manager+8C; signed capacity; RET4 | 00A7C171 (3) |
| 00A7D5C0 | ECX=manager+8C; reference*; RET4 | 00A7D630 (3) |

All ten functions have complete exported listings except reserve's hidden tail.
False no-return at CALL 00A7C14A -> `_free` hides **00A7C14F..00A7C161 inclusive**.
Disk-byte decoding gives mov edx,[esp+1c]; mov eax,[esp+30]; add esp,4; pop edi;
pop ebp; mov [esi],edx; mov [esi+8],eax; pop ebx. Parent repair requested.
No missing function start was discovered.

## Validation boundary

See `reports/voice_slot_start.json` for current build and focused-probe results.
The new functions are typed behavioral projections. Original ABI/SEH, installed
game playback, native differential behavior, alternate-engine queue consumption,
and actual virtual factory/FMOD execution are not validated. Intrusive lifetime,
mission clock, polling, warning flag side effects, factories, nonzero native slot
mapping and alternate playback are mandatory host services with no fallback.
