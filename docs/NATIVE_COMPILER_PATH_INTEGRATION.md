# Actual compiler inputs, reflection and special texture loading

Addresses: 00b35be0, 00b372d0, 00b34e20, 00b3aea0, 00b3a750, 00b5bc60,
00b5b830, 00b5b960, 00b2c2d0, 00b3f2d0, 00b5bf70, 00b5b9e0,
00b5bbc0, 00b5bed0, 00b5bd10, 00b5be10, 00b5df70, 00b5bb20,
00b5df00, 00b5ba80, 00b36800, 00b34aa0, 00b346e0

This integration advances actual shader-builder fields and compiled metadata,
and the cube/volume arms of the native texture loader. The modules operate on
the existing native storage and ownership domains. They do not complete the
material compiler continuation or demonstrate a running rebuilt game.

| Component | Accepted worker evidence | Integration |
| --- | --- | --- |
| System fields | `006672b7`, `NATIVE_SHADER_SYSTEM_FIELDS.md` | Six owned files imported in `5b449fee`; actual B0h builder and 1Ch fields |
| Reflection | `c23ab65c`, `NATIVE_COMPILED_SHADER_REFLECTION.md` | Reviewed merge `4900b5c3`; real D3DX table and existing 88h owner |
| Cube/volume loading | `6ba1b7cc`, `NATIVE_TEXTURE_LOADING_CACHE.md` | Eight owned files imported in `653cd6b1`; shared canonical resource companions |
| System constant registry | `c8e54592`, `NATIVE_SYSTEM_CONSTANT_REGISTRY.md` | Reviewed merge `82ab9e0a`; actual 52 records and shared singleton lifetime |
| Interpolators | `8ac49e42`, `NATIVE_SHADER_INTERPOLATORS.md` | Reviewed merge `1ce1c128`; actual field selection and two-byte mappings |

The system-field constructor clears the actual name header before copying and
publishes scalar fields only after that copy returns. Vertex and pixel appenders
each allocate sixteen independent fields and retain partially acquired storage
on host failures. Native interpolator selection and mapping execute between the
two appenders. Their accepted implementation reads descriptor70 then74, preserves
separate usage cursors across both, and appends to the existing mapping arrays.
These helpers still require explicit composition in the compiler continuation.

Reflection consumes the live registry publication and real D3DX constant table.
It writes system counts before register indices, appends unknown FLOAT records
to the actual owner, and writes the end register and sampler mask before releasing
the table. Its retained operation preserves the table, temporary names and actual
array acquisitions if a borrowed operation fails.

The actual registry constructor produces all 52 records in the existing 20h
layout. Its base publishes the live singleton before initializing the derived
array. Array growth, forward old-name release, reverse shrink and normal terminal
cleanup retain their native order. The same publication and lifetime domain now
feed an original/source reflection fixture; no projected registry is used in that
composition. The registry binding permits one unfinished operation, and retains
the exact current copied source if a borrowed callback fails.

Special texture loading retains the device captured before image inspection.
The retry arm obtains the current renderer for recovery and re-reads stream size
and data while reusing that captured device. Actual cube/volume constructors,
pools and source references join the existing canonical resource domain. Guard
exit precedes common stream release. Volume destruction notifies before releasing
its retained source; cube destruction has the independently documented order.

## Preserved validation

The first combined checkpoint is
`local/checkpoints/653cd6b1/compiler-path-temporary/validation.json`. Its 49
hash-verified artifacts preserve the exact temporary include, production sources,
combined library, build log and five focused fixtures. The production Win32 target
and both CTests passed. That checkpoint deliberately identifies its two temporary
source registrations and is not a default-build claim.

The five fixture programs cover installed-descriptor system fields, original/source
real-D3DX reflection (including the retained-failure child), actual physical-file
cube/volume loading, existing texture record storage, and existing resolution-failure
retention. Across the three component reports, 377 numeric call/transfer rows passed,
including four resolved indirect transfers; symbolic COM/virtual calls retain their
separate evidence limits.

All five components are now registered in the normal build. The final default
Win32 build at `82a2179e` and both CTests passed. Thirteen focused fixture
programs passed against the registered library. Their original run at
`a52a7891` preceded a names/docs/reports-only merge; all three
linked libraries and all pinned runner/fixture inputs remained byte-identical,
and the default build was rerun after that merge. The final five component reports
have 847 checked numeric transfer rows, including four resolved indirect rows,
with zero failures.

The immutable final checkpoint is `local/checkpoints/82a2179e/compiler-path-default/validation.json` (146 artifacts).
`reports/native_compiler_path_integration.json` records the exact code revision,
worker archives, Ghidra synchronization and validation limits. Local CMD runners
now propagate every nonzero compiler/linker result and check the environment setup;
archived original runners remain unchanged.

## Analysis synchronization and limits

The primary applied all 23 accepted names/evidence comments through the Ghidra
write lock, preserved previous annotations, saved the project, read back the
comments, and refreshed the affected exports. In particular, B3F2D0 is the actual
volume-pool allocation wrapper, not a static destructor.

These are new MSVC Win32 interfaces, not drop-in native entry points. Private FH3
unwinding is not reproduced. Failed operations retain acquired state and prohibit
unsafe retirement instead. Reflection accepts populated successful COM outputs
and in-range system semantics; native invalid-output/out-of-range behavior remains
outside its domain. The texture fixture does not execute recovery, enabled-guard
reentry, cache bootstrap or the complete original loader. Compiler-tail success,
full rendering parity and gameplay remain unverified.

## Native constant headers and vertex compilation

The next integrated batch adds eight actual constant-header/formatting routines
and three vertex-compilation/physical-write routines. The constant header uses
the same 52-row registry, actual builder output, shared scratch and pooled strings.
It preserves signed formatting, captured temporary releases, current count/limit
reads and DWORD cursor wrap. The original/source fixture compares exact text and
allocation traces, including retained failure state. Array-base replacement and
scratch mutation during allocation are assembly-reviewed, not exercised by that fixture.

Vertex compilation captures the actual device before D3DX9_40 compilation and
the VFS manager before name allocation. Physical `.vsa` writes and diagnostic
cleanup precede vertex-shader creation; code/message releases follow it. The real
HAL/VFS fixture compares shader function bytes and 158-byte diagnostics, then
checks code-null messages, retained allocation failure and physical count/carry.
The failure leg can truncate the final `.vsa`; the earlier successful comparison
is recorded in the pinned fixture log. Relocated original instructions, rebound
dependency calls/table entries and shared existing helpers remain explicit limits.

The final default build at `6fbdee9e` passed both CTests. All 15 focused fixture
programs passed at `44d9fdfd`; the intervening merge contains only
docs/reports. All three libraries and all 40 runner/fixture input hashes stayed
unchanged, and the default build was rerun after that merge. Seven component
reports have 936 checked numeric transfer rows and zero failures, including the
same four resolved indirect rows. The 11 new names/comments were applied under
the Ghidra write lock, saved, read back and exported with prior values preserved.

The immutable checkpoint is `local/checkpoints/6fbdee9e/compiler-leaves-default/validation.json` (167 artifacts). The earlier
146-artifact default checkpoint remains unchanged. Pixel compilation and struct
declarations continue in separate leased worktrees. These integrated helpers do
not install the full native source generator or a successful B3B3C0 continuation;
native FH3, full material compilation, rendering parity and gameplay remain unproven.

## Validation after concurrent string-storage integration

The later shared checked-string/input-settings code was merged before another
default build and a fresh run of all 15 focused fixture programs. Both CTests and
all 15 fixtures passed at `fa6211f1`. The final validated revision is
`fa6211f1`; all seven accepted component source/header/report hashes remain
unchanged. No result from the earlier library was substituted for this fresh run.
The immutable final checkpoint is `local/checkpoints/fa6211f1/compiler-leaves-final/validation.json` (174 artifacts), including
the changed shared storage sources. The earlier 167-artifact checkpoint remains
preserved. This refresh changes neither the reconstruction scope nor the stated
native ABI, FH3, full compiler and gameplay limits.
