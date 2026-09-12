# Input configuration reconstruction closeout

Addresses: 00A93C80; 00A92D40; 00A93020; 00A933F0; 006965A0; 004D8CD0;
00698680; 004DCEB0; 00698730; 00A93920; 00A92260; 00A93880. Analysis: 00698A10.

This closes the work underway when the user asked to finish the previous session
and dispatch no new work. All three workers finished or documented their current
packets and released their leases. No follow-on packet was dispatched.

## Completed source

- Action configuration and activation share the recovered record, binding and
  listener services. Context updates compute an unsigned maximum with floor1 and
  always refresh actions, retaining native callback-sensitive pointer reloads.
- The deadline callback performs sixteen real action-edge checks and map writes,
  preserving ordered comparisons, live publication reads and rounding before
  map insertion.
- Configuration construction/destruction uses the actual embedded524h storage.
  Its existing4C8h Lua owner is at offset **zero**. Five checked-vector buffers
  are released in native order before closing that Lua state.
- Configuration cleanup implements unregister/reset. The stateless
  `NativeInputConfigurationStorage` supplies actual assignment/erase operations
  extracted from existing class configuration, which now shares those helpers.
- Deadline-map storage and lookup reuse existing tree mechanics with the actual
  12h header,18h nodes and signed keys. They return cells in the caller's map,
  retaining duplicate values and allocation/returning-CRT effects.

`GameInputActions` exposes configuration and context updates through the existing
runtime's sole action publication. `GameInputDeadlineCallback` binds the recovered
schedule to borrowed game/map storage and defaults to the concrete signed-map
adapter; its explicit provider overload remains available. No duplicate owner or original entry point
is introduced by these source composition changes.

## Evidence and validation

`reports/input_configuration_ah_integration.json` records the final combined build
and focused fixture receipts. Component reports retain their worker revisions and
archive hashes. Earlier AG evidence in `reports/game_input_actions.json` does not
validate this newer revision.

Twelve native signatures/names were saved, read back and re-exported. The five
`*_ah_prototypes.json` receipts preserve prior names, comments and signatures.
Library names remain unchanged. The owner EH handlerC7ECC0 was defined from ten
verified native bytes. Loader698A10 required clearing seven erroneous CALL-return
edges and recreating its truncated body. It now contains1,137 instructions and
zero gaps through699B7D. The initial repair's tail warning is superseded by its
explicit follow-up and `native_input_configuration_load_function_definitions.json`.
This is analysis coverage, not a completed loader implementation.

The deadline differential executes original subscript/hint instructions while
sharing the separately verified concrete link layer; it is not an independent
native-link/EH test. Valid-tree and checked-vector range requirements remain in
the component reports. Source interfaces are not original ABI replacements.

## Deferred work

The actual698A10 Lua/configuration parser remains incomplete. Its exploratory
packet closed without another source implementation. The ignored handoff is
`local/native_input_configuration_load_closeout_ah.md`, SHA256
`3e49ca0cd7e525f6e83a1ecf5df8f92bcf00b285a2ff1c5149bdd37234c2f79b`.
It records descriptor6974F0, loader stages and unresolved append adapters.

698A10 conditionally opens Lua but executes `Inputs.lua` on every invocation.
Descriptor6974F0 reads a live flag byte at configuration+4CC; that is not a Lua
owner pointer. The descriptor, full InputModifiers/Inputs/action-binding parser,
and application wiring of canonical game/configuration/map ownership remain.
No game was launched, activated, polled or modified, and no single-instance mutex
was bypassed. Application/gameplay validation remains outstanding.
