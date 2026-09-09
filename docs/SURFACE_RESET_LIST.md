# Borrowed surface reset list

D3D9SurfaceRegistry projects renderer fields+1B0Ch/+1B10h/+1B14h into a borrowed
pointer vector. Explicit append corresponds to the registration block in
00b2a7c0; it is not an implementation of that complete render-target factory.
The list neither retains nor releases wrappers. Owners must unregister before
destruction and keep every registered binding alive through traversal.

Removal00b25630 takes ECX=array, a pointer to the target pointer on the stack,
returns found in AL and RET4. It searches the first matching pointer, replaces
that entry with the last unless already last, then decrements count. Missing
entries leave the list unchanged. It does not release the removed surface and
does not preserve ordering. Renderer forwarding helper00b27d60 selects+1B0Ch;
it is annotated but not counted as another C++ routine. The typed method uses
the target pointer directly and requires a valid non-overflowing list state.
Native spare-slot contents and allocator/capacity behavior are not modeled.

The concrete surface callback fragments call existing00b3d510 release and
00b3d550 recreation helpers in list order. Recreation proceeds through all entries
and reports the first failed HRESULT; native ignores HRESULT. Existing helper
preconditions remain: recreation expects the previous COM reference released.
The class does not perform device Reset or unbind/default-owner/listener work.

## Traversal and ownership limits

Native loops retain a raw cursor but reload array base/count after each virtual
call, comparing cursor against the resulting end pointer. They are not safe
against arbitrary removal, reallocation or destruction during callbacks. The
typed supported domain requires stable storage, membership and live unique
bindings throughout traversal; it does not substitute a robust mutation-safe
enumerator for the native loop. Append/removal themselves retain native duplicate
and first-match behavior, but factory-created valid reset lists contain distinct
wrapper identities.

SURFACE_RESET_TRAVERSAL documents full release/restore gates and order. In
particular, release handles default depth before four render-target owner slots.
Restore initializes existing default wrappers and does not rerun initial capture
or its final depth bind. Generic resource lists and listeners also participate.
These are still required before this list can form a complete renderer lifecycle.

SURFACE_REGISTRATION_AUDIT establishes that offscreen factories explicitly append
while default capture does not. The support singleton getter is not per-surface
registration. Native wrapper destructors remove themselves; typed owners currently
call removal explicitly before their cleanup. No fake native factory or destructor
registration hook has been introduced.

## Evidence and validation

The removal/helper/append bytes are retained as hashes in
`reports/surface_registry_audit.json`; full traversal hashes are in the independent
handoff. All live batches verified `bsp`, `/battlestationspacific.exe`. Five
function names/comments were added or updated with prior annotations preserved,
the project saved, and exports refreshed. Only removal is whole-routine coverage;
append and traversal remain fragments.

The existing offscreen reset probe now traverses this list to release two surfaces,
resets the real D3D9 device, and recreates both. It checks their descriptions and
then removes the first entry, verifies swap-with-last and a missing-entry result,
and removes the final entry while COM surfaces remain owned. It explicitly
unregisters before surface cleanup. Both existing CTests, the Win32 build and
all renderer/material/installed-asset checks passed. Results are in
`reports/surface_registry_probe.txt`. No new test target was added; no native
execution differential, full reset or gameplay equivalence is claimed.
