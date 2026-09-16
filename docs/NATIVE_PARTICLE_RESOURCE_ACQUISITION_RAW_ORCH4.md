# Raw particle cache acquisition

`acquire_native_particle_resource_00870dd0` reconstructs the complete 1,338-byte
body at 00870DD0..00871309. The native interface is ECX inner cache, four stack
arguments (name8h, forwarded word, retain-new, allow-load), EAX resource and
RET10. Only the low byte of each flag is read. The descriptive name is a hypothesis.

## Behavior and concrete providers

The routine captures the current platform publication and pumps load messages
before initializing local names. It normalizes the input, searches every alias
of each current record, constructs and normalizes a cache key through current
slot+4, then searches only the first alias of each record if that key differs.
A match in this second pass appends the original normalized input as an alias
before reading the current resource. A null resource does not imply a cache hit.

Every nonnull cache hit calls current slot+C to retain, including when loading
or retain-new is disabled. A miss with allow-load disabled returns null. Other
misses call current slot+8, construct a real 2Ch record and alias sentinel, query
the actual VFS date, append the record, and retain the new nonnull resource only
when requested. A null loader result still creates a record. Slot identities
are decoded to the genuine 86BA10/86BA60/871400 source implementations; numeric
image addresses are never invoked as host function pointers.

All strings, alias nodes, count growth, loader/parser, date query, record append
and cleanup use existing concrete providers. Sentinel/predecessor captures and
current link reloads follow the original body. Name copies reload source and
destination fields after raw resize. Date words are copied from the returned
pointer, not an assumed output identity. Loaded resources, inserted aliases and
earlier cache mutations are not rolled back after a later failure.

## Application contract

The context borrows the same actual string cells, loader/parser graph, VFS date
services/publication, original cache profile storage, and CRT validation policy.
The BDD340 raw overload uses raw strings for its own local name; its existing
traversal and provider children retain their actual-storage interface. Both
string services must identify the same owning pool.

Load events require the current 0109CF04 publication, the actual owner identity
to which the source service is bound, and that existing `ResourceLoadEventHost`.
The complete platform route can supply `PlatformServices`; the current startup
route has `GamePlatformServices::load_events()` and must supply its actual bound
identity separately. A different published owner is an explicit source binding
boundary. No default pump or arbitrary-owner lookup is fabricated. Original
D0DB40/D0DAF0 table views are borrowed through slot+C; unsupported profiles or
rebound slot targets stop explicitly at the reached dispatch.

The one-shot acquired invocation owns 23 stable native DWORD locals before its
optional loader invocation. The incoming parser builder kind is explicit.
Failed invocations and their borrowed domains must survive unresolved provider
obligations; existing failed VFS frames require process lifetime. Destruction
does not replay native cleanup or reclaim a loaded resource.

## Cleanup and evidence

Handler C96018 selects FuncInfo DC7FA0 and map DC7FC4: five states, magic19930522,
flags1, no try/IP/ESType entries. Previous states are `[-1,0,1,1,1]`. Actions
C95FF0/C95FF8/C96000/C96008 destroy the input, resolved key, temporary key and
temporary record name; C96010 destroys the whole temporary record. Normal
cleanup disarms each owner before returning its captured buffer. Secondary
exceptions during C++ unwind terminate. The native five-state schedule does not
own the loaded resource. The three-byte gap at 870E6D is skipped alignment.

The complete body, five actions/handler, FuncInfo/map and both profile views
match 1,512 installed/live bytes. Forty direct body CALLs and five unwind tails
pass the live call verifier; four indirect CALLs are documented separately.
The handler tail at C9601D has no containing Ghidra function and is verified by
its complete live/installed bytes. The strict Win32 build and all three
existing CTests pass. An independent full-listing source review found no mismatch.

Seven complete copied-original/source comparisons cover no-load miss, new loads
with both retain flags, first/later alias hits, a null cached row before a hit,
and a null cached row before a new load. Real physical VFS, file-date and parser
providers run; observations include record names/aliases/date words, counts,
resource references, low-byte flags, RET10, and genuine singleton/pool drain.
Source-only checks cover replay and unsupported platform/profile cleanup states.
A reached unsupported date-visitor slot after a successful load also verifies
state4 cleanup: the temporary record is destroyed, no cache record is published,
the constructed resource retains reference count1, its loader child remains
complete, and a retry of the failed acquisition is rejected before the pump.

The original parent calls bridge to source providers, and its virtual slots use
fixture callable bridges; the source uses installed numeric table words. The
fixture's platform policy is an observer. The resolved-key alias-insertion path,
allocation faults, other provider failures and second-exception behavior are
reviewed statically. These checks do not establish original child/EH execution, native
ABI/FH3/SEH/CRT identity, complete application installation or gameplay.
Full receipts are in `reports/native_particle_resource_acquisition_raw_orch4.json`.
