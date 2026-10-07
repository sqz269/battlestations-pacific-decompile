# Pilot-bot task owner source port (CC11)

Packet `cc11_land_owner_cpp`: **complete conditional normal owner control flow, unbound**.
Evidence target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 little-endian,
base00400000; the BSP client verified it before each live batch. Worker Ghidra analysis was
read-only. The primary's native audit is [LANDING_TASK_OWNER_CC11.md](LANDING_TASK_OWNER_CC11.md).
Names are descriptive hypotheses; CRT/library names are retained.

The new source facade has explicit stable task handles, borrowed active/retired state views and
required allocation, hook, scalar-delete and cleanup providers. It supplies no native task layout,
owning smart pointer, original vtable, default adapter or GameUnitsHost connection.

| Native range, end exclusive | Coverage | Original ABI | Source function |
| --- | --- | --- | --- |
| 00999E40..00999EE0 | complete assembly, normal flow | ECX=bot, RET | retire_all_native_pilot_bot_tasks_00999e40 |
| 00999EE0..00999F42 | complete assembly, normal flow | ECX=bot, RET | drain_native_pilot_bot_retired_tasks_00999ee0 |
| 0099A830..0099A848 | complete assembly | ECX=bot; tail JMP00999EE0 | disable_and_drain_native_pilot_bot_tasks_0099a830 |
| 0099A720..0099A7F8 | complete normal member flow; private EH excluded | ECX=bot, RET | destroy_native_pilot_bot_task_owner_members_0099a720 |
| 0099A010..0099A015 | complete raw thunk | ECX=plan, tail JMP004B7EF0 | embedded provider contract; no new entry wrapper |
| 00695870..00695931 | existing callback-owner normal contract reviewed | ECX=callback owner, RET | required callback provider; existing observer service available |
| 00875B30..00875B84 | complete assembly contract reviewed | ECX=bot base, RET | required base cleanup provider |

## State and retirement

Native active storage is pointer/count/capacity +58h/+5Ch/+60h; retired storage is
+64h/+68h/+6Ch. Source views borrow those roles through references, not a binary-layout
overlay. A handle is a caller-supplied identity with a stable live mapping. Array relocation
never changes task identity. This interface does not establish object ownership or cached
plane/squadron/observer lifetime merely by holding a token.

Retire-all appends active entries in their original order behind the existing retired prefix.
When count equals capacity, native00999E5C/60 publishes `capacity=2*old_capacity+2` BEFORE
allocation. The source makes the same publication, asks the required allocator for native
`capacity*4` bytes and source `capacity*sizeof(handle)` storage, copies only the used prefix,
frees the old array if present, then publishes the replacement at00999EA7. It appends each
active handle/increments retired count and finally clears active count. It invokes no task hook
or task destructor and leaves the unused active entries as they stand.

The provider must return successful disjoint live handle storage. Count must not exceed
capacity. Growth and its native four-byte request must not overflow; an old growing capacity
at most1FFFFFFEh satisfies that native request bound. Source-size multiplication must also be
representable. Actual process allocator/new-handler identity, failure/private exception handling,
corrupt counts, overlapping storage and native faults remain outside this port. No rollback or
strong exception guarantee is invented; capacity has already changed if allocation throws.

## Drain and disable

Drain calls the current non-null retired head's virtual+58h at00999EFA unconditionally.
It then **reloads** array/head at00999EFC/FF, applies the later null guard, scalar-deletes that
reloaded handle with flag1 at00999F0B, shifts the remaining prefix and decrements retired count.
The unused tail is not cleared. Capturing the pre-hook handle for deletion would change the
observed native sequence. A null after the hook skips deletion; it does not make a null input
to the initial unconditional hook valid.

The source contract admits a controlled hook replacement/clear of only the current head slot,
provided any replacement resolves to a valid live task. That is a source contract/fixture domain,
not proof of observed native runtime reentrancy. Reentry into owner methods or mutation of array
bases/counts/capacities during providers is excluded. Stable task mappings and all view references
must survive every admitted callback. Other arbitrary callback mutation is not established.

Disable writes +10h=1, then +11h=0, calls retire-all, and tail-routes to drain. It does not
delete the bot. Ordinary retirement is still deferred until a caller selects the native drain
schedule; source retirement itself does not make that scheduling decision.

## Separate member cleanup

Member cleanup first requires the native owner/callback profile-entry publications at0099A742/748.
It walks only active tasks: scalar-delete flag1, then clear that active slot; **no+58h hook**.
Then it invokes reverse embedded-plan destruction for owner+84h, stride1Ch, count2,
destructor0099A010 (last record +A0h, then +84h). It frees the retired array without deleting
its retired task objects, frees the active array, destroys actual callback owner+1Ch, and calls
base cleanup. It does not free the bot itself;0099A850 is the separate scalar wrapper.

The native leaves array pointer/count/capacity publications unchanged after freeing them. This
one-shot source view becomes unusable afterward; it does not normalize fields, replay cleanup,
run the drain implicitly, or fabricate deletion of remaining retired objects.

| Native site | Required callee/provider | Contract |
| --- | --- | --- |
| 00999E68 | 00BF55BE `operator new` | bytes=4*new capacity; cdecl ADD ESP,4 at00999E71 |
| 00999E9F | 00BF6989 `_free` | free old retired array; ADD ESP,4 at00999EA4 |
| 00999EFA;00999F0B | head vtable+58; reloaded head vtable0 | hook, then delete1; task layout/resolution external |
| 0099A83B;0099A843 | 00999E40;00999EE0 | retire-all then tail-JMP drain |
| 0099A776 | active task vtable0 | delete1 only, then slot zero |
| 0099A7A3 | 00BF7C6E `eh_vector_destructor_iterator` | owner+84h,1Ch,2,0099A010; stdcall RET10h |
| 0099A7B0;0099A7C0 | 00BF6989 `_free` | retired array then active array; ADD ESP,4 at B5/C5 |
| 0099A7D2 | 00695870 | actual callback owner+1Ch, observer detach/array release |
| 0099A7E1 | 00875B30 | actual owner base; stamp00CFD99C; conditional locked detach and +4h clear |

Listings mark gaps after CRT free because of a stale no-return property. Live bytes at
00999EA4,0099A7B5,0099A7C5 are all `83 C4 04` (ADD ESP,4), followed by the documented
normal continuations. The source includes those continuations; worker analysis changed no flow flags.

0099A010 is the raw five-byte JMP004B7EF0. Its existing normal observer-base wrapper and
00695870's actual observer service can be reused when a caller supplies their real storage/context.
The existing game-array iterator accepts only three different stride/destructor pairs and rejects
1Ch/0099A010; the port does not pretend it supports this one. Required embedded cleanup must
supply the actual reverse pair, including observer effects. Base00875B30 reads +8h/+Ch, resolves
its real lock, detaches from the referenced owner via00874E60, clears +4h and releases the lock.
These providers require actual layouts/services; names and source fixture effects do not implement them.

## Validation and remaining binding

MSVC19.51.36244.0 x86/toolset14.51.36231 compiled the source object with
`/EHsc /std:c++17 /MD /O2 /DNDEBUG`. One ignored manifested source fixture passed growth1->4,
retired prefix/active append order, retirement without destruction, old/new task identity,
post-hook head reload, disable flags-before-drain and the separate member cleanup sequence.
The fixture's controlled head20->40 replacement proves source reload behavior only. Its task
map, arrays, plan/callback/base records and allocator are fixture storage, not original runtime,
native allocator, ABI, observer-lifetime, queue, reentrancy or gameplay proof.
The existing live call verifier passed9 direct/tail rows; three task virtual-call rows are
explicitly indirect. `git diff --check` passed; the final staged check is recorded at closeout.
The worker did not run a full CMake build; the primary owns serialized main build/integration.

The audited producer installs0099A880/profile00D1F348 at plane+DF4h, with slot+10h0099A830.
For that profile, destroyed notification removes squad membership before DF4 disable/drain;
+6DCh is a different bot. Earlier base notification/observer effects and cached task+404h lifetime
remain unproved. This source owner does not preserve native objects by assumption, repair death
ordering, dequeue landing queues, or wire Boolean task flags into GameUnitsHost.

The next adapter needs a concrete task arena/stable identity map and real active/retired ownership,
valid land-task cached context through hook/delete, actual task hook/scalar profiles, owner/embedded
observer services and native drain call schedule. It must also preserve the deferred BE copy/receive
route and required live reindex publication from the previous packet. Native private EH/faults,
arbitrary structural mutation/concurrency, scalar owner deletion, full ABI and gameplay remain open.
