# Particle resource construction and text-buffer cleanup

Two complete native storage bodies are reconstructed in
`src/native_particle_resource_loading.cpp`. These are descriptive names, not
recovered symbols. The raw pool context borrows the application's actual
publication, shutdown gate, and singleton-manager cells.

| Entry | Complete bytes | Original ABI | Source |
| --- | ---: | --- | --- |
| `00AF45D0` | 263 | ECX resource, stack raw name header, RET4, EAX resource | `construct_native_particle_resource_00af45d0` |
| `00AF5620` | 50 | ECX text buffer, RET | `destroy_native_particle_text_buffer_00af5620` |

## Construction

The 90h resource writes the refcounted base identity and count, then D5D958.
It zeros its name at +8 before testing source-header identity, and clears the
two inline child counts at +30 and +54. A nonaliased name uses genuine 41DD40
with preserve enabled, then reloads the source length and current source and
destination buffers in listing order. The constructor preserves every other
byte except its explicit defaults and flags.

The three read-only image DWORDs are CE3D08=42C80000 (100),
CFA424=453B8000 (3000), and CEB4B0=42700000 (60). They supply +60, +68/+6C,
and +74. Byte stores occur in order +65, +64, +66, +70, +78, +79, +7A;
+58=1 and +5C=30; zero DWORD stores occur at +8C, +7C, +80, +84, +88.
Unused child slots, padding, and unknown bytes remain untouched.

FH3 handler CBABF3 references DF2A40 and unwind map DF2A30: state1 to0
destroys the current +8 name through CBABE8/41DD20, and state0 to-1 calls
BD30F0 through CBABE0. The source uses a true-unwind guard: a second C++
exception terminates before further cleanup. The earlier state0-only region
contains raw stores; hardware faults in this region are outside the source's
C++ exception contract.

## Text cleanup

AF5620 captures +14 and frees it when nonnull, then destroys the current +C
raw name through the genuine pool getter and return functions. It leaves both
released pointers and all other fields stale. The missing ADD ESP,4 after
BF6989 was restored under the Ghidra write lock with full-body byte preflight;
the repair is recorded in the h8 particle flow report.

## Evidence and validation

`reports/native_particle_resource_loading_orch4.json` records full PE/live
hashes, prior documentation, exact calls, the unwind map, and the probe receipt.
The strict MSVC Win32 build and three existing CTests passed. A temporary,
manifest-embedded probe compares both complete copied original bodies with
source, bridging five direct calls to the genuine raw pool providers and CRT
free/memmove, and relocating only the three read-only constants.

Four constructor cases cover empty, small, large, and self-header names;
three destructor cases cover null and allocated text buffers with small and
large names. Full 90h images match after normalizing allocated pointer values;
the probe also checks return identity, copied names, pooled block return, and
the destructor's stale fields. An independent assembly/EH review found no
discrepancies. These checks do not execute original FH3/SEH failure paths,
establish binary replacement ABI compatibility, or validate gameplay.

## Remaining loader dependency

AF5850 is now reconstructed in `native_particle_text_loader.cpp`; see
`NATIVE_PARTICLE_TEXT_LOADER_ORCH4.md`. Its complete 221-byte body composes the
actual VFS resolver, file-manager open, and stream length/read/release providers.
The caller explicitly owns the retained resolution frame and actual text/name
storage; exposing the borrowed runtime services remains separate wiring work.
The historical source-absent entries in this packet's report describe its
starting boundary before that j10 extension.

AFAD00 is now a complete Layer reader using the genuine raw line/token/suffix
providers and raw parameter builder. AF4700 is also complete; its bounds fields
retain the native atof-to-x87-store schedule. Both passed full caller-body
original/source comparisons. See `NATIVE_PARTICLE_LAYER_READER_ORCH4.md` and
`NATIVE_PARTICLE_BOUNDS_READER_ORCH4.md`. AF40E0/AF9F50 preparation now composes
all five proven particle profiles with their genuine methods and actual string
pool; see `NATIVE_PARTICLE_PREPARATION_RAW_ORCH4.md`.

The full AF4BA0 resource parser and dependent 86BA60 loader remain absent.
AF9FB0's three emitter parser targets (B03EC0, B02FD0, B02210), nested B00CE0
particle parsers, and property/resource ownership domains still require complete
raw composition. B01150 and derived particle constructors currently have bounded
host-binding interfaces; their real raw construction remains a separate task.
Object resource loading must compose the existing VFS/name-resolution, resource
manager/cache and actual renderer/model ownership domains. Generic callbacks or
known function names do not establish those contracts or gameplay readiness.
