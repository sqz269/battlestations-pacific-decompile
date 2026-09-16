# Resource cleanup with actual emitter parameter pools

The AF4280/AF46E0 overloads taking `NativeParticleTypeLifetimeContext` compose
the complete resource cleanup bodies with the actual shared string cells and
F8D344 parameter pool. The earlier string-only overloads retain their existing
Layer support and callable-emitter boundary. Both use one resource-body
implementation, preserving current reads and the original unwind schedule.

The emitter-aware path recognizes the five verified terminal profiles below:

| Profile | Actual scalar target |
|---|---|
| D5DBC4 | AFA350 emitter definition base |
| D5DEBC | B03B40 Cone |
| D5DE88 | B02FB0 Sphere |
| D5DE48 | B01EA0 SmartArea |
| D5DC38 | AFACE0 Layer |

All slot-zero targets are BD30E0. After the single reference decrement reaches
zero, the captured terminal chain reloads the child's current table and invokes
slot four with flags 1. These are genuine scalar bodies; nested emitter-owned
particle types use the same context and actual parameter pool. Other table
profiles still require callable Win32 slots. Object model terminals retain their
canonical renderer binding boundary.

## Preserved resource behavior

AF4280 is the complete 224-byte ECX-owner destructor. It stamps D5D958, walks
the inline emitter and Layer arrays forward using current signed counts, clears
their counts, returns the captured name block, and invokes the reference base.
It captures the Layer count before clearing the emitter count. Pointers and
unrelated object bytes are left unchanged. AF46E0 is the complete 30-byte scalar
wrapper: stack flags, RET4, same owner in EAX, free only for bit zero after
successful destruction.

Handler CBAB83 uses FuncInfo DF29AC and map DF299C. State 1 invokes CBAB78 ->
41DD20(owner+8); state 0 invokes CBAB70 -> BD30F0(owner). The captured normal
name pointer precedes state 0, and state -1 precedes the base call. The source
guard remains `noexcept` during unwind.

## Validation boundary

The report retains complete live/PE proof for both bodies, the whole handler/map
and all relevant table pairs, prior annotations and reconstruction records,
direct call checks, build results and the focused probe receipt.

The ignored local probe compares both original bodies against the emitter-aware
source using genuine string/base/CRT providers and the actual Windows reference
decrement import. It checks complete 90h object images, callback changes to
current row counts, surviving references and scalar flags 2/3. Additional source
cases check exception cleanup and the complete Resource -> emitter -> particle
type / Layer chain, including real parameter-slot return order and pooled name
and material release. The existing string-only Layer probe is also rerun because
the resource body is shared.

These are new C++ entry interfaces, not replacement binary thunks. Native FH3,
SEH, allocation failure, concurrent mutation and gameplay remain unvalidated.
This supplies a concrete lifetime provider; application call sites must pass
their actual parameter-pool context to select the new overload.
