# Native viewport host identity registry

Addresses: `00B1F850`, `00B1F8F0`, `00BD30E0`. Host association support for the
reviewed design in `docs/NATIVE_COCKPIT_VIEWPORT_LIFETIME_BH.md`; zero new native
bodies or bytes. The native viewport remains 34h with its original count at +04.

`NativeViewportRegistry` implements the existing `CameraViewportResolver` using
caller-owned intrusive records. Each persistent cockpit construction block will
provide two separate `Storage` objects, one per viewport construction. The new
registry does not yet implement that block or wire the camera/helper producers.
One `NativeViewportRegistryBinding` installs a borrowed registry before any
managed admission; all construction runtimes must use that same registry.
Competing installations, including a nested installation of the same registry,
are rejected. The native owner receives no host pointer or additional count.

The record progresses from unused through reserved to live, then retired.
Cancellation instead changes a reserved record to cancelled. A move-only token
identifies the registry and reserved record; move assignment cancels its previous
credit, and self-move does nothing. Valid registry operations use no allocator
or user callback. Diagnostics before mutation may allocate an exception.

After successful native construction, `constructed` directly emplaces a
`CameraViewport` over the same actual owner in its record. Copy construction is
never used: it would create a detached diagnostic viewport. The association
checks the supported D5E5F8 profile, positive actual count, reserved token and
unique live address. This is a host contract, not arbitrary-profile dispatch.
The admitted allocation overload first validates the token, then moves it into
a local guard before calling the existing native allocator/constructor. Renderer
callbacks cannot consume the caller's now-empty token. Construction failure
keeps the original native cleanup and cancels the unused record; the caller
subsequently forgets that record. Registration succeeds without allocation
before the overload returns the owner to its publisher.

The registry's single intrusive list includes live and inert records. Lookup
compares integer identities only and returns a view only for a live record;
it never reads the native owner. The common source deleting-destructor entry
`delete_native_viewport_owner_00b1f8f0` retires any exact registered identity
before either native profile store, typed lifetime end or physical free.
The existing flags0 path also retires it. Unregistered diagnostic owners have
nothing to retire. Native count operations, profile stores, free selection and
the captured returned address are unchanged. The hook is host bookkeeping,
not a recovered call added to the native listing.

A retired view object stays alive at its original address while its native
references become unusable. `forget_quiescent` destroys this inert host object
and unlinks its record only after the caller establishes that every semantic
frame call and cached `viewport()` borrow has ended. It never reads the dead
owner, clears renderer+1904, retains an owner, or delays native free. Reuse of
the same actual address with a different unused record resolves the new view;
the old borrowed view is never rebound. Forgetting live/reserved records or
destroying unforgotten storage terminates as a host lifetime-contract violation.
Registry binding teardown requires an empty record list and host quiescence.

## Evidence and verification boundary

The integrator reread the complete live B1F8F0 listing through target-verified
BSP CLI: B1F8F0..B1F913, 36 bytes, ECX owner, stacked flags, EAX original,
RET4. Native calls are BD30F0 at B1F8F9 and BF65AC at B1F906, followed by
ADD ESP,4 at B1F90B. The approved design retains the separately reviewed
construction, publication, renderer-borrow and native-reference evidence.
No new native body or original ABI compatibility follows from this host API.

Build and focused source fixture results are pending. The intended external
fixture checks stable views across flags0 destruction and same-address reuse,
nonterminal/terminal count transitions, token cancellation and explicit host
quiescence. Its manually prepared owner is fixture setup, not producer or
original-binary evidence. Existing native differential checks remain separate.

Mutations are serialized and all storage/runtime addresses are stable. Callbacks
between registry operations may use other prepared records. This does not
establish a production scheduler contract for terminal destruction during a
renderer field read. Late native constructor failure may preserve a published
viewport; its future construction block must stay alive until actual terminal
retirement and host quiescence. Camera/helper producer composition, native EH,
native renderer execution and game validation remain open.
