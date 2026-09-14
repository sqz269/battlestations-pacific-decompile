# Raw world configuration leaves

These three complete entries copy the caller's public argument into actual
receiver storage. Their descriptive names are caller-based hypotheses.

| Entry | Inclusive body | Effect |
| --- | --- | --- |
| `00B6FE50` | `00B6FE50..00B6FE5C`, 13 bytes | Copy the full DWORD to receiver+190h |
| `00B1FFB0` | `00B1FFB0..00B1FFBC`, 13 bytes | Copy the full DWORD to receiver+1D84h |
| `00AD5750` | `00AD5750..00AD5759`, 10 bytes | Copy the low public-argument byte to receiver+10h |

Original ECX holds the receiver and entry ESP+4 holds the public DWORD; every
entry uses `RET4`. The two DWORD entries load EAX and leave it equal to the
supplied word. The byte entry loads AL only and preserves upper EAX bits. All
three preserve ECX, EDX, nonvolatile registers and flags. They perform exactly
one store of the stated width, with no other receiver-memory writes, calls,
allocation, normalization, validation, or local exception frame.

The source uses naked Win32 entries. An unused fastcall EDX parameter lets C++
callers supply the original ECX receiver and public stack word without a shim;
pass `nullptr` for that source parameter. The source declarations return void;
the exact observable EAX/AL effects remain in their instruction bodies. No
meaning beyond those bits is inferred for the original return convention.

All 25 live incoming calls were checked: 22 calls to B6FE50 in 12 functions,
two calls to B1FFB0, and one call to AD5750. Complete caller listings and capped
argument preparations are retained in `local/ds-caller-contexts.json` and its
pinned listing files. Inputs to the camera entry include zero, all ones and
packed words produced by caller-side operations; it is not a zero-only hook.
Some callers push their color word before calling a camera getter, then call
the leaf with the returned receiver; the original public stack load remains
in the leaf.

In `004DE610`, B6FE50 is called at `004DE891` and `004DF7B0` with the actual
camera at game+19FCh. B1FFB0 is called at `004DE8A8` after the caller computes
`2-current setting`; its other caller `008D5B80` performs the same subtraction
from a settings field. Neither subtraction belongs to the store leaf. The
AD5750 caller at `004DE9B2` zero-extends scene-record+A90h, reloads the actual
owner from `00F8C210`, and pushes that word. The leaf preserves any low byte;
it does not convert it to `bool`.

Caller-supplied backing and lifetime must make the actual destination writable.
No host owner type, guessed constructor, callback, or substitute allocator is
introduced. Existing typed settings hooks and direct camera zero stores are
composition sites, not implementations of these general entries. Other world
configuration calculations and provider integration remain separate work.

The report pins original installed/live bytes, prior names/prototypes, incoming
call rows, source hashes, build results, and full generated-body comparisons.
No new tests or worker runtime fixture are added. Byte identity and compilation
do not prove ownership, validity of arbitrary receivers, caller behavior under
hardware faults, game integration, or gameplay. The original game and Ghidra
analysis are unchanged by this worker.

Primary validation at exact clean source `dae36fd24b77c9fa0f7d26ad64f26265682297a4` passed the MSVC Win32 Release build and both existing CTests with 2623 unchanged tracked build inputs. All three complete generated code sections match every original byte (13/13/10 bytes), with zero relocations. One ignored local differential case executed the source AD5750 entry and its unmodified original ten bytes on separate writable fixture buffers: public word DEADC0A5 and incoming EAX12345678 produced byte A5 and EAX123456A5, with the other 63 bytes of each 64-byte backing buffer unchanged. The DWORD leaves were not executed in this fixture. No new repository tests were added. Fixture backing is not a constructed game owner; world-builder integration and gameplay remain unvalidated.
