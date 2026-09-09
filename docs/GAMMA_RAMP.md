# Renderer gamma ramp setter

Raw routine `00b21960..00b21b3a` is renderer primary virtual `+F0h` (table word
`00d5f198`). It has ECX renderer, one stack float argument, RET 4. This audit
verified saved project `bsp.gpr`, `/battlestationspacific.exe` before each batch
and compared the complete 474-byte routine to the installed PE. No code,
Ghidra mutation, gamma application or display change was performed.

## Gates, cached value and API call

The routine enters the existing optional renderer guard when global `0108d6dc`
is nonzero, then checks renderer byte `+1B53h`. If that byte is zero, it leaves
without reading/updating the cached gamma or building a ramp. Constructor
`00b32410` initializes bytes `+1B53h` and `+1B54h` to zero. Later capability
initialization and their exact capability-bit sources are outside this bounded
audit; their runtime values must be supplied from that initialization.

When enabled, it loads cached float `+196Ch` and the requested stack float onto
x87, compares using FUCOMIP, and uses LAHF/TEST AH,44h/JNP to skip exactly
ordered-equal values. Positive and negative zero therefore compare equal;
unordered NaN comparisons do not take the skip. It pops the remaining input
on the skip path. The comparison is numerical, not raw bit equality.

For a changed or unordered value, the routine starts x87 normalization with
FADD, then copies the original input DWORD using MOVSS into cache `+196Ch`
before FDIV and the normalization spill. The cache records the **requested
input**, not clamped normalized gamma or exponent. Native exception behavior
can interrupt this sequence; it is not an atomic cache-plus-device update.

After generation it calls device virtual `+54h` with the stdcall arguments:

```text
device->SetGammaRamp(swap_chain=0,
    flags=(renderer.byte_1B54 != 0 ? 1 : 0), &stack_ramp)
```

The ramp is three identical arrays of 256 WORDs. The input is not a direct
gamma exponent. There is no HRESULT handling because this D3D method returns
void. The cache is updated before the API call, so a cache-equal later request
does not force another device application.

## Exact numerical pipeline

Direct literal reads established double constants 10 at `00ce3dc0`, 20 at
`00ce3d88`, 255 at `00ce4b48`, and 65535 at `00ce5f90`; single 1 is at
`00d7a24c`. For ordinary finite values, the pipeline is:

1. Load input float into x87, add double 10, divide by double 20, then spill
   to float32. There is no intermediate float32 spill between addition/division.
2. Clamp that float32 normalization to [0,1] using the x87/SSE branches below.
3. Load the chosen float into x87, double it by FADD ST0,ST0, compute reciprocal
   using FLD1/FDIVRP, and spill the exponent to float32.
4. For integer i from 0 through 255, FILD i, divide by double 255, spill sample
   coordinate to float32, reload it, then load the float32 exponent.
5. Call `00bfeb10` with x87 ST1=coordinate and ST0=exponent. Its result is
   spilled to float32 before reloading and multiplying by double 65535.
6. Save x87 control word, OR its rounding bits with `0C00h` to select truncation
   toward zero, FISTP a signed DWORD, restore the original control word, and
   write the converted integer's **low WORD** to blue, green and red arrays.

There is no final float32 spill after multiplication by 65535. There is no
explicit final clamp to [0,65535], saturation instruction, or nearest-integer
rounding. The word-store order is blue, green, control-word restoration, red.
Intermediate x87 results obey the active control word's precision and rounding;
the temporary truncation mode is introduced only around the integer conversion.

The loop's i increment occurs before WORD stores, so addressing i*2 plus the
adjusted array bases fills exactly indices 0 through 255. The native local
stack ramp is passed synchronously and is not retained by this routine.

## Range and exceptional behavior

The lower clamp compares x87 zero against the normalized float using FCOMIP.
Strictly negative ordered values select positive zero via XORPS; nonnegative
and unordered values proceed to the upper test. Upper COMISS compares against
float 1, and JBE retains the original value for less/equal **or unordered**.
Strictly greater values select 1. Consequently NaN is not converted to zero
or one by these branches when exceptions are masked. Signaling/quiet NaN
exception behavior depends on the x87/SSE comparison instructions and masks;
do not replace this with a generic `std::clamp` claim of bit parity.

With ordinary masked arithmetic, inputs <= -10 reach normalized zero and the
reciprocal step has division by zero, ordinarily yielding positive infinity.
Inputs >= 10 reach normalized one and exponent 0.5. Input zero gives normalized
0.5 and exponent 1. Positive/negative infinities reach upper/lower endpoints
respectively after normalization; NaN can propagate into the exponent and power
helper. Unmasked exceptions can interrupt before a completed ramp or cache/API
update. Underflow, denormals, signed zero and MXCSR modes can affect this route.

For invalid/out-of-signed-DWORD FISTP with the x87 invalid exception masked,
the integer-indefinite DWORD has low WORD zero; without masking, conversion
can fault instead. This explains why simply casting a host NaN or saturating
to 65535 does not reproduce the native path. Full exceptional-input parity is
not implemented or empirically validated in this audit.

## Power helper dispatch

`00bfeb10` is an internal x87 power entry, not a normal stack-argument C function.
It selects the fast branch only when all of these exact tests hold:

- global DWORD `0109eea0` is nonzero;
- `(MXCSR & 1F80h) == 1F80h`;
- `(x87_control_word & 007Fh) == 007Fh`.

The fast path tail-jumps to `00c19260`. That wrapper aligns its stack to 16
bytes, exchanges x87 operands, spills/pops coordinate then exponent as two
doubles, and calls `00c19279`. The fallback exchanges operands, spills/pops
coordinate as double, stores exponent as double while keeping it on x87,
loads the exponent's high DWORD into EAX, then calls `00bfeb6d`.

The fallback has additional control-word adjustment and extensive zero,
negative-base, infinity/NaN and runtime math-error handling. Its direct helpers
and `00c19279`'s power core were not reconstructed here. The mathematical
operation is power, but substituting modern `pow` or `powf` would not establish
identical rounding, special values, errno/callback behavior or FP flags. A full
gamma port must recover/reuse that arithmetic contract or explicitly constrain
and validate its supported domain. The original lookup-table/polynomial internals
were not chased beyond these immediate dispatch helpers.

## Startup and reset consequences

Startup calls virtual `+F0h` with positive zero at `00b2b1c0..00b2b1c8`.
This produces exponent 1 **only if** the capability byte is enabled and the
ordered cache comparison permits an update. It is not authorization/evidence
that startup always writes a gamma ramp. Even the exponent-1 path contains
float32 sample spills and truncation, so an idealized `i*257` identity ramp
should not replace the recovered numerical path without a WORD comparison.

Reset path `00b29670` loads cached `+196Ch` at `00b2990f` and passes that value
to virtual `+F0h` at `00b29923`; the inspected related startup/reset path
`00b2abd0` does the same at `00b2adae..00b2adc2`. A finite unchanged ordered
cached value is skipped by the setter. Earlier cache mutation or NaN can change
that outcome, so the surrounding reset orchestration must preserve the call
and gates rather than unconditionally applying a ramp. Capability-setting and
cache initialization sites beyond these direct observations remain unresolved.

## Verified byte ranges

Ends are exclusive. Ignored evidence is in
`exports/bsp/owner_textures/gamma/evidence.json`; raw assembly includes the
missing-function setter through its final RET 4.

| Start | End | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `00b21960` | `00b21b3a` | 474 | `395daf39c6ea3f1871fd11d6aea7b1d6e5d9d2005f6d876b39fd3ffc26ef894a` |
| `00bfeb10` | `00bfeb64` | 84 | `b3beb57cc454f176724b8b08a07f64930e11c1f4cc2178a697a31fa3aeb67b1c` |
| `00c19260` | `00c19279` | 25 | `e4a6378ddfeae050ac73aedab430795bad2103d132257fa2e3c6477c01f71a4d` |

These are static byte-identity checks. No monitor gamma, power-helper native
differential fixture, or game reset validation was performed by this task.
