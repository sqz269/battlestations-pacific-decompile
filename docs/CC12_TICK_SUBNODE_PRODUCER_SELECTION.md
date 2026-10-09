# Tick subnode producer selection: metadata and current Source only

Baseline: `12f249d3ab42e33b6ec910aafafc9eb2cd217a85`.
Packet: `cc12_tick_subnode_producer_selection`.
The companion report pins the eight fresh metadata responses, current Source
excerpts, and prior accepted reports. This packet adds no Source, Native,
Original-ABI, build, runtime, or gameplay credit.

## Recommended next gate

Select **007F1DE0..007F1E67**, a **136-byte metadata span with 41 instructions,
one basic block and two recorded direct calls**, for Root Astra's bounded
receiver/stack gate. Its descriptive Ghidra name is
`BSP_SquadronLaunchTask_Construct`; the current metadata signature is
`undefined ...(void)`. The actual entry registers, stack arguments, return,
profile writes and node placement remain **unknown at this selection boundary**.
No Native body was opened here. Metadata counts and names do not establish ABI.

This is the smallest candidate with an explicit production construction
counterpart in current Source. `GameUnitsHost` marks a
`PlaneSquadronHostRecord` task live, initializes its timer/deck/send fields,
and records `construct_007f1de0`. The stored object is a typed squadron record;
it is **not** raw Native task/node storage. The existing source comment about a
38h allocation, base+310h and squadron arguments is a lead for the next gate,
not newly verified Native evidence. The `00D08AE4` numeric profile in its header
comment likewise establishes neither a profile store nor a callable slot zero.

The three smaller metadata callers do not supply this construction counterpart:
0071C470 and 00876120 have no exact current Source address/name hits;
008FBC80 has attach-address/slot constants but no Source attach implementation.
These remain possible later caller packets, not prerequisites automatically
added to this one.

Proposed next ownership, subject to Root's gate:

- Address: only `007F1DE0`; no neighbor or implicit child ownership.
- Outputs: `docs/CC12_SQUADRON_LAUNCH_TASK_CONSTRUCTOR_ABI_READINESS.md` and
  `reports/cc12_squadron_launch_task_constructor_ABI_readiness.json`.
- First establish the actual entry/exit and whole owned construction schedule.
  Identify whether and where actual node backing and any profile are written.
- Existing 00876020 consumer evidence is a dependency. The other metadata
  callee, `FUN_007f0f80`, remains an unopened dependency; do not silently claim
  its effects. Any required child, profile, table, handler or slot target needs
  a separately bounded gate after the owned caller evidence identifies it.
- A later Source adapter needs stable real node/parent backing, its publication
  domain, and a concrete callable deletion binding. None exists merely because
  the current gameplay projection records this constructor's address.

## All eight candidates

Fresh `bsp.py ghidra proto` and `callees` calls verified project `bsp`, program
`/battlestationspacific.exe`, x86 and image base through the existing CLI client.
The table gives inclusive **metadata spans**, not byte-verified bodies.

| Entry | End | Span | Instructions / blocks | Current Source counterpart |
|---|---|---:|---:|---|
| 0071C470 | 0071C47C | 13 | 4 / 1 | No exact address/name hit; only the fresh metadata edge to 00876020. |
| 00720180 | 00720442 | 707 | 153 / 8 | `GameDirector` typed defaults and per-unit vector resize; abstract `construct_command_array` model hook. No raw tick-node provider. |
| 007E1E20 | 007E2006 | 487 | 114 / 4 | Constructor comment; `GameUnitSlot` vectors/clocks support projected neighbor behavior. No actual Native neighbor-node construction. |
| 007EABC0 | 007EADFE | 575 | 154 / 20 | `GameUnitSlot::actuator_block_dec` stores a typed `PlaneActuatorBlock`, with explicitly substituted channel enable/rate values. No raw Node links/profile. |
| 007F1DE0 | 007F1E67 | 136 | 41 / 1 | Explicit task construction projection in the production units host; typed `launch_task_*` fields in the squadron record. Selected. |
| 00818110 | 0081828E | 383 | 118 / 18 | `pending_explosions` stores `(unit index, damage)` pairs, later drained through a local vector. No Native scheduled object/node. |
| 00876120 | 00876175 | 86 | 31 / 7 | No exact Source address/name hit. Metadata callees include getter, unlink, reparent and critical-section imports. |
| 008FBC80 | 008FBCD2 | 83 | 37 / 4 | Gun-bot attach address/slot constants and field-offset comments; no Source attachment body/provider. |

Each has a fresh metadata callee edge to 00876020. This does not prove which
value supplies its receiver/node, allocation ownership, class, profile or slot.
The metadata `call_count` differs from the callee list where imported calls are
present; it must not be interpreted as a complete physical call-site inventory.
No candidate has a demonstrated concrete raw subnode storage route in the
bounded Source checks used here.

## Production storage and hook boundaries

The tree contains `UnitInstanceCreationHost`, not a `UnitConstructionHost`
definition. Its `construct_tick_node` is pure virtual. The sole model call is
`create_unit_instance` passing `block + kUnitLayoutOffTickNode`, `block`, `0`.
Exact searches across `src`, `include` and `tests` found no override and no
production call to `create_unit_instance`; only its declaration and definition.
This does not establish executable construction of unit+310h backing.

`destroy_tick_element_310` similarly has a pure virtual declaration and one
modeled dispatch call, with no implementation found in those roots. Its
00875490/00874F00 comments cannot close actual storage or deletion contracts.
`GameFixedStepHost::next_element` ignores its inputs and returns false. Its
message about empty groups is source behavior, not a fresh Native execution
observation.

The selected task's actual Source lifecycle is explicit: the units host writes
the squadron record; `run_landing_queue_006cd240` iterates registry records and
directly calls `base_launch_task_tick_007f1f00`; that function clears the live
flag when no member remains. The source itself labels this scheduling placement.
It does not invoke 00876020, allocate a raw task, install a callable profile,
or dispatch a Native node slot zero through this route.

The actual singleton dispatcher has a different concrete contract:
`GameSingletonHost` borrows the canonical process F878CC cell into its deletion
bindings, and the `D0DEA0` case calls the admitted registry scalar-retirement
service with the passed owner, low flags byte and actual publication cell.
That service is for the raw8 registry owner. It is not a generic child-node
deletion service. Numeric comments/constants for task or bot profiles do not
add a child-profile case to that dispatcher.

## Accepted consumer context and version qualification

- Source83: the admitted 00874E60 raw unlink leaf requires actual list/node
  backing and preserves its ordered, alias-sensitive field schedule. It does
  not allocate, validate membership, free, or resolve any profile.
- Source69: this denotes the admitted adapter for the **69-byte Original
  00875960** caller. Its accepted compiled Source snapshot is **82 bytes / 29
  instructions**, with four relocations. The Source fastcall API borrows the
  actual node-input cell and both actual publication cells, using RET 0Ch;
  Original's RET 4 and value argument are different contracts.
- Readonly100: accepted 00874F00 evidence reaches current captured-node
  profile/slot-zero dispatch with flag 1. The actual target and effects remain
  unresolved; it must consume that flag and preserve the loop registers on a
  normal return. This selection supplies no free/cleanup substitute.
- Readonly243: accepted 00876020 evidence establishes old-parent node+4 as the
  actual unlink receiver and ordered insertion at the target receiver. It
  does not identify a producer/profile. Original C964E8 EH remains unopened.

At the stated checkout baseline, the latest Source69 primary report's input
pins replay **57/57**; older Source83 replay **51/53**, and older getter replay
**37/38**. The older mismatches are the recorded CMake and typed legacy-owner
changes, enumerated with both hashes in the report. The three build checks,
Core members, raw83/getter250 compiled receipts, typed-owner EH evidence and
application-map selection are **historical accepted snapshots**. This worker
did not reread/rebuild artifacts or run tests, and ongoing Root CMake work is
outside this baseline. Source text equality does not freeze those artifacts.

The report records current excerpt/file pins and search coverage. Validation
is limited to JSON structure, metadata completeness, pins, excerpt hashes and
the exact two-file diff. No Source/CMake/ledger/GPR mutation, Native body/profile
read, test, probe or admission accompanies this recommendation.
