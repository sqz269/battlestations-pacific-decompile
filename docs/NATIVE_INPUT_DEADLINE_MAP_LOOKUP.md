# Deadline map subscript dependency assessment

Assessment baseline: primary `7652e949d777dfe32f9152980f1367d5709367a3`.
Only this document and its report are owned by this packet. No native routine
or source implementation was added, and no Ghidra or library name was changed.

The current public tree helpers do **not** provide a thin composition for the
actual4D6900 signed-int to float subscript. The hardware-layout insertion code
is tied to a different node and key representation. Reusing it through casts
would access outside the actual deadline allocation and change its ordering.
The 18h helpers supply useful leaves, but not insertion or checked hints.

| Contract | Actual deadline map | Current hardware-layout adapter |
| --- | --- | --- |
| Header |12h; opaque word0, head4, unsigned count8 |Same offsets |
| Node allocation |18h |28h |
| Key |Signed int32 at+C |14h key: four stream words and signed count |
| Value |Float bits at+10 |Unowned pointer at+20 |
| Color / sentinel |+14 /+15 |+24 /+25 |
| Comparator |Ascending signed scalar |Descending signed count, then ascending unsigned stream words |
| Length check |Unsigned count >=1FFFFFFEh |Unsigned count >=0AAAAAA9h |
| Iterator |8h owner/node |Same layout, but traversal reads the incompatible sentinel byte |

These are source constraints, not inferred from helper names. In primary
`src/native_hardware_layout_tree_insert.cpp`, lines22/23 hardcode color/nil,
line148 allocates28h, and line159 hardcodes the limit. Its node initializer at
line132 invokes the specialized pair copier. In
`src/native_hardware_layout_tree_key.cpp`, lines48..68 implement the specialized
comparator; the copy at lines111..117 reads the mapped word from pair+14.
The key/header definitions in the corresponding headers agree with those
bodies. Neither public insertion function accepts allocator/layout/comparator
policies. A pointer adjustment cannot preserve links at0/4/8 and simultaneously
move only the sentinel, color and payload offsets. Copying into larger nodes
would create replacement canonical storage and ownership, outside this task.

The actual map producer is established separately: CC9E30 initializes the
single E18A7C header with an18h sentinel; CD9EC0 destroys it.4C27E0 allocates
ordinary18h nodes, then reads its five stack arguments and stores left/right/
parent, scalar key, scalar mapped word, color and nil0. Its RET14h confirms the
argument count. Padding16/17 is preserved. The allocator is the existing
BF681B-compatible malloc/new-handler service, paired with the existing free.
It is not the sentinel allocator: even their write schedules differ.

4D6900 takes ECX=actual header and one stack key pointer, returns node+10 in
EAX and ends in RET4 at4D697D. It captures the signed key once for lower-bound
iteration at4D6916, then rereads it at4D6935 and4D693C. A missing key is copied
with positive-zero mapped bits into the pair before4D6957 calls4D3CD0. That
callee takes an8-byte output plus hint owner/node and pair pointer, RET10h.
The returned owner/node are reloaded at4D695C/4D695E. The checks at4D6965 and
4D696F invoke real returning-capable invalid-parameter handling; after the
second handler returns, the function still returns node+10. An unconditional
throw, default value or repaired iterator would change this contract.

The routine serves both the global map E18A7C and actual game+5C8. The former
has16 calls from4D8CD0 and two from4D92B0;4D9420 and4D9480 each call it twice
with ECX=game+5C8. A future adapter must accept the actual header, rather than
hardcoding the global or substituting the separate WorldTickState std::map.

The following parts can be reused now:

- `rotate_left_native_int_pointer_tree18_0086a2f0` matches4BDD00 across all78
  bytes; `rotate_right_native_int_pointer_tree18_00869810` matches4BDD50 across
  all82 bytes. Their raw node addresses, link offsets, sentinel reads and write
  order agree exactly.
- `increment_native_int_pointer_tree18_00869a20` matches4B75B0 across99 bytes
  except direct CALL/JMP displacements. It keeps the actual iterator owner and
  supports a returning installed CRT invalid-parameter handler.
- Existing86AC00 sentinel allocation and86AA60/86EE50 full-range cleanup are
  proven reusable in `NATIVE_INPUT_ACTION_DEADLINE_MAP.md`. They do not implement
  insertion or construct a key/value pair.
- The public `NativeHardwareLayoutTreeLengthError` owning transport can carry
  this insertion error. It has the same28h D69260 storage,411700 constructor,
  411940 copy and411780 destruction. The required19-byte counted message is
  `map/set<T> too long` including its terminator. The helper that prepares and
  throws it is currently private; constructing the public transport still
  requires proper completed-message cleanup. Native EH/RTTI identity remains
  an explicit source boundary.
- The real SDK `_invalid_parameter_noinfo` service and shared CRT allocation/
  free services are available. A new adapter must preserve their actual
  returning-handler behavior rather than install a successful stand-in.

Insertion is the remaining work.4D3CD0 performs checked hinted insertion and
falls back to4D1F40 unique insertion. They require18h predecessor traversal
4B7520 and owner-validating iterator equality4B5AC0, which have no compatible
public source helper. The hardware predecessor reads+25 and cannot be used.
4CF010 links and repairs the tree: its native unsigned limit is tested before
allocation, current count is incremented afterward, current head is reloaded,
root is made black, then output NODE is published before OWNER. Its length-error
message is assigned at4CF053 before arming cleanup at4CF061, followed by411700
and the D69260 throw. The existing hardware link function has useful reviewed
logic but cannot be delegated to without changing its public implementation.

A subsequent packet therefore needs an explicitly scoped stateless storage
adapter, or a narrowly parameterized extraction of the existing insertion
logic with deadline-specific allocation/layout/comparison contracts. It must
still prove hint/duplicate handling, alias capture, publication and validation;
merely replacing the comparator is insufficient. Suggested disjoint files are
`native_input_deadline_map_storage.hpp/.cpp` with their own doc/report. Changes
to existing helper files require a fresh lease and coordination. Those four
candidate insert/18h-leaf header/source files were unleased at this assessment,
which is only a point-in-time observation.

Validation here is static: current primary source hashes are in the report,
three pairs of original native leaf spans matched disk/live bytes and each
selected destination has a current source provider. The
CALL audit checked30 rows with0 failures. No C++ changed, so no build or fixture
was run. Future insertion proof should use one focused raw-storage fixture
covering signed ordering, duplicate value/address preservation, rotations,
allocation/key aliases and returning/throwing validation. The prior manually
populated map lifetime fixture is not insertion evidence. No SDK, game or
application execution was performed.
