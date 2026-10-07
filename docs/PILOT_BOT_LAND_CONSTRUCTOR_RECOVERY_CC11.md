# Retained land constructor recovery (cc11)

This is native evidence for the next storage/constructor packet. No constructor,
arena, profile or observer adapter was implemented by this recovery. Descriptive
names and higher-level field meanings remain hypotheses. Exact live/disk byte
receipts are in `reports/pilot_bot_land_constructor_recovery_cc11.json`.

The ordinary bodies reviewed completely are `009F9CE0..009F9D78`,
`009AFE70..009AFFEB`, `009B2E50..009B302B`, `009B3240..009B3307` and
`009F9980..009F99A6`; all ends are exclusive. Original receivers are ECX;
the head/lower/composite/outer constructors return their original identity in EAX
and pop two stack arguments, while the alias binder pops one. Composite and
outer constructors install private x86 EH frames; ordinary ordering does not
establish unwind parity.

## One retained storage producer

`009F9CE0` receives the plane and a binary32 reference-speed argument. It stamps
profile `00D21C74`, stores the input plane at approach+4 (task+3FC), copies
plane+538 to +8, copies plane+9D4 to +C (task+404), and copies plane+DF4 to +10.
It then reads plane+DF4 independently again, takes that object's DWORD+34,
multiplies by 248h with native 32-bit IMUL and obtains row+Ch from the current
`00F8A30C` base. That identity goes to +14. Alias slots +18/+1C/+20 are cleared.
The two DF4 reads must not be folded into an assumed caller snapshot.

The head reloads plane+538 for its speed calculation: `FLD [class+188]; FDIV
[stack reference]; FSTP binary32 spill; FLD1; FLD spill; FCOMIP; FSTP ST0`.
JBE selects the actual binary32 one at `00D7A24C` for a ratio <=1 **or unordered**;
otherwise MOVSS retains the spilled quotient. +24 receives the result and +28
receives actual binary32 minus one at `00D7A260`. A source expression for the
formula is insufficient evidence for ambient x87 PC/RC, exception flags and
spill behavior. The existing `bot_task_speed_ratio` remains a pure data helper,
not proof of this complete constructor or native FP environment.

## Lower land publications and arithmetic

`009AFE70` first calls the complete tuning singleton, spills +52C via FLD/FSTP,
and calls the full head on the same approach storage. It stamps `00D1FDB8`,
stores the input block at +2C (task+424), reads block+7C once and stores that
owner at +30 (task+428). It then independently reloads the **original input
plane's** +9D4 and calls complete `006C0B50` on the original block. The returned
record identity goes to +34 (task+42C). It may differ from the squadron captured
by the head at task+404. The record is not a Boolean admission result or an
entry index.

After that call it writes +44=0, +4C/+48=one and +50=1. The remaining numeric
and pose phases preserve these native dependencies:

| Phase | Native observation and publication |
| --- | --- |
| First random | Capture a fresh complete singleton; stream ECX=1, bounds binary32 `0x3F666666` and `0x3F8CCCCD`; multiply returned ST0 by **that captured tuning object's current** +4E4, FSTP +54. |
| Second random | Capture another complete singleton, same stream/bounds; multiply ST0 by current +4E0. Observe approach+8 before FSTP +58; use that captured class object's +18C for the next calculation. |
| Class scale | FLD class+18C, FMUL binary64 0.75 (`00CEC9D8`), FSTP +5C. Intervening MOVSS stores set +60=positive zero, +64=10 and +A4=positive zero. |
| Third random | +AC gets binary32 0.5. Stream ECX=1, bounds positive zero and 0.5; retain `FCHS; FSTP +B0`, including a possible negative zero. |
| Owner pose | Reload retained owner+30; if its byte+C8 is zero, call complete `00414DB0` on that captured owner. FLD/FSTP owner+FC/+100/+104 to approach+38/+3C/+40. |
| Plane pose | Reload retained plane+4; if its byte+C8 is zero, refresh that captured plane. FLD/FSTP plane+100 **overwrites +3C**, then call complete `009AFAF0(0)` on the approach before returning its identity. |

The existing shared `native_particle_random_range_00bd2f10` and
`native_particle_random_state_range_00bd2e60` preserve the actual x87 path. The
latter spills/reloads binary32 before returning ST0. Their current RandomThreads
domain and scalar/global bindings remain required; a fresh seed, independent
random owner or arbitrary C++ callback result is not an admitted replacement.
Caller multiplication, FCHS, stores and ambient state still need verification.

## Composite and outer constructor ordering

`009B2E50` runs the complete lower constructor first, constructs the registry
at approach+B8, stamps composite/registry profiles, then constructs MoveTo at
+CC and Follow at +108 using the observed constants and retained owner. It
initializes the remaining land states at +1A0/+1C0/+1E0/+200/+228/+254, including
their owner, flags, counters and profile words, clears +274/+275, and calls
complete `009AF9A0` to register eight state names. MoveTo observer services
concern that state's target+2C; this is not registration of task+404 or plane+3FC.

`009B3240` calls complete `0099C6F0(owner,kind=3)` first. It then reads the
owner's current plane+50 and constructs the composite approach at task+3F8
with that plane and the original block argument. Afterward it captures the
retained plane+3FC, stamps profiles `00D1FFA0`, `00D1FF94` and `00D1FF90`, and
invokes slot+38 on the **plane's embedded +72C object**. A false result selects
task+620 (park). A true result reloads task+3FC and calls complete `007B8AD0`;
leader selects task+4C4 (MoveTo), otherwise task+500 (Follow). The selected
state identity is published at task+310 and its entry slot+4 runs before
`009F9980` binds the approach aliases.

The alias binder stores task+4, task+314 and task+38C at approach+18/+1C/+20.
Its native null case clears only the +18 command identity; integer LEA still
produces addresses 314h and 38Ch for the other two slots. A translated C++
null fallback must not silently turn all three into null.

## Source boundary

The current sparse `BotTaskHost` model has no stable canonical retained-field
storage and cannot supply these constructors. The current `GameUnitsHost`
queue projection lacks native queue-record+4 identity and slot-tail observer/
message transitions, so it cannot supply complete `006C0B50`.

A conditional source caller may require complete lower services on the **same
caller-owned stable task storage**, with actual profile and state-entry
bindings. That is not an actual arena/constructor adapter. Producing all five
retained identities in source requires the full head/lower storage sequence,
the arithmetic above, faithful complete queue/geometry/random services and
fresh versus captured observations. Constructor/arena ownership, raw mapping,
private EH/fault cleanup, observer lifetime, concurrent mutation, complete FP
status, original ABI and live game behavior remain open.
