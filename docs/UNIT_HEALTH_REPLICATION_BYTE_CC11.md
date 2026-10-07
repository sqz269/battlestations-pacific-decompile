# Unit health replication byte numeric conversion

Packet `cc11_health_replication_byte` recovers the numeric conversion at
`00877C58..00877C77` inside `00877B90`. The new
`replicated_health_byte_numeric_00877c58(float, const volatile uint32_t*)`
loads a binary32 fraction, multiplies by the native double 256, calls the
existing shared CRT converter, and clamps its signed low EAX word to 0..255.
The descriptive name is a hypothesis, not a recovered symbol.

This is a numeric fragment with a new C++ ABI. The original setter, existing
one-argument partial byte helper, callback/session policy and replication
store/message logic remain unchanged. No conversion-state binding is invented.

## Native evidence

Read-only `bsp.py ghidra` queries verified the configured `bsp` project,
`/battlestationspacific.exe`, x86 little-endian 32-bit language and `00400000`
image base before each query batch. The configured project file is
`C:/Users/sqz269/bsp.gpr`. Live disassembly, constant bytes and refreshed
exports establish:

| Address | Instruction and consequence |
| --- | --- |
| `00877C53` | Call `00923BE0`; the fraction is returned in ST0. |
| `00877C58` | `FMUL qword [00D0DEE0]`; bytes `0000000000007040` are double 256. |
| `00877C5E` | Call `00BF7420`. |
| `00877C63..00877C69` | Signed `TEST EAX,EAX / JGE`; negative low EAX becomes zero. |
| `00877C6B..00877C72` | Signed `CMP EAX,255 / JLE`; larger positive low EAX becomes 255. |
| `00877C77..00877C84` | Compare against `[ESI+374h]`, skip unchanged, otherwise begin storing/sending. These side effects are outside the numeric fragment. |

`00BF7420` reads the actual dword at `0109EEA4` on every conversion. Nonzero
selects `FSTP double / CVTTSD2SI EAX`; zero jumps to the existing complete
`00BF7456` x87 implementation. That fallback performs its native qword integer
store, residual adjustment and special-case stack pops; only low EAX reaches
the byte clamp. The image's stored zero at `0109EEA4` does not establish the
live runtime value. The new entry does not use that stored zero as a policy.

Both shared helper bodies in `src/native_render_batch_keys.cpp` were checked
against live disassembly and reused without changes. The numeric fragment
contains no ordinary C++ float-to-int cast and no duplicate CRT implementation.

## API, timing and floating-point domain

The caller must supply the actual readable mutable conversion-state address.
The numeric entry passes that pointer in ECX after the multiply; the shared
helper dereferences it at conversion time. There is no cached mode value,
null fallback or replacement global. Pointer lifetime and synchronization are
the caller's responsibility. Probe fixture words are explicitly test inputs,
not production bindings to `0109EEA4`.

The new cdecl entry takes a float at `[ESP+4]` and state pointer at `[ESP+8]`,
returns the byte as an int in EAX, and uses caller stack cleanup. Its
initial `FLD` adapts the binary32 argument to the native incoming ST0 contract.
It does not accept arbitrary extended-precision ST0 values or call the getter.
The normal getter already spills/reloads binary32; its provider/cache/release
semantics remain a separate integration boundary.

The entry and reused converter leave ambient x87 CW and MXCSR controls
unchanged. With at least two free x87 slots they consume only their temporary
values and preserve earlier live stack entries. Native arithmetic/conversion
instructions retain their status and exception behavior; the float adapter
adds its own load boundary. Returning results and masked exception flag bits
were checked under PC 24/53/64 and all four x87 rounding modes, with matching
MXCSR rounding, all exceptions masked, and DAZ/FTZ disabled. Unmasked faults,
resumption, arbitrary pre-existing status, other MXCSR settings and a live game
binding were not exercised. No absolute instruction timing or complete native
setter ABI compatibility is claimed.

## Focused validation

The existing health probe was extended locally, with no new tracked test suite.
The MSVC Win32 `/O2 /W4 /WX /fp:strict` build links the actual production
`src/unit_damage.cpp` and shared converter source, with `/MANIFEST:EMBED`.
The reference transcribes the original multiply and signed clamp instructions
and calls the same already-recovered CRT helper. It is a hardware/listing
fixture comparison, not execution of the original game function.

All 624 comparisons passed: 26 binary32 inputs, 12 PC/RC combinations and both
runtime modes. Inputs cover signed zero/subnormal values, both byte transition
boundaries, finite signed-integer overflow, infinities and signed quiet/signaling
NaNs. For every comparison, reference and production calls preserved two live
x87 sentinels, TOP and control settings; their masked x87/MXCSR exception bits
matched. Another 48 calls changed the same volatile word through zero, one,
`80000000h`, then zero, confirming selection is refreshed and any nonzero word
selects the SSE path.

Two finite witnesses demonstrate why a cast or fixed conversion mode is wrong:

| Fraction bits | Scaled value | Zero mode low EAX / byte | Nonzero mode low EAX / byte |
| --- | --- | --- | --- |
| `4B800001` (16777218) | 4294967808 | `00000200` / 255 | `80000000` / 0 |
| `CB000001` (-8388609) | -2147483904 | `7FFFFF00` / 255 | `80000000` / 0 |

These are converter-domain witnesses; the ordinary capped unit getter does not
normally produce these finite fractions. Masked NaN/infinity inputs reach
different raw conversion results but clamp to zero in both modes.

Existing checks also remain clean: 336 setter-prefix comparisons, 672 session
mode checks, 240 prior byte checks, 336 setter state checks, 192 checks each for
fraction/getter/released return, and 576 numeric state checks. Generated assembly
confirms the dword load, qword FMUL, shared helper call, signed branches and
absence of control-state writes in the new entry. Source comparison confirms
all previously existing source bodies are unchanged.

Artifacts and hashes are recorded in
`reports/unit_health_replication_byte_cc11.json`; rerun the focused fixture with
`local/cc11_health_replication_byte_check.cmd` in the worker worktree. The primary
integrator owns the full CMake build, metadata/Ghidra annotations and merge.
This packet is reconstructed and fixture-tested; game integration, native ABI
replacement and live gameplay validation remain unestablished.
