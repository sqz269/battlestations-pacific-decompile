# World tick composition after subtree invalidation

A bounded actual-storage Source pair for `00904BF0` and `00904600` is now
composable from existing ordinary-body dependencies. The new concrete
`0042ED50` closes the earlier missing subtree callee. The remaining clock,
entity-table and record-storage qualifications can be explicit caller borrows;
an application producer is not a prerequisite for writing this qualified leaf.
This conclusion supplies none of those borrows and does not establish a
production World tick, full World owner or application call path.

The audit finishes on published main `24229ec42`, retaining Source commit
`fdd610b56`. It reuses the complete prior tick evidence: `00904BF0` is 69 bytes /
28 instructions and `00904600` is 1,507 bytes / 323 instructions. No Ghidra
query, native recapture, Source edit, build, test or execution probe ran here.
The paired report pins current inputs and identifies the reused evidence.
All 23 prior tick input hashes still match. The integrated primary subtree
review records a normal Win32 build with three checks passed and an exact
38-byte / 13-instruction object after resolving its sole self-call relocation.
That earlier one-function / 38-Original-byte admission is not new audit credit.
The subtree entry remains absent from the game map.

## Minimum proposed Source pair

Propose only `include/bsp/native_world_current_tick.hpp` and
`src/native_world_current_tick.cpp`, guarded for MSVC Win32. A concrete new
ordinary C++ surface can be:

```cpp
struct NativeWorldCurrentTickContext {
    const volatile float& mission_clock_00f876a4;
    const volatile float& negative_zero_00d7a208;
    const volatile float& one_00d7a24c;
    const SingletonLifetimeCallbacks& validation;
};

void update_native_world_entities_00904bf0(
    void* actual_world, float delta, const NativeWorldCurrentTickContext&);
void run_native_world_matrix_pass_00904600(
    void* actual_world, float unused_delta, const NativeWorldCurrentTickContext&);
```

Names are descriptive hypotheses. This explicit context borrows existing
storage and services; it owns no clock, World, table, node or service. Its
referents must outlive the call and all callbacks. There is no default context,
private timeline, singleton, provider factory, whole-World table or token
adapter. No `noexcept`, recovery or rollback policy should be added.

Both original entries take World in ECX and one float stack argument, then
RET 4. The explicit C++ interface above is a new calling surface, not an
original binary entry. The outer routine must call the concrete inner routine
unconditionally after the entity walk, using the same captured World and
context. A loop around the projected matrix pass would remain incomplete.

## Every ordinary-body dependency has a concrete composition

| Dependency | Current usable Source and exact boundary |
| --- | --- |
| X rotation `00B64640` | `build_native_particle_rotation_x_00b64640(void*, const float*, negative_zero, one)`; fastcall ECX destination / EDX angle with the two explicit stack operands. It returns the supplied destination. |
| Y rotation `00B646E0` | Corresponding Y API with the same explicit current-operand contract. |
| Z rotation `00B64780` | Four-argument `build_gui_rotation_z_00b64780(CameraMatrix&, const float&, negative_zero, one)`; ordinary C++ call returning void, not the original ECX/EDX/EAX contract. Use a real local `CameraMatrix` and its known destination for the subsequent copy. |
| Multiply `00413920` | `multiply_native_camera_matrices_00413920(left, unused_edx, destination, right)`; naked ECX-left / stack-destination-right / RET 8, EAX destination. Incoming EDX is unused and overwritten. No external callee or extra global service is required by this body. |
| Subtree `0042ED50` | `invalidate_native_subtree_pose_0042ed50(actual_child)`; naked ECX receiver / plain RET, actual child and post-recursion sibling storage. |
| Invalid parameter `00BF6713` | Borrow `SingletonLifetimeCallbacks::invalid_parameter(context)` and preserve all returning continuations. The callback must be the genuine intended service, not a no-op or an invented always-throw policy. |
| Free `00BF65AC` | Call existing `singleton_lifetime_free(captured_node)`, whose body calls `std::free`. Every released node must belong to that matching Source CRT allocation domain. |
| Entity +DC / +88 / +D8 | Direct calls through the actual current entity tables at native capture points. Their executable Source targets and method ABI are caller qualifications; no projected callback interface or complete World table is needed. |

`CameraMatrix` is `std::array<float,16>`. Real uninitialized local objects can
serve as the 64-byte scratch matrices without a new matrix type, zero-fill or
reinterpretation of a live entity/World object. The Z overload's void result
does not require a new raw provider or unresolved stub: its destination is
already known. Keep the native ordered sixteen-DWORD result copies; they are
bit copies, not x87 copies or matrix arithmetic.

The X/Y/Z implementations explicitly retain separate x87 FSIN and FCOS
binary32 spills, SSE signed subtraction, stores and current constant reloads.
The negative-zero and one operands must refer to real retained storage. The
existing `GameNativeReadOnlyData::data_at` interface can supply the genuine
four-byte `D7A208` / `D7A24C` spans; `GameNativeParticleRuntime` already borrows
them with `required<float>`. This audit creates no mapping or binding. Never
replace these input references with private literals or capture one value for
all the helper calls. The matrix body also reloads current one for its clamp
and translation-matrix construction.

The validation callback type alone does not prove service behavior:
`GameNativeResourceApplication` uses `_invalid_parameter_noinfo`, permitting a
returning installed handler, while `GameNativeVfsApplication` supplies an
unconditional throwing callback. The latter is not an automatic World binding
merely because the structure type matches. A qualified borrowed service must
remain live and must retain the reached continuation if it returns. The
`destroy_registered` member is not used by either tick body.

## Actual storage and table borrows

The outer routine reads World+4 once, then that actual first/last/count
header's first pointer. `allocate_native_world_chain_headers_009037f0` already
supplies genuine 0Ch headers in the singleton allocation domain. The tick does
not inspect their count/last fields, World+8 or the World's primary table.
Nonempty chains supply actual entity pointers with active byte +5C and next
link +38. Each current entity remains valid through the post-callback +38
reload. No saved-next traversal or header restart is equivalent.

The inner routine retains the address World+4B0, loads its current sentinel
at +4 and count at +8, and begins at sentinel.next. A raw 6Ch node has next+0,
previous+4 and a 64h-byte record at +8. The record contains entity+0,
translation+4/8/C, angles+10/14/18, duration+1C, base matrix+20..5F and start
time+60. `create_native_world_matrix_sentinel_004c3080` provides only raw
self-linked sentinel storage; it does not publish the World fields, populate
records, establish their entity lifetimes or supply a complete list owner.
The projected `WorldLayout::list_head = 1` is not an actual sentinel.

The caller must supply coherent current header/sentinel/record storage and a
matching node-free domain. Returning callbacks may change the observed fields;
the source must retain native captures and reloads. They must not invalidate
storage that a later native read still requires. If retained as a stored
context, all borrowed owners must also outlive every eventual use. No full
World allocation size, primary table or producer is inferred from this field
footprint; construction, normal destruction and application wiring stay open.

For entity +DC, require ECX=that actual entity, one callee-popped float argument,
and no consumed result. For +88, require ECX=the captured actual entity, one
callee-popped matrix-pointer argument, and no consumed result. For +D8,
require ECX=the freshly reloaded actual entity and no stack argument or
consumed result. Tables must contain real callable Source methods with those
contracts. Numeric entries read from the original PE are data, not callable
Source implementations. This audit supplies no entity/table producer.

## Same live clock, supplied by a qualified caller

The coordinated clock audit at commit `5725b0ba0` identifies genuine mutable
storage at `GameMissionFrameHost::Impl::fixed_clock.simulation_clock`, but no
admitted shared production owner/binding for `F876A4`. Native reset `00874640`
and fixed-step writer `00875BB0` must target the same cell used by this leaf.
The frame World clock, World host clock, AI clock and unit summary clock are
distinct values. None is a justified default or substitute.

A caller that establishes that shared-cell identity, initialization/reset,
writer authority and lifetime may lend its actual float by reference. The
absence of a production owner does not prevent this bounded consumer Source.
`UnitGenericInputContext` already uses a `const volatile float&` consumer
contract, without thereby supplying the owner. Volatile preserves explicit
cell reads; it establishes no synchronization, lifetime or FPU equivalence.

At every reached record, including an inactive record, load the same current
cell once with the native MOVSS bit copy and spill it before the first
validator. After that validator, subtract current record.start through x87
and spill elapsed to binary32. Retain that elapsed across the later callbacks;
a callback changing the clock affects the next record, not this elapsed.

## Non-negotiable arithmetic and mutation schedule

The Source pair should keep explicit x87/SSE blocks and actual byte/pointer
accesses at the complete native schedule boundaries. Generic C++ arithmetic,
`std::clamp`, trig functions, vector iteration or cached node projections do
not establish these stages:

- For +DC, capture the entity table, FLD the original delta, load slot+DC,
  reserve its argument, set ECX, FSTP binary32 and call. Reload that same
  entity+38 afterward. Repeat the FLD/FSTP argument path for the unconditional
  inner call even when the chain was empty; the inner float is unused.
- Divide captured elapsed by the current duration using x87, spill phase to
  binary32, then preserve FCOMIP lower and COMISS upper clamp branches. Both
  JBE keep paths retain unordered phase. Negative zero is retained by the
  lower keep path; no generic comparison rewrite or zero normalization.
- Preserve translation's x87 stack and three binary32 product spills, plus
  the current-one and zero matrix stores. The phase remains on the x87 stack
  through this work. At the failed comparison path `0090478A..91`, pop it
  before the returning validator and reload it afterward. Do not silently
  drop that conditional pop/reload boundary.
- For each X/Y/Z angle, multiply current record angle by phase, spill to
  binary32, reload, FCHS and spill again before the concrete rotation helper.
  Keep the current-operand helper's own arithmetic and ordered raw copies.
- Capture the active entity at `009046A1` before later validators. Capture
  its table at `00904A1E` during the Z-result copy; retain its slot+88 address
  across all four multiplies. Load the actual function only at `00904ADD`.
  Multiply `((Rz * Ry) * Rx) * translation * actual record.base`; the final
  right operand is the actual base address, not an earlier copied matrix.
- After +88, validate and reload record.entity; capture its child head before
  clearing +C8/+10C, call the new concrete subtree helper for each child and
  reload that child's +44 afterward. Validate again and reload record.entity,
  its current table and +D8. Do not reuse the earlier +88 receiver/table.
- After +D8, validate and reload current entity active state. Inactive expires;
  otherwise validate, reload current duration and compare captured elapsed
  with FCOMIP/JA. Only ordered strict greater expires; equality and unordered
  retain. The retained path validates before reloading node.next.
- Erase captures the node before its validator; tests the retained header
  address before capturing successor, but calls that validator afterward.
  Compare against current sentinel, validate/recheck and skip deletion if
  still sentinel. Write previous.next, reload both node links, then write
  next.previous. Free that captured node, decrement the current DWORD count
  after free and continue with the pre-captured successor. No clamp, scalar
  entity deletion, vector erase, one-removal limit or early exit is added.

The matrix body has 18 native validator CALL sites, including the unreachable
`CMP EDI,EDI` branch at `0090461F`; returning reachable sites retain all stated
captures/reloads. Together with three rotations, four multiplies, two entity
virtuals, subtree and free, its complete inventory is 29 CALL instructions.

## Boundary of this recommendation

There is no unresolved ordinary-body dependency requiring a fake global,
unresolved link stub, projected token service or invented whole-World table.
Qualified storage, callable entities, the intended validator and common clock
authority remain mandatory caller obligations. They are not supplied by this
audit or by merely declaring the proposed context. Body composition is ready
for a bounded Source packet; production/application composition is unproven.

Future implementation still needs its normal Win32 build and independent
emitted arithmetic/call/control-flow review before admission. General native
register/private-frame, FH3/SEH, faults, concurrent mutation, runtime and
gameplay equivalence are not established here. Partial completed writes must
remain on a throwing Source call; no owning rollback or termination policy
belongs in the leaf. This audit changes exactly its dedicated document/report
and awards zero new Source or Original-function credit.
