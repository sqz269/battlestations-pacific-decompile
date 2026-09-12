# Native VFS derived manager

Addresses: 00BEDA60, 00BEDAC0

The actual derived manager now composes the reconstructed base manager, physical
factory getter and factory-list insertion. These descriptive C++ interfaces retain
the complete recovered behavior; they are not original ABI/FH3/SEH replacements.

| Routine | Original ABI | Coverage | Native extent |
| --- | --- | --- | --- |
| BEDA60 construct | ECX A0h owner; RET0; EAX original owner | complete | BEDA60..BEDAAE, 79 bytes |
| BEDAC0 deleting destructor | ECX owner; DWORD flags; RET4; EAX original owner | complete | BEDAC0..BEDADD, 30 bytes |

BEDA7D first invokes complete BE1DC0. Only after its success does BEDA82 arm
state0; BEDA8A writes D68D04. BEDA90 gets the physical factory through BED990 and
BEDA98 passes the captured result to BE0660 with ECX restored to the original
owner. No extra manager or factory publication is created by this wrapper.

Both contexts must use the application's same actual 01090AA0 publication.
The string-pool adapter also needs this raw lifetime domain. Its bridge and the
corresponding shutdown bindings are being composed separately; the source review
and standalone compilation do not establish that runtime composition.

FuncInfo E02044 points to the preceding unwind map E0203C: state0 has previous
state -1 and action CC74E0. That action loads ECX from EBP-10h and jumps to
BE1F60. The C++ guard is armed only after base construction and calls this same
full destructor on getter or registration failure. It adds no owner free,
publication rollback or factory-list rollback. Original FH3/SEH handler execution
and simultaneous cleanup failures have not been exercised.

BEDAC0 always invokes BE1F60, tests the low flags bit, optionally frees the original
owner through BF65AC, and returns that original pointer. High flag bits have no
effect. The original table word D68D04[0] is BEDAC0. The deleting body already
includes its final RET4; the three-byte ADD ESP,4 after BEDAD0 is a listing gap
repaired locally in this batch. No global CRT free annotation was changed, and
the refreshed body still includes its original final RET4.

Six spans totaling 175 live bytes agree with the installed PE: the two bodies
(109 bytes), unwind action, FH3 handler, exception metadata, and first profile
word. An independent review checked five direct call rows and 37 physical input
pins. Its sealed 59-file manifest is
df6be41cb97361b86b8066a24c3f7e82062c91e6449cd68f9713d07a2cb2c41a.
The integrated source passes the strict MSVC Win32 build and both existing
CTests. A composed original/source fixture is pending in this report revision.
Game behavior remains unvalidated. See reports/native_vfs_derived_manager.json
for the exact native spans, calls, source hashes and validation boundaries.

Dependencies: docs/NATIVE_VFS_MANAGER_LIFETIME.md,
docs/NATIVE_PHYSICAL_FACTORY_PACKET_MAP.md and
docs/NATIVE_VFS_FACTORY_REGISTRATION.md. Library routines remain external
contracts and are not counted as new reconstructions.
