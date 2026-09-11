# Plain gameplay component terminals

Addresses: 0086B7E0, 0086D0A0, 0086D0E0, 0086D100, 0086D120, 0086D140,
0086D160. Packet `orch3_plain_component_terminals_ae`.

The existing raw component destructors are now callable with the only dependency
the six plain classes actually consume: the application's `NativeStringStorage`.
Existing `EffectScalarComponentContext` overloads forward to the same bodies;
their readers and Sound/Waterdrops dispatch retain their existing dependencies.
This adds no substitute sound, texture or renderer service.

`GameplayEffectPlainComponentLifetime` binds definition-array zero callbacks to
the current primary table words. For bound classes it checks slot0 is the actual
BD30E0 generic destructor, reloads the current component table as BD30E0 does,
and dispatches current slot4 with flags1. It does not allocate a component, keep
a registry, manufacture a reference counter, retain, decrement again, or inspect
a freed object. Unbound classes go to the required remaining lifetime; a bound
table changed to an unsupported callback fails explicitly. Borrowed table
bindings and the actual string storage must outlive their component owners.

| Class | Native table | Scalar destructor | Native extent |
|---|---|---|---|
| Shake | D0D5F4 | 86D0A0 | 2Ch |
| ConstRumble | D0D76C | 86D0E0 | 30h |
| SlopeRumble | D0D78C | 86D100 | 30h |
| SquareRumble | D0D7AC | 86D120 | 40h |
| Light | D0D654 | 86D140 | 48h |
| Splash | D0D674 | 86D160 | 2Ch |

Each scalar wrapper is exactly30 bytes, including RET4. ECX is the component;
one stack argument carries flags; EAX returns the original pointer even after
free. All call86B7E0, test flags bit0, optionally call BF65AC, and return. Neither
the wrapper nor BD30E0 decrements the count. Flags2 therefore do not free.

The complete103-byte common destructor86B7E0..86B846 writes D0D570, captures
the existing name pointer at+C and length at+8, and returns its block through
419CC0/BD1510 when nonnull. It leaves the name header and callback modifications
untouched. BD30F0 then changes only the primary word toCEB130. No reference
count, scalar payload, padding, borrowed owner or event request is reset.
The new overloads reuse this same established implementation.

Live FuncInfo DC7510 selects the one-state map DC7508: state0 -> -1 via
C95530, loading the saved owner at EBP-10 and jumping BD30F0. The original
exception dispatcher was not executed. `NativeStringStorage::release` remains
the established nonthrowing interface: failure while lazily recreating a pool
terminates within that bridge. This packet verifies normal destruction and
does not claim original exception/SEH parity or broaden that interface.

## Validation

The installed executable SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`
and the live `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` bytes match
for every complete body and supporting table/map span. Current Ghidra bodies
already include all scalar tails; no flow repair or prototype change was needed.
Compiler-generated scalar labels are preserved, with new evidence appended.

Strict Win32 build and both existing CTests pass. One ignored focused probe
executes the original103-byte destructor, six30-byte scalar wrappers and14-byte
BD30E0 generic zero body, using the actual shared native string pool. It checks
ESP balance, full owner images with only dangling name addresses normalized,
null and nonempty names, callback changes to the name header and count,
flags0/2 preservation and flags1 free. Current-table mutation checks prove the
binding does not silently dispatch a stale scalar implementation. Original
zero dispatch uses a fixture table pointing to the copied original code.

The existing AD point lifetime fixture now transfers all four constructor
references (Shake and three rumble components) to the actual definition array,
instead of retaining one fixture reference per component. Each named component
reaches count0 and is destroyed by its current terminal, in reverse array order
(SquareRumble, SlopeRumble, ConstRumble, Shake), while all three event owners
still exist. No manual component destructor/free remains. Actual definition
cleanup then completes; event-row cleanup frees the three20h event owners and
leaves their independent requests alive. The fixture's existing name-reentry,
node-pool return, manager/cache and constructor-failure checks also pass.

This is application composition and original-body fixture evidence. It does
not establish Light/Splash factory behavior, remaining component-family
terminals, native exception ABI, physical device output or gameplay validation.

## Follow-up packets

Continue binding remaining component factory owners and terminal classes under
their actual storage contracts. The broader scalar dispatcher still serves
Lua-loading paths; connect this plain lifetime to those application compositions
that already bind its six current table profiles. Event update/cancel consumers
and the native request object boundary remain separate follow-ups.
