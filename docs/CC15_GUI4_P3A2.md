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

## Remaining AC4C50 inputs

AAA710 requires shared mutable white/black guards and quartets at F8BCF0/F8BCE0 and
F8BCDC/F8BCCC, plus AA7220's E12FC4 Y-scale. These differ from Text's F8BE38 quartet and must not
borrow it. Their installed DATA and lazy initialization behavior are being recovered before the
property bindings are supplied. `properties_composed` stays false until the actual reader,
widget transform/visibility/children, canonical Model/Group, and retirement providers all exist.
