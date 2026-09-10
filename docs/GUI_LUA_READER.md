# The GUI's Lua reader

Addresses: 00AC6600 00B6A020 00B669C0 00B69D40 00B66CA0 00B67980 004425C0 00441A70 00441210 00442220 00BD8E20 00BD7A20 00BD7130 00BD68D0 00BD6830 00BD5EB0 00BD5F50 00BD5790 00BD63B0 00BD61C0 00B65F50 00B65FB0 00B66050 00B660A0 00B661B0 00B66250 00B66270 00B66290 00B662B0 00B66420 00B66A60 00B67080 00B67190 00B67690 00B67700 00B67720 00B67800

The reader behind the descriptor visitor that `00AAA710` drives. The short
answer is that the GUI creates its own private Lua 5.1.1 interpreter for every
page it constructs, runs two script files through it, wraps the globals table in
a small stack-of-objects reader, hands that reader to the widget walk, and then
closes the interpreter again before the screen constructor returns. Nothing of
the page's Lua state survives the constructor: the widget tree is the only
record of the script.

## The chain

`00AC6600`, `__thiscall(ECX = screen, const NativeString *name, void *model,
char flag)`, `RET 0Ch`. Its object half is covered by
docs/GUI_LAYOUT_LOADER.md; the script half is:

1. `00AC6718`: `00B66BD0` constructs the interpreter owner on the frame, at
   frame + 30h.
2. `00AC672B`: `00B6A020(65h)` creates the state and opens libraries.
3. `00AC673F`..`00AC675A`: a NativeString `_Common` (`00D5CB64`) is built on the
   frame, `00AC5600` turns it into `interface/_Common.lua`, and `00B69D40` runs
   it with the second argument zero. The composed path's buffer is returned to
   the sized storage pool immediately afterwards.
4. `00AC678D`..`00AC67A0`: `00AC5770` does the same for the page's own name, and
   `00B69D40` runs `interface/<page>.lua`.
5. `00AC67DA`: `00B67980` writes a LuaObject for the globals table into a 14h
   byte frame slot.
6. `00AC67E3`: `004425C0` constructs the reader from that object, by value.
7. `00AC6804`: the reader's vtable `+4h` with `(0, "GuiScreen")` (`00D5CB24`)
   descends into the page's global table.
8. `00AC6812`: the screen's own vtable `+18h` with the reader as its argument.
   That is the widget walk, and it reaches `00AAA710`.
9. `00AC681F`: the reader's vtable `+8h` ascends again.
10. `00AC6825`: `00B6D890(screen->node, 0)`.
11. `00AC682A`..`00AC6857`: the reader is destroyed (vptr to `00CE44FC`, the
    object vector cleared by `00441210`, vptr to the base `00CE374C`) and then
    `00B669A0` closes the interpreter.

The literal `GuiScreen` is at `00D5CB24`, not `00D5CB20`;
docs/GUI_LAYOUT_LOADER.md is four bytes low. That doc also reads `00B6D890` as
part of the Lua tail because it sits near `00B67980`. It is not: `00B6D890` is
`__thiscall(node, new_parent)`, `RET 4`, and it rewrites node + A4h, unlinking
through `00B72220` (`BSP_NodeRootList_Unlink`) and relinking through `00B721F0`.
It is a scene-node reparent, left unnamed here because the scene node belongs to
another packet.

## Creating the interpreter

`00B6A020`, `__thiscall(owner, unsigned library_mask)`. In order:

- `00A6A260` `luaL_newstate`, stored at owner + 4h. Owner + 0h becomes 1, the
  flag `00B669A0` tests before closing, so the owner owns this state.
- `00A67390` `lua_atpanic` with **`00B669C0`**.
- the library loop over the table at `00D62BB8`: `{const char *name, lua_CFunction
  opener}` pairs terminated by a null opener, walked with a doubling bit. For
  each set bit it runs the standard Lua 5.1.1 sequence `lua_pushcclosure`
  (`00A67B20`), `lua_pushstring` (`00A67A50`), `lua_call(L, 1, 0)` (`00A68090`).
- `00A673D0` `lua_gettop`, kept at owner + 8h as the base of the stack.
- a tail that also uses `lua_setfield`, `luaL_loadstring` and `lua_pcall`; that
  is the wrapper's own bootstrap and was not followed.

The table, in bit order:

| Bit | Name | Opener | Opened by the GUI |
| --- | --- | --- | --- |
| 01h | `""` (base, `00CE3A0C`) | `00A67210` | yes |
| 02h | package (`00D62BB0`) | `00C2FF90` | no |
| 04h | `table` (`00CF8344`) | `00A66010` | yes |
| 08h | `io` (`00CF8340`) | `00A652B0` | no |
| 10h | `os` (`00CF833C`) | `00A64190` | no |
| 20h | `string` (`00CF8334`) | `00A63910` | yes |
| 40h | `math` (`00CF832C`) | `00A61C50` | yes |
| 80h | `debug` (`00CF8324`) | `00A61620` | no |

`00AC671D` pushes 65h, so a page script sees base, `table`, `string` and `math`
and nothing else. No file access, no process access, no `require`, no `debug`.
`00AC6600` registers no C functions of its own, so every helper a page calls
(`CreateStandardMenu`, `GetMISCols`, ...) is Lua defined in `_common.lua`.

`00B669A0` is the matching close: `lua_close` (`00A68A90`) when the state is
non-null and the owner flag is set, then the state pointer is nulled.

## What an erroring script leaves behind

`00B69D40`, `__thiscall(owner, const NativeString *path, char obfuscated)`,
`RET 8`, runs `00B66CA0` on the path, then asks the VFS singleton at `0109CEEC`
through `00BDEF90` for the other files that match it and runs `00B66CA0` on each
of those too, in the order the search returned. The list is 8-byte entries and
is freed afterwards (`00427110`, then `_free`). So a page script can be
overridden or extended by a mounted package without touching the base file.

`00B66CA0`, same signature, `RET 8`, is the part that matters:

1. the VFS singleton's vtable `+4h` opens the path with mode 2. A null stream,
   a `+18h` that reports not-open, or a `+30h` size of zero each return having
   touched nothing. **A missing page script is silent.**
2. the file is read into a `malloc` buffer through the stream's `+24h`.
3. if the second argument is non-zero, every byte is transformed in place
   (`00B66D1F`..`00B66D7B`: nibble rotation with a marker byte). The GUI passes
   zero at both call sites, so the transform never runs on a page script; it is
   not reconstructed.
4. `00A6A160` `luaL_loadbuffer(L, buffer, size, chunk_name)`, where the chunk
   name is the path's own text or the empty string at `0108FF2C`. **The return
   value is never tested.**
5. the stream is released and `00A68090` `lua_call(L, 0, LUA_MULTRET)` runs the
   value on top of the stack. **`lua_call`, not `lua_pcall`.** No `lua_pcall`
   appears anywhere on the page-loading path; the only one in this subsystem is
   in `00B6A020`'s own bootstrap.
6. the buffer is freed.

The consequence is that there is no per-page error recovery of any kind:

- a script that does not compile leaves its error *message* on the stack where
  the compiled function should be, and step 5 calls that string. Lua raises
  "attempt to call a string value".
- a script that raises at run time raises directly.

Either way the raise is unprotected, so Lua 5.1.1's `luaD_throw` finds no
`errorJmp`, calls the panic function and then `exit(EXIT_FAILURE)`. The panic
function is `00B669C0`: `lua_gettop`, `lua_tolstring` on that index, discard the
result, return 0. Nothing is logged and nothing is repaired. **A malformed page
script terminates the process rather than skipping the page.** The `exit` step
is read from the matched Lua 5.1.1 source rather than from this binary's
`luaD_throw`, which was not disassembled.

That sharpens a note in docs/GUI_LAYOUT_LOADER.md: `_debugtexts.lua` ships
malformed, and if anything ever loads it the game does not merely fail to build
that page.

## The reader object

`004425C0`, `__thiscall(this, LuaObject root)`, `RET 14h` — the argument is one
20-byte LuaObject passed by value, which is why the caller subtracts 14h from
ESP before `00B67980` fills it. The constructor writes the vtable `00CE44FC`,
zeroes the three vector pointers and pushes the root into the vector, then
destroys the by-value argument.

Layout, 14h bytes:

| Offset | Field |
| --- | --- |
| +0h | vtable, `00CE44FC` |
| +4h | secure-SCL proxy word of a `std::vector<LuaObject>` |
| +8h | `_Myfirst` |
| +0Ch | `_Mylast` |
| +10h | `_Myend` |

The element size is 14h, proved three ways: `00441210` steps the destroy loop by
14h, `00BD8E20` takes `_Mylast - 14h` as the back element, and `00BD7130`
divides the byte span by 14h with the 66666667h reciprocal.

`00441210`, `__thiscall(&vector_proxy)`, `RET`, destroys every element with
`00B67700`, frees the block and nulls the three pointers. `00441A70` is the
object destructor that calls it and then frees the object. `00442220` is the
vector's push_back.

The base class vtable is `00CE374C`: seven slots, `+0h` a destructor
(`00410610`) and `+4h`..`+18h` all `__purecall` (`00BF698E`). So the reader is
one implementation of an abstract six-operation interface, and `00CE3768` holds
a second implementation (`00410760`, `004106A0`, `004106C0`, `00410700`,
`004106D0`, `00410730`, `00410750`) that the GUI does not use.

### The six virtuals

Each of them first takes the back element of the vector — the table currently
being read — and looks the key up in it with `00BD5790`.

`00BD5790`, `__fastcall(ECX = out LuaObject, EDX = parent LuaObject, key_kind,
key_value)`, `RET 8`, returns ECX. It default-constructs the output
(`00B65F50`), switches on the key kind, assigns (`00B67690`) and destroys the
temporary (`00B67700`):

| Kind | Path |
| --- | --- |
| 0 | `00B67800`: `lua_pushlstring` + `lua_gettable`, the value is a `char *` |
| 1 | `00B67720`: `lua_pushnumber` + `lua_gettable`, the value is an `int` |
| 2 | `CVTTSS2SI` on the value, then the same integer path |
| other | nothing runs; the output stays unbound, which reads back as not-nil |

| Slot | Address | Signature | Behaviour |
| --- | --- | --- | --- |
| +4h | `00BD8E20` | `__thiscall(kind, key)`, `RET 8` | look the key up and push the result. A missing key pushes nil and the walk continues. |
| +8h | `00BD7A20` | adjustor thunk `ECX += 4`, `JMP 00BD7130` | pop_back. |
| +0Ch | `00BD68D0` | `__thiscall(kind, key, field_type, dest, default_type, default_value)`, `RET 18h` | look up; nil takes the default through `00BD61C0`, otherwise the value through `00BD63B0`. This is the slot the property descriptors use. |
| +10h | `00BD6830` | `__thiscall(kind, key, field_type, dest)`, `RET 10h` | the same with no presence test: an absent key reaches `00BD63B0` as nil. |
| +14h | `00BD5EB0` | `__thiscall(kind, key)`, `RET 8`, returns `bool` | `!IsNil(lookup)`. |
| +18h | `00BD5F50` | `__thiscall(KeyArray *out)`, `RET 4` | enumerate the current table's keys. |

The `(field_type, dest)` and `(default_type, default_value)` pairs are the same
8-byte record: a dword tag and one machine word. `00BD61C0` reads the tag from
the field pair only, so a default whose own tag disagrees is still written as
the field's type.

## The 500-entry key array

`00BD5F50` writes entry *i* at `out + i * 8` and keeps the count at
`out + 7D0h`. 7D0h is 2000, so the buffer is **250 pairs of 8 bytes followed by
an int count** — the "500 entries" of docs/GUI_LAYOUT_LOADER.md are 500 dwords,
and that doc's `[ESP+804h]` count is the same field: its array base is
`ESP+34h`, and 804h − 34h = 7D0h. `00AAA710` clears the 7D0h bytes to FFFFFFFFh
before the call, so an unfilled slot has tag −1.

The array caches **keys, not descriptor indices and not interned strings**. For
a string key the word is the `char *` `lua_tolstring` returned, which points
into the live Lua string and is only valid while the interpreter is open —
which is why `00AAA710` copies it into a NativeString before doing anything
else with it.

The walk itself:

```
table = back();
first = true;
IterateFirst(table, &key, &value);          // 00B67080: lua_pushnil + lua_next
while (!IsUnbound(key))                     // 00B66420: key.kind == 0
{
    if      (IsString(key))  { out[n].tag = 0; out[n].text    = GetString(key);  n++; }
    else if (IsInteger(key)) { out[n].tag = 1; out[n].integer = GetInteger(key); n++; }
    else if (IsNumber(key))  { out[n].tag = 2; out[n].number  = GetNumber(key);  n++; }
    IterateNext(table, &key, &value);        // 00B67190: lua_pushvalue + lua_next
}
out->count = n;
```

Three facts worth keeping:

- the tags are the same 0/1/2 the key kinds use, so an entry can be handed
  straight back to `+4h` as a key. That is exactly what `00AAA710`'s child loop
  does with the tag-0 entries.
- a key that is neither a string nor a number advances the iteration but is not
  recorded and does not move the count.
- nothing bounds checks *n*. A table with more than 250 keys writes past the
  array, over the count and into the caller's frame. No shipped page comes
  close, but the check is genuinely absent.

## A key becomes a typed value

`00BD63B0`, `__fastcall(ECX = the looked-up LuaObject, EDX = the field pair)`,
`RET`. One switch on the field tag. `00BD61C0`, `__thiscall(ECX = the field
pair, the default pair)`, `RET 4`, is the same switch for the default.

| Tag | Field | From the script (`00BD63B0`) | From the default (`00BD61C0`) |
| --- | --- | --- | --- |
| 0 | native string | `lua_tolstring`, `strlen`, resize + `memcpy`; a null pointer resizes to zero | `strlen` + `memcpy` of the default `char *` |
| 1 | `int` | `lua_tonumber` narrowed by `__ftol` | the default word |
| 2 | `float` | `lua_tonumber` | the default word as a float |
| 3 | `bool` byte | `lua_toboolean` | the default byte |
| 4 | `int` handle | integral number through `0109CED4`, table through `0109CED8`, anything else ignored | the default word, with no resolver |
| 5 | `float[3]` | `t[1]`, `t[2]`, `t[3]` | 3 floats from the default pointer |
| 6 | `float[2]` | `t[1]`, `t[2]` | 2 floats |
| 7 | `float[16]` | `t[i][j]`, both 1..4, written row major | tail call to `004134F0` (`BSP_Matrix_Copy4x4X87`) |
| 8 | `float[4]` | `t[1]`..`t[4]` | 4 dwords |
| 9 | — | no case at all | no case at all |
| 0Ah | `float` | `lua_tolstring` then the CRT parse at `00BF8417` | the default word as a **float** |

`GuiValueTag` in include/bsp/gui_layout_loader.hpp names tags 0, 2, 3, 5, 6 and
8 from the base widget descriptors and marks 1, 4 and 7 as unrecovered. This
fills all three and adds 9 and 0Ah.

Two asymmetries are worth naming because a reader would assume them away:

- tag 0Ah parses a string on the value path but takes a plain float on the
  default path. It is the only tag whose two halves disagree about the payload.
- tag 4's number test comes first and is `00B66A60`, "a number whose value is
  integral". A non-integral number never reaches the second resolver, and the
  second resolver's test is `00B661B0`, which compares `lua_type` against 5,
  `LUA_TTABLE` — not a string. Both resolvers are installed by `00BD4FC0` and
  neither target was followed.

For tags 5..8 the element reads go through `lua_gettable` on the value itself.
A missing index is nil and `lua_tonumber` turns it into zero, so a short table
yields trailing zeros rather than an error; a value that is not a table at all
is indexed anyway, which in Lua 5.1 raises, which is fatal here.

## The Lua API boundary

Everything the reader does reaches the interpreter through the thin accessors
below, so a rebuild links the matched Lua 5.1.1 library and ports none of it.
The LuaObject is 14h bytes: `+0h` the owner, `+4h` a kind (0 unbound, 1
pseudo-index, 2 stack reference), `+8h` the index or reference, `+0Ch` and
`+10h` unread here. `00B67980` builds one with kind 1 and index FFFFD8EEh,
which is −10002, `LUA_GLOBALSINDEX`.

| Accessor | Lua 5.1.1 entry point |
| --- | --- |
| `00B65F50` construct | none |
| `00B65FB0` IsNil | `lua_type` `00A675A0` == 0 |
| `00B66050` IsNumber | `lua_type` == 3 |
| `00B660A0` IsString | `lua_type` == 4 |
| `00B661B0` IsTable | `lua_type` == 5 |
| `00B66A60` IsInteger | `lua_type` == 3 then `lua_tonumber` `00A67770` |
| `00B66250` GetBoolean | `lua_toboolean` `00A677E0` |
| `00B66270` GetNumber | `lua_tonumber` |
| `00B66290` GetInteger | `lua_tonumber` then `__ftol` |
| `00B662B0` GetString | `lua_tolstring` `00A67810` |
| `00B66420` IsUnbound | none: kind == 0 |
| `00B67080` IterateFirst | `lua_gettop`, `lua_pushnil` `00A679C0`, `lua_next` `00A683A0` |
| `00B67190` IterateNext | `lua_gettop`, `lua_pushvalue` `00A67570`, `lua_next` |
| `00B67690` assign / `00B67700` destroy | the ref helper `00B66DE0` |
| `00B67720` GetByIndex | `lua_gettop`, `lua_pushnumber` `00A679D0`, `lua_gettable` `00A67C20` |
| `00B67800` GetByName | `lua_gettop`, `lua_pushlstring` `00A67A10`, `lua_gettable` |
| `00B67980` GetGlobals | none |
| `00B66CA0` | `luaL_loadbuffer` `00A6A160`, `lua_call` `00A68090` |
| `00B6A020` | `luaL_newstate` `00A6A260`, `lua_atpanic` `00A67390`, `lua_pushcclosure` `00A67B20`, `lua_pushstring` `00A67A50`, `lua_call`, `lua_gettop` `00A673D0`, `lua_setfield` `00A67E40`, `luaL_loadstring` `00A6A190`, `lua_pcall` `00A680E0` |
| `00B669A0` | `lua_close` `00A68A90` |
| `00B669C0` | `lua_gettop`, `lua_tolstring` |

## Callers and callees

- `00AC6600`: one caller, `00AA5840` (`BSP_GuiManager_LoadPage`). Callees
  include `00AA9390`, `0041DD40`, `0041E870`, `00AA6720`, `00AC5600`,
  `00AC5770`, `00B66BD0`, `00B6A020`, `00B69D40`, `00B67980`, `004425C0`,
  `00B6D890`, `00441210`, `00B669A0`, `00419CC0`, `00BD1510`.
- the reader's six virtuals have 2 to 25 callers each, all through the same
  construct-descend-read-ascend idiom; `004426C0` is the smallest example and
  reads `_device.platform` into an object field.
- `004425C0` has 37 callers, `00B6A020` has 50 and `00B69D40` many more, so the
  reader and the interpreter owner are general engine facilities and the GUI is
  one client. `0046DF00` (`BSP_SceneDatabase_LoadSceneFile`) is another.

## Uncertainties

- **`00AC5600`'s exact signature.** It is `__thiscall` with an 8-byte
  NativeString stack argument and `RET 8`, and ECX is a caller frame slot that
  comes back holding `{length, char *}`; the composed buffer is returned to the
  sized storage pool with size length + 1. Whether ECX is a return buffer or a
  small object was not settled, and it belongs to the layout-loader packet.
- **`00B6A020`'s tail** (about 220 instructions past the library loop) uses
  `lua_setfield`, `luaL_loadstring` and `lua_pcall`. It is the wrapper's own
  bootstrap, it is protected, and it was not followed. Whether it installs C
  functions that a page script could call is therefore open.
- **The two handle resolvers** at `0109CED4` and `0109CED8`, installed by
  `00BD4FC0`, were not followed, so tag 4's meaning is "an id the game resolves"
  and nothing more precise.
- **The obfuscation transform** at `00B66D1F` was read only far enough to see
  that it is a nibble rotation with a marker byte and that the GUI never enables
  it. It is not reconstructed.
- **`exit(EXIT_FAILURE)` after the panic** is read from the matched Lua 5.1.1
  source. This binary's `luaD_throw` was not disassembled to confirm it.
- **The wrapper's provenance.** The shape (a `LuaState` owner with a mask, a
  refcounted `LuaObject`, `DoFile`-style entry points, the `dofile` / `dobuffer`
  keywords in segment 87) matches the LuaPlus family, but no version string was
  found, so the wrapper is documented from its own code rather than matched.

## Routines with no Ghidra function

- `00B669C0`, the panic handler, ends at `00B669D6` inclusive. Its ten
  instructions are followed by `int3` padding and Ghidra has defined no function
  there, so the name has to be applied after the function is created.

## What each routine reached

| State | Routines |
| --- | --- |
| analyzed, reconstructed, build-tested | `00BD63B0`, `00BD61C0`, `00BD5790`, `00BD8E20`, `00BD7A20`, `00BD5EB0`, `00BD6830`, `00BD68D0`, `00BD5F50`, `004425C0`, `00B66CA0`, `00B69D40`, `00AC6600` (script half) |
| analyzed, modelled as a host method | every accessor in the API boundary table, `00B6A020`, `00B669A0`, `00B669C0`, `00B67980` |
| analyzed only | `00441210`, `00442220`, `00BD7130`, `00441A70`, `00B66BD0`, `00B6D890` |
| read, not analyzed | `00B6A020`'s bootstrap tail, `00BD4FC0`, `00B66D1F`'s transform |

Nothing here is ABI compatible or game validated. The reconstruction in
include/bsp/gui_lua_reader.hpp and src/gui_lua_reader.cpp keeps the conversion
rules as pure functions over an evaluated value and over a live object, and puts
every interpreter call behind `GuiLuaHost` and `GuiLuaScriptHost`.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `lua_state_bootstrap` | 00B6A020 00B6A0A6 00BD4FC0 | docs/LUA_STATE_BOOTSTRAP.md | The 220-instruction tail of `00B6A020`: what `luaL_loadstring` + `lua_pcall` install, which C functions reach the script namespace, and what `00BD4FC0` registers at 0109CED4/0109CED8. |
| `lua_script_overrides` | 00BDEF90 00427110 00B66D1F | docs/LUA_SCRIPT_OVERRIDES.md | The VFS search behind `00B69D40`'s second pass and the byte transform `00B66CA0` applies when its flag is set: which packages can override a script and what an encrypted chunk looks like on disk. |
| `lua_object_refs` | 00B66DE0 00B67690 00B67700 00B65F50 | docs/LUA_OBJECT_REFS.md | The LuaObject reference model: what kind 1 and kind 2 mean, how `00B66DE0` refs and unrefs, and whether a stored object survives the stack frame that made it. |
