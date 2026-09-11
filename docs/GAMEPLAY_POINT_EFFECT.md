# Gameplay point-effect shake component

Packet `orch3_gameplay_point_adapter_aa` reconstructs the first concrete point-effect
component factory in `src/gameplay_point_effect.cpp`, together with its accumulator
and the base admission leaf. The full gameplay-definition-to-point-constructor
adapter remains outstanding. Names below are descriptive hypotheses.

Evidence comes from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified against the installed executable with SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
`reports/gameplay_point_effect.json` records byte hashes, validation and annotations.

| Native span, inclusive | Original ABI | Reconstructed behavior |
| --- | --- | --- |
| `0086B7D0..0086B7D4`, 5 bytes | ECX component, two unused stack arguments; AL=1, upper EAX unchanged; RET8 | Base component virtual+1C admits without reading any input. |
| `0086A820..0086A902`, 227 bytes | ECX component, stack point effect; EAX=0; RET4 | Shake virtual+18 applies an immediate distance-scaled side effect when positive. |
| `0042C3D0..0042C44B`, 124 bytes | ECX target, stack float; RET4; no meaningful result | Add to target+1C0, store the float sum, then clamp against minimum+1C8 before maximum+1C4. |

The actual shake table at `D0D5F4` contains `868DE0` at +14 (its Lua reader),
`86A820` at +18 (factory), and `86B7D0` at +1C (admission). The existing reader
identifies byte+20 as `Persistent`, float+24 as `Radius`, and float+28 as `Strength`.
The base predicate is a verified leaf for tables that select it. It is not a
success fallback for particle, sound or unknown current virtual implementations.

## Captured storage and current target

The factory first checks `Persistent`; any nonzero byte returns null without
loading the game, target or subject. A null current `[E188A8]+1ED4` target also
returns null before reading the subject. Otherwise it captures that target and
refreshes its pose through `414DB0` only if byte+C8 is zero. It then loads the
subject's current +110 node and calls `B6DB70` if its world-valid bit2 is absent.

It copies all three captured-node world-position floats (+120/+124/+128) using
x87 loads/stores before subtracting the captured target's +FC/+100/+104 position.
Each delta spills to float. The established `42B2F0` length routine supplies the
float distance. The factory computes `(1 - distance / Radius) * Strength` on x87,
with no intermediate float gain spill and no gain clamp; only the final strength
spills. The ordered `strength > 0` comparison rejects zero, negative and unordered
values. In particular, a negative gain times a negative component strength can
still apply a positive contribution.

Only on that positive path does the factory reload the CURRENT
`[E188A8]+1ED4` target and call `42C3D0` on it. The target used for distance and
the target receiving the write are deliberately separate loads. The native
factory returns null on every path, including a successful side effect. This
does not allocate, retain or transfer a component event owner.

`create_point_shake_0086a820` composes the existing `ForceEventSpatialHost`
contract. `ForceEventSpatialRuntime` supplies the concrete current-field lookups
and the actual `414DB0` algorithm through the existing `PoseRefreshResolver`.
The accumulator writes raw fields of the returned target's actual `identity`;
there is no copied target state, extra reference count, cached global or separate
amplitude owner. Required host lookups must return the actual live associations.

## Accumulator details

`42C3D0` writes the rounded float sum to +1C0 before loading the bounds. It
captures the sum again via x87 and captures minimum+1C8 via x87. If the ordered
minimum comparison wins, it writes that captured minimum and never reads maximum.
Otherwise it captures maximum+1C4 and chooses maximum only when the captured sum
is ordered above it. The last stores use MOVSS, retaining the captured float bits.
This ordering matters for inverted bounds, signed zeros, NaNs and the x87
environment. The implementation preserves the spill and comparison sequence
instead of substituting `std::clamp` or SSE arithmetic.

The interpretation of these target fields as shake accumulation follows the
verified caller and component reader. The other callers of `42C3D0` have not
been reconstructed by this packet. Native valid-span preconditions still apply,
including a nonnull second target on the positive path. No new recovery or
rollback is invented for invalid owners or hardware floating-point exceptions.

## Validation and remaining work

`scripts/build.ps1` passes with strict MSVC Win32 flags and both existing CTests.
One ignored differential probe, `local/gameplay_point_probe_aa.cpp`, executes
the complete 5/227/124-byte original bodies, explicitly checks native ESP balance,
and checks the base predicate's stale upper EAX bytes. All function bytes and the
32-byte shake table match the live program and installed file, including every
RET immediate.

The probe compares full target and node storage plus x87 exception/TOP bits for
11 selected accumulator inputs and 11 factory inputs across all 12 combinations
of 24/53/64-bit precision and four rounding modes. Inputs cover the bounds,
inverted limits, signed zero, NaNs, infinity, float-sum rounding, gain sign,
persistent/null-target short circuits and dirty pose/node refresh. It also injects
a fixture-only target and subject replacement after the real target refresh,
confirming distance from the captured target and accumulation into the new one.
This injection is not claimed to occur inside the pure application pose resolver.

The original factory's refresh/length CALLs are rebound to their existing
canonical C++ implementations. Its accumulator CALL enters the copied original
124-byte body; its two game-global operands address the actual fixture game slot.
The C++ run uses the same concrete spatial runtime and real refresh algorithms.
No permanent tests or new test framework were added.

This is reconstruction and fixture validation under masked FP exceptions, not
gameplay, concurrency, unmasked-exception or binary ABI replacement validation.
The actual application table dispatch, remaining component predicates/factories,
canonical gameplay-definition reference binding, and concrete `PointEffectConstruction`
adapter for `8689C0` still need integration. Existing definition raw storage and
the typed constructor's name/reference interfaces must be reconciled without
copying the actual name header or introducing a second intrusive count.
