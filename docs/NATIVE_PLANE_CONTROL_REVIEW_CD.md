# Incoming plane-control review: two reproduced floor mismatches

Review of main revision `23f78d03c0fdb5d40890936193ddbfb5adfc94fb` found two concrete mismatches in `plane_control_axis_step_007da710`. A focused MSVC Win32 diagnostic executes the original arithmetic fragments and reproduces both against the source at that revision. The source owner still leases the affected plane-control files; this packet records the findings without changing those files or their Ghidra annotations.

| Case | Original arithmetic | Incoming source |
| --- | --- | --- |
| Flat floor, zero polynomial rate | `0.600000024`, bits `3F19999A` | Default `0.150000006`, bits `3E19999A` |
| Crossing floor, current=1, prior interpolation product=0.5 | `1.5 × 1 × 0.5 = 0.75` | `1.5 × 1 = 1.5` |

## Flat floor

The unchanged executable and live Ghidra bytes at `00CE3D30` are `9A 99 19 3F`. This is float32 `0.6`, not `0.15`. `007DABBC` loads those bytes into XMM5. There are no intervening XMM5 writes before the flat-floor branch at `007DAC48`: it compares the computed rate with XMM5 and, unless the rate is greater, stores XMM5 to the rate slot at `007DAC53`.

`PlaneRotationFactors::idle_floor` defaults to `0.15f`. Its header comment cites the correct address and bit pattern but decodes them incorrectly, and `PLANE_CONTROL_RATE_LAW.md` repeats that value. With current=0, target=1, a zero polynomial rate, flag=false and step=1, the copied floor fragment produces `0.6`; the source axis step returns `0.15`. The remaining candidate arithmetic and clamp do not alter the floor in this example.

Required correction: use the original `0.6f` floor and correct the current formula/documentation. Passing an independently chosen `idle_floor` still represents a source parameter; it does not establish the native constant.

## Crossing floor

The source currently computes `crossing_gain * abs(current)` with default gain `1.5`. The original computes an additional product:

1. Two calls to `00419010` in `007DAABD..007DAB36` form interpolation results from the pitch-angle and forward-speed/stall inputs. `007DAB36` multiplies their results; `007DAB46` stores the product to `[ESP+38h]`.
2. `007DABE4` reloads that stored product onto the x87 stack. The stack pointer does not change between this load and the crossing-floor arithmetic.
3. `007DAC1C` loads the sign-masked current value, `007DAC20` multiplies by the original double `1.5` at `00CE3D78`, and **`007DAC26 FMUL ST1` multiplies by the still-live interpolation product** before the float32 floor store at `007DAC28`.

The crossing-floor source interface has no input for that product, and the new `GameUnitsHost` caller does not calculate or supply it. A value of 1 cannot be assumed from the native listing. The diagnostic supplies 0.5 directly at this established arithmetic boundary: the original fragment gives a floor of 0.75. For current=1, target=0, a zero polynomial rate, the crossing flag=true and step=0.125, this yields a candidate of 0.90625 within the unchanged [0,1] interval. The incoming source returns 0.8125 from its unscaled 1.5 floor.

Required correction: carry the recovered interpolation product through the source rate-law contract and its actual caller, preserving its native computation/store order. Do not replace it with an arbitrary default or fold it into the fixed compiled gain without documenting and supplying that input. The recovered native calculation reads mutable tuning and actual unit/controller fields; the host's missing pitch/owner inputs remain separate admission gaps.

## Diagnostic scope and integration status

The probe retains two PE/live-matching fragments: `007DAC48[17]` and `007DAC08[62]`. It appends a return to each, relocates the crossing branch's outward jump to that return, and rebinds the original double-constant address to identical host data. Explicit register/x87 inputs are supplied by naked wrappers. Its source object is compiled from the pinned incoming `plane_control_rate.cpp` and header, with `/O2 /fp:strict /W4 /WX /EHsc`; the executable includes a manifest. The two comparisons reproduce the discrepancies and exit successfully **as a diagnostic of failures**. They do not constitute a passing reconstruction test or full original `007DA710` execution.

The broader incoming C++ changes were reviewed for the camera/GroupParams integration: actual resource-instance construction/destruction and type publication keep their graph/external-owner limits explicit; the private Lua tuning adapter avoids overwriting the mission interpreter's class table. The plane host still lacks bot-produced controls and several native inputs. Existing zero-control compatibility runs cannot establish correctness of the two floor branches demonstrated here. This review does not certify native x87/NaN/ABI equivalence of the remaining high-level plane arithmetic or accept compatibility trajectories as gameplay proof.

The source registration registry is also leased by `cc7`. Camera/GroupParams source and its standalone paired proof remain published on `agent/orch4-20260910`, with project registration and main integration pending. See `reports/native_plane_control_review_cd.json` for exact source, bytes, and diagnostic hashes. No changes to the original installation, leased source, or other owner's Ghidra metadata were made by this review.
