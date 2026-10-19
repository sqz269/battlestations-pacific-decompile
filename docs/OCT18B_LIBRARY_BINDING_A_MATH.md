# OCT18B library binding A: math and x87 audit

Audit date: 2026-10-18. Baseline: `3d581c328`, branch
`agent/oct18b-libA-math`. This is an audit of library boundaries, with no game
run, host rewiring, new vendor implementation, or Ghidra mutation.

Fourteen math families have verified calls from original native game code:
**one bind-safe, thirteen behaviour-sensitive, zero layout-bound**. The unit
is a family row, not a function, site, or existing reconstruction record.
The conversion family has two roots, so these rows cover fifteen root
addresses. Three additional roots have only Lua callers in the inspected
direct-call inventory and are excluded from the fourteen-family tally.
Native source call evidence is not proof that a particular gameplay path ran.

Only conversion-mode divergence was numerically demonstrated in this audit.
The sensitive classification identifies a concrete argument-precision,
exceptional-input or FP-environment obligation whose output feeds game state.
Its proof state is separate: one row has observed conversion-mode divergence;
twelve rows have explicitly identified obligations with current-stock parity
pending. This does not assert that a stock difference occurred during a game
run, or recommend thirteen new vendor bodies. In particular, original x87/SSE
dispatch alone does not prove that a modern stock result differs on finite,
already materialized operands. None of the inspected floor/ceil/fmod callers
establishes a complete finite-input guard that would admit its whole contract
as qualified bind-safe, although the finite value subdomains are identified.
Their shared mathematical names do not establish bit or environment parity.

## Evidence boundary and coverage

Every live Ghidra batch used the guarded `bsp.py ghidra` client, which verified
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. The installed PE is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The primary's raw inventory is
`local/oct18b_library_binding_coverage_raw.json` in the `oct18b-libA` worktree.
It combines snapshot callers, bytewise E8/E9 rel32 scanning, and bytewise
absolute-dword target scanning. Counts below are **raw corpus counts**, including
library callers, repeated sites in one caller, jumps, and scan false positives.
They are not confirmed game-call counts. In particular, a nearest preceding
function start is only a proposed owner. Live `proto` and instruction-context
checks established the representative owners/sites in the next table.

`bsp.py callers` was queried for every root; live callers were also checked for
conversion, floor, sin and cos. Raw scanning matters: sqrt has 272 snapshot
caller functions but 416 candidate rel32 sites. There are zero absolute-dword
target hits for all eighteen root addresses below and the four reviewed
support entries `00C19260`, `00C19279`, `00C27489`, `00C28548`. This does not
exclude computed targets, unlisted interior entries, or script dispatch.

| Root | Role | Snapshot caller functions | Raw rel32 sites | Absolute dwords |
|---|---|---:|---:|---:|
| 00BF701A | x87 atan2 entry | 116 | 166 | 0 |
| 00BF7030 | x87 sqrt entry | 272 | 416 | 0 |
| 00BF7420 | mode-selected conversion | 137 | 245 | 0 |
| 00BF7456 | x87 signed64 conversion | 3 | 4 | 0 |
| 00BF8490 | x87 atan entry | 11 | 13 | 0 |
| 00BF857A | x87 fmod entry | 38 | 65 | 0 |
| 00BF85B0 | stack-double floor entry | 39 | 57 | 0 |
| 00BF9940 | x87 acos entry | 11 | 15 | 0 |
| 00BFA020 | x87 tan entry | 2 | 5 | 0 |
| 00BFA4EC | binary64 finite predicate | 2 | 8 | 0 |
| 00BFA7B0 | stack-double ceil entry | 3 | 6 | 0 |
| 00BFA910 | x87 log entry | 12 | 16 | 0 |
| 00BFE5E0 | x87 sin entry, Lua only in scan | 1 | 1 | 0 |
| 00BFE740 | x87 cos entry, Lua only in scan | 1 | 1 | 0 |
| 00BFE870 | x87 asin entry | 3 | 4 | 0 |
| 00BFEB10 | x87 pow dispatcher | 5 | 7 | 0 |
| 00BFEDA0 | x87 log10 entry, Lua only in scan | 1 | 1 | 0 |
| 00BFEEEC | x87 exp entry | 12 | 13 | 0 |

The saved `00BF85B0` listing has a broad multi-region body through `00C0F464`;
its 29,670 listing lines are not a 29,670-instruction floor routine. The live
body bounding range confirms this broad analysis shape. This audit reads the
entry, actual branch destinations and exact caller instructions, and does not
attribute intervening CRT code to floor. No listing repair was attempted.
Raw candidates `0043542A` and `00B83FDD` returned “No instruction at address”;
neither is admitted as a call. The latter was an exploratory guessed site.

## Family decisions and actual argument domains

These are static, per-boundary decisions. Pending proof does not assert a defect
in current source users. Several users already preserve native kernels.
Domains are established at the cited site, not assumed for every raw caller.

| Family / category | Proof state | Verified original game caller and exact site | Argument range and consequence |
|---|---|---|---|
| atan2 / behaviour-sensitive | parity pending | `00414EB0` heading, site `00414EB6` to `00BF701A` | Raw float y then x become ST1=y/ST0=x. No finite/nonzero guard. Zero pairs, signs and nonfinite bits are possible at this boundary. Result is float-spilled before subtracting promoted-float pi/2 and conditionally adding promoted-float 2pi. Current `_CIatan2` preserves the calling shape; original/current CRT parity remains unresolved. |
| sqrt / behaviour-sensitive | parity pending; existing native pin | `00419440` vector length, site `00419491` to `00BF7030` | Squared components and sum are float-spilled in native order. Ordinary finite nonoverflowing sums are nonnegative; overflow, NaN and subnormal cases are not excluded. Original sqrt has control-word, diagnostic and signed-zero handling beyond a bare FSQRT. PC53 followed by final float rounding needs separate comparison against direct modern float sqrt; no mismatch on this finite game domain is claimed. |
| conversion / behaviour-sensitive | mode divergence proved; source pin matched | `0041BD10` grid normal, site `0041BD57` to `00BF7420`; `00B51DF0` sort-key preparation, site `00B51EB1` to `00BF7456` | First site adds native double 1 to a float result, passes ST0, and consumes EAX. Sort-key site loads float entry+14h and consumes EAX after the signed64 route. No blanket signed32 range/finite proof is present at either boundary. The runtime dispatch flag changes both result and status location; details below. |
| atan / behaviour-sensitive | parity pending | `0042CF10` clamped asin helper, site `0042CF82` to `00BF8490` | Ordered input beyond +/-1 returns an endpoint; for ordinary input in [-1,1], atan receives the float-spilled ratio x/(sqrt(1-x*x)+1), ordinarily in [-1,1]. Ordered comparisons do not exclude NaNs. Native FPATAN/error/restore policy remains different evidence from a host numeric call. |
| asin / behaviour-sensitive | parity pending | `00401190` float adapter, site `00401194` to `00BFE870` | Unrestricted incoming stack float is loaded into ST0; there is no [-1,1] guard in this adapter. Result is float-spilled. Domain errors and NaNs therefore cannot be removed from the binding contract. This proves a native adapter call, not runtime use of every adapter. |
| acos / behaviour-sensitive | parity pending | `0059F7C0` map projection, site `0059F98D` to `00BF9940` | Receives a stored float from map-direction processing, with local sign selection, and float-spills before sign multiplication. Intended geometric inputs do not establish that every caller supplies a finite value in [-1,1]. Both game and Lua callers exist. |
| tan / behaviour-sensitive | parity pending | `0060ABD0` HUD plane effects update, site `0060C557` to `00BFA020` | ST0 is float-spilled half of the float at actual camera+80h. The nearby multiply uses native double 0.5. No finite, period or pole-distance bound is checked here. Current host FOV or slope assumptions cannot widen this into a universal safe tan substitution. |
| log / behaviour-sensitive | parity pending | `0074F490` leak manager initialization, sites `0074F4AE` and `0074F4C5` to `00BFA910` | First input is the float-spilled sqrt of actual settings+40Ch; second is native double 2 at `00D7A308`. The setting is not proved strictly positive here. Zero/domain errors or prior invalid sqrt can reach log. The first log result is float-spilled before division by log(2). |
| exp / behaviour-sensitive | parity pending | `00B4CEA0` Gaussian distribution, site `00B4CEC6` to `00BFEEEC` | For finite x,y and finite nonzero rho, receives float-spilled -(x*x+y*y)/(2*rho*rho), ordinarily <=0. No rho!=0 or finite guard exists in the routine. Input underflow, zero denominators and nonfinite intermediates remain relevant. The exp result is float-spilled before a separately rounded sqrt/reciprocal product. |
| pow / behaviour-sensitive | parity pending; existing native pin | `00B21960` renderer gamma, site `00B21A57` to `00BFEB10` | Base is float-spilled i/255 for i=0..255. For ordinary normalized gamma n in (0,1], exponent is float-spilled 1/(2*n), hence >=0.5. n=0 produces +infinity under masked arithmetic; NaN survives ordered clamps. Power is float-spilled, multiplied by double 65535, then x87-truncated and stored as low WORD. No generic powf/saturation substitution is admitted without qualification; a stock-result mismatch on this domain has not been demonstrated. |
| fmod / behaviour-sensitive | parity pending outside qualified finite value domain | `0085ABA0` turning-gun target angles, site `0085ABBD` to `00BF857A` | Dividend is the incoming float angle; divisor is the promoted-float 2pi double at `00CE3828`. For finite, already materialized operands and nonzero divisor, a stock fmod is the same mathematical remainder operation. Input magnitude/nonfinite values are not bounded here, and FP-status/error parity has not been checked. The remainder is float-spilled before native interval folding. No finite value mismatch is claimed merely from x87 versus SSE. |
| floor / behaviour-sensitive | parity pending outside qualified finite value domain | `004396F0` HUD digit-gauge round, site `004396FB` to `00BF85B0` | Incoming float is stored as a stack double; caller removes 8 bytes at `00439704`. No finite guard. Floor of the same finite binary64 input is a fixed, exact operation, so a stock value binding is appropriate within that domain. The result is float-spilled, then abs(input-floor) is compared to 0.5 for a possible +1. Original exception/NaN/dispatch effects and complete caller parity remain unverified, not proved different. |
| ceil / behaviour-sensitive | parity pending outside qualified finite value domain | `00B83F30` camera render targets, site `00B84106` to `00BFA7B0` | Receives a float-spilled size expression divided by actual scale, then stored as a stack double. Ceil of the same finite binary64 input is fixed and exact. The inspected setup does not establish a positive-finite scale for every call. Result is float-spilled before later x87 integer conversion; FP-status/exceptional parity remains unverified, not proved different. Lua ceil independently confirms the role. |
| finite predicate / bind-safe | exact integer-body stock comparison | `00794750` camera direction delta, site `0079484C` to `00BFA4EC` | Takes an already stored stack double and tests exponent!=7FF, returning exactly 0/1. All binary64 bit patterns are covered by the integer proof below. This says nothing about parity of the upstream normalization, atan2/asin or float-to-double stores. |

The claimed call instructions, checked with live instruction-context and live
containing-function metadata, are recorded explicitly for mechanical checking:

```text
00414EB6 CALL 00BF701A
00419491 CALL 00BF7030
0041BD57 CALL 00BF7420
00B51EB1 CALL 00BF7456
0042CF82 CALL 00BF8490
00401194 CALL 00BFE870
0059F98D CALL 00BF9940
0060C557 CALL 00BFA020
0074F4AE CALL 00BFA910
0074F4C5 CALL 00BFA910
00B4CEC6 CALL 00BFEEEC
00B21A57 CALL 00BFEB10
0085ABBD CALL 00BF857A
004396FB CALL 00BF85B0
00B84106 CALL 00BFA7B0
0079484C CALL 00BFA4EC
```

## Original body versus the current stock boundary

The original one-argument atan, asin, acos, tan, log, sin and cos wrappers read
actual `0109EEA0`, MXCSR and x87 CW. An enabled dispatch flag, all MXCSR
exception masks (`MXCSR & 1F80 == 1F80`) and x87 low mask bits
(`CW & 7F == 7F`) permit their SSE2 branch. These checks do **not** require
nearest rounding or a particular precision. The fallback first FSTs a
binary64 view without popping the x87 argument, classifies it through
`00C08418`, and enters its native x87 kernel. Exp has the corresponding gate
before its descriptor-dispatch route. Floor and ceil accept stack doubles
and use their own gated SSE2/fallback implementations.

The sin and cos fallback bodies contain FSIN/FCOS followed, for reduction
failure, by FPREM1 looping over the native extended constant. The log fallback
contains FLDLN2/FYL2X. Atan contains FLD1/FPATAN. These are specific old CRT
algorithms and environment/error routes, not proof of modern numeric parity.
The two-argument atan2 and fmod roots load their descriptor into EDX and tail
jump to `00C07EE0`. Its live listing saves x87 CW, reads `00E16BD0`, conditionally
stores binary64 operand views, then invokes the binary dispatcher/error exit.
That shared state and register-input ABI are part of the binding.

Original sqrt stores a binary64 view and preserves ST0 before its classifier
and worker. The worker includes FSQRT, control-word preparation/restoration,
negative-zero handling and `00C27489` error dispatch through native support.
The existing `legacy_crt_math` source retains the associated 32-byte exception
record and 112-byte FPIEEE continuation layout, owning errno access and bypass
state. Those are layout/state contracts within the sensitive sqrt family,
not extra family rows credited as game-native direct calls here.

The rebuilt tree has both current CRT uses and preserved native pins:

- `src/geometry_helpers.cpp` and `src/avoid_zone_arc.cpp` use the actual host
  `_CIatan2`; `src/vector_helpers.cpp`, `src/camera_decomposition.cpp` and
  `src/camera_position_modes.cpp` use host `_CI` entries. Their explicit x87
  load/spill order is useful evidence, but does not identify the host CRT's
  internals with the installed game's CRT.
- `src/system_camera_axes.cpp` preserves the original sqrt wrapper/worker;
  `src/legacy_crt_math.cpp` preserves its qualified exception provider.
- `src/native_render_batch_keys.cpp` preserves both conversion paths and is
  registered in `CMakeLists.txt`. The audit's focused probe checks this actual
  source. In contrast, the older `ftol2_low_00bf7420` in `src/cc13_big1.cpp`
  explicitly models signed64 truncation only; its comment does not describe
  the dispatcher’s signed32 SSE route. Its affected gameplay inputs were not
  demonstrated to reach a differing case, so this is an audit limitation,
  not a proven gameplay defect.
- `src/native_crt_pow_dispatch.cpp`, `src/native_crt_pow_fallback.cpp` and
  `src/native_crt_sse2_pow.cpp` retain the old dispatch and algorithms, with
  explicit borrowed contexts. `src/native_renderer_gamma.cpp` calls that
  dispatcher. They are existing qualified source interfaces, not proposed
  stock replacements. The existing pow audit's static instruction/table
  proof and unrelated eight native math seeds do not supply pow numerical
  differential validation.
- Current game hosts also use `std::sqrt`, `std::atan2`, `std::sin`, `std::cos`,
  `std::log`, `std::fmod` and other modern operations. MSVC Win32 `/fp:strict`
  is visible in the build configuration; it does not restore the old CRT
  global state, instruction schedule or x87 register-input API.

## Proven conversion-mode divergence

`00BF7420` reads actual DWORD `0109EEA4`. Nonzero selects
FSTP binary64/CVTTSD2SI EAX, while zero selects `00BF7456`.
The latter duplicates ST0, FISTPs a signed64 under current x87 rounding,
reloads the integer, subtracts, and corrects EDX:EAX toward zero; its
integer-indefinite special path has low DWORD zero. SSE signed32 indefinite
is EAX=80000000 instead. The two routes also accumulate status in different
FP units. A C++ cast or saturation is not a universal replacement.

One ignored probe transplanted the disk/live-equal 171-byte span
`00BF7420..00BF74CA`. SHA-256:
`79bef72f6967ef5ec8414c36f64fdb35a6167893f6dca509f73ef9312a2786c2`.
The only reached absolute data relocation is the dispatcher’s actual mode
pointer, replaced with the probe’s mutable mode word. The adjacent interior
helper bytes in that span are not invoked. Relative branches retain their
native destinations. No original process or CRT initialization is executed.

The probe compares that original entry to the actual current
`src/native_render_batch_keys.cpp` provider, supplying ST0 and its source
context correctly. It checks EAX, x87 SW and MXCSR on both paths, plus EDX on
the signed64 path. Eight masked control words cover 53/64-bit precision and
all four rounding modes; thirteen inputs include ordinary fractions, range
edges, signed zero, infinity, qNaN and an extended intermediate.
**208 original/source comparisons passed, zero failed; fifty cases had
different low DWORDs between the two original dispatch modes.**

| Input / x87 CW | Mode zero, x87 EAX | Mode nonzero, SSE EAX | Observed flags |
|---|---|---|---|
| 2147483649 / 027F | 80000001 | 80000000 | x87 SW=0020 vs SSE MXCSR=1F81 |
| -2147483649 / 027F | 7FFFFFFF | 80000000 | x87 SW=0020 vs SSE MXCSR=1F81 |
| 4294967295 / 027F | FFFFFFFF | 80000000 | x87 SW=0020 vs SSE MXCSR=1F81 |
| qNaN or +infinity / 027F | 00000000 | 80000000 | x87 SW=0001 vs SSE MXCSR=1F81 |
| extended 1-2^-54 / 037F | 00000000 | 00000001 | binary64 spill rounds to 1 on SSE route |

The last case is within the signed32 numeric range. It demonstrates why an
adapter that first narrows ST0 to double can change a finite truncation result.
The actual state of `0109EEA4` in a game session was not observed. Both original
modes were exercised deliberately, so no default runtime mode is inferred.

Toolchain: MSVC x86 19.51.36244, `/O2 /MD /EHsc /W4 /WX /fp:strict /arch:SSE2`.
The executable links with `/MANIFEST:EMBED`; its embedded manifest was extracted
successfully with `mt`. Probe executable SHA-256:
`435adb53e86343236e06c1efbd22ff21826e2ee950e4748068ebb6732df58c66`.
Provider source SHA-256:
`fe3101c51500415607fc9513a7528853b2285a64d1560bc8684e87b2b05e75e1`.
Ignored evidence in this worktree: `local/math_conversion_probe.cpp`,
`math_conversion_bytes.py`, `math_conversion_bytes.json`,
`math_conversion_probe_result.txt`, `math_conversion_probe.manifest`,
`math_conversion_artifact_hashes.json` and `build_math_probe.cmd` (all under
`local/`). This is focused original/source evidence, not original-address ABI,
unmasked-exception, concurrency or gameplay proof.

## Exact finite-predicate stock comparison

Original `00BFA4EC` reads the binary64 exponent using integer MOV/AND/CMP/SETNZ
and returns exactly exponent!=7FF. It executes no FP instruction and reads no
errno, CRT global, TLS or callback state. The installed x86
`C:/Windows/SysWOW64/ucrtbase.dll` `_finite` export at RVA `0006C8C0` was read
directly from that DLL and both returning paths inspected. It performs the
equivalent integer high-DWORD mask `7FF00000`, returning exactly 1 or 0.
Neither body changes x87/MXCSR or has a hidden object layout.

The stock DLL SHA-256 is
`e4f8f92eef00ff6a0c6d418af756a59d5a4b818d0d1de0d96cc193b4d6759432`.
The evidence is `local/math_stock_finite.py` and `local/math_stock_finite.json`.
This admits that specific scalar predicate binding for the same readable
binary64 argument bytes. It does not admit an upstream C++ rewrite or require
the modern camera host’s `std::isfinite(float)` to emit the identical program.

## Script-only roots and remaining limits

| Root | Exact Lua call | Native-direct status |
|---|---|---|
| sin 00BFE5E0 | `00A6167D` in `math_sin` | no direct native game callers or target pointers found |
| cos 00BFE740 | `00A616DD` in `math_cos` | no direct native game callers or target pointers found |
| log10 00BFEDA0 | `00A61A2D` in `math_log10` | no direct native game callers or target pointers found |

```text
00A6167D CALL 00BFE5E0
00A616DD CALL 00BFE740
00A61A2D CALL 00BFEDA0
```

These are **unreached only in the inspected native-direct inventory**, and
excluded from the fourteen-family classification. Lua registration and
script dispatch remain possible; absence of E8/E9 or absolute target words
does not prove that the script API is unused. Their wrappers were inspected,
but no stock-equivalence claim is made. Game-inline FSIN/FCOS and new host
`std::sin`/`std::cos` calls are different questions from calls to these roots.

Priority is: retain the already correct conversion pin; preserve sqrt/error
and pow pins and their actual runtime contexts; constrain or separately verify
remaining current `_CI`/modern numeric boundaries before granting exact parity.
The family table states concrete admitted domains and unresolved portions.
No broad test suite was added, no game/display gamma was changed, and no
repository C++ or build file was edited. Additive name-ledger evidence leaves
all current names intact; tentative role descriptions remain hypotheses.

Mechanical call verification checks the sixteen native and three Lua call
instructions above. The contiguous conversion-span warning is expected: the
probe intentionally contains two separate native entries. The rejected raw
candidate sentence is an intentional non-call claim, not admitted coverage.
