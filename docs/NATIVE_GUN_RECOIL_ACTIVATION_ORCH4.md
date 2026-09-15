# Native device-class recoil loop and conditional model load

Owned address: `00730CB0..00730D4A` (155 bytes).
Name: `BSP_GunClass_RunRecoilLoopAndLoadModelResource` is a descriptive
hypothesis, not a recovered symbol. Source entry:
`finalise_native_gun_class_00730cb0` in `native_gun_recoil_activation.cpp`.

## Source and integration boundary

The complete ordinary body is represented. An MSVC Win32 naked helper
transcribes every SSE/x87 instruction and branch; the C++ tail reads current
class DWORD `+38h`, calls the existing full
`load_native_damageable_class_model_00879590(class, 0, context, acquired)`
when nonzero, and returns byte 1. No new virtual host or replacement loader
was introduced. The helper is private to this translation unit.

| Routine | Original ABI | Source coverage |
| --- | --- | --- |
| `00730CB0..00730D4A` | ECX actual device class; no stacked arguments; AL=1; plain RET | Complete ordinary body, including post-RET cleanup branch |
| `00879590` (required existing source) | ECX class; stacked enemy flag, low byte significant; RET 4 | Existing canonical model-resource implementation, unchanged |

The caller borrows the existing `NativeDamageableClassModelContext` and owns
its `NativeDamageableClassModelAcquired` frame. If `+38h` is zero this routine
never touches the frame. Otherwise the frame must be fresh and associated
with that context; normal completion/failure and retained nested-frame rules
remain those of the existing model implementation. This routine neither
replays the frame nor translates exceptions. A model load may publish `+50h`;
the new body itself writes no class field, including activation byte `+44h`.

This closes the source body for device vtable slot `+14h`. It does not close
factory `00443090`, wrapper `00443490`, slot `+10h` body `00731A50`, device
Lua readers, bullet resolution, or a complete native vehicle activation path.
The source interface is not an original-register ABI/FH3/SEH bridge.

## Evidence and exact floating behavior

Verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The parent repaired the original missing function
definition; this worker only queried/exported Ghidra. The complete repaired
listing was reviewed. Body `00730D47..00730D4A` is reached by the branch from
`00730CEE`, despite appearing after the RET at `00730D46`.

`007327B0` produces the accessed fields: `+A8h=BackSpeedStart`,
`+ACh=BackSpeedFact`, `+B0h=DistMax`. The source does not invent a separate
profile layout. These are raw accesses to the caller's actual class storage.

1. `MOVSS` loads `+B0h`; `UCOMISS` compares with float positive zero. The
   original `LAHF; TEST AH,44h; JNP` sequence bypasses the x87 body for ordered
   equality, including negative zero. Unordered input reaches the x87 path
   unless its preceding unmasked SSE exception interrupts execution.
2. The initial path loads `+A8h`, stores/reloads it as float, adds double
   positive zero, stores/reloads the sum as float, then compares the resident
   distance against the sum using `FCOMI`. `JA` takes the distinct cleanup tail.
3. Otherwise `+ACh` is loaded and held on x87. The multiplication/addition
   loop retains the exact `FXCH`, `FLD`, `FMULP`, `FADDP` and float-store order.
   Its `JBE` includes unordered values. No iteration cap, finite-value gate or
   NaN replacement is added; the original can fail to terminate for some inputs.
4. Both normal and initial-exit paths pop exactly the native temporary stack
   values before the integer gate. The implementation does not explicitly
   reset the x87 unit or write its control word/MXCSR; native instructions
   retain their natural status effects.

The two constants are local copies of verified bits, not reads through native
addresses: `00D7A218` is `00000000` float +0 and `00D7A258` is
`0000000000000000` double +0. Integer backing storage preserves the original
four/eight-byte load widths. The separate helper preserves the instruction
schedule, not original code addresses, private ESP addresses or unused register
bits. The public byte result represents the original AL contract only.

## Call and caller audit

| Containing function | Call site | Native | Coverage |
| --- | --- | --- | --- |
| `00730CB0` | `00730D3C` | `00879590` | Complete existing source; actual class and stacked zero |

Live xrefs to `00730CB0` are eight data references at vtable cells
`00CE4548`, `00CE4574`, `00CE45B0`, `00CE45F4`, `00CE4628`, `00CE465C`,
`00CE4690`, `00CE46D4`. These are `+14h` in the base and seven derived
device-class tables. The audited `00443490` invokes the current slot with ECX
set to the resolved class and no pushed arguments. It reloads the vtable after
the preceding slot `+10h` call. No per-type argument is being discarded.
The only two direct callers of `00879590` are this routine and `00879AA0`;
the canonical callee continues to receive the explicit enemy DWORD, zero here.

## Focused validation

The ignored fixture is `local/gun_recoil_c3/probe.cpp`, built against this
worktree's actual Release `bsp_core.lib`, Lua and zlib libraries. Its original
155-byte body and both constants are extracted from the installed PE and
compared with live Ghidra bytes. Only the two constant operands and the one
relative call are relocated. The call adapter invokes the same genuine
`00879590` source; it does not return a fabricated successful load.

The paired fixture exercises zero/negative-zero gates, the forward cleanup
tail, an equality that repeats the loop, inexact arithmetic at x87 precision
24/53/64 with different rounding controls, unmasked SSE signaling-invalid,
and unmasked x87 unordered-invalid. Successful cases compare return byte,
x87 status/control, MXCSR, one preserved preexisting x87 sentinel and the
whole unchanged class image. It also checks the existing acquired frame is
fresh when the model call is skipped and complete when that call is reached.

The reached model path uses a nonzero name length and an already populated
model pointer. The canonical callee reads that pointer only to take its
existing-model guard. Unused fixture services abort if invoked. This proves
conditional composition, not fresh resource loading, native class publication,
parsing, or model binding; those services retain their existing evidence.

Hardware exceptions are caught only by the fixture, which compares the
exception code and selected saved floating context. This does not establish
original SEH/FH3 identity, exception instruction/stack address identity, or
recovery equivalence. Masked unordered nontermination is preserved by the
literal branch schedule but is not dynamically exercised. No game or
executable reachability result is claimed.

Validation passed: strict Release MSVC Win32 build, both existing CTests
(`reconstructed_math`, `tool_tests`), the sole live direct-call row, and all
nine original/source fixture pairs. Inexact cases produced matching x87
precision flags, and both invalid-operation cases produced `C0000090` in
both executions. The report retains outcomes and input hashes. No permanent
test cases were added.
