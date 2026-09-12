# Sound registration in the application lifetime manager

Addresses: 00415350, 00BD0400, 00BD0C30, 00BCFCA0, 00BD0D70,
00A7B190, 00A880E0, 00A7B230, 00A88180, 00A88770, 00A882C0,
00A778D0, 00A77970, 00A89A40.

SoundLifetimeAccess borrows either the application's actual01090AA0 publication
cell or the existing semantic fixture domain. It creates no private lifetime
domain and never drains the manager. GameSingletonHost exposes this borrowed
access and binds a concrete GameSoundRuntime before sound registration. The
runtime and all its services must survive GameSingletonHost::shutdown.

The manager view resolves00415350 before evaluating register/unregister/reorder
arguments. Registration therefore still captures the first manager's section,
publishes the sound owner, resolves the manager again, and then reads the current
sound publication. The section guard always releases its captured section, even
if a callback changes a publication. The actual path uses Win32 Enter/Leave and
the actual1Ch section's recursion word+18; the semantic path retains its existing
section provider. Raw registration/removal/reorder call the existing complete
BD0C30, BCFCA0 and BD0D70 entries.

The raw BD0400 loop is unchanged: pop, read the object's current identity DWORD,
delete with flags1, and reread the live vector count. Its finite source deletion
map now admits these concrete runtime-owned profiles:

| Profile | Source deleting operation | Storage |
|---|---|---|
| D5B44C | A883B0 sound system | C++ projection with identity DWORD at offset0 |
| D5B460 | A88750 sample cache | C++ projection with identity prefix |
| D5B478 | A89B40 event query lock | Existing query owner projection |
| D58F78 | A790D0 dialog manager | Actual234h storage and borrowed dialog bindings |

D5B210 belongs to the unregistered resource cache at sound owner+54. It is not
an admitted singleton profile. The concrete runtime releases unique ownership
before flags1 deletion. Manager/cache matching uses allocation identity rather
than current publications; their recovered base destructors still unregister
the current publication. Query and alternate owners are externally created;
their publication must preserve allocation identity until actual deletion.
Unknown profiles or missing bindings remain source contract errors.

SoundSystemOwner's first DWORD is now its identity. Its remaining fields are
canonical C++ state and references, not native178h layout. This integration
does not establish original object ABI, arbitrary virtual dispatch, hardware
fault/SEH equivalence, or gameplay validation.

Validation: strict Win32 Release and both existing CTests passed. The existing
installed-resource fixture passed three normal and two failure cases through
the semantic domain. A second ignored manifested executable repeated them
through the actual14h manager. Its drain case held five live registrations:
sound system, sample cache, query lock, dialog manager and an independent actual
gameplay-effect owner. It verified released section/vector storage, cleared
publications, balanced native strings and FMOD/VFS files. Explicit shutdown
also removed every sound registration before the raw drain. No permanent tests
were added. Independent review found the cache-profile error and mutable
publication/allocation dispatch mismatch; both are corrected.

Application phase5 construction and frame playback remain a follow-up. The
host accessors make that composition possible; this packet does not yet invoke
sound from GameStartupHost.
