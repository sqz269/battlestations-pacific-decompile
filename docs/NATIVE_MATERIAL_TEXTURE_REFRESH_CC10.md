# Material texture refresh (cc10)

`native_material_texture_refresh.cpp` implements the complete normal control
bodies of B19000 and B1BC70 using the existing material storage, Lua object
providers, native string pool, texture cache and canonical reference owners.
It supplies the B19000 leaf required by B24DD0 in
`platform_renderer_activation.cpp`. It does not add application scheduling,
create another renderer/Lua/cache owner, or change the effect-owner module.

| Entry | Exact body, end exclusive | Coverage | Original ABI |
|---|---|---|---|
| `00B19000` | `00B19000..00B191CE`, 462 bytes | Complete normal body, explicit source boundaries below | ECX material, RET0; preserves EBP/EBX/ESI/EDI |
| `00B1BC70` | `00B1BC70..00B1BD1D`, 173 bytes | Complete normal body, protected source Lua error boundary | ECX Lua owner; stack fresh output/key; EAX output, RET8; preserves ESI |

The two complete spans (635 bytes) matched the live existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and the installed PE.
The nine-byte listing gap at `B19037..B1903F` is alignment code
`LEA ESP,[ESP]; MOV EDI,EDI`, not an omitted branch. No missing Ghidra function
is claimed; the report explicitly contains an empty `no_ghidra_function` list.
All analysis was read-only and existing names were preserved.

## Existing producers and domains

This packet reuses `NativeMaterialEffectBaseStorage`, not a new layout. Its
actual C4h constructor B18D60 clears eleven texture pointers at `+0C`, the
signed WORD texture count at `+38`, eleven 8h names at `+3C`, and the signed
WORD name count at `+94`. B18FB0 writes a selected name, extends `+94` using an
unsigned comparison against the sign-extended current count, and copies the
current header through the established string provider. B187A0 is the existing
texture-release/count-reset producer. The new refresh writes only the same
name/texture count cells and texture slots, alongside their reference effects;
it does not write the material dirty flag, fallback, descriptor or pass arrays.

`NativeRendererLuaOwnerStorage` already places its actual 4C8h Lua owner at
`+4`; B1BA30/B1BB90 are its existing constructors. B1BC70's receiver is this
outer owner, not a Lua interpreter or a separately created globals table.
Both native callers were checked: B19071 and B44C54 capture current F8D434,
pass a fresh output header and a native name header. B19000 has one observed
caller, B24DF3, passing the current effect-registry row's resource.
B44C00 applies its own `"Default"` fallback and binding operation after this
same resolver. The proposed name `BSP_RendererLua_ResolveStringOverride` is
therefore broader than texture-only use; it is a descriptive hypothesis.

F8D438 is passed directly as a C string on B1BC70's nil arm. It is zero-filled
native data, with only that observed reference and no observed writer. This
does not prove runtime immutability: the provider reads its current C-string
bytes. It is not an eight-byte NativeString. The other arm passes the empty read-only C
string at CE3A0C. `NativeMaterialTextureNameLiterals` borrows explicit bindings
for both addresses. The source provider owns neither cell.

## Native order

B1BC70 constructs a globals view from receiver `+4`, then looks up the key's
current `data+4` as a C string through B68100/B67800. Null data selects an empty
key, and header length is not used. It destroys the globals view before testing
the returned tracked value. Nil constructs output from F8D438. Every nonnil
value goes through B685C0: an exact Lua STRING supplies its C-string content;
numbers, booleans and other kinds use CE3A0C. No numeric coercion or recursive
name resolution occurs. Output construction truncates embedded NULs just as
0041E870 does. The tracked result is destroyed before returning the original
output pointer.

B19000 enters when the original unsigned WORD name count is nonzero. For each
index it zeroes a selected-name local, checks/extends the current sign-extended
name count, then captures current F8D434 and calls B1BC70. A nonempty resolved
name is copied; otherwise it checks/extends the name count again and copies the
current original name. Resize callbacks can change source fields, so the copy
uses the existing actual-header helper that rereads source length/data and
destination length/data after resize. Native BF7680 is represented by the
existing overlap-capable copy implementation.

An empty selected name leaves the texture slot and texture count untouched.
Otherwise B19000 captures current F8D394 and its virtual `+64`, passes the
selected header and DWORD zero, and receives a texture. Verified table
`D5F0A8+64 = D5F10C` contains B319B0. The source dispatches the real
`load_native_renderer_texture_00b319b0` provider after validating that current
profile; it never executes the numeric native table as host code.

After the return, B19000 extends the current signed WORD texture count if
necessary and captures the current old slot. When pointers differ it publishes
the returned pointer first, increments its actual atomic `+04` when nonnull,
then decrements the old pointer and dispatches its current terminal only on
zero. Regardless of identity, the returned nonnull pointer is then decremented
and its terminal dispatched on zero. Identity assignment therefore performs
only the returned-reference release; a null replacement still publishes null
and releases the old value. Existing `release_native_render_actual_owner`
resolves the canonical companion only on zero and checks that it borrows the
same actual atomic. No second ownership map or shadow counter is introduced.

The resolved local is released before the selected local through the same
00419CC0/BD1510 pool domain. The loop then increments its DWORD index and rereads
the sign-extended current name count for an unsigned comparison. It does not
capture an initial extent or clear stale texture slots beyond the new extent.
Producer-valid storage has at most eleven rows; the source explicitly rejects
an index outside that actual array rather than reading arbitrary memory.

Stack evidence: both resize calls clean eight bytes, B1BC70 and renderer+64
clean eight, BF7680's caller adds 0Ch, and the two BD1510 calls clean 0Ch.
00419CC0 consumes none of the three arguments already pushed for BD1510.
EBX begins at zero, temporarily holds the old texture at B1910E and is restored
to zero at B1915C; EBP holds the loop index after the original aligned frame
setup. The entire listing was checked, including those writes and both tails.

## Binding and exception boundaries

The primary binding supplies `NativeMaterialTextureRefreshContext` with:

* `GameNativeRendererApplication::texture_cache()`, whose `strings` and
  `textures.current_renderer_00f8d394` are borrowed directly;
* that application's `actual_owners()` registry, shared with the texture
  creator companions;
* the existing volatile F8D434 Lua-owner publication;
* the verified D5F0A8 numeric profile data and explicit nil/non-string literals.

Each refresh requires a fresh, persistent operation. Its eleven step frames
hold actual temporary headers and existing `NativeTextureCacheAcquired` child
records at stable addresses. These records are diagnostics/lifetime storage,
not extra texture references. Normal completion releases both local names and
consumes each returned reference. Failed frames cannot be replayed or discarded
while their child state is unresolved; destructors deliberately reject that
misuse rather than inventing rollback or an application cleanup schedule.

The source B68100 lookup is expressed through the existing same-Lua-frame
protected B67800 adapter after the same key-data capture. On Lua error it throws
the existing source status exception after restoring that frame's entry stack.
Other completed mutations and child acquisitions remain observable in the
retained operation. This is not the original CBC5F0/CBC8E9 FH3 handler/unwind
implementation, original stack aliasing, CRT exception identity or binary ABI.

## Verification

The isolated Win32 build and its two existing CTests passed. The report-call
checker validates every numeric call row; import and current-reference virtual
calls are separately identified as indirect. The exact table and import
operands were checked read-only.

One uncommitted `/MD`, `/MANIFEST:EMBED` x86 probe compares original B1BC70 with
the source using actual Lua 5.1.1 and a constructed native BD1480 string pool.
Eight cases cover nil, string, number, boolean, empty string, embedded NUL,
null key data and a changed nonempty nil-fallback buffer. Fresh output begins with a dirty preimage; output identity/text,
Lua stack height and tracked-reference cleanup match. Three original B19000
executions with controlled renderer/cache/terminal leaves establish the exact
publication/reference trace, identity and null replacement, empty preservation,
expanding name count and current Lua-owner reread.

That probe does **not** execute source B19000 through the complete canonical
texture cache. B19000 source is build checked and reviewed against the complete
listing and native control observations. Actual texture loading, cache misses,
graphics, application activation, native FH3/SEH and gameplay remain untested
by this packet. The report keeps those limits separate from B1BC70's actual
original/source comparison.
