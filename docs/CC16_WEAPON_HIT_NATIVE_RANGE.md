# Weapon accuracy: actual conversion-cell handoff

This packet corrects an existing live function without adding original body
credit. `008383D0` was already reconstructed and its PC24 cohort is already
ON. The independent `kWeaponHitAccuracyNativeRangeBound` guard stays OFF.
Old public APIs and the current cohort remain intact; the new explicit-cell
overloads are used only by the guarded ship-AI caller.

## Genuine producer and lifetime

The actual cell is not inferred from the host CPU or replaced by a default:

1. `GameNativeCanonicalDataOwner` adopts the checked bootstrap reservation,
   validates the supported original PE, and owns its mutable pages.
   `GameNativeMutableCrtData::feature_word()` returns the actual volatile
   DWORD at `0109EEA4`. Its initial zero is verified PE-loader state, not a
   fabricated mode value. The canonical owner intentionally survives for
   process lifetime.
2. `game_main.cpp` borrows that reference through `native_crt_feature` and
   passes the same cell to
   `GameNativeDynProcess::initialize_convex_pool_once_00cc89c0` before the
   application's live ship AI runs.
3. `GameNativeDynProcess::crt_conversion_0109eea4()` verifies completed
   startup and returns that retained reference. The guarded live
   `weapon_hit_probability` caller borrows it after loading the real four
   weapon profiles, then passes the reference through `008386F0` to the
   corrected `008383D0` overload.
4. The kernel passes only the address in ECX to the existing qualified
   `native_crt_truncate_st0_00bf7420`. That converter executes
   `CMP DWORD PTR [ECX],0` at conversion time. No earlier C++ value read,
   bool projection or permanently captured mode participates.

The original live caller is `006EB060`, not `006EB050` (which belongs to the
preceding function). Its current subtype-to-profile mapping, range fraction,
target length, authored profile contents, four-profile order and serialized
identities are unchanged. The consumer adds no initialization or write to
NativeData, the installation, saves or profiles. Startup and Dyn service
files are read-only dependencies of this packet.

## Native instruction and spill contract

Target-verified `bsp.py` queries inspected project `bsp` at
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
The complete existing body is `008383D0..00838526`, 343 bytes. The normal
RET8 starts at `00838509`; the tail clamp block ends with the JMP at
`00838525`. The older header's end `0083851E` was inside that tail block and
is corrected as documentation, with zero new body credit.

The size interpolation is retained from the current implementation. Its
explicit float result boundary is proven by native FSTP at `008383FE`.
Starting with that stored size weight, the new kernel follows the native
post-interpolation range and profile-blend instructions. This packet does
not broaden the inherited size-interpolation helper's numerical contract.

At `00838469`, `FMUL ST1` leaves the extended result of t times -10 in ST0.
It reaches the existing converter directly, retaining the lower live x87
values. There is no C++ float/double argument, pre-rounding or extra spill.
The converter itself chooses the native double-spill/CVTTSD2SI route for a
nonzero cell, or its established `00BF7456` route for zero. Its existing
qualification, error policy and low-EAX semantics are inherited. Neither
vendor body nor converter implementation changes here.

The kernel also retains `FMUL 10`, integer FISUB, the float range-weight
spill/reload, the lower-weight float spill, the large/small blend spills,
and the final result float spill/reload. No control word or MXCSR setting is
forced. Original function calling convention remains __thiscall, two stack
floats, RET8 and ST0 result; these C++ overloads are not binary ABI adapters.

The first comparison is actual `FCOMIP` followed by JC at `00838437`.
Unordered sets CF and enters the next comparison, rather than the legacy
C++ negated-less-than low bucket. The subsequent COMISS/JC at `0083844A`
also sends unordered to the conversion path. Later JBE clamps retain their
native unordered behavior. Integer -1-minus-EAX and signed index clamps run
as native integer instructions, avoiding a C++ out-of-range cast policy.

Immutable constants were checked against live original bytes: threshold
`00D7A3A0` is `00 00 00 A0 99 99 B9 3F` (exact promoted float0.1), -10 at
`00D0A198` is `00 00 00 00 00 00 24 C0`, and +10 at `00CE3DC0` is
`00 00 00 00 00 00 24 40`. These are proven values, not substitute mutable
globals. Profile size58h and accuracy-array offsets08h/30h are asserted.

## Validation and limits

The standard `scripts/build.ps1` Win32 Release build passed, followed by
`reconstructed_math` and `tool_tests` (2/2). The first build exposed Windows
min/max macros through the added runtime header; a source-local NOMINMAX
guard corrected that compile issue. The final build log is
`local/weapon_hit_native_range_build_final.log`.

Current object disassembly shows FMUL ST1 at helper+73h, address load into
ECX at +75h, and the existing converter call at +78h, with no intervening
spill. FCOMIP/JB and COMISS/JB preserve both initial unordered branches.
The original blend spill epochs and final float reload are present. The
default-OFF ship-AI object contains zero references to the conversion-cell
getter. These are compile/static proofs, not runtime acceptance.
The live checker verified all14 original call rows with zero failures; the
Markdown evidence check reported no failures, unchecked claims or warnings.

The OFF build's `bsp_game.exe` SHA-256 is
`5ea16d4cb249f70f50d5e97c3ce99fd67ddd1ffa2c7120d03e9f071b8c148d16`.
No game, ON build, WINS change or original-install write was performed.

The smallest recorded existing converter check is the38-case probe in
`reports/camera_frame_state_audit.json`: CW007F/027F, both runtime modes,
and two RGBA cases including negative, large, NaN and infinity inputs. It
compares the original extended multiply/conversion path against the existing
binding. Its named `local/camera_scoped_validation.log` and old fixture
worktree are absent at their recorded locations, so it was not rerun here.
That check covers converter exceptional inputs, not the new accuracy
function's initial unordered gate. The newer8-pair ambient-color check is
also recorded, but its old fixture path is absent. No new runtime FP or
original-body comparison is claimed by this source packet.

## Root-owned gates

First compare the new explicit-cell helper against hash-pinned original
machine code using the smallest existing private-payload infrastructure that
can honestly close all calls/data relocations. Exercise the concrete finite
multiply/conversion boundary and initial-unordered risk, mutable mode0/1,
and representative PC24/PC53. Synthetic relocated cells must be labelled;
they do not establish live startup or original-CRT execution proof.

After root accepts that evidence and allocates runtime slots, run its frozen
OFF counterpart and matched ON mission matrix at the exact integrated head.
Keep the accepted PC24 cohort fixed; toggle only the new guard. Preserve
the existing seeds/draw policy, frame caps, retained artifacts and comparison
rules. Root coordinates any resulting timing or WINS decision. Until those
gates pass, the new live path remains OFF and activation is unaccepted.
