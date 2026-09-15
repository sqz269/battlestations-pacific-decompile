# Native render-resource constructor, CR

`00B14A10..00B14F5B` constructs the actual `6ACh` render-resource service.
Startup still records this call as unimplemented. This packet reconstructs its
complete source schedule and registers it in `bsp_core`; it does not yet connect
the application to a native renderer or replace the GUI sprite bridge.

The Ghidra project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Live bytes and the installed PE match for the
1,356-byte body, ten compiler support bodies totaling 109 bytes, the cleanup
table, renderer dispatch cells, literals and borrowed constant words. Every
instruction belongs to its expected function. No listing repair was needed.
The old pseudocode has incorrect stack/register projections, including a
discarded `noise.dds` copy. The complete assembly is the source authority.

The original interface is ECX = receiver, no stack arguments, EAX = the same
receiver, plain RET. Names are descriptive hypotheses. The new context and
persistent-operation parameters are source interfaces, not original ABI.

`construct_native_render_resources_00b14a10` calls the existing `B0F020` base
publisher before storing the derived profile and fields. It preserves the
interleaved MOVSS loads as raw DWORD reads and writes, including register reuse
and nonascending stores. It invokes the existing `B0CD80` parameter initializer
at receiver+`2AC`. Unwritten bytes are preserved; the whole object must not be
zeroed as an inferred constructor operation.

The remaining normal schedule is:

1. Allocate `40h`, construct `B1FBB0`, publish at `+1D4`.
2. Reload the current renderer for each call. Its original D5F0A8 slots
   `128/12C` dispatch the existing `B24DC0/B20090` surface getters. Pass the
   borrowed results through retaining `B1FAB0/B1FB00` setters.
3. Construct native temporary names and invoke current renderer+`64`, qualified
   to `B319B0`, for `black.tga` and `noise.dds`. Publish at `+668/+67C` before
   capturing/releasing their temporary name data through the current raw pool.
4. Allocate `CCh`, construct the existing `B52550` texture helper and publish
   `+34`. Resize the member header at `+684` to four unterminated bytes and copy
   the current `XXXX` literal.
5. Allocate `24h`, call the existing `B3C800` cockpit constructor with its
   persistent admission, publish `+0C`, and return the original receiver.

The actual surface and texture contexts must share the renderer publication
cell and string adapter. Base publication and raw strings must share the raw
singleton-manager cell. The allocation/free pair, current profiles, constants,
literal storage, camera admission and existing cache children remain explicit
borrowed dependencies. Unknown dispatch targets fail as source boundaries;
there is no successful fallback provider.

The nine-state map at `DF45D8` contains base, member string, renderer-record
vector, string vector, raw allocation, black-name, noise-name, raw allocation,
and raw allocation cleanups. States 4 through 8 return to state 3; states 3
through 0 unwind the members and base. Constructor unwind does not roll back
already published frame targets, textures or helpers. Existing failed cache
frames and camera block records retain their own diagnostic/lifetime contracts.
The caller must keep those frames alive; it cannot replay the constructor.

Validation on the final source:

- Tracked MSVC Win32 build and both existing CTests pass.
- One focused case was added to the existing local D3D9/resource fixture. The
  first `40h` allocation observes the already published service and throws.
  A constant deliberately aliases receiver+84 to detect eager constant reads.
  The check verifies the observed alias result, untouched storage, initialized
  member headers, state-3 vector/string/base cleanup, cleared publication and
  manager removal. No later frame/texture/cockpit call is reached in this case.
- The retained three-parser mesh fixture also passes, with its existing actual
  D3D9 buffers and all 57 canonical companions retired. That is regression
  coverage for dependencies, not execution of this constructor's later calls.

The initial focused run caught an incorrect store to member+684 instead of
member+688. The source and local preparation script were corrected; the final
build and fixture were rerun successfully.

No new repository test suite was added. Successful full constructor execution,
original-machine-code differential execution of B14A10, original FH3/SEH,
complete service destruction, application wiring, native rendering and gameplay
remain unvalidated. Next work is the successful composition with actual frame
surfaces, six texture loads and cockpit ownership, followed by application
integration. Earlier one-frame application smoke evidence does not cover this
new constructor.

Evidence: `reports/native_render_resources_cr.json`,
`reports/native_render_resources_flow_cr.json`,
`reports/native_render_resources_annotations_cr.json`, and
`reports/native_render_resources_integration_cr.json`.
