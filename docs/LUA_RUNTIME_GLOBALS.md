# Lua runtime global sources and startup order

Addresses: 0108ff20, 0108ff24, 0108ff28, 00439100, 00cd7ce0, 00ce0d60,
008d44c0, 008d5b50, 00b6a020. `LuaRuntimeGlobals` remains the existing value
type; the new functions initialize and publish its recovered native sources.
Descriptive names are hypotheses, not recovered symbols.

## State before the first input tables

`make_initial_lua_runtime_globals_0108ff20()` returns `x360comp=false` and an
empty region. These are established startup values, independent of the settings
object or an assumed country. The image contains `.data` at RVA `00a08000`,
raw size `00010000`, virtual size `00297edc`. The globals beginning at VA
`0108ff20` are at section offset `00287f20`, beyond the raw bytes: the image
loader zero-fills the byte and both NativeString words. Ghidra's image bytes
at `0108ff20..0108ff2f` also read as zero.

The CRT initializer pointer at `00ce3544` names `00cd7ce0`. Its entire disk
body, `00cd7ce0..00cd7ceb`, is `PUSH 00ce0d60; CALL atexit; POP ECX; RET`.
It registers cleanup without initializing a value. The cleanup function
`00ce0d60..00ce0d82` returns a nonnull `0108ff28` allocation through the
sized pool, using size `0108ff24+1` and alignment one. It was incorrectly
inventory-labeled a static initializer; it is an atexit destructor.

| Native point | Consequence |
| --- | --- |
| `0073da94` -> `005547d0` | First input singleton creates `InputScriptStartup`; its constructor opens the persistent and temporary Lua owners. |
| `0073da9b` -> `006a7be0` | The following explicit load is already guarded. |
| `0073daa5` -> `008d8190` | Settings file/language loading happens after those Lua owners opened. |
| `0073e1af` | The constructed game object is retained in application `+14h`, later in the initializer. |
| `0073e477` -> `004e5540` | Game startup selection can subsequently reach `004dd5b0` and publish compatibility. |
| `0073e485` -> `008d5b50` | Settings apply eventually queries and publishes the user-geography region. |

`00b6a020` turns the initial byte into `X360COMP=false`. Its null-region
branches emit no `REGION` assignment, so `REGION` is nil in a fresh state.
An empty C++ string is the existing owner's equivalent projection of the null
NativeString pointer. Calling Windows geography before initial input Lua open
would change the established native ordering.

## Compatibility source and publication

Live xrefs and a complete `.text` search for the little-endian absolute
addresses agree: `008d44cc` is the only direct write to `0108ff20`; the only
other reference is its read by `00b6a020`. Setter `008d44c0..008d4518` takes
ECX=settings and one stack byte, with `RET 4`. It writes settings `+b0h`,
mirrors that byte, then checks the game/console-owner/state pointer chain.
When a command sink exists it issues `006b8ad0(text,0,0,2)`, choosing
`X360COMP=true` or `X360COMP=false`.

Current direct caller sites are `004dd6a3` (game init once), `005f661e`
(options commit), `0067ce92` and `0067d501` (press-start/sign-in), and
`008d7253` (settings archive read). None is an earlier application bootstrap
writer. The archive call follows the `XboxCompatibilityMode` bool field read
at `008d7202..008d724a`, with default false and key literal `00d15c38`.
`004dd5b0` republishes the retained settings byte `00f88a30`. The settings
constructor's false `+b0h` initialization does not itself mirror the global.
No command-line or options-text source for the startup byte was found.

`publish_lua_xbox_compatibility_008d44c0` composes the existing settings setter
with its global mirror and returns the existing optional command text. The
caller must still deliver a returned command to its actual live `006b8ad0`
sink; the helper does not pretend delivery occurred. Both native storage and
command ordering are preserved for the existing normalized bool projection.
Arbitrary noncanonical native char values are outside that existing interface.

## Geography region source

`user_geography_region_00439100()` reconstructs the full function
`00439100..0043917f`: cdecl, no arguments, EAX points to a static string,
plain RET. It makes the actual recovered Windows calls:

1. `GetUserGeoID(GEOCLASS_NATION)`, literal `10h` at `00439103`.
2. If the result is not `-1`, `GetGeoInfoA(id, GEO_FRIENDLYNAME, buffer, 100, 0)`,
   literal type `8` at `00439119`, using the native 100-byte buffer length.
3. On a successful call, compare the returned C string with `_stricmp`.
   `United States` and `Canada` return `USA`; `Japan` returns `JAP`.
   Every other string and either API failure return `EU`.

The literals are `EU` at `00ce4280`, `USA` at `00ce4284`, and `JAP` at
`00ce4288`. This preserves the native friendly-name lookup with language
argument zero, including its comparison against English literals. No locale,
language-selection, registry, command-line or module-directory substitute is
introduced.

Settings apply calls this helper at `008d6112`, measures its C-string length,
resizes the NativeString at `0108ff24`, then copies bytes to `0108ff28` at
`008d614e`. The only other `.text` references to these string globals are
Lua owner reads and the destructor. `publish_lua_region_008d6132` implements
that assignment using the existing value type. The old `SettingsApplyHost`
callback `publish_module_directory` and `SettingsApplyEnvironment` field
`module_directory` were incorrect interpretations; they are now
`publish_lua_region` and `user_region`. The actual module path helper is
`00439040`, whose separate bootstrap fields are unchanged.

Opening an owner snapshots the globals into its environment. Region
publication does not rewrite existing Lua states. Compatibility publication
can separately update the console state's global through the returned native
command; that does not replace the global byte used by future owners.

## Scope and validation

This is a new C++ interface, not fixed-address storage or a binary replacement.
Only `00439100` is a newly reconstructed whole native routine. The factory
models the image state; the publication helpers compose bounded fragments with
existing modules. Native atexit registration, sized-pool destruction and
complete settings-apply/console execution are not newly reconstructed here.

MSVC Win32 Release build passed. After all eight existing native math seeds
matched the original installed executable, both existing tests passed. These
math checks do not validate the new globals against native execution.

One ignored fixture mounts the installed game directory through the existing
VFS manager and opens two real Lua 5.1.1 owners using the installed 657-byte
fundamentals and real DoFile callback. It verifies initial nil `REGION`, false
compatibility even when an unrelated settings object is true, later published
region/compatibility on a newly opened state, the unchanged first state's
globals, and publication without a console sink. The actual Windows geography
call returned `USA` on this run. It does not fixture all geographical branches,
execute a native console command, or constitute a native differential or game
validation. No installation files were modified and no permanent tests added.

At analysis time Ghidra lacked the function `00cd7ce0..00cd7ceb`; the primary
integrator was given its exclusive end `00cd7cec` for definition before names
are applied. This worker used Ghidra read-only. Existing comments were preserved
by appended ledger evidence; name application, project save and refreshed
annotated exports belong to integration. See `reports/lua_runtime_globals.json`.
