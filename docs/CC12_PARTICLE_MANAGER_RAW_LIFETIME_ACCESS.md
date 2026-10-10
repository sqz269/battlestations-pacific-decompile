# Particle-manager raw lifetime access

`00AF06A0` and `00AF0740` now accept the application's actual `01090AA0`
publication cell through the existing borrowed `SoundLifetimeAccess`. The legacy
`SingletonLifetimeDomain` constructor remains supported. This bounded Source
change follows accepted readiness review `01e9b3434`, integrated at baseline
`8ee6d5c8a506762dfbac0ffd60a36cc512b77114`.

Only two C++ files changed: `include/bsp/native_particle_model_manager.hpp` and
`src/native_particle_model_manager.cpp`. The access object has two
three-argument `noexcept` constructors: publication/domain/one and
publication/raw-manager-cell/one. Both compiled constructors contain only
reference/member stores and return. They do not call, read through the borrowed
cells, allocate, fetch, publish or register anything.

Both bases use the existing `CapturedSoundLifetimeSection` to capture and enter
the **first** resolved manager's section. They then resolve a **distinct second**
manager into a saved `SoundLifetimeManagerView`; only after that getter returns
do they read the current volatile `F8C274` publication for register/unregister.
Construction publishes its receiver before the second getter. Destruction
clears only after unregister returns, releases the first captured section, and
then stamps the generic base profile. Existing ordinary C++ unwind/profile
behavior remains; no retry, rollback, alternate registry, null shortcut or
failure-time clear was added. The unused local `CapturedSection` was removed.

The actual `0x34` storage, weak arrays, derived and scalar Source bodies remain
unchanged. The C++ access context grows from 12 to 16 bytes. This changes emitted
code even where a historical Source body is unchanged: the complete 230-byte
`00AF0B10` compiled section has identical relocations and exactly one changed
byte, section `+0x6B`: displacement `08 -> 0C` for the access object's `one`
reference. This is not a change to the actual manager's layout or a binary ABI
compatibility claim.

Before mutation, the complete candidate sources, library/member, all 52 CL
read/write/command/item logs across 13 worker Release projects, and all three
actual recorded header-consumer objects were retained. The recorded consumers
are `native_particle_model_construction.cpp`, `native_particle_model_lifetime.cpp`
and `native_particle_model_manager.cpp`. Each object is qualified using
`Cl.items` plus its full read, write-group and command records. The incremental
write log also preserves an older broad group; that group alone does not prove
fresh compilation. The ordinary build log names all three newly compiled files.

The two other consumer objects retain identical code sections and relocation
identities after recompilation. The complete manager object changes from
14,073 to 14,202 bytes. The accepted Root member and this worker's old member
are retained separately, with distinct hashes and provenance. The new whole
worker object is byte-equal to its complete Core archive member:
`144a44c22e7dc30e3a66d242c4ad7e3f34d9aaf4fbe623cf763a417db741eb3e`.

`./scripts/build.ps1` passed the normal Release MSVC Win32 build and the two
existing CTest checks, `reconstructed_math` and `tool_tests`. No new tests or
fixtures were added. The build emitted the duplicate `spawn_request_id_matches`
LNK4006 warning; those Source files are unchanged by this packet.

The separate live Git check passed for all 4,214 complete indexed blobs and
current Source/build inputs. Corrected ZIP-only replay passed for 8,645 complete payloads,
both compiler cohorts, complete archive members/objects, all eleven preserved
Source function bodies, complete Native bases of 145/153 bytes, both compiled
access constructors and the full compiled base sections. The verifier reads
only the ZIP and decodes bytes using Capstone; it performs no Git/live-checkout
reads, compiler/executable launches, Ghidra calls or file writes. The invoking
shell retains its stdout/stderr separately.

Root's independent review found that the original generic COFF parser selected
file-header bytes for `.bss` when its raw-file pointer was zero. The corrected
parser retains the declared size separately and records an empty raw payload.
All six before/after consumer objects have a four-byte `.bss` declaration with
zero file-backed bytes; each now replays explicitly. The original verifier and
ZIP are preserved with separate hashes. This evidence-only correction changed
no C++ source or build artifact and required no build repeat. The report records
the corrected helper hash and both bundle provenances.

Copy `portable_replay.py` and `evidence.zip` from
`local/cc12_particle_manager_raw_lifetime_access/`, then run
`python portable_replay.py evidence.zip` with Capstone available.

No startup caller, Phase 8 marker, deletion dispatcher, shared helper, CMake
registration or Ghidra state was changed. There was no game/startup execution,
FMOD bypass, new SDK/audio/OS probe or original-executable read. Existing
retained Native evidence supplied the instruction spans. The accepted FMOD
environment stop still bounds runtime evidence. Candidate names remain absent
from the worker PE map; name absence alone is not universal code-identity proof.

Activation still needs canonical `F8C274` ownership/context lifetime, raw deletion
admission and the Native caller allocation/failure contract before locale/fonts.
Original register ABI, private FH3/SEH, hardware faults, startup continuation and
gameplay remain unvalidated.

[Complete report](../reports/cc12_particle_manager_raw_lifetime_access.json).
