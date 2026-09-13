# Orchestrator 6 reconstruction batch T

Addresses: 007C6E90, 008455A0, 0080DFC0, 00952050, 0092589D, 009258AC, 00924180, 009248D0, 00923660, 009256A0, 009256D0.

T connects actual mission unit observer prefixes to the application runtime, adds four exact callback providers and reconstructs the pending entity lock singleton. Names remain hypotheses. Native constructor/destructor projections and source composition are distinguished in each component report.

| Change | Evidence |
| --- | --- |
| [Callback providers](NATIVE_UNIT_OBSERVER_CALLBACKS.md): plane, shipyard and two distinct RET4 noops | 12 native/source pairs, 34,384 output bytes and 39,072 input bytes per side; zero differences |
| [Unit lifetime](GAME_UNIT_OBSERVER_BINDING.md): stable owner prefixes, guarded aliases, borrower release | Actual source-host edge fixture clears surviving endpoint arrays; combined run tears down all 77 created units |
| [Pending lock](NATIVE_PENDING_ENTITY_LOCK.md): actual raw manager and F899E8 publication | Five relocated native entries, canonical providers, 516 verified native span bytes |
| [Process deletion map](PENDING_ENTITY_LOCK_BINDING.md) and [mission wiring](GAME_OBSERVER_MISSION_BINDING.md) | Actual lock registration/reuse/deletion probe and unit teardown before singleton drain |

MSVC Win32 Release and both existing CTests passed at `9fa6cfb89f582f989f5a29a729b745e657bc0928`. Executable SHA256: `639a8a9947a800875a804f960eac40160c00e04ab6f91ea17e701ebefb9b77a9`. The 120-frame USN01 run produced 18,557 finite trajectory rows, 241 unchanged Airfield2 samples, 2,400 avoidance queries, 1,080 cruise reads, 10,080 generic ticks and 420 valid world registration nodes. All 77 created units ran callback-before-observed endpoint teardown while the dispatch owner was live, before the raw singleton manager drained.

The pending process owners remained empty through 240 passes. The separate compiled-provider lock probe establishes its actual process publication and finite-map drain; this does not establish pending-lock use or nonempty queue delivery during gameplay. Callback source providers have native-byte fixture coverage, but actual gameplay callback routing remains separate. Complete native constructors, repeated mission loading, original exception/concurrency behavior, binary compatibility and visual/gameplay parity remain unproved.

The evidence archive retains 350 hash-verified worker artifacts, plus 43 root composition-probe artifacts. Twelve reviewed names/comments were saved and exports refreshed. A transient SQLite index lock was resolved by refreshing after export. The pending lock worker's deviation from its read-only brief is recorded with exact prior annotation values and reviewed tool-lock evidence; no body, signature, instruction or data mutation occurred in that annotation batch.

## Follow-up packets

U advances destroy/kill producers, cancellation, and scene lifecycle notification independently. Library helpers remain required contracts unless complete bytes establish reuse of an existing canonical provider.
