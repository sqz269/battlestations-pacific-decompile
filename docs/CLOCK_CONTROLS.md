# Clock control and timestamp sampling

Implementation update: `src/frame_clock.cpp` now implements addition, sampling,
pause/resume and mode disable along with the previously reconstructed fixed
setter. FRAME_CLOCK records the integrated probe result and ABI/ownership limits.
The original read-only audit below documents the native evidence.

Read-only audit on 2026-09-09. Each live batch verified project `bsp`, program
`/battlestationspacific.exe`, image base 00400000. The concrete clock has vtable
00d68d50; application initialization allocates 80h bytes at 0073d48f before
constructor 00bedfb0. These findings complement STARTUP_NEXT_HANDOFF; the parent
owns reconstruction of clock update, initialization and timestamp subtraction.

## Control slots and original ABI

| Slot | Address | Inputs / return | Behavior |
| --- | --- | --- | --- |
| +0C | 00bedae0 | ECX clock; RET | Enter paused state once and save sampled timestamp |
| +10 | 00beddc0 | ECX clock; RET | Resume once and shift clock origin by paused duration |
| +14 | 00bee050 | ECX clock; EAX pair pointer; RET | Return this+20h, current elapsed pair |
| +18 | 00bee060 | ECX clock; EAX pair pointer; RET | Return this+30h, previous elapsed pair |
| +20 | 00bee080 | ECX clock; stack output pair; EAX output; RET 4 | Sample current timestamp, with mode-dependent behavior |
| +24 | 00bedb20 | ECX clock; stack signed int32 milliseconds; RET 4 | Enable fixed-counter mode and calculate increment |
| +28 | 00bedb60 | ECX clock; RET | Clear fixed-counter mode byte +69h |

These addresses are descriptive hypotheses, not recovered source symbols. Ordinary
Ghidra xrefs to pause, resume, sample and fixed-counter setter are vtable data
references only. This does not imply they are unused: callers dispatch virtually.

## Fixed-counter control

00bedb20 first writes byte +69h = 1, then calls QueryPerformanceCounter with
destination this+78h. It sign-extends the supplied milliseconds using CDQ,
multiplies by the 64-bit frequency at +60h through 00bf7df0, then signed-divides
the low 64-bit product by 1000 through 00bf7d40. It stores the resulting increment
at +70h/+74h. The multiplication precedes division; replacing it with floating
seconds or dividing the frequency first changes behavior.

No range check rejects zero or negative milliseconds. There is no update of
start pair +10h, current +20h, previous +30h, interval +40h, paused byte +68h or
saved pause sample +50h. Switching off through 00bedb60 only writes +69h = 0;
it does not query QPC or reconcile those pairs. HRESULT/BOOL failure from QPC
is ignored in the native routine. Exceptional signed arithmetic should follow
the parent audit of the CRT helpers rather than introducing C++ overflow UB.

Application initialization at 0073daaa tests global byte 00e1ae81; if nonzero,
0073dab3..0073dac0 calls singleton01090ab0 virtual +24h with argument 32h (50
milliseconds). This proves a concrete startup caller and a nominal 20 Hz fixed
step, but the meaning and writers of that global gate are not established here.

## Timestamp getter has two different sources

00bee080 checks only fixed-counter byte +69h. If set, it copies all four DWORDs
of the already-computed elapsed pair at +20h into the caller's 16-byte output.
Otherwise it calls QueryPerformanceCounter into a local 64-bit value and copies
that absolute value plus stored frequency +60h into output. Both branches return
the output pointer in EAX and clean four stack bytes.

It does not consult the paused byte +68h and does not subtract the origin in the
normal branch. Therefore the fixed-mode result is elapsed time while normal mode
is absolute QPC time. This distinction is direct assembly evidence; do not make
the interface uniformly absolute or elapsed as an unsolicited correction.
Existing worker code 00b33c20 invokes virtual +20h to obtain the timestamp used
in its rate calculation (see RENDER_WORKER and worker_event_evidence).

## Pause and resume

00bedae0 returns immediately when already paused. Otherwise it sets +68h = 1
**before** invoking virtual +20h, then copies that returned 16-byte pair to +50h.
It does not force interval +40h to zero in the pause setter itself; the next
clock-update call performs the paused interval subtraction.

00beddc0 returns immediately when not paused. Otherwise it sets +68h = 0
**before** invoking virtual +20h, computes sampled-now minus saved +50h using
00530890, and adds that duration into the origin pair +10h via 00bedb70.
It does not update current, previous, interval or frame counter itself.

For an unchanged fixed-counter mode, repeated paused updates leave +20h alone,
so the sampling calls ordinarily subtract equal elapsed values. For normal QPC
mode, shifting the origin excludes time spent paused. Transitions between modes
during pause are not normalized by these routines and have not been exercised;
preserve the observed sequence and avoid claiming cross-mode parity from a
normal-mode check.

## In-place timestamp addition prerequisite

00bedb70 takes left pair in ECX and a pointer to right pair on the stack;
returns left in EAX with RET 4. It leaves left frequency unchanged. With equal
frequencies, it adds right ticks using ADD/ADC (64-bit modular arithmetic).
Otherwise it calculates low64(right ticks * left frequency), signed-divides by
right frequency using the same CRT helpers as subtraction, then adds the result
modulo 2^64. The full function is 83 bytes, 00bedb70..00bedbc2. Exact self-alias
works for the equal-frequency path because it reads the right high word before
writing the left high word; arbitrary partial overlap is not validated.

## Byte evidence

Each inclusive range below matches the installed executable and live saved
program byte-for-byte. Hashes cover the complete respective routine:

| Range | SHA256 |
| --- | --- |
| 00bedae0..00bedb19 | c1bfdf1902e283c95d94bc952aa532df00f5982ad09dbc0a0ca1776fcdeb64f4 |
| 00bedb20..00bedb5c | 962c16ccbdc50f4bbea70b7156ecc985b2a657e3b59055badbf7574455c52659 |
| 00bedb60..00bedb64 | 0ed46bcfc8a418c5b9a98e2eb60e77aaf5912e184946f0c04aa284b595d59b43 |
| 00bedb70..00bedbc2 | 70223900e6338b4fca80c5a81c6de7a518fd972d6ac3c70f4137106470afde48 |
| 00beddc0..00beddfb | b72e4b428c5785fc93a3c61cffcaddab4a14637acc9617cceb512a8e8d1a0e3c |
| 00bee050..00bee053 | dadecdeeff0b9454fd9dc08c558a9148187b2b1bec017fe8b92430570b86012a |
| 00bee060..00bee063 | 0f44d97c9f42d2e1a1e7956b0bb8e4b07c70feb205b095c08c6fc79b67e90094 |
| 00bee080..00bee0dc | 7e990a52cc76f8cc7ef3b1607abf21f2250505786338e6a1b8a5949499a4f756 |

No shared metadata, C++, Ghidra annotations or original game files changed in this
audit. There was no build or runtime test. These controls do not establish the
clock singleton's complete lifetime or the full application frame lifecycle.
