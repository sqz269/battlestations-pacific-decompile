# Unit health unordered inputs (cc11_health_nan)

Addresses: `00877B90`, `00923BE0`. Read-only dependencies: `00876260`,
`00BF7420`, `00BF7456`. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified by the standard Ghidra client before live
queries. Descriptive names are hypotheses; these remain typed projections.

The setter now preserves a NaN request instead of replacing it with positive
zero. Its replication helper returns byte zero for unordered input before the
C++ integer cast. These are the two behavior changes. Existing finite arithmetic,
host switches and callback timing are unchanged.

This supersedes the NaN-storage limitation recorded by the earlier guard/pair
packets. It does not remove their callback timing, ownership or ABI limitations,
and their prior runtime pair did not test this change.

## Setter branches and storage

The pseudocode's `0.0 <= request` loses the unordered case. The listing is the
evidence:

| Sites | Native behavior under masked exceptions |
| --- | --- |
| `00877BAB..00877BC1` | Load current/request onto x87, `FUCOMIP`, `LAHF`, `TEST AH,44h`, `JNP`. Only ordered equality returns. Unordered continues even when both NaN payloads match. |
| `00877BC7..00877BD7` | `COMISS 0,request` followed by `JBE` bypasses the positive-zero replacement for unordered, equal and positive requests. Only ordered negative requests select positive zero. |
| `00877BD9..00877BF7` | Spill/reload maximum, `FCOMIP request,max`, `JBE`. Only ordered greater-than selects maximum. Unordered retains the original request from XMM1. |
| `00877BFA..00877C04` | Store XMM0 into `+370h`. The x87 loads may quiet signaling operands in their own copies; the stored request comes from the original XMM bits. |
| `00877C0C..00877C20` | Subtract double 1.0 from maximum using ambient x87 precision, compare stored health, and set `+2E8h=-1` only for ordered greater-than. There is no else/reset write. |

The correction is `requested < 0.0f`, replacing `!(0.0f <= requested)`.
The upper comparison and equality gate retain their existing forms.

| Input situation | Data result |
| --- | --- |
| Equal finite values, opposite signed zeros, or equal signed infinities | Return before clamp; preserve existing health bits; no callback eligibility. |
| Current is NaN, request is finite | Equality does not return; apply the ordinary request/max branches. |
| Request is quiet or signaling NaN | Write original request payload/sign/signaling bits; marker is not set. |
| Maximum is NaN, nonnegative request | Upper comparison is unordered, so retain request; marker is not set. |
| Maximum is NaN, negative request | Lower branch still stores positive zero; marker is not set. |
| Unequal negative request, including negative infinity | Store positive zero without consulting the upper clamp. |
| Positive infinity request and finite maximum | Select maximum. With positive-infinite maximum, retain infinity and do not set the marker. |
| Unequal negative-zero request with positive maximum | Store negative zero. A negative maximum can still win the ordered upper comparison. |

The marker result in `UnitHealthWrite` means "perform the set-to-minus-one
write," not the current contents of the native marker slot. Callback eligibility
remains a write result unless the session mode is the multiplayer client value
2. Native `00877C48` reloads session mode after the callback and host mode 1 then
reads the getter; the typed snapshot and the host's existing callback timing
remain partial contracts.

## Getter and both replication conversions

`00923BE4..00923BEE` returns positive zero when `+5Dh` is set and leaves cache
`+164h` unchanged. Otherwise `00923BF7` invokes vtable slot `+110h`.
For a unit, `00876260..00876274` loads current, divides by maximum, spills to a
float and reloads it. There is no zero-maximum guard.

The getter spills that virtual result again at `00923BF9`, floors ordered
negative values at zero via `FCOMIP 0,value / JBE` (`00923C03..00923C16`), and
caps ordered values above float 1.0 via `COMISS / JBE` (`00923C27..00923C34`).
It stores float bits into `+164h` at `00923C16` or `00923C41`; this is not an
integer numeric conversion. Unordered values survive both clamps. A signaling
NaN has been quieted by the masked x87 load/arithmetic/spill path at this point.

`00877C53` calls that getter. `00877C58` multiplies its ST0 result by double
256.0, then `00877C5E` calls `00BF7420`. The runtime dword at `0109EEA4` chooses
between two different conversion paths:

| Mode | Masked NaN conversion | Setter's final byte |
| --- | --- | --- |
| Nonzero, `00BF7429..00BF743B` | Spill to double, `CVTTSD2SI`; EAX is signed integer-indefinite `80000000h`. | Negative check at `00877C63..00877C67` selects zero. |
| Zero, `00BF7456..00BF74CA` | `FISTP qword` yields `8000000000000000h`. Low EAX is zero; `00BF74B5..00BF74BF` masks off the high sign bit and takes the two-pop shortcut. | Zero survives the signed clamp at `00877C63..00877C77`. |

Thus `health_fraction != health_fraction` can return zero before the typed
integer cast for the masked data-result contract. Generated optimized Win32
assembly confirms unordered branches bypass `CVTTSD2SI` both in the helper and
in its inlined setter paths. Merely observing the old compiler's output of zero
would not make a C++ NaN-to-int cast defined.

The constants were read live: `00D7A24C = 00 00 80 3f` (float 1),
`00D7A210 = 00 00 00 00 00 00 f0 3f` (double 1), and
`00D0DEE0 = 00 00 00 00 00 00 70 40` (double 256).

## Precision and exception boundaries

The setter, fraction and getter do not install an FP control policy. The
subtraction/division obey the ambient x87 precision and rounding controls;
SSE comparisons and conversions use MXCSR. Existing `X87_CONTROL_WORD.md`
records the original startup/Direct3D evidence. The earlier reconstructed ON
pair logged PC53 before Direct3D (line 26) and PC24 at the fixed step (5978).
Those logs do not measure the original executable at this setter.

The focused probe used all exceptions masked, x87 PC24/53/64, all four rounding
modes, matching MXCSR rounding, and DAZ/FTZ disabled. It found these existing
marker disagreements with the double C++ projection:

| Current / request / maximum | x87 setting | Native marker set | Typed marker set |
| --- | --- | --- | --- |
| 0 / 2^25 / 2^25 | PC24 nearest or upward (`007Fh`, `087Fh`) | No | Yes |
| 0 / 2^54 / 2^54 | PC64 nearest or upward (`037Fh`, `0B7Fh`) | Yes | No |

At PC24 nearest, `2^25 - 1` rounds back to `2^25`. Changing the clamp must not
silently change this separate arithmetic contract. The marker remains as it
was; the four observed differences are reported as known gaps, not passes.

Quiet/signaling distinctions also affect exceptions: native signaling loads,
`COMISS`, `FCOMIP`, division, and conversion can raise FP exceptions. The source
uses different instruction domains and the new byte guard skips the native
invalid conversion. No equality of FP status, exception timing, instruction/data
pointers, or unmasked exception behavior is claimed. Unmasked conditions may
interrupt before a nominal write/callback. DAZ/FTZ variants and independently
different x87/MXCSR rounding modes were not tested.

## Validation

One ignored local probe, `local/cc11_health_nan_probe.cpp`, transcribes the
native setter prefix and getter instructions and reuses the existing recovered
CRT assembly in `src/native_render_batch_keys.cpp`. It is a bounded instruction
comparison, not execution of the complete original setter or original game.

Build/run: `cmd /c local\cc11_health_nan_check.cmd`, MSVC Win32,
`/O2 /Gy /W4 /WX /fp:strict`, linked with `/MANIFEST:EMBED`.

- 336 prefix cases: 28 input rows across 12 PC/rounding settings. Storage bits,
  ordered equality and callback eligibility match after correction. Before
  correction, 60 NaN-storage cases failed.
- 672 additional client/host data and eligibility checks pass; 120 failed before
  correction. These do not prove full native callback/replication ordering.
- 240 byte comparisons across both CRT modes pass, including positive/negative
  quiet/signaling NaNs. Released getter returns zero without a cache write.
- Four known marker differences remain; no unexpected marker difference in
  this matrix. Getter input fraction 2 returns 2 in the typed projection while
  native getter returns/caches 1; this existing gap is deliberately retained.
- Strict compilation and `git diff --check` pass. Native callsite checks cover
  `00877C53 -> 00923BE0` and `00877C5E -> 00BF7420`; the getter's indirect call at
  `00923BF7` was checked in the listing.

Before/after logs, generated assembly and executable are under the worker's
ignored `local/`; hashes and outcomes are in `reports/unit_health_unordered_cc11.json`.
There are no new tracked tests or shared build edits. Full integrated build/CTest
belongs to the primary. No game run, native ABI compatibility or broad IEEE
conformance is claimed.

## Remaining bounded work

The next arithmetic packet can own `00877C00..00877C20` within `00877B90`:
reproduce the stored-float load, maximum load, `FSUB` double 1.0 and ordered
`FCOMIP/JBE` marker decision under the caller's actual x87 control word, without
changing that word. Reuse the two concrete precision witnesses above. Do not
replace the expression with an algebraic shortcut or infer PC from a constant.

A separate getter packet must retain `00923BE0`'s release/cache/virtual-dispatch
contract and the `00876260` division/spill precision. The current pure getter
omits floor/cap/cache, and the typed finite byte conversion still requires the
scaled value to fit int. Infinity/out-of-range casts, native runtime mode
overflow behavior, callback mutations, and full FP-state parity remain outside
this correction. No host behavior switch was changed.
