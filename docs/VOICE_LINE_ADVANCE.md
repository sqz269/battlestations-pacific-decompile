# Voice-line clip advancement

`005B91E0` is a voice-line playback advance routine, provisionally named
`BSP_VoiceLine_AdvancePlayback`. The native ABI is ECX=line, no stack arguments,
RET, boolean result in AL. Its inclusive body ends at `005B92F3` (one-byte RET).
The typed reconstruction is `src/voice_line_advance.cpp`.

Assembly establishes the complete sequence:

1. Read line `+18`; return false immediately for `-1`.
2. Load the current voice manager at `[[00E198C4]+A4]`, compute its slot address
   as `manager+8+24*index`, and poll through `007027B0`. A nonzero poll result
   returns true without advancing the clip index.
3. Increment current line `+1C` with dword wrap. Skip records with signed
   sound ID `+8 < 0`, comparing the unsigned index to the current vector count.
   Exhaustion writes `-1` to `+18` and returns false.
4. Copy the selected record/resource index into Clip12 with vtable `00CF0DD0`.
   Re-read line `+18` and the global manager, then call recovered `005B9050`
   with a **null bank**. Return true regardless of the newly started slot state.

The manager reload before start matters if polling reenters and replaces the
global owner. Later clips do not retain the speaker bank used by construction.
The caller `005BC640` remains a separate unreconstructed manager-update body.

Native vector access uses `005B5E50` and a checked-range failure at `005B925C`.
The C++ projection assumes a valid native-size vector and initialized `+1C`
for active lines; it does not reproduce corrupt-container termination or
native allocator/exception ABI. An unresolved nonzero slot mapping must preserve
the native index/address behavior or fault. No silent slot-zero substitution is
provided. No installed-game clip progression has been validated.

Validation and Ghidra annotation status are recorded in
`reports/voice_line_advance.json` and the enclosing voice-services report.
