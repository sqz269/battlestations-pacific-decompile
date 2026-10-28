# GUI P3a2 provider prerequisites

P3a1 composes AA5840 through the page tail but rejects a miss before native construction while
`properties_composed` is false. P3a2 must compose the real AC4C50 -> warm AC59A0 -> AAA710 chain,
initially for a Screen/root with no children. A1's startup scene domain supplies the shared
scene/camera/light/fog inputs. Shader preload and Text preparation stay in their existing phases.

The page-chain switch remains OFF. This packet does not call the page loader, change bridge
ownership, or claim native page runtime/lifecycle acceptance.

## Warm-manager page lifetime binding

The manager previously fixed its private lifetime context to `NoPageOwner`; populated AA6340
could only reject at page virtual +20 or +04. The application now exposes
`bind_page_lifetime_host`, returning a unique `PageLifetimeBinding` token over one borrowed
`NativeGuiPageLifetimeHost`. The provider must dispatch each CURRENT page call separately;
AA6340 still reloads native vector bounds and the page between the two calls. No combined
host-tree retirement replaces that schedule.

Binding is permitted once, in the manager's published phase, with the constructor-initialized
page-vector begin/end/capacity all null. These are manager+18/+1C/+20, relative to the vector at
+14; +14 is its opaque allocator word, not the begin pointer. Nonempty or previously allocated
vectors, repeated binding, self-delegation, and binding before publication or after drain reject
before changing any field. Token allocation precedes callback publication.

The binding adds no native owner/reference, changes no manager field or vtable, and performs no
native call. Unbound dispatch preserves the original two `NoPageOwner` exceptions. No current
host creates a token, so the existing warm-manager path stays unbound.

The provider is declared before its retained token and survives shared native drain plus
`after_singleton_drain()`. Token destruction before that acknowledgement terminates; it cannot
silently unbind live callbacks. The manager also terminates if destroyed while a token survives.
After acknowledged drain, token destruction clears only the host callback/token pointers.
The used-binding marker never resets, and there is no replacement/unbind API for a live manager.
As with the repository's other borrowed C++ contexts, callers must retain the provider object
itself for the full token lifetime; no extra owning pointer is introduced.

Required host ordering before the first binding: after shared native drain, call the manager's
`after_singleton_drain()` ahead of page-application reset, and keep manager reset after it.
The token deliberately requires that explicit completed-drain acknowledgement rather than
inferring completion from the cleared publication alone. Until P3a2 binds a token, the existing
unbound destruction order remains valid.

Construct all providers, retained frames, allocations and throwing startup logs before binding.
Binding must be the last throwing construction step, or the complete bound graph must survive a
later constructor failure. Ordinary stack unwind must not drop the token before native drain.

Evidence: `src/native_platform_focus_owners.cpp` AA5D70 initializes +18/+1C/+20;
`src/native_gui_media_focus_lifetime.cpp` AA6340 reads begin at +18, current end at +1C, and calls
the page +20 and +04 entries separately. This is an application C++ binding, not a recovered
Original type or binary ABI.

Validation at this commit: source inspection and `git diff --check`; build and runtime gates
pending. No game runs were started. An empty-vector default run can verify inertness but cannot
exercise the new delegated page retirement; that proof requires later P3a2/P4 activation.

## Canonical base-widget property state

AAA710 requires shared mutable white/black guards and quartets at F8BCF0/F8BCE0 and
F8BCDC/F8BCCC, plus AA7220's E12FC4 Y-scale. These differ from Text's F8BE38 quartet and must not
borrow it. The existing retained `GameNativeGuiTextState` now supplies distinct VA-named views of
these cells; no additional process owner or recovered/native type is invented. Its original
Text fields and accessors keep their existing identities. E12FC4 and E12FD4 remain two separate
0.75 cells. The backing views do not perform lazy color initialization or change any guard.

Installed image: `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The PE image base is 00400000. Its .data section has raw size 10000h and virtual size 297EDCh.
F8BCCC..F8BCF3 lie at section offset 183CCCh, beyond raw data, so both quartets and both DWORD
guards begin as loader positive zero. E12FC4 lies at section offset AFC4h, with exact disk bytes
`00 00 40 3F` (float bits 3F400000). Live Ghidra bytes agree with these values. The configured
project was `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, checked before the
read-only DATA/listing batch; no Ghidra annotations changed.

The listing establishes the lazy behavior the existing raw property source must retain:

- AAA8F2 tests bit0 of the low byte of F8BCF0; AAA8FB loads the live D7A24C one; AAA903 ORs the
  complete DWORD guard with 1; AAA90A/912/91A/922 store the four white lanes in order.
- AAA975 tests bit0 of F8BCDC; AAA981 ORs its full DWORD with 1; AAA988/990/998 write three
  positive-zero lanes; AAA9A0 loads live D7A24C; AAA9A8 writes alpha.
- AAA9FD repeats the CURRENT white guard test for HighColor, after intervening property calls.
  A prior white result must not be cached across those callbacks.
- AA7239 loads E12FC4 with FLD; the host state exposes its same mutable float cell rather than
  substituting the bridge's `kGuiAspectYScale` constant.

The raw `native_gui_widget_property_prefix.cpp` still owns these guard tests and stores; this
dependency only supplies their shared preimages. AAAED0's future provider must borrow the same
base-widget views. No alternate white quartet, eager initialization, or per-page reset is added.

## Remaining AC4C50 inputs and validation

`properties_composed` stays false until the actual reader, widget transform/visibility/children,
canonical Model/Group, and retirement providers all exist. Canonical state and manager binding
are prerequisites, not proof that AC4C50 or a native Page has executed. Source/data/listing review
and `git diff --check` are complete. The combined dependency build at 590CC866D passed
`scripts/build.ps1` and both existing CTests (2/2); receipt:
`local/codex_gui_dependencies_build.log`. No new game run or native page lifecycle acceptance
is claimed.

## Qualified raw field resolver adapter

`NativeLuaFieldResolverAdapter` in `object_handle_resolvers.hpp/.cpp` supplies the two real
fastcall cell types required by `NativeLuaFieldValueBindings`. It is currently uncalled.
It borrows the startup host's installed `ObjectHandleResolverSlots` and canonical
`ObjectHandleTables` aliases, not copied table counters/pointers or another object registry.
The canonical tables are available from `GameNativeEntityRegistryProcess::handle_tables()`.

Native/source signatures and admission:

- 006AD080 receives the full 32-bit handle in ECX, returns pointer bits in EAX, and executes RET.
  Its source callback takes `(uint32_t, const ObjectHandleTables&) -> void*`. The fastcall thunk
  preserves the full input bits, reads the CURRENT typed source cell once per invocation, calls
  that signature directly, and returns all pointer bits. There is no function-pointer cast.
- 00888AA0 receives an actual 14h LuaObject in ECX, returns userdata bits in EAX, and executes RET.
  The typed source callback takes `(GuiLuaHost&, GuiLuaRef) -> void*`. The adapter reads its
  CURRENT cell, admits the installed `object_from_lua_table_00888aa0` target only, and composes
  that body's raw-storage specialization: B67800 at 888AC3; B662D0/lua_touserdata at 888AD2;
  capture the pointer before B67700 at 888AE5. No GuiLuaRef or registry-handle projection is used.
  A different table target rejects and is outside this qualified binding domain.
- BD4FC0 stores ECX/EDX/stack callbacks to 0109CED4/CED8/CEDC and returns RET4. Native 006AD0D0
  installs 6AD080/888AA0/6AD0C0. The startup host remains the source publication owner; the
  adapter never changes those source slots or publishes a callable pointer at an Original VA.

The adapter owns two distinct fixed fastcall thunk cells. These are Source ABI bindings, not
Original DATA. A scoped activation selects the borrowed context, with nested scopes restoring
their predecessor on normal/C++ return. TLS points to the caller-retained invocation frame,
not the automatic scope object. That frame owns the exact raw lookup temporary; callbacks retain
their CURRENT target diagnostics and result bits. Complete frames may be reused; entered or
failed frames cannot be replayed or destroyed. Callback failures latch adapter retention.

A Lua nonlocal transfer may bypass the scope destructor. The retained active frame, adapter
count and TLS link must survive; the caller must follow the existing sticky failure/_Exit
policy and never resume, silently clear TLS, or drain borrowed dependencies. This contract does
not qualify native FH3/SEH, Lua nonlocal unwind recovery, arbitrary replacement table callbacks,
binary detours, or native Page/game execution.

The exact native signatures and order were checked in the live listing after verifying the bsp
target/program. The adapter and captured-return strengthening at 76FB1EB96 passed
`scripts/build.ps1` and the two existing CTests (2/2):
`local/codex_gui_raw_resolvers_capture_build.log`.

One focused local Source fixture passed at that clean commit:
`local/codex_gui_resolver_fixture_76fb1eb96/report.json`. It pins source/library/executable hashes
and links an MSVC Win32 probe with an embedded manifest. It checks full-word zero before low16
truncation, high ECX/EAX bits, CURRENT source callback replacement, two distinct canonical table
views, nested normal/C++ scope restoration, real Lua 5.1.1 table-to-userdata lookup on actual
NativeLuaObjectStorage, reuse after completed temporary destruction, balanced Lua stack/tracked
references, and final adapter quiescence. The return uses a captured local across cleanup;
diagnostic metadata is refreshed from that capture and cannot substitute its return value.

The fixture executes Source bodies and linked Lua, not Original executable bodies. It does not
prove a binary replacement ABI, native FH3/SEH/nonlocal-unwind recovery, Page execution, or game
parity. No game run or new tracked test suite was added. `properties_composed` remains false.
