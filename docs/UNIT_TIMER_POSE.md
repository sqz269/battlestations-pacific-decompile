# Timed unit damage point binding

This closes the position flow inside the already reconstructed `00956600`
timer update and reconstructs the complete wrappers `0042D7E0` and `00414D10`.
The timer now requires the actual unit's borrowed `PoseRefreshView`, uses the
existing canonical refresh and affine-point routines, and passes the captured
world XYZ explicitly through both announcement and effect host boundaries.
`UnitTimerState` no longer owns a duplicate `pose_valid` flag.

## Native evidence

Analysis used `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`;
`python tools/bsp.py ghidra count` verified the target before live batches.
The snapshot counted 63097 total functions; later live reads counted 63100 as
other workers completed unrelated function creation. This packet made no Ghidra
mutations. Raw wrapper exports are in shared ignored `exports/bsp/functions/`.

| Address | Original contract and complete extent | Reconstruction |
| --- | --- | --- |
| `0042D7E0` | ECX owner; EAX owner+CCh; no stack args. Eight instructions, 25 bytes, final RET at `0042D7F8` (one byte). | Test the owner's actual byte C8; call `00414DB0` only for zero; return the existing world matrix reference. Any nonzero byte skips refresh. |
| `00414D10` | ECX output XYZ, EDX input XYZ, stack matrix; EAX output; RET4. Nineteen instructions, 64 bytes, final RET4 at `00414D4D` (three bytes), inclusive end `00414D4F`. | Reserve disjoint three-float scratch, call canonical `004142E0`, then copy X/Y/Z to output and return it. |
| `00956600` fragment | The enclosing routine remains ECX unit, stack scaled delta, RET4. Point selection/capture `009567A3..0095682D`; downstream pointer uses at `0095686B..00956870` and `009568AD..009568D5`. | One captured XYZ per fired record, carried explicitly to both host services. |

The small wrapper at `00414D10` has a hidden EDX source in its decompile.
Assembly establishes the register arguments, the `004142E0` call, all three
scratch loads/output stores, and return pointer. Its output may overlap its
source or matrix because the canonical kernel consumes both inputs before the
wrapper writes the output. The public wrapper uses three writable `float`
elements for output so matrix overlap does not require an invalid array cast.
It adds no math, default matrix, perspective divide, or validation policy.

For a crossed damage threshold, `009567A3..009567AE` checks the signed anchor
index against the previously captured descriptor count and zero. In range,
`009567B0..009567C0` first resolves the actual descriptor+28h array element
(12-byte stride), `009567C3` obtains the refreshed unit world matrix, and
`009567CF` transforms the anchor. `009567D4..009567E9` snapshots its XYZ.
The lookup returns a borrowed source reference before refresh; the coordinates
are read by the transform after refresh, matching the native pointer order.

Out of range, `009567F0..009567FA` checks the actual unit/ESI C8 byte and
conditionally refreshes that same owner. `009567FF..0095681B` then reads actual
unit+FC/+100/+104, which are world matrix CC indices 12/13/14. Both paths join
at `00956823..0095682D`, completing the same stack XYZ. The C++ local is a
snapshot, so later host changes to the pose do not replace it.

Behind the nonnegative announce id, native vtable+34 is called at `0095685C`,
then `0095686B..00956870` passes the captured point address to `0049C940`.
Read-only inspection confirms that function forwards its first stack argument
to `0049C000`; the latter copies three words from that point into its records.
The existing announcement label is descriptive, not a recovered symbol or a
complete classification of those objects.

For an effect template, `009568AD..009568B1` passes the same stack point and
`009568AA..009568AC` pushes zero options before the ownership chain ending in
`008689C0` at `009568D5`. Read-only inspection of `008689C0` shows the supplied
XYZ copied into globals F87640/F87644/F87648; the conditional scene transform
is gated off by the timer's zero transform flag. The consumer must not silently
transform the already-world-space point a second time.

## Required bindings and limits

`unit_update_timers_00956600` now requires `PoseRefreshView& actual_pose`,
borrowing this same unit's +3C/+74/+C8/+CC/+10C storage and canonical parent
resolver. No copied hierarchy, flag, or matrix is introduced. The caller owns
the view and backing fields and must keep them alive throughout the update.

`UnitTimerHost::descriptor_anchor` remains an explicit pure lookup into the
actual descriptor. It must return existing source storage that survives pose
refresh; it must not fabricate a point, refresh a pose itself, allocate a new
anchor, or mutate the owner. The actual transform and fallback refresh are
reconstructed locally, not supplied by a success-assuming host callback.

Descriptor record/count lookup and record reload behavior, vtable+34 output,
the manager reached through E188A8+21D0, effect allocation, handles/reference
counts, scene owner+4A4, and `00440490/008689C0/004845D0/00440A30` lifetimes
remain required existing host bindings. Both downstream methods receive the
captured XYZ by reference for the synchronous call. They must copy it if native
ownership needs retained coordinates; retaining this stack reference is invalid.
This packet does not reconstruct those owners or claim completion of all of
`00956600`, unrelated timer updates, or the full unit object constructor.

## Verification

`./scripts/build.ps1` passed with MSVC Win32 `/W4 /WX`, followed by both existing
CTests (`reconstructed_math`, `native_math_differential`). This does not mean
the newly closed point branch has a native differential test.

The one focused, ignored `local/unit_timer_pose_fixture.cpp` also passed using
the built core library, `/W4 /WX /fp:strict`, and `/link /MANIFEST:EMBED`.
It checks borrowed lookup-before-refresh, a valid anchor transform, dirty
negative-index fallback, already-valid positive out-of-range fallback, captured
XYZ stability across host mutation, actual byte/cache updates and returned matrix
identity, plus input/output and matrix/output aliasing in `00414D10`.
Command and log paths and the fixture source hash are in
`reports/unit_timer_pose.json`. No permanent test suite was added.

These are exported, reconstructed, build-tested and focused-fixture-tested
interfaces. They are not original object/ECX ABI replacements and have not been
game-validated. The original installation and game process were not modified.
