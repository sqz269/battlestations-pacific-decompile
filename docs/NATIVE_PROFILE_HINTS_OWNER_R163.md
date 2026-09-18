# Actual profile-hints owner lifecycle (R163)

## Result and naming

native_profile_hints_owner.hpp/.cpp recovers four complete normal bodies over
the actual 50h owner and borrowed E17664 publication. The name is provisional:
it follows the existing AllSeenHints reader evidence, not a recovered symbol.
The source implements the observed storage and lifetime without assigning
meanings to the other unknown fields.

| Entry | Bytes | Original contract |
| --- | ---: | --- |
| 00426250 | 172 | ECX actual50h owner, EAX owner, RET |
| 00426300 | 17 | ECX owner, RET; clear publication and install base profile |
| 00426320 | 41 | ECX owner, stack flags, EAX captured owner, RET 4 |
| 004C1E90 | 189 | No inputs, EAX singleton, RET |

The shared source manager's BD0400 deletion dispatcher now admits profile
CE3A44 through this actual scalar destructor and its borrowed context.
NativeSingletonDeletionBindings gains a pointer at offset160 and is now164
bytes; this is a source-only bindings structure, not native object layout.
The ordinary application still uses its existing projected hints owner.
The raw language-catalog producer and its process binding remain open.

## Constructor and ownership

Construction writes profile CE3A44 at +0, zero at +8/+10/+24, and 32h at +0C.
It then zeroes a private 8h pooled string header, resizes it to ten characters,
captures its data and current length plus one, copies from CE3A38, and returns
that captured allocation/size through the actual pool. The literal is
"_auto_.log" including NUL. The current decompiler suppresses this copy/return
path; the implementation follows the verified assembly and executed bytes.
It is a temporary, not an owner string field.

After the temporary work, construction zeroes +38/+3C/+40/+44/+48 and finally
writes FFFFFFFF to +30. It preserves the other 36 owner bytes. No blanket
initialization or extra allocation is introduced.

Plain destruction unconditionally clears E17664, then writes CE3818 at +0.
It does not compare the publication with the receiver or unregister anything.
Scalar deletion performs those stores and frees the captured receiver only
when flags bit0 is set. It returns that address even after free and does not
clear the publication again after the free callback.

## Getter and source failure boundary

The initial nonnull publication returns its captured value without manager
access. The slow path resolves the actual raw manager, captures its +10
critical section, enters it, and increments its physical +18 tracked depth.
It rechecks the publication after entry. If still null, it allocates raw50h,
constructs it when nonnull, and publishes the result.

It then resolves the manager again, captures the CURRENT publication after
that lookup, and registers it through the actual BD0C30 source. Release always
uses the first captured section. The slow return reloads the publication after
LeaveCriticalSection. A nullable allocation still reaches registration with
null, as in the native body; the default source allocator normally throws on
exhaustion.

NativeProfileHintsOwnerOperation retains explicit source progress, including
the temporary, allocation, publication/registration target and captured guard.
If a source callback escapes, an acquired guard can remain held. The caller
must resolve those actual ownerships and release the guard before acknowledging
diagnostic cleanup. Replay is rejected and the operation destructor rejects
unresolved failure. No automatic rollback or original FH3/SEH cleanup is
claimed. The new interface therefore requires a retained operation and is not
the original no-input callable ABI.

## Evidence and validation

The configured BSP Ghidra project/program was verified. The previously absent
functions at 426300 and 426320 were defined from live bytes matching the PE,
under the write lock, then saved/exported. All four bodies have complete
instruction listings. Nine spans match live/PE bytes: 419 code plus40 data.
All ten direct CALL sites are checked against the live function bodies.

Strict MSVC Win32 /MD /W4 /WX /fp:strict and all three existing CTests pass.
One focused copied-native/source fixture has seven paired groups and1307
matching observed bytes. It covers:

- The entire50h owner, including 36 preserved preimage bytes, actual returned
  temporary contents, and unconditional plain destruction.
- Flags2/3 scalar behavior, return identity, and a publication write during free.
- Fresh and fast getter paths, another publication appearing during Enter,
  replacement of both manager and publication during the second lookup, and
  nullable allocation followed by actual registration.
- Actual raw pool/manager services, source BD0400 deletion, captured-section
  depth, and a drain involving two different actual managers.

The manager and raw-pool helpers are existing source services on both lanes;
the copied scalar/constructor/getter bodies execute their original instructions
with explicit relative-edge and import adapters. Allocation preimages are
controlled, pointer identities are normalized, and both lanes use the host
CRT and actual Win32 sections. A source-only registration failure checks
retained published allocation/lock, replay rejection, explicit cleanup and
final pool drain.

The nullable-allocation test removes its null diagnostic entry before draining;
ordinary native deletion would otherwise dereference that null. A superseded,
unregistered allocation in the publication-mutation test is cleaned up by the
fixture. No original allocator failure internals, private-stack aliases,
native FH3/SEH, asynchronous races, hardware-fault cleanup, ordinary startup
binding or gameplay parity is established.

See reports/native_profile_hints_owner_r163.json for hashes, saved annotations,
call verification, immutable archives and combined-build validation.
