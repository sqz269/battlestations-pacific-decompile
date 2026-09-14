# Installed cache and descriptor compilation through actual owners

The source `B3C3A0/B3B3C0` compiler now has a passing fixture using the installed
`debugshader0F3` cache programs and both installed Lua descriptors. One actual
VFS mount, raw singleton manager and string pool serve cache acquisition,
fundamentals, nested `DoFile`, descriptor parsing and compilation. This is a
composition validation of existing source, not new game startup registration.

The owned worktree is `agent/orch5-20260911`, based on `ad598d83`. Cache source
remains an unchanged explicit-object copy from the separately leased cache
packet; root libraries retain their published hashes. `cmake/startup.cmake`
and other owners' tracked files were not changed.

## Reached route

- Actual physical-provider pool, `BEDA60` manager/factory construction and
  `BE1890` mount of the installed loose-file root precede cache acquisition.
- `B3A600 -> B38A70 -> BDF310 -> B35340` publishes 1,628 cache records and
  retains the physical stream at position 2,250,700, with read flags 2.
- `B43B00` reads `shaderfx/common/debugshader.shfx` and
  `shaderfx/lights/dummy.shfx`, generation 3. Its real bootstrap uses the
  `00884770` fundamentals cache and real `B69E00/B69D40/BDEF90` callbacks.
  The installed include executes twice. VFS accounting reaches six reads and
  2,258,511 bytes: cache, fundamentals, debug descriptor, dummy descriptor,
  and both include reads. The fresh manager has no override suffixes; the
  override producer executes its empty-list path.
- Parsed data has priority 23, one render target, two root vertex fields,
  one interpolator, two root render-state rows, no mode render-state rows,
  and empty sampler arrays in both descriptors.
- Full `B3C3A0/B3B3C0` returns an actual pass with both native-layout shader
  owners, completed child operations, normal builder cleanup and released
  temporary shader references. Nine canonical render owners are registered;
  the real platform loading child executes once. Cache cursor becomes 158.

Direct3D9 HAL `GetFunction` returns exactly the installed cache bytes:

| Cache row | Program | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| 156 | debugshader0F3.vso | 432 | cbc4d11e94dfd24513e4d433bebb1c6a4926f47146509c6643ea32ca027d586a |
| 157 | debugshader0F3.pso | 444 | 1010de3ba58511098c5d1800239975d09175cf3b1ceb010b82688b388840033a |

An independent parse of the installed CTAB records supplies these expectations;
the source reflection owners have the same register indices and counts:

| Stage | Constant | Native semantic | Register | Count |
| --- | --- | ---: | ---: | ---: |
| VS | cViewProjMat | 5 | 15 | 4 |
| VS | cVtxElemScale | 24 | 77 | 2 |
| VS | cVtxElemOffset | 25 | 79 | 2 |
| PS | cElapsedTime | 39 | 34 | 1 |
| PS | cVisibility | 43 | 77 | 1 |

Both material-constant arrays are empty. Render states 7 and 14 are zero;
all sampler rows are pruned. The original 306-byte `B3B280` body executes
twice on the empty branch and preserves the explicit entry preimages
`7B2954AA/192EF0C3`; this branch consumes neither word.

## Validation and limitations

Fresh Win32 compilation uses `/O2 /Oy- /W4 /WX /fp:strict`; linking embeds the
manifest. The sixth probe capture passes all checks. Captures 1–4 stopped
before native data mapping because unrelated mapped reservations occupied
required bands. The final harness starts a suspended child, reserves five
fresh bands before its initial thread, then validates and releases only those
untouched private reservations immediately before the existing mapper runs.
The mapper still refuses occupied addresses. Delayed D3D9 loading alone did
not resolve that collision. Capture 5 completed the compiler but correctly
failed the inherited synthetic register-zero assertion; capture 6 checks
the installed CTAB register values above.

Twenty profile spans (780 bytes), the original sampler body (306 bytes),
and six call-site spans (30 bytes) were compared with fresh live Ghidra bytes
and the pinned installed PE. The six direct call rows are checked separately
against live function membership and listing. The map pins ten selected
root-library provider symbols and three explicit cache symbols; the
fundamentals getter is compiled into its callback, not a separate linked
getter symbol. Runtime module paths come from physical mapped-file queries.

The original `B3C3A0` ABI is ECX effect, EDX descriptor, seven stack arguments,
EAX pass, `RET 1Ch`; `B3B3C0` takes ECX builder and three stack pointers,
`RET 0Ch`. `B43B00` takes ECX descriptor and two stack arguments, `RET 8`.
The reconstructed APIs retain their explicit C++ contexts. This fixture does
not establish original FH3, incidental registers or binary replacement ABI.

Supplied fixture inputs remain explicit: `USA` region, PC flag, stack
preimages for descriptor fields, mode 0/generation 3, the matched program
name, directly selected dummy descriptor path, scalar effect/renderer
storage and an actual hot white texture. It does not run `B46950` admission,
native basename resolution, cold texture loading, source compilation,
variants writes, archive startup, complete teardown, drawing or gameplay.
No whole-cache destructor is claimed. The successful process intentionally
retains its owners through exit. Installed inputs are hash-identical before
and after every attempt.

Private commands, source copies, failed captures, successful capture,
compiler dependencies, binaries, assets and mapped modules are retained under
`local/native_installed_shader_compilation`; the report contains the immutable
checkpoint pointer and published-library source provenance.

## Follow-up packets

Feed this installed-data route through the canonical effect admission parent
after the leased cache and secondary-pass sources are handed off. Resolve
actual basename discovery and descriptor/request production there. Preserve
the current source/ABI/draw boundaries when connecting a real submission.

Immutable checkpoint: `local/checkpoints/ad598d83/installed-shader-compilation/validation.json`,
SHA-256 `f8ce0915419de9a5604157405edcafaddac9708591ccdd72f164a0f2d17ab6ee`. It freezes 1,103 artifacts and
80 physical Win32 modules. All 221 root provider sources appearing in this
link match the unchanged libraries existing immutable source closure.
