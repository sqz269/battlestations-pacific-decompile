# Debug and game-resource factory profiles in the raw drain

Addresses: 00bd0400, 00be9600, 00716520, 00716560, 0051f460, 007175d0, 00d68b94, 00cfd84c.

This integration extends the existing finite deletion map used by the actual
14h singleton manager. It does not add another reconstructed original body or
change the recovered BD0400 pop-before-delete/recount loop. The six newly
reconstructed BB bodies are recorded separately in `NATIVE_GAME_RESOURCE_FACTORY.md`
(five, 347 bytes) and `NATIVE_STRING_VECTOR_LOOKUP.md` (one, 118 bytes).

| Registered profile | Exact popped address | Source deletion entry |
| --- | --- | --- |
| D68B94 | Unadjusted 34h debug owner | 00BE9600, using its borrowed construction context |
| CFD84C | Eight-byte game-resource factory +4 | 00716520, which subtracts four before 00716560 |

`NativeSingletonDeletionBindings` appends two context pointers and grows from
64 to 72 Win32 bytes. Existing observer, input-settings and other bindings retain
their order and behavior. The contexts must outlive the drain and must borrow the
same actual raw manager, publication cells and string pool as construction. A
missing context still reaches the existing explicit unsupported-profile error.
Neither CFD850 nor transient CFD7F8 is admitted as a registered owner profile.

The popped owner drives deletion even when its current publication points
elsewhere. Debug deletion releases member strings before its base clears the
current debug publication. Factory secondary deletion adjusts only in 00716520;
the dispatcher must not subtract four itself. Its primary deletion clears current
E19B90 unconditionally and frees the captured primary allocation for flag bit 0.
The separate WinMain F8D31C alias is outside that context.

## Evidence and validation

The source review checks both map branches against the recovered owner bodies,
native profiles and original/live PE byte captures. The factory's original
normal-path comparison uses shared test-only services and is distinct from the
actual raw drain exercise described here. Original ABI details remain in the
owner reports: these new context-bearing C++ interfaces are not binary drop-ins.
BD0400 originally consumes ECX and returns with RET; the existing source drain
also receives explicit borrowed bindings.

The development strict MSVC Win32 build and both existing CTests passed. One
source-only drain case then passed with actual manager, owners, string pool and
CRT frees, including five populated strings. Its observation wrapper forwards
allocation and release arguments unchanged to `ActualNativeStringPoolStorage`.
The initial slots are `[debug, pool, factory+4]`. Draining deletes the factory,
then the pool, then pops debug before its first string return recreates/registers
a pool. Cleanup releases sizes `[10,9,6,8,7]` and the recount loop drains the new
pool before destroying the manager's vector and section.

The pre-review successful development attempt is retained as
`local/native-bb-raw-drain-attempts/attempt02`; it used a two-file uncommitted
source overlay. An earlier attempt compiled but stopped during dependency
collection before execution; it is retained as `attempt01`. Neither is an
exact-commit acceptance receipt. Review added direct assertions for recreation
on release one and the new sole slot's identity, plus library and project-file
provenance checks. These stronger inputs will be replayed in a fresh exact-commit
gate, without rewriting the development outcomes.

The local alias assertion only preserves caller-held pointer bits. It does not
exercise the actual process F8D31C cell. The case also checks unconditional current
publication clearing, untouched replacement sentinels, manager ownership remaining
with its caller, and empty manager state after drain. It does not independently
instrument frees, execute original FH3 or validate gameplay. Production source
freshness requires the enclosing strict build, beyond object/archive equality.

## Application binding remains separate

At this integration's review, `game_hosts.cpp` and `game_hosts_singletons.hpp/.cpp`
are leased by another orchestrator. The prepared three-file process plan remains
unapplied. That packet must provide actual E19B90 storage, retain a separate
F8D31C alias, and bind the factory context before registration. It must keep the
context alive through normal and exceptional cleanup. The native factory Create,
resource-manager/cache and parser dependencies remain independently incomplete.
This change establishes two actual raw-drain profiles, not successful application
startup, resource creation, binary ABI compatibility or a runnable game rebuild.
