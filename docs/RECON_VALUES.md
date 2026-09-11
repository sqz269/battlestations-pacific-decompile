# Recon table installation

`install_recon_values_00803a40` reconstructs the complete normal flow of
00803A40 and its five direct game helpers. Despite the startup callback's
previous name, this code installs no numbers or other recon settings. It
ensures this Lua table structure, preserving every existing non-nil value:

```text
recon
  [0], [1], [2]
    enemy, neutral, unknown, own
      mothership, destroyer, torpedoboat, battleship, cruiser,
      cargo, landingship, levelbomber, divebomber, torpedobomber,
      fighter, reconplane, kamikaze, submarine, landvehicle,
      landfort, airfield, shipyard, path
```

The relation order and category order above are execution order. With a stable
current owner and no existing entries, the routine creates244 empty tables in
total. The final19 tables in each relation remain empty. NativeString,
registry-reference readers, numeric conversion and script execution are not
part of this routine. It calls the repository's real Lua5.1.1 implementation.

## Actual owner and stack behavior

00803A46/A4B/A56 resolve `[00E188A8] -> game+1A08 -> mission host+04`.
The result is the Lua instance whose `+04` slot contains `lua_State*`. This is
distinct from the embedded game+1A0C owner used by MarkerClasses. The outer
routine captures the instance once in ESI. `ReconLuaInstanceView` borrows the
actual mutable state slot by reference, preserving that identity across calls.

006B8190 pushes the key and uses ordinary `lua_gettable` on the globals
pseudo-index-10002. Only `lua_type(...,-1)==LUA_TNIL` causes a pop, key push,
empty table creation, ordinary `lua_settable`, and a second lookup. Thus
global metatable callbacks remain observable. Each Lua primitive reloads the
captured instance's current state slot. Existing false, numbers and other
non-nil values are not replaced; subsequent child access still requires a
valid table, as in native code.

00803750 and008037D0 each construct a fresh8h scope: vtable00D08E5C at+00,
borrowed instance pointer at+04. They capture `lua_State*` once, call
`lua_checkstack(state,2)` and ignore its result. The integer form uses rawgeti,
then on nil pops and creates/rawsets/rawgets an empty table. The named form
pushes the key and rawgets at-2; on nil it pops, pushes key/table, rawsets at-3,
then pushes the key and rawgets at-2. All primitives within each helper use
the captured state. No registry references are created.

008039E0 independently reloads the current game, mission host and instance
for **each** category at008039F0..008039FF. It reads that iteration's category
pointer from00E0B590..00E0B5DB, constructs a named scope, then pops through
that scope's captured instance's **current** state slot. The category pointer
table is a required live19-entry view; the header also provides its verified
initial literals for source-level bindings. The table is not snapshotted.

Inlined scope cleanup restores vtable00D08E5C and executes
`lua_settop(current_state,-2)`. The relation scopes and indexed scope always
retain the outer instance, even when the current game's mission instance
changes during a callback. After all three integer indices,006B8210 pops
the root through that original instance. Cleanup is explicit: there is no
automatic destructor, baseline stack reset or added protected Lua call.

## Address and ABI evidence

All six complete assembly listings were exported read-only from
`C:/Users/sqz269/bsp.gpr`, program`/battlestationspacific.exe`, verified by the
CLI for each live query batch. The current Ghidra signatures omit the ECX and
stack inputs of the helpers; the C++ declarations follow assembly instead.

| Address | Original ABI | Final instruction | Inclusive end | Instructions |
| --- | --- | --- | --- | --- |
|00803A40|No native inputs; RET|00803B4D, RET,1 byte|00803B4D|76|
|006B8190|ECX=instance; stack key; RET4|006B8205, RET4,3 bytes|006B8207|37|
|006B8210|ECX=instance; tail call|006B8218, JMP lua_settop,5 bytes|006B821C|3|
|00803750|ECX=fresh scope; stack instance,int32 key; EAX=scope; RET8|008037C0, RET8,3 bytes|008037C2|41|
|008037D0|ECX=fresh scope; stack instance,key pointer; EAX=scope; RET8|0080385C, RET8,3 bytes|0080385E|47|
|008039E0|No native inputs; RET|00803A34, RET,1 byte|00803A34|23|

The read-only flow audit found zero gaps in every body. No missing function
starts, repairs or function definitions are required. No Ghidra writes were
performed. Proposed descriptive names and evidence are in
`reports/recon_values.json`; the parent owns annotation/ledger integration.

Library callees are reused, not ported: lua_settop00A673E0,
lua_type00A675A0, lua_pushstring00A67A50, lua_gettable00A67C20,
lua_rawget00A67CA0, lua_rawgeti00A67CD0, lua_createtable00A67D10,
lua_settable00A67E10, lua_rawset00A67EA0 and lua_rawseti00A67F00.
The unnamed00A672F0 is also Lua5.1 lua_checkstack: its complete31-instruction
body performs the2048-element limit check, stack growth and ci->top expansion
seen in the vendored`lapi.c:95`. It is reported for parent naming only.
00A67EA0's provisional library name matches the complete rawset body, including
the GC barrier and two-value pop (`lapi.c:667`); that library identity is retained.

## Validation and limits

The standalone MSVC Win32 Release build passed, as did the isolated worktree's
existing reconstructed_math test (1/1). One ignored real-Lua fixture passed
under C++17 with /W4 /WX. The fixture's global `__newindex`
callback publishes `recon` and switches the current mission instance. The
outer hierarchy remained on its captured state while all category helpers
used the new current state (229 owner resolutions). It also checked raw access bypasses metatables,
false and table identity survive, indices are0..2, and both stacks balance.
Files and a parameterized MSVC C++17 recipe stay under`local/`; there is no
tracked test or broad suite.

The context/host and8h scope representation are new C++ interfaces, not
drop-in native register/vtable ABI replacements. The native instance owner,
live pointer table and stack preconditions must be supplied by the real host.
Native Lua allocator/GC scheduling, failure/panic transport and binary layout
parity are not established by stock Lua source reuse. No game execution has
been validated, and unrelated consumers or setters of recon values are outside
this packet. There are no unreconstructed whole-routine callbacks in this path.
