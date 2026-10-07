# Complete participant observer binding at 0077CC50

This packet reconstructs the complete 48-byte normal body at
`0077CC50..0077CC7F`. Native ECX is the actual 118h game participant; the
stack argument is an already-adjusted observed endpoint. The native body
returns with `RET4` and promises no EAX result. The new C++ interface adds a
borrowed `NativeObserverLifetime&`; it is not a binary replacement.

The participant's callback owner starts at +38h and its current first endpoint
is stored at +4Ch. The function captures the old identity once. Equal old/new
pointers return without stores, registration, or reference-count changes.
Otherwise it unregisters the captured old pair through complete `006952A0`,
stores the requested pointer at +4Ch, then registers a nonnull new pair through
complete `00694A60`. Each observer provider takes the existing recursive shared
lock. The binding routine itself has no outer lock and adds none. Unregister
failure precedes publication; register failure follows publication. Native
FH3/SEH or asynchronous fault identity is outside the new C++ contract.

The actual caller at `00780309` in `00780120` supplies a participant from the
game's +18CCh participant table, and an endpoint returned by a unit's primary
virtual +140h. This establishes the two different owner identities. The
participant +38h callback is not the unit's own callback prefix. No virtual
endpoint dispatch, role policy, session routing, or global lookup is added to
this leaf. `00780120`, `00780670`, and gates `0076C500`/`0076C600` remain open.

One connected fixture runs the original body and current source through
bind A → bind equal A → rebind B → clear → cleanup. It uses:

- Actual 118h participants produced by the existing complete `004D6BA0` source
  constructor, including +38h/+4Ch initialization, the allocated pair-list head,
  and all three genuine empty native strings.
- Borrowed valid observed-prefix storage, initialized by the explicitly partial
  `00925CFF` projection. These are inputs to the observer contract; no complete
  unit constructor or gameplay world is claimed.
- Actual raw singleton-manager publications, complete observer-dispatch owner
  creation, genuine recursive Win32 critical sections, actual count/capacity
  edge arrays and `CF7E64` edges. Registration, lookup, reference decrement,
  endpoint removal, edge deletion, and storage frees use existing source
  providers. Unsupported virtual profiles throw instead of returning success.
- Complete source participant cleanup `004CB2F0`, observed-prefix cleanup
  `00695760`, and raw singleton-manager drain with concrete observer deletion
  bindings. Naturally empty participant references/strings remain empty; no
  unimplemented nonempty payload dispatch is claimed.

The original 48-byte body retains both native CALL opcodes and all branches,
loads, stores, and `RET4`. Only its two natural relative CALL operands are
relocated to fixed ECX/EDX bridges that invoke the complete observer providers.
Those bridges also check actual callback identity and the old/new publication
ordering. All 40 nonoperand bytes are compared before and after execution;
the eight operand bytes are the only admitted relocation ranges. This is a
bounded original-body comparison with source dependency bridges, not execution
of the original observer-provider machine code.

Validation passed 143 checks over the single connected sequence. Equal A keeps
the same edge and reference count one. Rebinding B removes A's membership and
creates the correct B pair; clearing leaves all three counts zero. Actual
shared-lock depth returns to zero after every operation; cleanup clears both
published observer owners through the raw manager drain.

The strict MSVC Win32 build used `/O2 /W4 /WX /fp:strict`, an embedded
`asInvoker` manifest, 13 current repository translation units and the fixture.
All 296 actually consumed repository/fixture inputs and 248 host headers are
hash-pinned. Support libraries were frozen before the integrator's rebuild.
The current helper compiles to 61 bytes/27 instructions: a whole COFF review
confirms equality early return, fixed unregister CALL, +4Ch store, then fixed
register CALL. Whole bridge COFF bodies were reviewed for native register and
stack adaptation. Live Ghidra, installed PE, and frozen native bytes match.
All four earlier reports and 334 recorded artifacts retain their hashes.

The report records exact pins, body/bridge/provider COFF evidence and the ignored
artifact directory `local/cc11_session_participant_binding_20261007_a`.
Full project registration/build and shared Ghidra annotations belong to the
primary integrator. No game validation, entity construction, session dispatcher,
or ABI-compatible release replacement is established by this packet.
