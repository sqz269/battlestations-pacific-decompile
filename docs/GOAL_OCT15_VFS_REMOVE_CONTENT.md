# Actual VFS content-package removal

`00BDD020..00BDD09D` (126 bytes) is reconstructed completely in
`src/native_vfs_remove_content.cpp`. The descriptive symbol remains a hypothesis.
The native interface is ECX=VFS manager, one stacked actual eight-byte string
header, `RET 4`, and no defined result. The new C++ interface carries the
existing concrete string-pool publication context explicitly.

The older `cc13_engine_core::remove_content_package_name_00bdd020` remains a
partial semantic projection: it constructs a stable list of survivors. This
entry instead borrows the actual vector at manager+48h and composes complete
`0043EA00` unordered erasure. A match is replaced by the last row and that
last row's storage is returned before count decreases. The same cursor is
examined again, so replacement order and a key aliasing mutable rows matter.

The listing captures count before begin for the initial end, compares equality
with a wrapping DWORD end, and reloads count before begin after each iteration.
It compares stored lengths before current-CRT `_stricmp`; empty/empty skips
both data pointers. Each comparison rereads the key. Empty vectors do not
read the key. The iterator local is updated on advance and passed by address
to the child, but the parent's cursor is not reloaded from it. There is no
local exception cleanup, rollback, range guard, or ownership transfer.

Both live callers were inspected: `00995475` in `00995150` passes the current
download row+8; `00995DA2` in `00995690` passes the current scanned row+8.
Both load the actual manager from `0109CEEC`. At `00BDD065`, CRT comparison
receives two pointers with `ADD ESP,8`. At `00BDD07D`, erasure receives the
manager+48h vector and the local iterator-cell address. The parent ends with
`RET 4` at `00BDD09B`. The live listing has 54 instructions and no gaps;
that is listing evidence, not a typed-flow or runtime equivalence proof.

Validation is recorded in `reports/goal_oct15_vfs_remove_content.json`.
Current CRT locale behavior is retained as a dependency. Original private CRT,
x86 stack/register/EH compatibility and gameplay have not been validated.
