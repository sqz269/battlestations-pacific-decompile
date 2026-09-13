# Native shader compiler lookups

Addresses: 00b347e0, 00b34890, 00b34920

`native_shader_compiler_lookups.cpp` reconstructs three complete normal bodies
using the actual descriptor, sampler, string, cache and material-state storage.
These are new C++ interfaces, not native binary ABI replacements. The existing
semantic `material_shadow_samplers` implementation is evidence only.
Descriptive BSP/C++ names are reconstruction hypotheses, not recovered symbols.

| Routine | Original inputs and cleanup | Coverage |
| --- | --- | --- |
| `00b347e0` sampler ordinal | ECX descriptor; one stacked actual eight-byte name; EAX ordinal or FFFFFFFFh; RET 4 | Complete B347E0..B34851, 114 bytes |
| `00b34890` binary-cache cursor lookup | ECX cache; one stacked actual eight-byte name; EAX actual row or null; RET 4 | Complete B34890..B34915, 134 bytes |
| `00b34920` descriptor render states | Observed caller ECX builder is unused; stack pass, descriptor; RET 8 | Complete B34920..B34969, 74 bytes, including nine skipped padding bytes |

The B34920 pseudocode omits the pass input. Both callers push a descriptor,
then EBP (actual pass), then put the builder in ECX. The helper never reads
ECX before replacing it with the stacked pass at the B5EC40 call. This records
the observed ABI without declaring the unused register a recovered parameter.

The sampler lookup reads current descriptor C8 as an unsigned count and C4 as
the pointer array. It loads each actual sampler's name length at +4 before the
query length. Equal zero lengths match without loading either data pointer;
equal nonzero lengths call CRT `__stricmp(row, query)`. It returns the first
overall ordinal, with no vertex, type or texture-source filter. After a miss,
it reloads the descriptor count and data rather than retaining a vector view.

The binary cache's separate loop counter starts at cursor00. Every iteration
uses the current cursor and current records0C to select a 16-byte row. On a
miss, the stored cursor and the separate loop counter each increment. On a
match, the stored cursor increments, then the result uses the freshly loaded
cursor and base: `base + cursor*16 - 16`. Thus a change during comparison can
make the returned row differ from the row that matched. Count rereads and
DWORD cursor/address wrapping follow the instructions; accessed extents must
remain live and accessible.

The actual byte 0108D6F1 is borrowed by reference and read once before any
cache or query dereference. A nonzero value returns null even if those pointers
are inaccessible. Its producer is ApplicationInitialize: command-line
`devshaders` sets it at 0073D520; `devrr` forces it to one at 0073D5DF. The
neighboring `genshaders` flag is a different byte, 0108D6F0.

The cache schema comes from producers, not the lookup's indexing alone:

- B3A600 allocates 10h and publishes the B38A70-constructed object to 0108D6EC.
- B38A70 clears +4/+8/+C and opens `shaderfx/shaders.bin` when the source-mode
  byte is zero. B35340 sets cursor00 to zero, stores the stream record count
  at +4, allocates rows with stride 10h, and publishes the base at +C.
- B35340 copies each counted name to row00/04, stores the byte count at row0C,
  allocates the blob at row08, and reads exactly that count into the blob.
- The constructor passed to the vector helper at B34C70 has no Ghidra function.
  Its pinned raw B34C70..B34C7C bytes clear row00/04/08 and return; row0C is
  left untouched until the loader writes it. No definition or annotation was
  added for this producer.
- The sampler name schema reuses B57B50/B41830 and
  `NativeShaderSamplerStorage`; descriptor B8/BC reuses the B442E9 -> B579B0
  render-state table producer and its existing eight-byte state/value rows.

B34920 loads each current row's value before its state, calls the unchanged
`set_native_material_render_state_00b5ec40`, then reloads descriptor count/data.
The setter borrows the actual pass+18 state owner and uses the shared singleton
allocation domain. There is no private state vector, copied global or substitute
setter. Its established accessible-extent checks remain a host boundary.

| Owned call site | Callee | Stack evidence |
| --- | --- | --- |
| B34825 | BF7FBF, CRT case-insensitive comparison | push query, row; ADD ESP,8 |
| B348DD | BF7FBF, CRT case-insensitive comparison | push row, query; ADD ESP,8 |
| B34954 | B5EC40, actual material render-state setter | ECX pass; push value, state; callee RET 8 |

The report carries all three calls and all eight direct caller sites in B3B3C0.
Read-only flow inspection found no flow gap in either lookup and only the
explicit JMP-skipped B34937..B3493F padding in B34920. There are no missing
normal branches or post-call fallthroughs in these three bodies.

`NativeShaderCompilerLookupOperation` is a persistent, one-shot host frame.
On a C++ exception it retains borrowed inputs, sampled arguments, cursor/index
and last call site, rejects replay, and terminates if discarded unresolved.
It performs no native FH3 unwind, rollback, private allocation reclamation or
external owner retirement interception. Callers must keep borrowed owners,
rows and any failed setter state alive and exclude retirement while the frame
is running or failed. Explicit diagnostic acknowledgement frees nothing.

Validation used `./scripts/build.ps1` after seed verification: default MSVC
Win32 Release, /W4 /WX, both existing CTests passed. One ignored local fixture
links the registered bsp_core and relocates the three original normal bodies;
all 322 code bytes match the installed PE and live Ghidra bytes. The 13 raw
constructor bytes and 40 command-line-string-region bytes are separate evidence.

The fixture compares ordinary, empty-name and length-mismatch lookups, first
duplicate/case behavior, live descriptor/query changes, cache base/cursor/count
changes and its single gate read. Render-state results and four allocation/free
events agree with live descriptor replacement during allocation. Both original
and source legs reuse the same existing actual B5EC40 setter; this is a test of
the forwarding helper, not a new independent setter differential test. Temporary
CRT import hooks exist only in the probe executable and restore the imports on
normal exit. A source-only allocation failure checks retained state, replay
rejection and the failed-frame guard's exit 77.

The fixture uses borrowed actual-layout names and ASCII CRT comparison; it does
not allocate strings or validate all historical CRT locales/invalid-parameter
behavior. Native exceptional unwinding, real shaders.bin loading, full compiler
integration, compiled shaders, rendered output and gameplay are not validated.
