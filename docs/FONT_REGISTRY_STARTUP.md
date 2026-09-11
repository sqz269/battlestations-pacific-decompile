# Font registry startup ownership

`FontRegistryStartup` composes the recovered registry loader with the existing
`VfsLuaScriptFiles`, `LuaScriptRuntime`, `PcStorageLuaOwner`, descriptor converter
and actual `FontResources` loader. It retains the constructed fonts after the
temporary Lua interpreter closes. Direct construction supports application RAII;
the separate singleton getter requires a real lifetime manager supplied by the
application. Every descriptive name is a hypothesis, not a recovered symbol.

This packet used read-only Ghidra. Every live CLI batch verified project `bsp`,
program `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`. Before-values
are retained in ignored `local/font_registry_startup_comments_before.json`.
Ledger names/evidence await the integrator's locked annotation/save/export batch.

## Native control flow and ABI

| Address | Original ABI | Established behavior |
|---|---|---|
| `007371d0` | cdecl, no arguments, EAX registry, plain RET at `0073728c` | Check `00f8bf44`; optional lifetime-manager lock; recheck; allocate `10h`; construct; publish before registration; unlock. |
| `00ac3690` | ECX registry, EAX this, plain RET at `00ac36e2` | Store vtable `00d5ca84`; allocate/self-link list sentinel at registry+8 via `00ac3400`; zero count at +Ch. |
| `00ac3400` | no stack arguments, EAX sentinel, plain RET at `00ac3419` | Allocate `0Ch`; next and previous links point to the allocation. The unused payload is not initialized. |
| `00ac3910` | ECX registry, stack(root, descriptor, language path), RET `0Ch` at `00ac3edc` | Temporary Lua owner; mask1; descriptor and VFS overrides; iterate global Fonts; construct and append each font; load its resources; advance; close Lua. |
| `00ac3610` | ECX registry, stack(root, language path), RET8 at `00ac365a` | Walk the existing list and call `00ad51d0` on each font. No registry-wide root or language store. |
| `00ac36f0` | ECX registry, plain RET at `00ac37c0` | Destroy fonts in forward list order, then list nodes and sentinel; clear the singleton publication. |

The loader's decompiler prototype loses a string argument and misrepresents stack
locals. Assembly `00ac3956` selects argument 2 for script execution. At the
per-font call, `00ac3d74..00ac3d99` pushes language path, alpha, GFX, DAT and root
for `00ad4c30`. The `scale_ratio` and `alphatexturescale` defaults originate in
separate `FLD1` sequences; both converted values are stored as float32 before the
constructor `00ad55c0`. The existing typed descriptor converter preserves those
numeric-only defaults and the boolean-only uppercase default.

At `00ac3b4d` the native code constructs an `ACh`-byte font from its name and
scalar settings. `00ac3b92..00ac3bcc` appends it before reading `datafiles[1]` and
calling `00ad4c30`. The next `lua_next` operation follows the resource load.
Repeated registry loads append and retain duplicate names. Lookup uses equal
byte length followed by CRT case-insensitive comparison and returns the first.

`0073bae0` supplies root `Fonts\`, descriptor `Fonts/Fonts.lua`, and the value
from the selected language's `fontpath` setting. See [APP_INIT_FONTS_GUI.md](APP_INIT_FONTS_GUI.md).
No separator or language default is inserted here. The resource loader uses
`root + language_path + GFX/Data`, while nonempty alpha uses `root + AlphaTexture`.
A missing alpha field defaults to nonempty `white.tga`; an explicitly empty alpha
uses unprefixed `white.tga`. These paths are passed to the shared resource search
and VFS binding; see [FONT_RESOURCE_OWNERSHIP.md](FONT_RESOURCE_OWNERSHIP.md).

## Ownership and integration

The application's binding owns a `FontRegistryStartup`, returns `registry()` from
`GuiStartupHost::font_registry`, and calls `load_lua_descriptors_00ac3910` with its
existing script files, Lua globals and a required `FontRegistryResourceLoader`.
That callback binds `load_font_resources_from_streams_00ad4c30_fragment` to the
application's device, live D3DX imports, resource-search/VFS stream resolver and
mip setting. A success with no font or either missing image owner is rejected.
There is no metadata-only successful fallback.

`FontRegistryOwnedFont` retains descriptor, root, language path and the resulting
`FontResources`. Font objects stay at stable addresses across appends, and lookup
returns the actual owner. Its resources own the DAT glyphs and two shared image
references; GUI consumers may borrow the font or independently retain images.
`registry()` is a compatibility view; callers must not mutate its metadata.

The descriptor parser now exposes `visit_font_descriptors_lua` and both the old
metadata adapter and the new owner reuse it. The owner opens mask1 through
`PcStorageLuaOwner`; fundamentals come from the shared VFS cache, and nested
DoFile uses `LuaScriptRuntime`. `executed_paths` on this new owner records
successful descriptor/override chunks only. Cached fundamentals and nested
DoFile execution remain managed by the common runtime and are not traced there.

The optional `get_font_registry_007371d0(published, lifetime)` implements the
double check, construction, publication and registration with the existing
`SingletonLifetimeManager`. Publication storage must outlive the owner. The
manager's destruction callback must delete this concrete owner; the destructor
clears publication. Deleting it outside manager teardown requires explicit
unregistration first. Direct RAII construction does not invent a private
singleton manager or register itself globally.

`set_language_path_00ac3610` preserves the registry traversal and invokes a
required per-font reload callback, even for equal paths. Its `00ad51d0` dependency
must compare saved paths, destructively clear changed resources, reload, and
update those saved paths. This packet does not substitute an initial-load
transaction for native reload.

## Failure and analysis boundaries

The supported successful path constructs and loads one font before visiting the
next. Malformed ordinary-table inputs are guarded by the existing converter,
which finishes conversion before append; native appends before reading resource
fields. On resource failure, this projection retains an appended descriptor with
null resources, and earlier completed owners remain valid. Native partial
construction can contain uninitialized image fields and is not claimed safely
destructible. Lua runtime exceptions and allocation exceptions propagate through
C++ RAII. These are explicit host boundaries, not native failure parity.

The typed owner replaces native pooled strings, list/sentinel allocation, font
layout and intrusive pointers with standard C++ ownership. Allocation throws
instead of reproducing native null-allocation registration. The singleton helper
receives an already established lifetime manager instead of looking up the native
manager global. The resource factory remains the device/VFS integration boundary;
renderer cache identity and binary drop-in compatibility are outside scope.

The erroneous no-return annotation on `_free` creates gaps in `00ac36f0`:
`00ac3755` is `ADD ESP,4`; `00ac3788..00ac3790` cleans the argument and continues
the node-free loop. The unassigned tail `00ac379b..00ac37c0` restores state,
clears sentinel storage and `00f8bf44` at `00ac37a6`, restores the base vtable and
returns. Endpoints here are inclusive final instruction addresses. Ghidra reports
no function for `00ac3755`, `00ac3788` or `00ac379b`; its current function body
stops at `00ac379a`.
Raw live bytes and installed executable bytes matched. No analysis repair was
applied in this read-only packet.

## Validation

`scripts/build.ps1` passed MSVC Win32 Release with both existing CTests. Eight
native seed windows matched disk. All six packet windows matched live Ghidra
bytes against the installed executable; exact hashes and extents are recorded
in [font_registry_startup.json](../reports/font_registry_startup.json).

Ignored `local/font_registry_startup_fixture.cpp` uses the installed descriptor
and fundamentals through a real physical VFS provider, shared search rules, Lua
runtime, hidden D3D9 device and system D3DX9_40 imports. Six fonts were retained:
Arial fonts contain 207 glyphs, Viper fonts 116. All 12 image owners contain actual
COM textures. `Fonts\white.tga` resolves through the recovered search lists to
`effects/white.dds`.

The same fixture checks append-before-load, rejection of success plus empty
ownership, prior stable font survival after a failed append, image release at
teardown, and the optional getter with the real `SingletonLifetimeDomain`:
one registration, one deletion and publication cleared. The probe embeds its
manifest and is built only under ignored `local/`; no permanent tests were added.
Installation files were read only. This is installed-data/device fixture evidence,
not native differential, visual rendering, full game startup or gameplay proof.
