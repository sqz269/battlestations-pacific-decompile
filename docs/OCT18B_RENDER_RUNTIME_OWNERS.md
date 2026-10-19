# OCT18B generated render texture owner admission

Primary's frozen ON08 executable/dump isolated teardown to a luminance quad's
material texture0: raw200D73DC decremented1->0, then the same actual-owner
registry rejected its missing companion. The stack runs material -> mesh
section -> mesh -> modelB750C0/B6F310/B6DFA0 -> post20B4E1F0 -> luminanceB50FE0
-> resourceB0F6E0. Preload/Application and installed ship creator retirement had
already returned. ON09/10 diagnostic logs repeated the missing-companion error.
This packet fixes that producer/terminal boundary; it adds no unique native
function or byte credit, changes no native counts, and supplies no cleanup skip.

The genuine ownership sequence explains why the final reference can belong to
a material. B2A070 constructs a real pooledD61948 owner viaB3F7B0 and returns
its creator credit. B4E020 stores that texture at holder08. B4CB10 borrows that
pointer without retaining. Luminance's `bind_sample()` passes it to B189F0,
which retains new04 before releasing the previous material slot. B50DD0
releases holders228..234 before post210 and the remaining post chain. B4E140
decrements holder08 and leaves the texture alive when materials still retain it.
The later last material release must therefore reach genuineB3F590 through its
canonical companion. The old runtime factory did not register one; VFS-loaded
textures already did.

Completed B2A070 creators now use an explicit
`NativeTextureLoadOwners::register_completed_runtime_creator` entry on the
application's existing texture owner service. It requires completed factory
and B3F7B0 records, the same original raw slot/result/constructor owner, no
returned slot or prior admission, actualD61948 and the exact owner/registry
domain. It reuses the existing Entry that aliases actual04, calls genuine
B3F590/pool return, unbinds and retires its metadata. There is no fabricated VFS
acquisition frame, new texture/COM/cache, second registry or extra retain.
Admission occurs after the complete native normal schedule; an admission
failure retains the completed creator and outer operation instead of running
native rollback or allowing replay. Appended operation fields are Source
diagnostics, and their registered flag records admission rather than liveness.

Direct already-zero terminals must retire that same companion when a holder,
binding cache, shader texture list or shadow owns the last credit. New
`delete_native_texture_2d_after_zero` dispatches the existing canonical
already-zero helper when its optional capability is enabled. It does not
decrement again, and a missing identity still fails. The Entry deliberately
calls rawB3F590 to avoid recursion. All production direct2D zero call sites
were changed to this helper, including holder, luminance, renderer binding/cache
clear, shader unload, directional shadow and render-resource fallback. Raw
B3F590 remains only in its body, the default-null helper and the Entry terminal.

Two appended context capabilities preserve existing source interfaces:
`NativeTexture2DOwnerContext::canonical_actual_owners` defaults to null and
retains the old raw terminal route. `NativeRuntimeTextureCreationContext::
canonical_owners` defaults to null and retains the old factory route. ON
ResourceGraph sets the latter to its existing `loading.owners`; primary appends
`mission_ship_requested()?&owners:nullptr` to DestructionGraph's texture context
after the surface profile. No primary file is edited by this worker.

The production source has one runtime creation context, ResourceGraph's.
Holder creation, initializer half textures, luminance readback and shadow depth
color/depth all borrow it. B3F7B0 has one production caller, B2A070. Named
B3F930 has one production caller, the existing owner-backed VFS loader. Thus
the ON capability covers the production2D creator domain; the default-null OFF
route does not require a new companion. Native references, retained sources,
COM ownership, pool IDs and destruction order remain with existing producers.

The CLI verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, before live analysis queries. Installed/live bytes
match for fullB189F0 (80bytes), holder08 releaseB4E1AA..1C5 (28bytes), borrowed
getterB4CB10 (4bytes), and holder-before-post loopB50E10..E39 (42bytes). SHA256:

| Span | SHA256 |
| --- | --- |
| B189F0 | ddcd3b96b4cce2f22b205119a4e9cf9a648c23a228bf1bb608373d76cd2b9116 |
| B4E1AA | c0204a42da143bceddf8f041e4478368e33db9de802752e8f83dd60085876528 |
| B4CB10 | 916ae10aa3fd76a4d5b25b2fb66e9df0fd04957c7ab10930268d1ed7808be689 |
| B50E10 | be261add676133aa7de9e5f6251c383653f6fcc8470220d9d1060c21cc067653 |

Ignored receipt: `local/oct18b_a1_runtime_owner_byte_proof.json`.
Original B2A070 uses ECX renderer, five stacked words, EAX owner and RET14h;
B4E140/B50DD0 use ECX and RET; B189F0 uses ECX plus slot/texture and RET8;
B4CB10 uses ECX and returns borrowed holder08 in EAX. B3F590 uses ECX plus
flags, returns its original address and RET4. The new context/companion routing
is Source metadata composition, not an Original register ABI replacement.
FH3/SEH, exceptional cleanup parity, concurrent entry and game parity remain
unproved. `git diff --check` passed. Primary owns the combined Win32 build,
OFF checks, ON drain and visual validation; this worker performs no run/build.
