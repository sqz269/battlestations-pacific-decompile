# Robot configuration creation, reading and ownership

`src/robot_config.cpp` reconstructs the normal `00901610` Robots loader,
`009013D0` registration, `00900AF0` lookup, `00900BB0` registry cleanup and
`009DBFE0` Navigator factory. All nine descriptor readers and validators are
implemented, together with the ten canonical scalar deleting wrappers. Names
are evidence-based hypotheses, except retained `CG_scalar_deleting_dtor_*`
identities. `reports/robot_config.json` records every address, native ABI,
function extent, remaining Ghidra gap and reconstruction boundary.

The evidence is BSP, project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Verified CLI exports, complete assembly and disk
bytes take precedence over damaged pseudocode. This packet makes no Ghidra
writes. The parent owns missing function definitions, flow repairs and final
annotations after the packet lease is released.

## Concrete entry and dependencies

```cpp
bool load_robot_config_00901610(RobotConfigRegistry&, RobotConfigAliases,
    LuaStateOwnerEnvironment, LuaScriptRuntime&, RobotConfigContext&);
void register_robot_config_009013d0(RobotConfigRegistry&, RobotDescriptor&,
    GuiLua51Host&, GuiLuaRef robots, RobotConfigContext&);
RobotDescriptor* find_robot_config_00900af0(RobotConfigRegistry&, const char*);
void clear_robot_registry_00900bb0(RobotConfigRegistry&, RobotDescriptorStorage&);
```

The context requires actual native-string storage, actual descriptor block
allocation/release and a reference to the live CRT conversion choice
`0109EEA4`. The mode reference and allocator must remain valid through the
call. The descriptor allocator supplies fresh exact-size storage or throws;
it must not value-initialize the block. The implementation does not invent
constructor values for untouched bytes. `NativeString`, `duplicate_00438e40`,
`release_duplicate_00438e40`, `PcStorageLuaOwner`, `LuaScriptRuntime`,
`GuiLua51Host` and `lua_numeric` are existing canonical services.

`RobotConfigAliases` supplies references to actual global slots `00F8A30C`,
`00E199A0`, `00E1999C`, `00E19998`, `00E19994`, `00E19990`, `00E1998C`,
`00E19988` and `00F8A688`. The temporary Lua owner is separate from the
current-game embedded Lua at `+1A0C` and mission host field `+1A08`; the parent
binds those unrelated startup fields separately.

## Loader sequence and storage

`00901610` constructs a temporary native Lua owner, opens mask `0x41` (base
and math), and runs these two override-aware scripts with obfuscation false:

1. `Scripts\global\luaMW_init.lua`
2. `Scripts\datatables\Robots.lua`

Each native string is resized to 29 bytes, copied with its terminator and
destroyed before the next phase. The loader holds globals and its looked-up
`Robots` LuaObject through all registrations and final alias assignments.
It releases Robots before globals, closes the owner, then returns true in AL
at `00901A93`; the native routine does not promise a full EAX value.

The first eight descriptors are allocated and immediately registered in the
order below. Each constructor writes base vtable `00D17DC0`, duplicates its
class-name C string to native `+8`, then writes the derived vtable. Native
`+4` and all parameter bytes are untouched during construction. The ninth
factory `009DBFE0` follows the same construction and publishes Navigator to
`00F8A688` **before** registration. The other eight aliases are registry
lookups only after every registration.

| Class | Allocation | Six-level stride | Vtable | Reader +4 | Validator +8 | Deleting +0 |
|---|---:|---:|---|---|---|---|
| AAFlakBot | CC | 20 | D17DEC | 008FC6D0 | 008FC930 | 008FE4A0 |
| TailGunnerBot | E4 | 24 | D17E50 | 008FCA10 | 008FCCA0 | 008FE4E0 |
| AAGunnerBot | 6C | 10 | D17EDC | 008FCD60 | 008FCEA0 | 008FE520 |
| PilotBot | DBC | 248 | D17DE0 | 009973B0 | 009999F0 | 008FE460 |
| ArtillerySubDirectorBot | FC | 28 | D17F20 | 008FCF30 | 008FD230 | 008FE560 |
| ArtilleryGunnerBot | B4 | 1C | D17FC4 | 008FD370 | 008FD580 | 008FE5A0 |
| TorpedoBot | 84 | 14 | D17FF8 | 008FD640 | 008FD7D0 | 008FE5E0 |
| DepthChargeBot | 84 | 14 | D18038 | 008FD880 | 008FDA60 | 008FE620 |
| NavigatorBot | E4 | 24 | D217DC | 009D53B0 | 009D56A0 | 009DFC70 |

Sizes/offsets are hexadecimal. The shared header is exactly 0Ch:
vtable DWORD `+0`, `NoTargetTimeUntilRest` float `+4`, independently allocated
C-string pointer `+8`. Typed structures and size/offset assertions describe
the actual Win32 bytes; explicit `unconsumed_*` arrays preserve Pilot holes.
No per-descriptor ownership or parameter sidecar substitutes for game storage.

## Registration and field operations

`009013D0` receives ECX=descriptor and EDX=Robots LuaObject. It captures the
name (or `unnamed MBotClass`), looks up its class table, stores exact
`0x7F7FFFFF` (FLT_MAX) at `+4`, then looks up `NoTargetTimeUntilRest`. A value
that is not nil goes through native float conversion; this reference stays
alive until after insertion.

The six level lookups and virtual+4 calls are ordered `SPNormal`→1,
`MPNormal`→3, `SPVeteran`→2, `MPVeteran`→4, `Elite`→5, `Stun`→0. Every vtable
word is reloaded **after** its preceding Lua lookup and the level reference
is destroyed after that reader returns. A fresh virtual+8 call follows all
six; its AL result is ignored. The name pointer is then reloaded and inserted
into the unique registry, followed by NoTarget and class-table destruction.

The nine readers perform 181 scalar stores per complete set of class levels
(1,086 over six slots). `reports/robot_config_evidence.json` records each key
literal, native lookup/store address, destination offset, array index, scope
and fallback bits. This is independent native listing extraction rather than
a field list inferred from names. The row tables preserve:

- Ordinary `00B66270` Lua conversion followed by its float32 spill.
- `00B66330` defaults only when the looked-up tracked object is not a Lua
  NUMBER. Numeric strings take the compiled fallback here; ordinary reads
  coerce those same strings. Exact fallback bits are retained.
- Pilot `NumRockets` calls `00B66270` then the CRT integer path `00BF7420`.
  The canonical numeric kernel preserves float32 rounding and reads the live
  SSE2-mode reference after conversion, with no zero fallback or saturation.
- A new named-parent lookup for **each** array component, followed by indexed
  lookup, conversion/store, component release and parent release.
- Native's repeated `AAFlakBot.AngleErrMin` key at offsets `+10` and `+14`.
- The four retained Pilot `Rocket_*` scopes: lookup from the level root,
  release old scope, copy the still-live looked-up object, destroy temporary,
  then read fields from that retained scope.

These direct readers use the existing Lua host and numeric kernels; generic
reader aggregate behavior is not substituted for different native accessor
semantics. Dispatch supports the nine concrete native vtable identities in
the table. An unknown table is an explicit `invalid_argument` boundary,
rather than invented read/validation/destruction behavior.

## Validators and registry lifetime

The seven substantive validators retain the actual native inline SSE/x87
instruction sequences. This preserves `COMISS`/`FCOMI(P)` unordered flags,
`JB` versus `JBE`, x87 operand ordering, float spills, six-level strides and
x87 cleanup on both return paths. Constants include float `1.0`, double
`400.0`, double `100.0` and exact double bits `0x401921FB60000000`.
Pilot `009999F0` and Navigator `009D56A0` are exactly `MOV AL,1; RET`.

Native registry `00F89994` is a unique ordered map with sentinel at global
`00F89998` and count at `00F8999C`. Node offsets are left/parent/right at
0/4/8, key pointer at C, key-ownership byte at 10, descriptor pointer at 14,
color at 18 and nil byte at 19. The comparator first checks pointer equality,
orders null before nonnull, then uses CRT `_stricmp < 0` (`0043A610`).

`std::map<const char*, RobotDescriptor*, RobotNameLess>` preserves these
lookup, uniqueness and pointer-ownership semantics. Native tree insertion,
balancing and checked-iterator representation are standard-library boundaries,
not a new tree port. Keys borrow `descriptor.name_08`; the temporary
`00438F80`/`00438FE0` wrappers in registration both carry false ownership.
A duplicate insertion keeps the old key/value and neither adopts nor deletes
the new descriptor. The caller still owns that rejected allocation.

`00900BB0` traverses in map order, invokes each nonnull descriptor's freshly
selected deleting slot with flags=1, then clears the tree. Canonical deleting
wrappers (including base `008FC5A0`) reset the base vtable, free nonnull name,
store null at `+8`, free the descriptor only if flags bit0 is set, then return
the original pointer even after freeing it. Existing duplicate-string release
and the required allocator provide actual release services. Node destruction
does not compare dangling borrowed keys. Native leaves all alias slots
untouched; callers must not dereference them after registry cleanup.

## Ghidra follow-up and validation boundary

Nine missing starts must be defined through their inclusive final RET bytes:
`008FC930..008FC9B9`, `008FCCA0..008FCD07`, `008FCEA0..008FCEDD`,
`008FD230..008FD314`, `008FD580..008FD5EA`, `008FD7D0..008FD820`,
`008FDA60..008FDAB0`, `009999F0..009999F2`, `009D56A0..009D56A2`.
Each final instruction is one byte. Full raw instruction evidence and spans
are in the report.

Each of ten scalar wrappers has two false no-return call-site gaps: ten bytes
after its `start+11` call to `00BF6989` (stack adjustment and name=null), then
three bytes after `start+28` call to `00BF65AC`. Each ends in three-byte
`RET 4` at `start+33`, inclusive final byte `start+35`, span 36h. Key wrapper
`00439000` additionally misses its one-byte `POP ECX` at `0043900E`, after
call `00439009`. No callee no-return flags or compiler/library names should
be changed. The report distinguishes stored listing gaps from complete disk
evidence; no repair is claimed yet.

`./scripts/build.ps1` passed MSVC Win32 Release and the existing CTest
`reconstructed_math` (1/1). The ignored real-Lua fixture is run with
`./local/run_robot_config_fixture.ps1`, optionally `-Repository <integrated
worktree>`. It passed script/library/override order, all 54 class/level calls,
repeated key/array lookups, defaults versus coercible strings, float32 integer
rounding, untouched bytes, all aliases, duplicate ownership and flags0/1
ordered cleanup. An independent worker matched every validator and
registration instruction sequence and found no concrete mismatch in this
canonical domain. No permanent tests were added.

This is not a drop-in native ABI replacement or a gameplay result. Descriptor
storage is exact Win32 layout, while LuaObject, temporary owner and registry
nodes use existing C++ interfaces. Native SEH unwind, allocation-failure/null
fault behavior, checked-iterator invalid-parameter handling and Lua
panic/longjmp transport are not claimed equivalent. Lua/VFS/CRT implementations
are reused with their existing documented limits. No game execution or native
differential validator run was performed.
