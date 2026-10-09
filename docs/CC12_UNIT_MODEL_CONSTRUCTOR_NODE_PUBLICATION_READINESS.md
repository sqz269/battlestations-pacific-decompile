# Unit-part constructor and model-root publication readiness

Root reviewed this bounded packet and replayed its retained Source/capture pins. It adds no implementation or new runtime credit. The current Root build authority is the separate 509-input primary review; historical Source121 and reported Source507 context remain scoped to their captures.

`007135C0` already has a complete Source constructor. This review identifies
no new Source-ready ownership composition within that one-body boundary.
The constructor receives its selected set, stores it at `model+160`, and
uses its existing `+0C` node. Genuine production `unit+360 -> model+160 ->
holder+0C` ownership remains held.

## Whole Native gate

The running Ghidra process names `C:/Users/sqz269/bsp.gpr`; the connected
program is `/battlestationspacific.exe`, image base `00400000`, from the
original installation. `007135C0..00713723` is **356 bytes / 99 instructions**.
Original PE bytes equal fresh live bytes; all 99 saved/live listing rows and
independent instruction starts agree. SHA-256:
`61cc4457032508218c120f796ae7eaa238e73f27e24162d91e69487a0e414a96`.

Assembly establishes ECX=fresh model storage, stack `(unit, selected_set)`,
EAX=the same model storage, and `RET 8`. It preserves EBX/EBP/ESI/EDI and uses
an FS exception frame with handler token `00C84B64`. The live prototype's
zero parameters do not describe this ABI. There are **13 call instructions:
12 direct, one indirect**, covering 11 unique direct targets. Ghidra's reported
count 12 matches the direct sites; the full listing also has the indirect call.

| Site | Target / whole required operation |
| --- | --- |
| 007135E8 | 004E6480 collision-base initialization |
| 0071362F | 007103A0 first circular-list sentinel |
| 0071364D, 00713665 | 007103C0 second and third sentinels |
| 00713689 | 00713380 damage-group construction |
| 00713699 | Current original unit virtual +10, ECX=unit, C-string result |
| 007136A0 | 0041E870 construct temporary pooled name, RET4 |
| 007136BA | 00B6F960 assign current holder's actual node name, RET4 |
| 007136D7 | 00419CC0 pool singleton getter; three pushed release arguments survive |
| 007136DE | 00BD1510 pool return, ECX=pool, `(data,length+1,1)`, RET0C |
| 007136F2 | 00712440 collision-shape construction |
| 00713701 | 00710AD0 spatial attachment when current +190 is nonzero |
| 00713708 | 00711C60 part-entry construction |

Only this constructor received a fresh Native gate. Callee contracts use
existing Source and prior evidence; no child Native bodies or unwind handlers
were expanded. Descriptive names remain hypotheses.

## What this body publishes

At `00713604`, the second stack argument is written directly to `model+160`.
The constructor does not retain it at that store. It writes unit aliases at
`+4C/+164`, initializes group/list/entry headers and calls the whole providers
above. After group construction, a nonnull original unit supplies virtual +10.
After temporary name construction, `007136A5` reloads current `model+160` and
`007136AB` reads its current `+0C` node before the name setter. Provider-driven
rebindings therefore cannot be replaced with the initial selected-set value.
`007136E5..EB` similarly reloads `+164` into `+4C`.

There is no own store to `unit+360` or `holder+0C`, no selected-set allocation,
and no resource parse/selection call in these 356 bytes. Opaque callee effects
are not excluded by that own-instruction conclusion. The node-name setter
consumes a node already supplied by the selected set.

## Existing Source and ownership boundaries

`src/native_unit_part_construction.cpp:61` preserves the exact caller schedule,
late set/owner reads and explicit C++ cleanup states. Concrete defaults provide
sentinels, groups, node-name assignment, entries and member cleanup. The
storage/attachment subclasses add actual base, collision and spatial providers.
Unit-name dispatch, current root access and selected-set cleanup composition
remain inherited required bindings. The latter has an existing actual release
helper and resource-instance terminal callbacks; an absent binding does not
mean the whole underlying operation is missing.

The older constructor document's statement that no node-name setter exists is
stale: `src/native_unit_part_entries.cpp:99` implements it and the constructor
default calls it. Its historical fixture and Native-EH limits remain separate.

| Chain stage | Existing Source evidence | Remaining production obligation |
| --- | --- | --- |
| `unit+360` | `native_unit_health_parts.cpp:115,123` calls required constructor binding and publishes its result | Genuine unit/class storage, current resource selector and concrete construction/lifetime composition |
| `model+160` | This Native gate and `native_unit_part_construction.cpp:73` store selected set; line 100 reloads it | Actual transferred instance identity and completed-provider domain |
| `holder+0C` | `native_resource_graph_builder.cpp:126` publishes its constructed root; 007137F0 Source wrapper reaches that builder | Establish that this is the actual current resource virtual +8 path and retain its real resource/node tree |
| Holder lifetime | Resource-instance constructors retain resource+8 but leave root+0C untouched; existing graph phase and destruction contracts are explicit | Do not publish or destroy an incompletely constructed graph as a completed selected set |

`NativeResourceGraphBuiltinDispatch` supplies actual plain-node, model-base
and group factories with canonical companions. It still requires complete
resource/item/foreign dispatch, type/profile data, parenting, postprocessing,
bones and lifetime domains. `NativeModelOwner`/`NativeModelEnvironment` create
the distinct 188h render-model family; they do not create this 1ACh unit-part
owner or publish `unit+360`.

Actual resource construction, load/cache and hierarchy-parser interfaces also
exist. Hierarchy parsing produces authored records; it does not build the
parented runtime graph. `GameNativeResourceApplication` owns real manager and
parser publication/registration domains. That composition does not establish
the class-resource selection or resulting unit model. The semantic GeomMesh
payload is explicitly not a native resource item or native owner.

Captured symbol searches across Source find the required construction bindings
and graph interfaces, but no production `game_*` consumer for this chain in
either this worktree or current Root. `GameUnitSlot` constructs partial
observer/callback projections, and its gunnery path explicitly records that
the process builds no model hierarchy. Thirty inspected/context files match
Root after LF normalization; this is Source consistency, not a build receipt.

## Decision and next evidence

No new single Source-ready prerequisite is established by this constructor
gate. The smallest useful next evidence packet is the actual class resource
owner/current profile and virtual +8 selection: establish which complete
provider returns the retained selected instance with its published node tree.
`build_native_game_resource_graph_007137f0` is an existing Source candidate,
not a target proved by this constructor. That separate gate should precede
any production constructor binding. No Native extension was opened here.

A raw root accessor, padded buffer owner, semantic-unit cast or callback
facade would not establish that ownership. Keep actual allocation, transferred
references, current-profile dispatch and failure lifetimes explicit.

The prior Source121 receipt is frozen historical context. Root reported a
new 507-input build with three checks; this worker did not inspect its artifacts
or award admission. No old artifact hash was compared with a current path.
This two-file readiness packet adds no C++, CMake, build, tests, probes, ledger,
Ghidra/GPR changes, Original-ABI, startup or gameplay credit.

Evidence: `reports/cc12_unit_model_constructor_node_publication_readiness.json`.
