# Raw string-vector integration

The separate raw storage bridge supplies reserve 426520, resize 427110,
append 4CDC20 and destroy 4D0FA0 over the actual 0Ch data/count/capacity header
and eight-byte counted strings. The older typed vector source remains unchanged.
It supplies no caller-owned header initialization, VFS provider or loaded locale.
The bounded transport contract is a27c4ec624660875674a62fb0422ef9428bf9c8cc7b7a127a1987822fb4305da.

The bridge must preserve current count/data reads, constructor identity captured
before zeroing, forward old-name release, and old-backing free before publication.
Shrink decrements the actual count before returning each string. Append increments
it only after copy returns. Destruction resizes to zero then frees current backing,
leaving the pointer and capacity stale. Native bare-RET selected cleanup supplies
no copied-element rollback; escaped effects and provider custody remain retained.

The internal raw key copy uses fenced byte accesses and the genuine raw resize
service. Pool allocation/return use the same actual AA8/AA4/AA0 cells. Matched
current heap and overlap/zero-copy policies remain qualified Source bindings,
with no vendor reconstruction credit or Original ABI claim.

The frozen vector revision is 1432b1ae88c59671825e32ef90f499b9122b2a57.
Primary full-source review and the strict Win32 object check pass; the old typed
files are unchanged. The combined Win32 Release build and existing checks pass
(2/2, 11.47 seconds) at 74b8f2617. Independent review accepts the exact vector
Source code with no scoped blocker.

The coupled VFS exists-entry revision is fc50974eb and the fallback borrow is
b03a95511. Both small changes pass independent source review and the same build.
The raw VFS existence/override bridge, raw16 growth and parser composition remain
separate dependencies. Startup gates and the EMPTY source-admission hold remain.
