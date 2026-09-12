# Actual 54h sound stream runtime

The callable reconstruction in `sound_stream_runtime.hpp/cpp` implements the
actual stream produced by `00A877D0`. It borrows the existing sound manager,
native string allocator and real FMOD operations. It does not construct a sound
system, mount files, emulate samples, or supply a successful fallback service.
The companion owner packet implements construction/destruction and the logical
dialog packet provides the concrete `00A865F0` table lookup.

The evidence is the saved `C:/Users/sqz269/bsp.gpr` project, program
`/battlestationspacific.exe`, checked before each live batch. Function exports,
assembly, caller/callee queries and original installed-image instruction replay
were used. No Ghidra edits were made. Names below are reconstruction hypotheses,
not recovered symbols. `reports/sound_stream_runtime.json` retains each address,
original name, ABI, graph edges, source mapping and verification boundary.

## Receiver and dependencies

`00A877D0` produces the actual 54h receiver: pause/stop/fade-out/fade-in bytes
`+08/+09/+0A/+0B`, fade factor `+0C`, caller gain `+14`, saved position/length
`+18/+1C`, state `+20`, original/resolved/third native strings
`+24/+2C/+34`, retained 20h table pointer `+3C`, FMOD sound/channel pointers
`+40/+44`, and the control-vector data/count/capacity `+48/+4C/+50`.
The unrelated `+10` word remains untouched. A control row is 12 bytes:
target/current/rate floats. A table descriptor is 20 bytes, with native name
header `+00`, channels `+08`, channel offset `+0C` and format `+10`.
The table owns descriptor data at `+08`, gain `+14`, total channels `+18` and
loop byte `+1C`. Sample pointers are never interpreted as bank or channel pointers.

`SoundStreamRuntimeContext` borrows the current `F8BBD8` publication, native
strings, FMOD host, live one word at `D7A24C`, fade-rate double at `CE3DC8`,
and the two native fallback string addresses. The observed constant bits are
`3F800000` and `3FD3333340000000` (the latter is the widened single-precision
0.3 value). It owns no singleton domain and does not alter lifetime APIs.

Every FMOD host member is one operation observed at a native call site. Output
pointers are explicit, including ignored outputs. `getOpenState` starts only
the state output at 2; its percent and starving locals remain unwritten until
the library writes them. Format outputs and memory-stat outputs likewise have
no invented initialization. `isPlaying` receives the low byte of the incoming
dt stack word after fade calculations have potentially overwritten that word;
an error which leaves the byte alone must preserve it. The installed C FMOD_BOOL
wrappers are not equivalent here. The integrator supplies the exported C++
byte-pointer methods for these two operations; this packet does not bind a DLL.

## Recovered bodies

| Address | Native entry and resulting behavior |
| --- | --- |
| `00A864F0` | ECX stream, float, RET4; raw gain store to `+14`. |
| `00A86670` | ECX stream, signed index/float, RET8; raw target/current stores, leaves rate. |
| `00A85B50` | ECX row, dt, AL changed, RET4; exact x87 row progression, clamp, NaN and signed-zero behavior. |
| `00A86150` | ECX stream, dt, RET4; advance rows, mix gains into three 64-float arrays and route speakers. |
| `00A87B60` | ECX manager, EAX `[ECX+170]`, RET; D5B44C virtual slot 8 speaker-layout getter. |
| `00A867B0` | ECX stream, native name pointer, RET4; set masks even when already active, then start asynchronous stream creation. |
| `00A86B40` | ECX stream, EAX open state, RET; query, release on open-error state 2, clear sound and invoke stop. |
| `00A86BF0` | ECX stream, RET; wait for pending open with Sleep(0), preserve rewind rules, stop/release only in state 2. |
| `00A86DE0` | ECX stream, RET; play paused on free channel -1, priority 0, format query, optional saved-position restart. |
| `00A874D0` | ECX stream, dt, RET4; source-pause gate, levels before fade, state-machine transitions and FMOD calls. |
| `00BD1BB0` / `00BD1CA0` | ECX actual 18h builder, DWORD argument, EAX builder, RET4; integer append with native pooled temporary. |

Except for the explicitly fastcall-bound row arithmetic, these are typed C++
interfaces with explicit service arguments, not drop-in native ABI replacements.
The speaker getter is a typed projection from the canonical manager state.
The current Ghidra database lacks its separate `00A87B60` function start: the
seven bytes `8B 81 70 01 00 00 C3` and D5B44C+8 entry establish the body.
The neighboring scalar-delete function must not absorb this second entry.
The base D5B000 profile has a pure-virtual slot, so routing rejects that profile.

For mixing, manager mute selects zero; otherwise manager `+6C * +4C` is rounded
once to float. Each row mixes current * fade * caller gain * table gain * global
with the native x87 spill points. Format 1 feeds the same channel to both sides;
format 2 splits adjacent channels; other formats fill their declared channel
range. Layout 1 routes the combined array to speaker 0, layout 4 to speaker 2,
and layout 2 routes left/right to speakers 0/1. Other layouts first clear speakers
2 through 5 using count 6, then route left/right. Handles and channel counts are
reloaded at each library call. Native descriptor bounds remain preconditions;
there is no invented truncation or remapping.

## State and teardown details

Start uses mode `100C0h | (loop ? 2 : 0)`, the resolved filename and null extra
info. Only FMOD result `2Bh` invokes the recovered memory-stat checkpoint. Start
sets state 1, clears four control bytes, and starts a fade-in from zero when a
saved position exists. Row rates are preserved. Missing nonempty names use the
real lookup's row-zero result, while an empty name enables every row.

The tick first checks current manager `+69`. When not paused at the source, it
updates levels before changing fade. Fade-out has priority; its zero endpoint
requests stop on the following endpoint check. Fade-in uses the actual widened
0.3 constant and `00415510` minimum semantics. SSE ordered equality and x87
unordered comparisons are preserved explicitly rather than replaced with
ordinary clamping expressions.

Starting state 1 queries open status. Ready status 0 writes state 2 and obtains
length before inspecting the stop request. It either performs ready/play,
zero-delta level routing and pause, or immediately stops. State 2 ignores the
query's return and continues pause/is-playing calls even after query-side
cleanup. Non-loop EOF writes state 3 and retains both handles. Stop in state 3
also leaves those handles untouched; the reconstruction preserves this native
edge. The owning FMOD system must remain alive through the stream-owner drain.

Stop in state 1 repeatedly queries until state 0 or 2, yielding with Sleep(0)
without an invented timeout. Its state-2 path obtains the raw saved position,
sets it to zero if below 1000 or beyond the stored length, otherwise subtracts
1000. It stops the channel, releases the sound, clears those pointers, then
resets state, flags and fade. The error-query recursion terminates because the
sound pointer is cleared before calling stop again.

Native diagnostic builders allocate and release their pooled strings, with no
separate log sink. The two numeric helpers use the builder's integer format
(default `%d`) and the same raw DWORD bits. The decompiler incorrectly removed
their copy/append blocks as unreachable; the assembly was read in full. The
host CRT formatting is limited to the native valid 64-byte temporary domain;
an overlong/custom invalid format is not promised native overflow behavior.
Temporary and builder string cleanup use the existing recovered operations.

## Verification and remaining boundary

Win32 Release built through `scripts/build.ps1`, with the seed comparison run
before both existing CTests; both passed. The focused ignored-local probe
replayed original installed-image instructions for 63,888 row cases, 5,808
fade cases and 672 complete mix cases. Comparisons cover raw output bits, row
mutation, x87 status flags, three x87 precision modes, four rounding modes,
NaNs, seven speaker layouts, and muted/unmuted gains. It patches only the
replay's external addresses/calls, with captured route outputs; it is arithmetic
and routing evidence, not an audio or game test. Original span hashes are in
the report. Probe/runner are `local/sound_stream_probe.cpp` and
`local/run_sound_stream_probe.ps1`; no permanent tests were added.

The source links the actual lookup from companion commit `821149b7`; the owner
is companion commit `33606628`. Real DLL binding, application composition and
installed streaming validation belong to the primary/facade packets. No claim
of audible playback, complete game behavior, or binary replacement is made.
