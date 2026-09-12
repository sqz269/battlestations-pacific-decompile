# Main-menu commands, Listbox state and dynamic Group spheres

Addresses: `005993A0`, `00530670`, `00530C20`, `005885D0`, `005886C0`,
`00941250`, `00A9AC30`, `00A9AC40`, `00A9AC90`, `00A9BE00`, `00A9BE60`,
`00A9C050`, `00A9C0A0`, `00A9C220`, `00A9C740`, `00A9C7C0`, `00A9C920`,
`00A9C990`, `00A9CCE0`, `00A9CD20`, `00A9D750`, `00A9DF40`, `00AA78F0`,
`00AA7D00`, `00B8E980`, `00B8E9F0`, `00B8EBE0`, `00B8F100`.

This batch joins three independently reviewed worker packets to the existing
canonical GUI, scene and menu owners. Source resides on the orchestrator's own
worktree; the final integration and exact tested code commit are recorded in
`reports/orch5_menu_listbox_group_batch.json`.

`5993A0` now has its complete normal caller sequence, including the unconditional
alpha write before a fresh widget-type query, Text low-byte decisions, separate
Listbox ordinal and row-D8 reads, and callback-sensitive reloads. The selected
widget callback at `598B60` has two stack arguments: the selected row and the
original Listbox. Native prompt/checkpoint string lifetimes are preserved. Two
independent assembly reviews found no remaining concrete sequencing issue.
Required page/profile/map providers are still explicit boundaries. This is not
a claim that the whole main menu or its listener wiring can run yet.

The new Listbox companion owns its one FC row list and borrows actual GUI row
owners. Selection, forced refresh, listener changes, sound-request production,
loaded78 and layout7C use that same storage. The append path of `A9D750` attaches
the actual owning layout allocation; `A9BE60` removes all matching FC identities
without deleting or detaching the widget. Properties `A9E400`, derived frame40
`A9D030`, non-null insertion positions and native scalar/copy teardown remain
open. Horizontal layout needs an actual Text companion for its measured width.

The type11 factory creates the real Listbox companion, with the proven bare-RET
constructed74. Current34 is supported while highlight index118 stays -1;
current60 is supported when active is true or auto-control11E stays zero.
Unsupported tails fail explicitly. Current38/3C and alpha/color/type use their
verified base slots. The inherited current64/70 allowlists still reject Listbox;
their table evidence alone does not imply completed frame/clip integration.

Review caught two host lifetime issues before promotion. An adapter operation
now spans current34/60 base callbacks and post-call checks, allowing native
reentry while preventing retirement of the active owner. Pure preflight checks
the entire retained subtree before the first scene-node release, so a nonempty
Listbox or active descendant is rejected before destructive work. Derived
teardown still follows the original logical-release ordering. These checks are
host safety boundaries, not recovered native side effects or a native destructor.

The prompt helpers select the first/last navigation row using the same screen
input-mode264 gate and current navigation widget24. Tactical-library requests
use actual existing manager/table/payload storage and pure borrowed screen-field
views. The corrected field mapping is mission pointer at9C, side index atA0.
`5885D0` stores selection and mode before requesting; `5886C0` requests first and
reloads the manager after payload callbacks. See `FRONTEND_PROMPT_NAVIGATION.md`
and `MAIN_MENU_TACTICAL_LIBRARY.md` for ABI and supported-domain evidence.

Dynamic Group sphere misses now invoke `B8EBE0` on the same actual Group owner
and physical178/17C attachment array. Before the first qualifying seed, children
with no low two flag bits are skipped; every trailing child is merged. `B8F100`
treats either cache bit as a hit, and ORs30 after aggregation without re-reading
mode175. Child48 dispatch is reloaded for each call. Live half and CRT inputs
are required only when a merge is reached. Static affine spheres keep their
previous finite-input and supported x87-control-word boundary.

The worker numerical fixture compared 93,600 extracted-native/reconstructed
results across x87 precision/rounding, overlap, NaN, infinity and subnormal cases.
It shares the already recovered length/CRT dependency, and excludes unmasked
exceptions, x87 condition-code and MXCSR equivalence. Actual-owner fixtures cover
Group attachment/cache behavior and Listbox callback/layout/retirement behavior;
the tactical fixture checks payload ordering and manager rebinding. Final build,
fixture logs, hashes and mechanical call totals belong to the batch report.
None of these is binary replacement, rendered-screen or gameplay validation.

## Follow-up packets

- Main-menu listener current08 `5966F0` is the actual Listbox listener target in
  CEFC48, distinct from screen+40 command current04 `5993A0`. Recover its caller
  contract and same-screen binding before claiming menu/listbox wiring complete.
- Implement concrete providers reached by `5993A0`, especially `584F50`,
  `594BF0`, `584750` and the profile/map/page dependencies, in bounded leases.
- Continue Listbox properties/frame/navigation, `A9BA90` automatic row state,
  linked highlight page `AA0F50`, non-null insertion and native teardown.
  Current64/70 integration needs the established owner lifetime/clip contract.
- Compose prompt navigation hosts and tactical-library screen ownership with
  the actual menu construction/bind-layout path. Borrowed field views do not
  establish a complete screen constructor or native record ABI.
- Continue other current48 child profiles and the complete particle/Text
  spatial provider. Dynamic Group completion does not close those dependencies.

## Promoted validation

Source batch `b8e8d6b4` was integrated with concurrent main and built at `02ea409f780e36b79b37c55ccc71e57cbe759179`: both existing tests passed. The tool promoted `666f524ba793171851fd58d81d649f2eb1a7a763` after a second main update; the six additional files were documentation only. Compiled inputs were identical. All three scoped probes were rerun against that integrated library and passed. The report pins both commits and hashes; this evidence does not apply automatically to future main changes.
