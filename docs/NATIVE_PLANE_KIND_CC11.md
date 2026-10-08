# Actual plane kind predicate

`native_plane_is_kind_0074e400` preserves the complete native entry
`0074E400..0074E436` end exclusive: 54 bytes and 19 instructions. Its strict
MSVC Win32 function is byte-identical to the installed PE and saved program,
SHA256 `286eac696db40b93f7100a7bac5761a6963a7cbf461ce260d645dfb3d227573e`.
There are no calls, globals, x87 operations or relocations.

Native ECX is the actual plane, the stack DWORD is the requested class word,
full EAX returns zero or one, and RET4 consumes that DWORD. The Source raw
fastcall spelling supplies ECX and an explicitly unused EDX before the stack
argument. This component convention does not bind an original class table or
establish replacement ABI for a complete class or higher caller.

The original compares the requested word in order with 15, 5, 4, 2, 1 and
zero. A match returns one before reading receiver storage. Otherwise it
compares against the actual live DWORD at receiver+C4 and returns the result.
The dynamic comparison preserves all 32 bits. Word six is not a fixed ancestor:
its answer still depends on the real C4 cell, and cannot be inferred from the
plane profile alone. No semantic handle, copied class value, default or
virtual service is introduced. Stable live backing through C8 is required
when that field is reached; faults and concurrent access remain unvalidated.

Current actual profile DATA `00D05F20+5C = 00D05F7C` contains `0074E400`.
Constructor instruction `007CFD78` stamps `00D05F20` into its actual receiver.
Both spans match the PE and saved program. They are structural witnesses,
not a fabricated callable Source table, a complete constructor or class
admission proof. The fixture calls the predicate directly and never installs
or executes a substituted profile.

One unique connected fixture uses a genuine live C4 DWORD in a guarded Source
receiver prefix. It executes all 54 unchanged, unrelocated original bytes and
the new raw entry on the SAME storage. Ten cases cover the six fixed ancestors,
a full-width dynamic match, a nonmatch, and two observations after changing
that same live cell. The setup values are controlled Source instrumentation,
not recovered constructor publications. All 32 checks pass: exact native EAX,
full Source EAX equality and complete backing/guard preservation.

The strict `/W4 /WX /fp:strict /O2 /Gy /MD` manifested PE32/I386 probe has two
fresh TUs, two project includes, 213 actual host includes, seven searched host
libraries and four selected Hostx64/x86 tools pinned before/after. No BSP
support library, CALL bridge, instruction edit or table relocation is required.
Whole COFF and input receipts are in `local/cc11_primary_plane_kind_run01/`.
No existing fixture family or tracked test was added or replayed.

This provides one genuine dependency for the actual virtual type operation.
Complete `0070DA00` speed refresh still requires the other actual profiles,
their type providers and live ship/nonship receiver/tuning ownership. This
predicate alone does not close that routine, detach, plane construction,
class dispatch, original heap/lifetime, faults, world or gameplay. Primary
integration adds the repository build, existing CTests and saved-analysis
receipts separately.
