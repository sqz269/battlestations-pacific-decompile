# Material effect reload recovery (CC10)

Addresses: `00B469A0..00B46A6E` (207 bytes), with native calls at
`00B469C8`, `00B469DF`, `00B469E9`, `00B469F0`, `00B469F7`,
`00B469FF`, `00B46A0A`, `00B46A11`, `00B46A1F`, `00B46A33`,
`00B46A53`, and `00B46A5A`. Source is recovered from historical
commit `769bdc3e3` and adapted to the current canonical owner APIs.

The native body tests the shared source-mode byte `0108D6F1`. A clear byte
returns without touching the effect. With it set, the body reads the current
`+B8` name, calls the diagnostic `004254B0` (a bare `RET`), releases derived
then base owners, rereads the name, and loads through `00B46950`. A false
result repeats both releases, constructs the 27-byte original literal at
`00D61C88` (`shaderfx/common/error.shfx`), reloads the **same** effect,
ignores that return value, and returns the temporary string to the same pool.
The one native FH3 fallback-name unwind has a source exception counterpart;
the source does not reproduce FH3 or the original ECX/RET ABI.

The reload context borrows `NativeStartupShaderModes` and verifies that the
program loader borrows the same variant byte. Production supplies this modes
object through `game_native_shader_process().modes()`, the same state used by
the source compiler and binary cache. The program context, retained owners,
string pool, effect storage, and original read-only literal service are reused
across both loads. The current `00B45EE0` producer allocates an actual 110h
descriptor and installs `NativeShaderDescriptorCallableBinding` on it before
publishing it at effect `+C4`. `00B41B10` dispatches that current callable
binding; the retained program frame must outlive its descriptor.

The historical implementation also supported an unbound numeric `D61A44`
descriptor through `NativeMaterialEffectDestructionAccess.actual_descriptor`
and a three-argument `00B41B10` overload. Those APIs are absent from current
main. This recovery checks the current descriptor word before `00B41B10` and
throws for numeric `D61A44`, so an original address is never invoked as a
host vtable. Recovering that domain requires a separate bounded packet:
restore a canonical descriptor terminal context from the current shader-state
list pool and read-only `D61A44`/`D621F4` profile slots, route numeric
`00B46930` and nested sampler `00B56FC0` teardown through it, and add the
corresponding `00B41B10` overload. No private resolver or owner registry is
introduced by this leaf.

The MSVC Win32 build and both existing CTests pass. A focused 32-bit probe
first calls the **current** `00B46950` loader, which acquires and binds its
descriptor. With source mode clear, reload preserves all 178h effect bytes.
A controlled descriptor reader then makes the next load return
false through a null program result; the fallback reads the original PE
literal and completes on the same effect. It observed four descriptor reads,
one attempted program build, old and failed bindings retired, and the final
binding live until explicit cleanup. A separate numeric-profile probe confirms
rejection before host table dispatch. These controlled children do not compile
real HLSL or establish renderer, game, or ABI parity. The read-only PE service
requires fixed low address space; one probe launch reported its band unavailable,
while the successful launch completed and exited zero.
