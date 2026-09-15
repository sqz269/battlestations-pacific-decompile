# SoldierClass registry erase and lifetime

Addresses: 004AF680, 004AF6C0, 004B0330, 004B0600, 004B09A0,
004B1210, 004B1280, 004B1330.

Eight complete bodies, 1,384 original bytes, have raw MSVC Win32 source in
`src/native_soldier_registry_lifetime.cpp`. Complete live bytes matched the
installed image in the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. SHA256, original ABIs, exact direct call rows and
exception metadata are in `reports/native_soldier_registry_lifetime_orch4.json`.
Names are descriptive hypotheses. The existing correct scalar-deleting-dtor
classification at 004B1280 is retained.

| Entry | Coverage | Reconstructed behavior |
| --- | --- | --- |
| 004AF680 / 004AF6C0 | complete | Minimum/maximum by nil-checked child links. |
| 004B0330 | complete | Checked iterator erase, successor, two-child transplant, red-black fixup, key/node release, count/output tail. |
| 004B0600 | complete | Right-recursive, left-iterative raw key/node drain. |
| 004B09A0 | complete | Returning-handler validation, full drain/reset and partial advance/erase loop. |
| 004B1210 | complete | Actual map drain, current-head free, head/count zero, publication clear and base stamp. |
| 004B1280 | complete | Complete destructor, optional owner free and captured-owner return. |
| 004B1330 | complete | Actual manager/critical-section/allocation/construction/publication/registration sequence. |

## Layout and ownership

The registry is 10h bytes: profile+0, preserved opaque+4, head+8 and count+C.
Tree APIs receive owner+4, so head/count are tree+4/+8. Each node is 1Ch bytes:
left/parent/right at 0/4/8, actual NativeString at C/10, borrowed mapped DWORD
at 14, color18 and nil19. Neither node cleanup nor registry teardown releases
the mapped class pointer. There is no host map or fabricated teardown callback.

Successor and rotations are the genuine shared source from
`native_soldier_registry_tree.cpp`; these APIs reuse `NativeKeyboardTreeIterator`
because its two words match the native checked iterator. Pool release uses the
actual `NativeStringRawPoolContext`, including real 00419CC0 and BD1510. It is
not the older host-storage tree specialization.

## Listing repairs and schedules

The old 004B0330 name `STL_xlen_throw_004b0330` was wrong. Its nil branch builds
an owning out_of_range; the remaining body erases and rebalances a tree. The
two-child transplant at 004B043C..004B0493 is live assembly even though the old
pseudocode omitted it. The repaired full body ends at 004B05FB, including the
post-free nonzero-count decrement and owner-before-node output stores.

004B0600's missing 004B0641..004B064B instructions continue the left drain
after free. It captures key data, then current left, then (only for nonnull
data) length+1 after recursively draining the right subtree. Pool creation may
throw before node free; no tree rollback or recursive cleanup is invented.

The parent restored these listings under the shared write lock and refreshed
exports. Repair records are `native_soldier_registry_flow_repairs_orch4_g7.json`
and `native_soldier_registry_body_repairs_orch4_g7.json`. This worker made no
live Ghidra mutations.

004B09A0 captures the current minimum before its first invalid-owner handler,
then loads first.node after the handler returns. On the full-range candidate
it captures the current head before validating last.owner. Its reset rereads
head between the root, minimum and maximum stores. Partial range checks the
captured first.owner against current last.owner, advances first, erases the
captured old iterator, then reloads first.node/owner. Returning handlers remain
observable; no unconditional failure or recovery branch replaces them.

004B1210 uses that actual range erasure, frees the current head, clears the
current tree's head/count, clears current E187F0, then stamps CE3818. It neither
unregisters itself nor frees the owner. The scalar deleter adds free only for
flags bit0 and returns the captured owner.

## Singleton and exceptions

004B1330's fast path returns its first publication read. Its slow path calls
the genuine raw 00415350 manager, captures section+10, enters it and increments
physical depth+18, then rechecks publication. It allocates 10h, invokes complete
004B11A0, publishes the result, reacquires the manager, reloads publication and
registers via real BD0C30. It releases the first captured section and reloads
publication for return. A null allocator result is still published/registered
according to the native sequence; no extra retry is introduced.

- 004B0330: handler C64948, FuncInfo D8CD08, map D8CD00. State0 is armed only
  after counted assignment; C64940 destroys the completed SBO temporary.
  Existing owning D6926C source exception transport supplies real native owner
  construction/copy/destruction while retaining host C++ exception identity.
- 004B1210: handler C64A18, FuncInfo D8CE28, map D8CE20. State0 invokes C64A10
  -> 004AF950, clearing publication and stamping base. A failed drain does not
  proceed to sentinel free or head/count clear.
- 004B1330: handler C64A63, FuncInfo D8CE88, map D8CE78. State1 frees the saved
  allocation through C64A58, then state0 destroys the guard via C64A50/00411EE0.
  Constructor failure therefore frees its allocation after constructor unwind;
  registration failure keeps the publication and owner. A second C++ exception
  from the true guard unwind terminates.

## Validation and remaining boundaries

Validation results are recorded in the report. These are new source interfaces,
not original ABI, FH3 or SEH replacements. Numeric profile identities are not
callable host vtables. Canonical mixed-owner manager draining still needs the
actual SoldierRegistry profile dispatch; registering a raw owner does not prove
that integration. No game startup or gameplay result is claimed.
