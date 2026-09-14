# Actual resource-support dispatch during singleton drain

The existing raw BD0400 singleton manager now admits the D62B64 resource-support
profile and calls the substantive B61D60 deleter with the popped owner and flags1.
The new optional binding borrows the same actual0108FEDC publication cell used by
construction. It does not compare that cell's current value with the popped owner.
The prior finite-profile cases remain unchanged. The binding is appended at byte
100; the existing 32-bit structure size check changes from100 to104 bytes.

BD0400 is the complete197-byte native body through BD04C4. It selects the last
current array slot, lowers the end before dispatch, reads the popped object's
CURRENT profile and slot0, and passes flags1. It skips null slots and reloads the
current count after each return. Once drained, it releases the actual section at
manager+10, frees the current vector at+4, then zeros +4/+8/+C. Its manager
allocation and actual01090AA0 publication remain owned by the caller.

D62B64's actual slot0 contains B61D60. The complete scalar body is41 bytes through
B61D88: test flags bit0, capture owner, unconditionally clear actual0108FEDC,
write CE3818 to the captured owner's profile, optionally free that owner, and
return its original address. The other four bytes of the8-byte owner are
untouched. The deleter does not unregister from the manager. It clears a rebound
publication without destroying the allocation now held in that cell.

The existing native_resource_support source already implements this destructor;
its body is reused without modification. Native address words identify supported
profiles and slots; they are never executed as original-EXE function pointers.
The map retains its existing contract error for a missing required binding or
unknown profile. It does not reproduce arbitrary native virtual dispatch.

The packet privately merges original migration checkpoint
54c998f9cc71d57695c29182b28b3bc928f65581 through
2bcb47aba4977054fa515dd2054cc1b7fbea699d. That dependency provides the
SoundLifetimeAccess getter overload used by the actual-AA0 fixture and preserves
the legacy Domain overload. Its source/header ownership and validation remain
separate from this packet's two manager-dispatch files.

Live bsp.gpr/PE equality covers both owned entry bodies, the profile word,
existing constructor and manager FH3 metadata. With explicit integrator
authorization, the official locked flow-repair tool clears the returning override
at B61D7B and recovers ADD ESP,4 at B61D80..B61D82. The full function ends B61D88;
no stored extent recreation is needed. The saved before/after repair record is
archived and the B61D60 export is forced. No callee-wide free annotation changes.

Validation includes eight native seeds before configure, the strict Win32 build,
both existing CTests and nine native direct/tail call rows. One local manifested
/MD fixture compares original/source B61D60 retained-storage behavior with flags2,
then registers a support owner using actual01090AA0, rebinds0108FEDC to a distinct
unregistered allocation, and drains the actual manager through the new case.
It verifies the publication clear, untouched replacement, released raw manager
vector/section, and caller-retained manager allocation/publication. The source
flags1 free is exercised without an allocation observer; the fixture never reads
the freed popped owner. Actual Win32 guard and section-delete providers are
measured from loaded modules in the x86 process.

BD0400's existing source C++ catch preserves the native state0 storage-cleanup
schedule (CC5450 to BD0220), and this change leaves it intact. The fixture does
not execute original manager FH3/SEH exceptions or establish native ABI identity.
Source/library/tool artifacts and measured runtime DLL files are archived
separately; later file hashes are not mapped-image hashes. No game validation is
claimed.
