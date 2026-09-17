# Application shader process state and startup modes

Addresses: 0073D410 (fragment0073D4C2..0073D603), 00CD7CC0, 00CE0D50,
00B623D0, 00B62500, 00B3A600, 00B3B140.

## Result

Normal application startup now initializes the canonical 0108FEE4 shader
state-list pool and executes the native shader-mode fragment before VFS startup.
One retained process owner supplies the four mode bytes and 0108D6EC cache
publication. The renderer borrows those cells and its existing VFS, raw string
pool, allocator and decrement import for the complete R95 cache lifecycle.

The full 0073BF80 material-preload loop is still unconnected. Public
`create_shader_cache`, `release_shader_cache` and `shader_cache_context` are the
actual application providers for that loop; cache creation is not moved earlier
in ordinary startup. A live/failed cache prevents singleton drain until its
native release completes. Failed acquired frames remain owned by the renderer.

## Startup contracts

`GameNativeShaderProcess` owns one actual38h pool and its source companion. It
borrows the SAME E188B4 allocator list as existing application pools, binds the
existing fixed-global allocate/return functions, and invokes CD7CC0 once.
Registration status is preserved; no rollback/retry follows a thrown attempt.
Native CE0D50 remains the real CRT exit callback. The process object is retained
through process termination so its cells and pool cannot disappear first.

CE353C/CE3540/CE3544 contain CD7CA0/CD7CC0/CD7CE0. The reconstructed startup
therefore initializes this pool after the existing hardware-layout pool and
before Lua globals. This verifies relative ordering, not completeness of the
entire original CRT initializer table.

The new normal-path 322-byte fragment consumes the application's second
initialize argument, captured in native EBP. It constructs and releases three
actual pooled string copies. Searches use case-sensitive CRT `strstr` and the
native signed DWORD-offset predicate; they do not tokenize options.

| Substring | Native byte write |
| --- | --- |
| `genshaders` | 0108D6F0, variants/cache generation |
| `devshaders` | 0108D6F1, source mode |
| `hiresmode` | 0108D4BA |
| `reloadresources` | 0108D4BB |
| `devrr` | If present, set source mode and reload to1 |

The first four writes explicitly set0 or1. The final override does not change
variants or hires. WinMain's existing `cachedload` argument produces0000;
the normal application now obtains those values from the recovered fragment.
The other preload/reload consumers must borrow these cells when integrated.

## Validation

- Strict MSVC Win32 build and all three existing CTests passed; no new repository tests.
- Six function spans, the startup fragment and five data spans total918 bytes,
  all equal between live Ghidra and the installed PE. Direct calls and the
  CE0D50 tail jump are checked mechanically; indirect calls are separate.
- A focused probe links52 current production application objects and3 current
  libraries. Normal startup registers the shader pool with atexit result0.
  Global and instance allocation use the same pool; two slots, row allocation,
  holder cleanup, preserved slab IDs and slot reuse pass.
- Six native/source mode cases match: empty, `cachedload`, all four flags,
  uppercase nonmatches, substring `devrr`, and substring `genshaders`.
  The copied original322-byte fragment uses real pooled-string source providers
  and CRT search adapters; only direct calls/global operands and its final
  return are relocated. This is not a whole-original application differential.
- Through the actual renderer API, one cache load publishes to the real source
  process cell. The mode/raw-string/VFS references are identity-checked. All1628
  installed records,2,205,720 bytecode bytes and ordered digest
  `2733cb68f3747826` match an independent disk parse. File consumption is exact.
  Native zero-count stream retirement clears publication and admits drain.
- Probe and unmodified application both exit0 after two ticks/one Present;
  worker joined; final device/API COM references0/0. Ordinary startup performs
  pool/mode initialization but does not yet invoke the shader cache bracket.
- Confirmed Ghidra names/comments are extended with this evidence, prior
  comments preserved, project saved and affected exports refreshed.

Evidence: `reports/native_application_shader_process_r96.json`; immutable
tested/integrated artifacts are listed there. Only one new fragment is claimed;
the pool and cache bodies were reconstructed previously.

## Remaining work

Compose descriptor parsing, effect cache/owners, shader compilation and
0073BF80's preload vector over this same application state. The descriptor
ordinal preimages and sampler B-16 residue interfaces still require production
bindings; no defaults or empty-sampler substitution are added here. B107F0's
material/post-effect graph, full startup and gameplay remain incomplete.

Registration failure, abnormal CRT termination, source-mode allocation failure,
native FH3/SEH/ABI identity, cache variants writes, pixels and gameplay were not
proved by this packet. The source interfaces retain failures; they do not
replace the original private exception frames.
