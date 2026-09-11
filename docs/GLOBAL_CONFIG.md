# Global configuration singleton and complete normal lifetime

Addresses: 00432650 getter,004324E0 constructor,004325B0 destructor,
00432710 scalar deleting destructor,008DBCD0/008DBDB0 embedded effect state,
004312B0 seven-string cleanup,00431210 four-vector cleanup,00432050 string-range
cleanup,004C3810/00524180 pointer-slot cleanup,004BA0D0/0054D440 null primitives.
`reports/global_config.json` records exact endpoints, original ABIs, full-span
live/disk hashes, classifications, remaining bindings and validation provenance.
Names beginning BSP are descriptive hypotheses, not recovered symbols.

The canonical singleton slot00F878E4 holds an actual2E8h allocation. Its constructor
writes vtable00CE3D98; pointer triples10/14/18 through50/54/58; seven empty native
string headersA8..DF; and the embedded two three-pointer arrays at2C0/2CC plus
float2D8. All other bytes, including allocator words and unresolved scalar fields,
retain their allocation preimage. The C++ shell preserves those representation
bytes without a float load or blanket zero initialization. Exact raw storage
does not establish native calling-convention compatibility.

00432650 returns its captured nonnull singleton immediately. On a miss it calls
the existing concrete00415350 lifetime getter and captures the manager's section10.
It enters that section and increments its recursion18 before the second singleton
check. A remaining miss allocates2E8h, constructs, publishes00F878E4, calls the
lifetime getter again, then reloads the singleton for00BD0C30 registration. It
decrements and leaves the captured section before the final singleton reload.
Every startup count, selected-name access and final2D8 reset now uses this body.

All singleton families must share the existing01090AA0 `SingletonLifetimeDomain`.
Its registered-owner dispatcher must route this owner's00CE3D98 deleting slot to
`scalar_delete_global_config_00432710`, while retaining the other owner dispatches.
Creating a private domain for this configuration would change lifetime ordering.

The embedded constructor initializes both pointer arrays and float18, then runs
the native paired release/clear loop. The embedded destructor walks three pairs
in ascending order. For each nonnull second-array pointer it invokes the current
virtual8 with argument0, reloads that slot, releases its current pointer and clears
after the callback. It then releases/clears the first-array slot. Finally it runs
reverse slot-destructor passes over the second array, then the first array. These
passes are observable when callbacks repopulate previously visited slots.

Pointer cleanup uses a real InterlockedDecrement at the captured object+4 and
dispatches its current virtual0 only at zero. Initial null slots are untouched by
the standalone004C3810/00524180 destructors. The explicit extra native clears in
the embedded loops remain. Virtual8's descriptive stop role is provisional; the
required binding must execute the actual object's current method with flag0.

The full owner destructor then destroys seven strings in reverse field order,
frees four raw vector buffers in reverse order and zeros each pointer triple
after freeing. It captures the first vector's begin/end and destroys its strings
forward, then reloads begin for the buffer free. Native string headers remain
unchanged after returning their buffers. The first vector triple is zeroed,
00F878E4 is unconditionally cleared even after reentrant publication of another
owner, and the vtable becomes00CE3818. There is no manager unregister call.
The scalar wrapper frees only for flags bit0 and returns the original address.

Existing native string/storage, CRT, compiler iteration and lifetime facilities
are reused.00432050's range loop is recorded as integrated string cleanup, and
the already-named null constructors retain their existing identities. The two
embedded routines are real aggregate construction/destruction bodies; their old
array-helper/vector-deleting inventory tags are disproven. The correct scalar
deleting name at00432710 is retained. Six false free-return gaps in00431210,
004325B0 and00432710 were repaired under owned leases and the Ghidra write lock.

Win32 Release and both existing tests passed. One focused lifecycle fixture
checks lazy registration, lock balance, untouched allocation bytes, deleting
flags, callback slot replacement/repopulation, reverse effect passes, string
release order and unconditional publication clear. The existing full startup
fixture also passes using the concrete getter. That fixture needed a real owner
destructor dispatcher because the concrete lifetime domain rejects missing
callbacks; this was a fixture correction, not a production failure.

Remaining boundaries are actual effect virtual8/virtual0 implementations,
configuration population, native SEH unwind, drop-in function ABI and gameplay.
No game installation or saved game state was modified.

Follow-up packets: investigate00432750/004329D0 configuration population and the
00871BA0 effect acquisition chain only after checking current leases. Connect
other00432650 callers to the same canonical owner and lifetime domain as their
own runtime contracts are recovered.
