# Native font application producer

`GameNativeFontApplication` connects the existing real renderer texture cache,
raw VFS, scalar DAT decoder and actual font resource owner. This is a C++ source
composition of the successful initial-load path, with persistent operation
frames. It creates actual native texture references and populated native font
associations; it does not create semantic image wrappers or copy a glyph map.

## Existing native evidence and scope

| Address | Original ABI | This component |
| --- | --- | --- |
| `00AD4C30` | ECX font; prefix/Data/GFX/AlphaTexture/extra string arguments; RET14h at AD517E | GFX, alpha, DAT production, existing scalar decode, actual image adoption and canonical publication. New source interface, not raw entry/SEH/tree/pool equivalence. |
| `00AC3910` | ECX registry; root/descriptor/language stack arguments; RET0Ch | Existing evaluated-Lua traversal and stable descriptor list, extended with an actual-resource loader arm. |
| `00AD51D0` | ECX font; root/extra stack arguments; RET8 | Unsupported destructive reload. Initial production is never used as a reload substitute. |
| `00AD53A0` | ECX font; no arguments; RET | Existing `NativeFontResources` typed payload/GFX/alpha retirement. Native string/tree/sentinel allocator and timing remain outside that projection. |

The established resource paths and flags come from
`docs/FONT_RESOURCE_OWNERSHIP.md`, `docs/NATIVE_FONT_RESOURCES.md`,
`docs/FONT_REGISTRY_STARTUP.md` and `reports/native_font_resources.json`.
The observed renderer64 target is B319B0. GFX site AD4CD0 and both alpha sites
AD4E29/AD4E6E pass flags0. AD4C30 opens DAT read-existing with flags2.
The scalar decoder already preserves SSE height/advance/width conversion,
first duplicate retention, required key0091, and embedded special records.
No new register/x87 interpretation, listing repair or Ghidra mutation is made.

AD4C30's decompile still exposes unaffiliated register/stack and float10
artifacts. This packet uses the existing proved source modules instead of
claiming their raw calling or allocation schedule has been recovered anew.

## Actual production

The constructor borrows one empty `FontRegistryStartup`, an already ready
`GameNativeRendererApplication` and the same `GameVfsHost`. It revalidates the
renderer texture/grid contexts, raw VFS routes, publication cells, actual string
pool and canonical resource registry. The raw renderer's current profile and
the saved original profile view must select D5F0A8/+64=B319B0. The numeric token
selects the concrete existing body; it is never called as a host pointer.

Each descriptor is appended before its resource callback, preserving the
existing Lua iteration order, stable owned descriptor addresses, earlier rows
on failure, and first-match name lookup. The compatibility vector contains
descriptor metadata only. Native rows borrow one published `NativeFontResources`
from the producer's `NativeFontResourceOwners`; they contain no `FontResources`
or semantic COM texture wrappers. Mixing loaded semantic and actual rows is
rejected. The semantic loader still uses the same common traversal and its
existing complete-resource checks.

One operation is allocated and retained before native image work:

1. GFX uses `root + language_path + gfx_file`, a real pooled native8h name and
   the current renderer B319B0/cache/actual owner route, flags0.
2. Alpha uses `root + alpha_texture`, or unprefixed `white.tga` when the authored
   alpha name is empty, through that same route with a fresh persistent frame.
3. DAT uses `root + language_path + data_file`, flags2, through the actual raw
   VFS open/read/release implementation exposed by `GameVfsHost::open`.
4. The complete bytes feed the existing `MemoryStream` scalar decoder. The
   resulting sole `FontData` allocation is transferred into the existing native
   owner, preserving its populated map nodes. Adoption publishes actual raw
   image aliases into glyph+18/+1C and transfers the owning embedded pair.
5. `NativeFontResourceOwners::publish` establishes the exact descriptor pointer
   association, then the stable registry row borrows that owner.

There is no additional image retain, per-glyph reference count, font metric
snapshot, fake resource identity, second texture cache or replacement native
declaration. Repeated equal image identities still transfer their two returned
references, with existing adoption validation. Native null image results are
preserved rather than converted into a pretend successful image.

DAT transport reads the full raw VFS stream before scalar decoding and adapts
its complete bytes to `MemoryStream`. It does not preserve the original
interleaving of individual virtual scalar reads, stream release, payload
allocation and embedded resource stores. Names use exact typed concatenation
and the real pool at the cache boundary; native concatenation/unwind timing is
not claimed. Invalid names, incomplete data and unsupported owner/profile
domains reject explicitly. These are bounded source guards, not native fault
or allocation-failure ABI emulation.

## Failure and retirement

The application permits one initial descriptor-load attempt. An entered failure
retains the earlier completed owners and the current operation, including
pooled names, raw image reference slots, cache/load acquisition frames,
unpublished adopted ownership and the exception. It neither rolls back nor
replays those effects. `requires_process_retention()` remains true; its
destructor rejects abandonment of entered work. Diagnostic `native_site`
identifies the corresponding observed body region/entry, not an invented exact
failure instruction inside a provider or the C++ transport.

After complete production, `native_fonts()` supplies the actual collection for
`GameNativeTextApplicationBindings`. Before Text construction,
`bind_text_factory()` must receive that same canonical factory, authenticated
by `native_font_resources()` pointer identity. An unrelated empty factory
cannot authorize retirement. The bound factory must remain alive through
`retire_fonts()`, and all exported Text borrowing must use it.

Retirement rejects retained raw Text allocations before any native release.
It then destroys fonts in load/list order through existing AD53A0 projection
and clears each borrowed registry binding only after the owner is completely
retired. Failed retirement is nonreplayable. A registry destructor rejects
remaining actual bindings before deleting their descriptor storage. Retire
Texts, retire fonts while their factory still lives, then destroy the Text graph
and producer before registry/renderer/VFS teardown. Independently retained
material images may survive font cleanup; their actual owner domain must live
until those final references are released.

## Verification and activation boundary

Strict MSVC Win32 compilation of the new producer and changed stable registry
passes `/std:c++17 /W4 /WX /EHsc /permissive- /O2 /MD /fp:strict`.
The producer source requires integrator-owned CMake registration in `bsp_game`.
The standard worker build passed both existing CTest entries (2/2, 10.39s),
covering the changed registry and dependent headers in the linked executable.
The new producer translation unit is separately strict-compiled until primary
source registration. Serialized actual font runtime probing is still pending;
no fresh native execution result is inferred from compilation. Evidence is in
`reports/game_native_font_application.json`.

This source moves the actual font producer prerequisite forward. Startup still
must call this native registry arm, bind its collection into the canonical Text
graph, and provide the actual GUI/material/effect factory owners. No active
native frontend rendering, raw ABI differential, language reload, faithful
world/menu/UI rendering or gameplay validation is claimed. A runtime process
exit0 and simulation counters do not establish those results.
