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

Integration prerequisite before the first binding: the startup host currently resets the page
application before calling the manager's `after_singleton_drain()`. The integrator must move
that acknowledgement ahead of page-application reset, while keeping manager reset after it.
The token deliberately requires that explicit completed-drain acknowledgement rather than
inferring completion from the cleared publication alone. Until P3a2 binds a token, the existing
unbound destruction order remains valid.

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
and `git diff --check` are complete; the dependency build is pending. No new game run or native
page lifecycle acceptance is claimed.
