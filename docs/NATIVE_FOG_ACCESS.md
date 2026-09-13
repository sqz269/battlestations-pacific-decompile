# Native fog owner accessors

Addresses: 00b84c90, 00b84ca0, 00b84cb0, 00b84cc0, 00b84cd0, 00b84ce0, 00b84cf0, 00b84da0, 00b84db0, 00b84e20, 00b84e30, 00b84e40, 00b84fd0

Thirteen complete native leaves (77 bytes) now access the actual fog owner.
The existing `SystemFogOwner` has its verified 94h layout and `SystemFogState`
at+8. Pass the owner itself; these leaves add no second field view or state
copy. Names and scalar units remain descriptive hypotheses. The complete
B84C60 ambient-color getter already exists and is reused.

| Native entry | Owner offset | Original result and ABI |
|---|---|---|
| 00b84c90..00b84c93 | +18 | ECX actual owner; zero stack slots; EAX owner+18; RET |
| 00b84ca0..00b84ca3 | +68 | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84cb0..00b84cb3 | +6C | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84cc0..00b84cc3 | +70 | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84cd0..00b84cd3 | +74 | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84ce0..00b84ce3 | +78 | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84cf0..00b84cf3 | +7C | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84da0..00b84da6 | +80 | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84db0..00b84db6 | +84 | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84e20..00b84e26 | +88 | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84e30..00b84e36 | +8C | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84e40..00b84e46 | +90 | ECX actual owner; zero stack slots; x87 ST0 from single FLD; RET |
| 00b84fd0..00b84fdd | +28 | ECX actual owner; one DWORD index slot; EAX owner+28+(index<<4); RET4 |

Scalar implementations are naked MSVC Win32 leaves containing the original
FLD and RET. The value remains in x87 ST0 without a local binary32 spill,
mask change, NaN normalization or native-to-host arithmetic substitution.
Underwater color returns owner+18 without a read. The directional getter
forms owner+28+(index<<4), wrapping in32 bits; its source API takes index in
EDX, whereas the native body reads one stack slot and returns with RET4.
No bounds check, retain, dereference or null substitution is added.

Existing scalar/color setters publish these same owner addresses. The owner
initializer requires a valid backing preimage and retains directional bytes
28..67. This packet does not assert complete constructor instruction parity.
B46A70 captures camera+184 for scalar/directional calls and reloads it for
its later color reads; that full gatherer remains incomplete across other
raw domains. Its subsequent x87 spills remain caller behavior.

Strict owned-source compilation passes. One private-process original-byte
fixture copies all77 bytes unchanged and compares66 scalar pairs (792bytes
of80-bit values/status), one underwater pointer and five directional pointers,
including wrapped indices with valid resulting addresses. The94h backing
remains unchanged. This is masked-FNINIT coverage, not unmasked hardware SEH,
full original-caller ABI, owner-constructor, concurrency or gameplay proof.
The fixture explicitly compiled this new source alongside the current three
libraries; registration and exact combined build/replay follow separately.
No repository test cases or framework were added.

## AZ integration analysis refresh

The integrator saved and read back all 27 AZ original signatures and complete
normal-body ranges, and refreshed exports. CBBBF0 and CBBC10 are ten-byte
analysis-only EH handlers defined under leases and the write lock. Earlier
missing-function observations remain worker capture history. The batch adds
22 complete body records and extends five existing bodies with raw interfaces;
the two EH definitions add no normal-body count. Exact combined validation
follows separately from worker fixture evidence.
