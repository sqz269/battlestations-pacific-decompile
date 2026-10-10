# Particle profile admitted constructor

This packet adds the `GeneratedModelLifetimeRuntime::BindingAdmission&&`
overload to `NativeParticleModelReference`. It implements only that Source
constructor path. It does not publish a registry entry, create a persistent
application record, implement a construction callee, or activate particle models.
The stable registry-entry and record work identified by the accepted admission
design remains held.

The C++ change is six header lines and twelve source lines: one public overload,
one private shared constructor, delegation from the ordinary constructor, and a
conditional call to the existing admitted `bind`. It follows the existing
`NativeCameraReference` pattern. There are no field, state, domain, Native body,
pool, scene identity, or `noexcept` changes.

The shared constructor still borrows the existing owner's native `+04` atomic
through the same `RenderCommandReference` constructor. It preserves the same
owner, access, actual lifetime runtime, disposal callback, phase initialization,
live-owner check, actual pool/table/disposal checks, `D5DA50` check, positive-count
check, and virtual-slot checks. Those checks run after Native construction and
before either binding path. No count check moves into storage preparation; no
Native count is initialized or retained by the new path.

The ordinary overload passes a null admission pointer and still calls ordinary
`runtime_.bind(*this)`. The new overload passes the address of the caller's exact
rvalue-reference token and calls `runtime_.bind(*this, std::move(*admission))`.
This is forwarding to the existing consuming overload, not construction of
another credit. On success that overload decrements the same runtime's pending
count and clears the original token. The same four callbacks are then installed
on the existing owner's scene attachment; its context and native bytes remain
unchanged. No extra ownership is transferred by this constructor.

Inactive, foreign-runtime, or attachment-kind tokens still fail the existing
validation. Scene/hierarchy mismatch, duplicate lifetime identity, absent or
different canonical scene attachment, and duplicate actual key retain their
existing failure behavior. Failure before the consuming append leaves the credit
active. Both public constructors remain potentially throwing. There is no
automatic fallback to ordinary binding, new rollback, retry, or swallowed error.
This does not make the future `noexcept` profile hook universally non-failing.

The allocation claim is limited to the existing valid, serialized admission
contract: reserve the lifetime credit in the same runtime before the intended
Native construction boundary, preserve all outstanding credits, and use stable,
nonallocating identity getters. The admitted append uses existing capacity.
Validation failures can throw and allocate an exception; invalid or concurrently
violated contracts receive no allocation guarantee. Registry insertion and
persistent record admission are outside this change.

Successful and failed-constructor retirement are unchanged. Final zero still
performs real derived/base destruction and actual pool return before runtime
unbind and disposal. Failed construction still requires the real base unwind to
have completed, then unbinds host bookkeeping and invokes disposal without
returning the raw slot. The outer raw-slot caller retains its existing single
return responsibility. This overload adds no cleanup call or new lifetime state.

The worker baseline is `637e162800c638995f0ab1c8e891a1b21edaef92`.
The captured after-Source is actual staged Git tree
`26a57f194b0535bd990d1b4fd64ef38f2cd617cb`; the final commit must preserve its
six Source root entries (`src`, `include`, `cmake`, `scripts`, `tests`, and
`CMakeLists.txt`). Separate live Git checks read all 4,224 baseline/current Source
paths and all 4,224 after Git blobs. The complete before/after Source comparison
allows exactly the two C++ paths above and reproduces their exact intended edit.

Both historical Source3352 manifests and all 3,352 project preimages per manifest
are copied. Before this edit, the typed-view epoch differs only in the two accepted
wide-string paths, while the wide raw-context epoch matches. After this edit,
the differences are respectively those four paths and the two particle-lifetime
paths. Three historical report-metadata inputs lie outside the 4,224 Source
paths; they are copied whole, re-read unchanged after the builds, and separately
verified against actual baseline Git. The historical manifests' four build
declarations are not materialized or granted fresh build credit.

The observed later main `71e9a0e5ddaf783d0b668917b0e33350b12fe42d` additionally
contains the locale-node header/TU and startup entry. That is a separate Source
epoch, not part of this frozen worker build. This packet does not claim that its
artifact counts or hashes describe later main, Root's build, or other workers.

The worker's existing `local/seed_reference.hpp` already matched the authorized
Root verified eight-math-seed prerequisite: 2,995 bytes, SHA-256
`26cdde71970ec8524c92d5e54e17c5587e817198ef16f3b6f2b8be25cd556d2c`.
Both source and used bytes are copied and pinned; the header was not rewritten.
No fresh Ghidra query or mutation was performed.

A fresh baseline build was necessary because the previous worker build preceded
the two wide-string changes. Normal `scripts/build.ps1` ran before the edit at
`2026-10-10T19:26:38.223420Z`–`19:26:57.328293Z`, and after the edit at
`19:27:53.468843Z`–`19:28:13.146799Z`. Both MSVC Win32 builds passed all three
existing checks: `reconstructed_math`, `native_math_differential`, and
`tool_tests`. Both logs retain the existing duplicate-definition LNK4006 warning.
No tests, target probes, model fixture, game launch, or new Native execution were
added; the standard existing math-seed check is the only Native differential
execution credited here.

Each build capture includes the whole Core archive, all 80 App objects selected
by the actual project and compiler tlogs, 56 complete compiler tlogs, and seven
relevant complete COFF objects. The three actual consumers of the lifetime header
are `native_particle_array_resize.cpp`, `native_particle_model_construction.cpp`,
and `native_particle_model_lifetime.cpp`. The other selected TUs are
`generated_model_lifetime.cpp`, `native_camera_reference.cpp`,
`scene_attachment.cpp`, and `native_model_owner.cpp`. Their actual tracked
dependency closure contains 106 paths. The 243 external compiler/SDK paths are
enumerated, not supplied as historical compiler-input preimages.

The complete archive walk has 1,991 entries, including 1,988 COFF members, in each
build. Every Core member and every App object is parsed for complete named code
sections and relocations. There are 174,273 such sections before and 174,276 after:
174,271 preserve exact bytes and relocations. The ordinary public constructor is
the one changed named section. Its old 53-byte unwind funclet moves to the private
constructor with exact instruction bytes and only the associated constructor
symbol renamed in its relocation. Four sections are added: the shared constructor,
the admitted constructor, that moved funclet, and an eight-byte `std::move` helper.
No other code section changes. All 80 App objects preserve their whole bytes.

The seven complete selected object indices have 1,967 sections before and 1,970
after. Their seven declared-storage sections with zero raw pointers retain empty
file payloads. Twenty complete selected functions before (4,825 bytes, 1,588
instructions) and twenty-two after (4,948 bytes, 1,633 instructions) are decoded.
All nineteen other previously decoded functions preserve exact bytes
and relocations, including `AF74A0`, `AF6C50`, `AF7F40`, both retirement paths,
ordinary/admitted binding, validation, reservation, lookup, and camera constructors.

| Compiled Source function | Before | After |
| --- | ---: | ---: |
| Ordinary particle-reference constructor | 363 bytes / 98 instructions | 51 / 19 |
| Admission overload | absent | 52 / 19 |
| Shared private constructor | absent | 383 / 105 |
| Existing admitted lifetime bind | 188 / 69 | exact |
| Existing pointer-vector `push_back` | 234 bytes | exact |

The ordinary wrapper pushes null at `+0x05` and relocates to the shared constructor
at `+0x19`; the admitted wrapper forwards the supplied token pointer at `+0x05`
and relocates at `+0x1A`. The shared compiled body checks the live C++ owner phase
at `+0x81`, the native `D5DA50` word at `+0xAB`, the borrowed count at `+0xBA`,
and the existing slots at `+0xDF`/`+0xFE`. Its admission branch is at `+0x10F`.
Relocations `+0x114` and `+0x11C` select admitted and ordinary binding; all four
callback installations follow at `+0x126`, `+0x132`, `+0x13C`, and `+0x146`.
These are offsets in reconstructed Source functions, not Original ABI claims.

The complete admitted bind validates the token, identities and canonical key,
calls pointer-vector `push_back` at relocation `+0x79`, then decrements the pending
count at `+0x80` and clears the token at `+0x85`. The complete `push_back` compares
end/capacity and branches to growth at `+0x15`. Its reserved path `+0x17..+0x29`
only stores the pointer, advances end and returns; it has no call or relocation.
The retained growth branch still has its allocation relocation at `+0x75`.
The supplied active credit proves why the reserved path is taken under the
existing serialized contract; allocation has not been removed from the generic
container function.

The complete Core/App direct-code-relocation census has no call to either public
particle-reference constructor. The only direct constructor calls are the two
wrappers calling their private helper in the same object. This adds a callable
Source API without claiming an application caller. The linked executable's
3,992,064-byte `.text` remains exact; whole EXE and map hashes change, and the map's
only changed line is its timestamp. `.rdata` differs and is retained whole;
whole-image identity is not claimed.

The finite copied proof is under
`local/cc12_particle_profile_admitted_constructor/`. Its standalone entrypoint
recomputes Git-object chains, Source qualifications and queries, compiler cohorts,
the full Core/App code indices, complete selected COFF sections/decodes, all
comparisons, and the admitted-path checks from the ZIP. After Python/Capstone and
codecs load, its audit guard permits file opens only for that ZIP and blocks
external opens, writes, process/network actions, library loads and directory
enumeration. Capture/build scripts are retained as provenance bytes, not executed
by replay. No recursive Root proof tree or live Git is needed by replay.

Both the in-place replay and a physical copy containing only ZIP and entrypoint
passed with identical results. Separate actual-Git/current-file verification is
recorded independently; the portable replay does not pretend to perform it.

- ZIP: `evidence.zip`, 88866998 bytes, SHA-256 `0622379986c0a71636b0f96ebd5df40f7f6d6c8138157986988814ff40490a2e`.
- Entrypoint: `portable_replay.py`, 33561 bytes, SHA-256 `11b9dc03d48d5c8ded0fc832c50da4b61f3355ec9cac96f811a4c7a89a1fec1e`.
- Payload members: 4471; uniquely checked entries: 4470.
- Machine-readable evidence: `reports/cc12_particle_profile_admitted_constructor.json`.

Run from any directory with Python and Capstone using the copied entrypoint and
ZIP paths: `python portable_replay.py evidence.zip`. The evidence establishes a
built, statically checked Source overload. It does not establish Original ABI
compatibility, fixture execution of this constructor, application integration,
or game-valid particle construction and retirement.
