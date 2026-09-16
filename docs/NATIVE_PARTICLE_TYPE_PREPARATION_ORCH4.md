# Actual particle type preparation

Packet `orch4_particle_preparation_i9` supplies seven complete preparation bodies
and a reusable raw-pool `0041E870` constructor provider. Descriptive names are
hypotheses. The owning source APIs operate on the actual Layer/type/string bytes;
they add no host owner, callback dispatcher, alternative pool or reference count.

## Entries and original contracts

| Entry | Inclusive end | Bytes | Original interface |
|---|---|---:|---|
| B075D0 | B07653 | 132 | ECX actual Layer; owned eight-byte string by value on stack; RET8. |
| B0A000 | B0A028 | 41 | ECX sprite type; RET. |
| B07660 | B07688 | 41 | ECX axial type; RET. |
| B087B0 | B087D8 | 41 | ECX floating type; RET. |
| AF8A30 | AF8A30 | 1 | ECX ignored; RET. |
| B0A0F0 | B0A0F0 | 1 | ECX ignored; RET. |
| B0A100 | B0A100 | 1 | ECX ignored; RET. |
| 41E870 | 41E8C2 | 83 | ECX actual8h header; nonnull C string on stack; EAX same header; RET4. |

Live vtable references establish the three preparation methods at
`D5DD2C -> B0A000`, `D5DCD4 -> B07660`, and `D5DD00 -> B087B0`.
Each is the corresponding type table's +14h entry. The actual RET leaves are
`D5DB14 -> AF8A30` (object +14h), `D5E05C -> B0A0F0` (tracer +14h), and
`D5E054 -> B0A100` (tracer +0Ch). The latter are original complete empty bodies,
not replacements for an unresolved preparation operation.

The three 41-byte methods read type+74h first, then type+14h, then that parent's
inline Layer pointer at +34h+index*4. This captures the Layer before name
allocation or any raw-pool singleton work. They construct the material names
`ParticleSprite.mvfm`, `ParticleAxial.mvfm`, or `ParticleFloating.mvfm`, then
transfer the actual eight-byte argument to B075D0. Pointer/index arithmetic
retains DWORD wrapping, and no index/pointer validation is added.

## Consumed name and exception ownership

B075D0 captures incoming data into EBX before capturing length into EDI. It
compares Layer+14h with the incoming stack argument's address. If different,
it calls raw41DD40 with the captured length and preserve=true. A nonzero
captured length triggers a copy from the captured data into the CURRENT
destination pointer using its CURRENT length after resize. BF7680's overlap
behavior is retained through memmove.

Normal cleanup disarms the state, then resolves the actual pool and returns
the CAPTURED data with CAPTURED length+1 (DWORD wrap), leaving the incoming
argument header untouched. If resize/copy throws instead, FH3
`CBB898 -> DF3A18`, map `DF3A10`, has the single row
`state0 -> -1, CBB890 -> 41DD20`. That action uses EBP+4: the CURRENT incoming
eight-byte header, which need not still match the initially captured values.
The source accepts an explicit actual consumed-header address to preserve this
alias/current-header behavior. The caller must not release it after the call.
The source unwind guard terminates if cleanup throws while unwinding.

There is no cleanup retry if the normal-path pool getter throws: the original
state is already -1. The three calling preparation methods have no own FH3
cleanup registration for the transferred argument.

## Reusable raw 41E870 construction

`construct_native_string_header_0041e870` and the
`NativeString::assign_0041e870(NativeStringRawPoolContext&, const char*)` overload
are in the existing native_string files. The old host-storage overload retains
its behavior.

Original 41E87A/41E880 zero both fields in the CALLEE before strlen. The source
therefore does not require a zeroed caller header and preserves source/header
aliasing at that point. It calls concrete raw41DD40 with preserve=true, then
reloads destination data and current length+1 before copying if data is nonnull.
Calling this constructor on an already owning header abandons the old pointer,
as native does. Empty text produces a zero/null header. No local unwind state
or implicit destructor is added.

## Evidence and validation

`reports/native_particle_type_preparation_orch4.json` contains full disk/live
bytes and hashes for all eight bodies (341 bytes), EH action/handler/map/info,
the six vtable slots and original name bytes. The repository client verifies
the existing `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe` before live queries. Original B075D0 and 41E870
comments/names are preserved. The six preparation targets initially had no
Ghidra function entries; the parent independently checked, defined, saved and
exported them. This worker made no Ghidra mutations.

The local original/source probe uses the complete eight original bodies with
external CALL displacements and three original constant-name pointers rebound
to the same concrete raw pool/resize and memory-copy providers. It covers
constructor initialization, empty text and text/header aliasing, normal consumed
name cleanup, actual destination/argument aliasing, all three material names,
complete pointer-normalized Layer bytes and untouched borrowed type/parent
bytes, and RET leaves with an unreadable receiver. It also checks the typed
raw-pool overload. No permanent test suite is added.

Final build, probe, exact call-row and index receipts are recorded in the report.
These are owning source interfaces with C++ cleanup semantics, not original
thiscall/FH3 binary entry points. The normal-path probe does not execute original
FH3/SEH handlers, OOM/fault paths, or actual pool-getter exceptions; those cleanup
states are source-inspected. Game and arbitrary ABI parity remain unvalidated.
