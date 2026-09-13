# Orchestrator 6 reconstruction batch X

Addresses: 00B0D7B0, 00B4EC90, 008674C0, 00484620, 0054D510, 004BD150, 004C1570, 004B7ED0, 004C4860, 004C4890, 00C64D28, 00C64F93.

X supplies the normal renderer-listener and wreck-effect provider sequences required by W, plus the actual mission-entity lock owner/getter and its shutdown route. The interfaces borrow existing owners and retain unresolved native dependencies explicitly.

| Change | Evidence and limits |
| --- | --- |
| [Listener and root invalidation](NATIVE_LISTENER_RENDERER.md) | Complete 25/43-byte bodies, three original-byte/source cases, existing canonical root provider. Actual renderer bindings remain required. |
| [Wreck effect providers](NATIVE_WRECK_EFFECT_PROVIDERS.md) | Complete 50-byte scan and 64-byte exact reference-provider reuse, 16 native/source comparisons. Eight native dispatch bytes are substituted in the fixture; original destructors and 8673B0 remain required. |
| [Mission entity lock](NATIVE_MISSION_ENTITY_LOCK.md) | Five complete normal bodies, 365 code bytes; native/source owner/getter/destructor and callback-rebinding evidence, plus source-only failure cleanup. Original EH transport remains unproved. |
| [Mission lock shutdown](NATIVE_MISSION_ENTITY_LOCK_DOMAIN.md) | Actual process F878FC getter/raw-manager composition checks the CE7548 route, including replaced publication before drain. |
| [Volatile reference correction](VOLATILE_REFERENCE_ADOPTION.md) | Removes W's extra slot read from discarding a volatile reference result. Corrected overloads generate identical x86 bytes; volatile body remains unchanged. |

MSVC Win32 Release and both existing CTests passed at `d026dc1f5c9a4e120d8d38ba60f1045d7188f583`. Executable SHA256: `e60deb9311b9528ca3433385617cde3594dc940b874cab109c4d0f23792dbae1`. The 120-frame USN01 compatibility run produced 18,557 finite trajectory rows, 241 unchanged Airfield2 samples, 2,400 avoidance queries, 1,080 cruise reads, 10,080 generic ticks and 420 valid world nodes. All 77 actual unit observer prefixes were torn down before manager drain while the owner remained live.

Pending queues remained empty through 240 passes, so this run does not establish new killed/wreck or renderer gameplay execution. Native/source fixtures and the actual lock-domain composition provide their stated narrower evidence. Full wreck coverage, application routing, native ABI/exceptions/concurrency and visual/gameplay parity remain incomplete.

The archive retains 822 worker artifacts, including compiler/SDK inputs, and 540 root proof artifacts. Two missing mission-lock EH selectors were defined from verified bytes; formal annotations preserve prior values and affected exports were refreshed. Existing functions were not recreated. Unattributed call sites and continuation membership gaps remain recorded.

## Follow-up packets

Y continues complete 955420 scene initialization and 8673B0 per-effect cleanup independently. The holder+04 discriminator evidence also requires a narrow correction to the older scene-entity header comment.
