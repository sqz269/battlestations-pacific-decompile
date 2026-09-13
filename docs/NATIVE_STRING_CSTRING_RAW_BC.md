# Raw-pool CString constructor composition

Address: 0041e870

The existing complete CString constructor gains an overload taking the
application's `NativeStringRawPoolContext`. The typed `NativeStringStorage&`
overload and unrelated physical-file routines remain unchanged. This extends
one existing reconstructed function record; it adds no newly reconstructed
native body. Preserve the established `BSP_NativeString_Assign` analysis name
until the integrator reviews annotations; existing evidence already explains
that the native body constructs a header rather than assigning a live string.

The complete native body is 41E870..41E8C2, 83 bytes. ECX is the actual
eight-byte destination header, one stacked nonnull NUL-terminated text pointer
is consumed by RET4, and EAX returns the saved destination. The source context
is additional C++ input, not a recovered original ABI argument.

Native order is retained:

1. Capture the source pointer, clear destination length then data pointer.
   A source alias into the header observes those stores during the scan.
2. Scan current bytes until NUL using DWORD-wrapping pointer arithmetic.
   There is no nullable-source fallback and no early snapshot of source text.
3. Call raw 41DD40 with the captured scan length and preserve1. Empty text
   matches the freshly cleared zero length, so resize skips the pointer and
   pool getter entirely.
4. Read the current destination data pointer. If nonnull, read current
   length+1 with DWORD wrap and copy that many current source bytes using
   overlap-safe memmove. Return the saved destination.

The new overload uses the existing byte scan and volatile raw header helpers,
and the existing complete raw-context resize overload. That resize resolves
the actual current 419CC0 pool before allocation or each nonnull old-block
return. Its getter exceptions escape. The context borrows actual publication
01090AA8, small-return gate 01090AA4 and manager publication 01090AA0; no new
pool owner, resolver, allocator callback or catch policy is introduced.

Full 41DD40 reads current header state after allocation, preserves the smaller
of requested/current lengths, resolves the current pool for a nonnull old
return, then writes the allocated pointer, requested length and terminator in
that order. Header/data aliases can change the values seen by 41E870 after
resize; the constructor does not reuse earlier pointer or length snapshots.
The native BF7680 library body contains the backward overlap path. Its
established `_memcpy` name is retained; the C++ composition uses `memmove` for
that behavior. As in the existing typed implementation, a wrapped zero copy
count omits the otherwise no-access C++ library call.

The complete live caller and xref inventories are preserved externally, along
with PE instruction-target validation for every direct E8/E9 transfer. This
inventory is broader than a proof of every caller's argument preparation,
ownership or lifetime. The cockpit-camera use is reviewed explicitly:
B3C863 pushes D6185C (`CockpitCamera`), B3C868 forms ECX from its temporary
header, and B3C86C calls this constructor. B3C886 passes that same header to
the camera constructor. Its later cleanup and enclosing holder remain
separate reconstruction work.

Worker evidence, exact counts, compiler/build checks and the one bounded
external fixture are recorded in `reports/native_string_cstring_raw_bc.json`.
The original parent bytes compose with the existing full raw resize and
current CRT memmove providers, so the fixture does not independently certify
all original descendant bodies. No new repository tests are added.

Private native stack aliases, unrestricted FH3/SEH, concurrent mutation, exact
entry-point compatibility and gameplay remain unvalidated. This overload is
in an already registered module; the worker baseline build includes it.
Final integration still requires an exact combined build and replay against
the integrator's current libraries.

Worker verification passed strict Win32 compilation, eight native seed
comparisons, the baseline build, both existing CTests and all three numeric
call rows. The default external recipe compiles only its probe and links the
selected checkout's three current libraries. It passed eight original/source
pairs comparing 11,396 normalized state bytes and six event DWORDs. Its
2,356 source/header/fixture/library input hashes remained unchanged.

The fixture covers empty and header-aliased text, nonempty cockpit text,
backward overlap, a terminator alias that changes the current copy count,
and real current getter validation that returns or throws after mutation.
For source starting two bytes before the cleared header, the scan captures
length two, then the final copy observes `A B 02`: resize has since stored
that length into the aliased source byte. The initial fixture expected a
NUL there; its failed logs are retained, and only that expectation changed.
The native and reconstructed implementations agree on this alias behavior.

The external immutable worker capture includes native bytes and listings,
the complete caller inventory and direct-transfer validation, the probe and
recipes, logs and comparison artifacts, all pinned repository inputs, and
the three exact worker libraries. Its archived report snapshot necessarily
precedes adding the archive's hash to the tracked report. Source/header
hashes identify the tested uncommitted worker build; the final combined
commit still requires the integrator's build and current-library replay.
