# Native game class registry cleanup (R131)

Addresses: `004A9AC0`, `004A9890`, `004A9790`, `004A9800`, `004A9870`,
`004A7140`, `004A7800`, `004A73A0`, `004A8AC0`, `00488BF0`, `004A7A40`;
consumed branches `004A9240`, `004A8F10`, `004A8FD0`; parent `004DCF90`.

## Result and scope

The parent teardown call at `004DD026` now has a concrete class-registry
cleanup binding. It deletes nonnull mapped payloads, returns owned string keys,
frees nodes, and clears six vectors while retaining the registry and vector
storage. Its cold singleton getter, destructor and scalar deleter are also
available, including canonical singleton drain of profile `00CE6C68`.

Eleven complete normal bodies total **1,135 bytes**. Three additional bodies
are references for their consumed branches only: current full-range tree erase
(`4A9240`, 201 bytes) and resize-to-zero (`4A8F10`/`4A8FD0`, 179 bytes each).
The existing 179-byte `492210` DWORD resize remains a shared source service.
All **1,873 bytes** match live Ghidra and the original executable.

“Class registry” is a descriptive hypothesis. `4A9BD0` references
`PlanePartClassWeights`, and the registry profile is beside `PlanePartClass`
text. This does not establish the full data schema or recover original names.

## Layout, ownership and ordering

The registry is actual `10h` storage: profile at `+0`, opaque word at `+4`,
head at `+8`, and count at `+C`. Publication is `E187BC`. Its tree subobject
starts at `+4`. Actual `1Ch` nodes contain links at `0/4/8`, owned string
length/data at `C/10`, mapped payload at `14`, and color/nil bytes at `18/19`.
The constructor establishes the sentinel and count without clearing the opaque
word. The subtree routine owns keys and nodes; mapped payloads are borrowed.

The global cleanup obtains the singleton for initial begin, each end comparison,
and final tree cleanup. It preserves iterator-owner checks, captured end values,
the payload callback followed by two null writes, and the subsequent iterator
increment. The subtree visits the right branch first, captures key data before
the left link, returns the key, frees the node, then follows the captured left
link. Sentinel root/left/right resets reload the current head in native order.

Six checked vector headers occupy `E1875C..E187BB`. Clear order is `5C`, `6C`,
`9C`, `8C`, `AC`, `7C`. The float arguments execute real `FLDZ/FSTP32`, including
their x87 status effects. Range erasure captures suffix size and new end before
`memmove_s`, and writes output position before owner. Growth is not added.

The singleton getter captures the manager lock, tracks recursion, checks the
publication again, allocates/constructs, publishes, then registers the current
publication through a fresh manager lookup. The scalar tests low flags bit 0.
Base cleanup clears the current publication and stamps `CE3818`. Source catch
cleanup follows the reviewed constructor/destructor base unwind and getter
allocation-release obligations. It does not reproduce native FH3/SEH.

## Analysis repair and evidence

Three false no-return gaps after `BF65AC` hid **53 bytes**: subtree 11,
destructor 39, scalar 3. Repairs used the shared Ghidra write lock and retained
prior state. The destructor body was explicitly restored through `4A986E`.
The final audit leaves one byte of non-CALL padding after `4A92BC` alone.

Evidence and exact call rows are in
[`native_game_class_cleanup_r131.json`](../reports/native_game_class_cleanup_r131.json)
and the separate flow-repair report. Prior annotations are preserved locally,
comments are read back, and affected exports are refreshed.

## Validation and limits

Strict MSVC Win32 build and all three existing CTests pass. An ignored focused
probe compares **28 native/source pairs, 90 snapshots and 119,104 bytes**:
cold/existing/recursively held managers, 0/1/7-node trees, both return-gate values,
null payloads and keys, 13/149/150-byte keys, repeated empty cleanup, four scalar
flag combinations, and range compaction with returning validation. Allocation,
key-return and free ordering are compared; payload scalar effects are controlled.
Each global case also verifies that zero formation clears x87 C1 while retaining
the pre-existing precision flag. A source failure preserves the payload and
retained diagnostic state; replay and a missing-context call are rejected.

The existing parent probe still matches **52 pairs, 3,537 boundary snapshots
and 141,402,076 bytes**, with four source failure/replay cases. It controls child
calls and verifies the new context/child-operation binding.

Canonical registry/pool drain uses real source services. Fixture allocation
initializes otherwise unspecified bytes. The copied existing DWORD resize uses
a controlled terminal range leaf. General partial erase, growth, callback
mutation of ownership, allocation failure, private stack aliases, native
exception ABI, malformed x87 stacks, concurrency and gameplay are unproved.
`C4DDE0` remains a pure address binding; virtual payload/terminal/embedded
bindings and raw-game application admission are also still required.

## Follow-up

Recover `C4DDE0` and its owned physics dependencies before admitting the actual
raw game lifetime. This packet does not make that path application-ready.
