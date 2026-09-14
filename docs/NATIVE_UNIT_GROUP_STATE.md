# Unit model-group state transition

Addresses: `00876EC0`, `0049C940`; canonical calls `00414DB0`, `00710BB0`.

`set_native_unit_group_state_00876ec0` reconstructs the complete 196-byte caller
through `00876F83`. Native ECX is the actual unit; a signed state is passed on
the stack and RET4 removes it. The borrowed C++ view uses a different interface.

Only a signed increase can announce. A nonnull current world, nonnull unit+354
descriptor and nonnegative descriptor+40 are required. Dirty pose C8 triggers
the canonical pose refresh. X is read before reloading the descriptor; Y and Z
follow. The three secondary coordinates are positive zero. After coordinate
capture, the selector is read from the captured descriptor, then the current
world and its actual+21D0 announcement manager are loaded without another guard.

The canonical 121-byte announcement wrapper now also accepts an actual manager
address. Negative selector and null allocation return before dereferencing it;
after allocating 30h bytes, the wrapper reads manager+08 and calls the required
whole49C000 constructor. A source constructor exception frees the allocation
and propagates. The existing borrowed-view interface shares this body.

Finally, the unit+360 model is captured BEFORE storing state+358. The captured
nonnull model receives the canonical group selector even for unchanged or lower
state. Announcement failure skips this tail. No owner, vector or node graph is
invented. Field bindings must be pure, nonthrowing reads of actual storage.

Seven pairs execute the full original caller and announcement wrapper against
the integrated source. They cover lower/negative states, null world, negative
descriptor, dirty pose, allocation-time owner/model changes, and raw signed-zero
and NaN coordinate bits. Fourteen retained result images match. A separate
source constructor exception verifies free and the skipped state/selection tail;
two original/source null-manager cases verify guarded dereference, and old-view
versus raw-manager execution verifies the late owner read. Thirteen direct call
rows are attributed to live Ghidra bodies. Five supporting spans match the PE.

Optimized production code retains X->descriptor->Y->Z, late selector/world/manager
reads and model capture before state write. Win32 and both existing CTests pass.
The proof retains exact objects, searched libraries, compiler-discovered headers,
tools, original/relocated bytes, operands and outputs under
`local/group_state_aa/manifest.json`.

The original side uses canonical source ABI bridges for pose and model selection.
The full announcement constructor and allocator are fixture providers. Native
FH3/SEH, hardware-fault timing, private frame aliases, concurrent mutation and
gameplay execution remain unproved. The application still needs actual bindings.
