# Native VFS file-date discovery

Read-only follow-up to `NATIVE_RENDERER_FILE_RELOAD_DISCOVERY.md`, based on
main `ec8c2947` plus that report's `568601ea` (`535c4db0` locally). The
companion JSON pins 81 original executable spans / 8,160 bytes against fresh
guarded reads of `bsp.gpr` / `/battlestationspacific.exe` and hashes 32 current
source-provider files.
No source, Ghidra, shared metadata, installed binary or saved-analysis change
is included. Names describe hypotheses, not recovered symbols. Source, build,
original-ABI compatibility and game validation remain separate claims.

The date route can be divided into small complete source packets. Its raw
FileStore lookup and package-zero leaves do not require reconstructing stream
ownership. Physical dates require the full indexed existence branch, the
unrecorded physical name-replacement body, and actual string-header adapters.
The existing `VfsMountContext` date implementation does not supply those raw
operations.

## Entry, output and manager identity

BDD340 is a complete 249-byte routine: ECX is the actual manager, stack
arguments are hidden 20-byte output followed by an eight-byte native name,
EAX returns the supplied output, and RET8 removes both arguments. It:

1. Captures the manager once in EDI. Builds a 24-byte stack visitor with
   profile D683B0 followed by five zero DWORDs.
2. Clears a local native string, copies the input through actual resize and
   current-field memcpy, then calls BEE690 on that copy.
3. Calls BDD0A0 with the captured manager, normalized copy and actual visitor.
4. Copies visitor+4/+8/+C/+10/+14 into output+0/+4/+8/+C/+10 in that order.
5. Disarms local-string cleanup before its normal pool return and returns
   the output address. It does not clear or otherwise restore the input.

The output is untouched until traversal returns. Copying and normalization
are complete before output writes, including when caller output aliases the
input header. There is no null-header, embedded-NUL, signed-length, provider
availability or normalization-success check. BEE690 has no boolean result.
Its actual-header implementation already exists in
`src/native_pooled_resource_path.cpp`, together with the actual substring,
resize and string-pool dependencies.

BDD340 itself does not read global 109CEEC. In contrast, physical date
BF3A80 reads the **current global manager** and byte +78 at its own entry.
That can differ from BDD340's captured traversal manager after a reentrant
operation. FileStore and both package date leaves ignore +78.

The actual A0h manager is published by BDA6F0 at BDA746, initialized by
BE1DC0 and finished by BEDA60 with profile D68D04. Relevant storage is:

| Manager offset | Recovered use |
| --- | --- |
| +00 | Current seven-slot provider-manager profile D68D04; +18 selects BDB040 |
| +18 | Last-error/status word; BDD0A0 writes FFFFFFFF before copying the name |
| +30/+34/+38 | Factory-list owner/sentinel/count |
| +3C/+40/+44 | Mount-tree owner/sentinel/count |
| +78 | Physical-date-disable byte; constructor writes zero |

D68D20 belongs to the adjacent frame-clock profile, as verified by its
separate constructors. Its methods must not be appended to the VFS profile.
No always-zero assertion about the mutable +78 byte follows from construction.

## Ordered mount traversal and callback

BDD0A0 is complete through BDD33F, 672 bytes; ECX manager, native name and
visitor stack arguments, RET8, no semantic EAX result. It copies the supplied
name again, without normalizing it, then uses actual tree nodes rather than
a copied mount vector. Iterator storage is `{tree_owner, node}`. The owner
is manager+3C and the initial node is `manager.head->left`.

| Mount node offset | Recovered field |
| --- | --- |
| +00/+04/+08 | Left, parent and right links |
| +0C | Signed priority |
| +10/+14 | Prefix length/data |
| +18 | Raw provider pointer |
| +1C | Copied ownership byte; not interpreted by date traversal |
| +20/+21 | Color/sentinel flag; traversal tests +21 |

The callback receives `node+10`, a 16-byte mount payload: prefix header,
provider+8 and ownership byte+C. BDCFC0 copies those fields, BE1740 passes
the raw provider without AddRef, and BE1330 orders higher signed priorities
first. Equal priorities descend right, preserving prior insertion order.
This is not longest-prefix selection. Mount construction normalizes/trims
prefixes separately; the date query does not redo that work.

For an empty prefix, every name matches. Otherwise the stored prefix length
must be unsigned-less than the copied name length; a substring of exactly
that length must pass 435C40, and the byte following it must be '/'. 435C40
first requires equal stored lengths, then applies empty-length gates and
CRT `_stricmp`. Matching is not length-bounded `memcmp`. The remaining
suffix is copied into another native string; nonempty prefixes use substring
start `prefix.length+1`, count 7FFFFFFF. Substrings retain the native `strncpy`
padding behavior after embedded NULs.

BDD0A0 loads current visitor+4, calls it with mount payload/suffix, then
reloads the visitor table and calls current +8. An AL-nonzero result stops.
Otherwise BD97E0 advances the existing iterator using current links and
sentinel byte +21; the next iteration reloads the tree's current head. There
is no vector snapshot, provider retain, structural-mutation recovery or
automatic manager replacement. Native invalid-iterator checks call BF6713
and continue at the next native instruction if that handler returns.

D683B0 is exactly `{BD9F30, BD9E80, BD9F00}`. BD9E80 is 66 bytes, ECX
visitor, mount/suffix stack, RET8. It reads mount+8, loads that provider's
current table+20, and passes a local 20-byte output plus the suffix. It then
copies five words from the **returned EAX pointer**, not unconditionally
from its own hidden-output buffer. It overwrites prior visitor words even
for a zero result. BD9F00 returns EAX 1 if any saved word is nonzero, else 0.
No exception guard or provider retention appears in either visitor method.

The stack visitor's cleanup BD90B0 only installs base profile D68380.
BD9F30 is its complete 31-byte scalar deleting wrapper; the stack path does
not call that deleting wrapper or free the visitor.

## Concrete provider domain

| Established construction route | Provider profile | Current +20 date body |
| --- | --- | --- |
| BED990 factory / BF4DF0 -> BF4D30 | D69168 physical directory | BF3A80, 399 bytes |
| 4FC150 factory / BE8120 -> BE80B0 -> BE7FA0 | D689E8 FileStore | BE5C80, 109 bytes |
| 736A90 factory / BB9D90 -> BB9CB0 | D64390 MPKG | BB9D50, 23 bytes |
| 736D00 factory / BBBBF0 -> BBB5B0 | D643C4 MSAR | BBB640, 23 bytes |

The first three factories are registered in that order by actual manager
construction and Initialize73D410. BDB040 walks the factory list and returns
the first nonnull creation result. BE1890 stores device ID at provider+10,
then registers that actual pointer through BE1740. Physical accepts a
nonempty system name ending in backslash; `persistent_data` selects its
accept-all flag. FileStore recognizes the named `filestore` provider and
shares the lazy object at factory+8. MPKG recognizes the final `.mpkg`.

MSAR is a separate native possibility: 73BC40 calls 736D00, registers its
factory, and can mount `ContentPackage/*.msar`. Fresh xrefs report no caller
or reference to 73BC40; this pass therefore does not claim it is reached by
normal startup. Its profile and date leaf are nevertheless concrete, and
the current typed three-factory manager does not model it. Its date leaf
does not require implementing archive opening. The base provider profile
D641A0 has `_purecall` at +20 and is not a date provider substitute. No
arbitrary injected/custom factory domain is claimed.

All these providers share the BB5590 prefix: current profile+0, intrusive
count+4 initialized to 1, copied original system-name header+8/+C and device
ID+10 initialized to FFFFFFFF. The date route does not change that count.

## FileStore and package leaves

BE5C80 captures FileStore's sentinel at +18 **before zeroing the caller's
five output words**, then calls BE5A50 on the current primary tree at +14.
Output stores run in descending offset order, +10 through +0. If the
returned iterator node differs from the earlier captured sentinel, it
writes all five words FFFFFFFF in the same descending order. The iterator
owner must be nonzero and equal to FileStore+14; the native CRT check may
return. There is no normalization, stream lookup, open, retention or EH
registration in this date body.

BE54D0 implements the actual primary-tree lower bound and BE5A50 completes
the find with 443D00. Empty stored lengths sort before nonempty ones;
nonempty comparison is `_stricmp`, with no stored-length tie-break. The
find returns `{actual_tree, selected_node_or_current_head}`. Raw primary
node fields are links+0/+4/+8, key header+C/+10, stream pointer+14,
color+18 and sentinel+19. FileStore construction creates the sentinel and
zero count at owner+1C. BE7760 normalizes insertion keys and BE6090 copies
the key/retained stream payload; the date lookup does not need those stream
ownership operations implemented in order to read an existing actual tree.

BB9D50 and the presently unrecorded BBB640 each write five zero DWORDs,
descending +10 to +0, return the hidden-output pointer in EAX and RET8.
They consume neither the provider nor the name argument. MPKG/MSAR do not
query entries or synthesize timestamps. A FileStore match stops traversal
even with physical dates disabled; a package zero result allows later
matching mounts to be tried.

## Physical date, name replacement and indexed existence

BF3A80's saved Ghidra body ends at BF3ACB, but its zero-disable branch jumps
into separately listed BF3ACC. The full native routine ends at BF3C0E:
399 bytes, ECX provider, output/name stack, EAX output, RET8. Nonzero current
global-manager+78 writes five zeros and returns without copying a name.

Otherwise it copies the supplied native name, calls current provider+18,
and ignores that call's AL. If the resulting data pointer is null, it passes
the saved empty C string at 109DBEC; otherwise it passes the current data
pointer to GetFileAttributesExA. The queried time is last-write time, using
FileTimeToSystemTime directly, without local-time conversion. Its five
words are year, month, day, `second + 60*(minute+60*hour)`, milliseconds.

An attributes miss calls GetLastError and the verified bare-RET diagnostic
4254B0 with the **original input name**, then writes zero date words. A
FileTimeToSystemTime failure is not checked: the native reads its output
buffer anyway. The seconds/milliseconds DWORD was zeroed beforehand, while
other SYSTEMTIME words were not all initialized. This gives no defined
zero-date or exception result for that failure. The typed adapter's
`runtime_error` changes this native behavior. Normal date output is written
before disarming/releasing the copied path.

Current D69168+18 selects BF39C0, a complete 188-byte native body without a
Ghidra function record. It calls current +10=BF3F70 with the mutable name.
False returns immediately without changing that header. True calls current
+1C=BF3970 into a temporary, copies the returned actual header back into the
supplied name, releases the temporary and returns AL 1. It does not replace
the name after a failed existence check; BF3A80 still queries the unchanged
name in that case.

BF3970 concatenates the provider's root header+8 with the suffix, inserting
no separator. After concatenation returns, it reads the **current** root
length as its slash-conversion start. It rereads result length/data while
changing suffix '/' bytes to '\\'; root bytes before that start are not
rewritten. Raw concatenation 4261A0 clears its output even on identity,
copies current left fields, then appends a captured right count at a captured
old output length. Its output-cleanup flag is armed only after initial copy;
it is cleared before exceptional output destruction.

BF3F70's full 586 bytes include all three actual modes:

- Empty stored input length returns false. A 435C40 match with last-success
  header+20/+24 returns true; then accept-all byte+28 returns true for any
  nonempty input. Both early paths avoid OS access and cache mutation.
- Otherwise current +1C builds a path. The routine rereads +28 and index
  count+34 after that call. If accept-all is then nonzero or count is zero,
  GetFileAttributesA checks the built path; any result other than FFFFFFFF
  succeeds, including directories. Success copies the original supplied
  suffix to +20; failure assigns the empty C string there.
- Nonempty index mode finds the suffix after the last '/' in the original
  name using 467CF0. That reverse search excludes position zero and returns
  -1 on a miss; it is not interchangeable with `std::string::rfind`.
  It finds the first equivalent basename via BF36D0/BDA260 in tree+2C,
  then walks the equivalent-key range with BD9860. For each reached node,
  435C40 compares the full name at node+14 with the original input. It stops
  at the captured end or a 449AF0 unequal basename. No success-cache update
  or OS query occurs in this branch.

The physical index node has links+0/+4/+8, basename header+C/+10, full
relative-name header+14/+18, color+1C and sentinel+1D. Constructor BF4D30
creates this empty tree at provider+2C/+30 with count+34=0, but that initial
state is not substituted for the reached nonempty branch. The writer that
populates a nonempty physical index was not established in this bounded
pass. The full reader's actual storage and operations are established.

## Native exception and CRT boundaries

| Routine | Recovered cleanup states |
| --- | --- |
| BDD340, FuncInfo E00778 | State0 -> -1 visitor-base reset CC6300; state1 -> 0 current copied name CC6308. Initial name construction is not protected by its own outer cleanup. Output precedes normal disarm/free. |
| BDD0A0, E00744 | 0 -> -1 main copied name CC62C0; 1 -> 0 conditional suffix temporary CC62C8, clearing flag bit2 first; 2 -> 1 suffix copy CC62E1; 3 -> 0 suffix copy CC62E1. Callback state3 excludes the already-destroyed intermediate. Prefix-comparison temporary has normal flag-bit1 cleanup but no additional unwind-map state. |
| BF39C0, E02828 | 0 -> -1 returned path temporary CC7AD0, armed after +1C returns. No mutation rollback. |
| BF3A80, E02854 | 0 -> -1 copied path CC7AF0, armed after initial copy. Disarmed before normal release. |
| BF3F70, E028E0 | 0 -> -1 built path CC7B60; 1 -> 0 basename CC7B68. Each normal release is preceded by its state change. |
| 4261A0, D84D50 | State0 invokes conditional output cleanup C5E830; flag bit1 starts zero, is armed after initial copy, and is cleared before destruction. |

There are no native catch maps in these routines. BF6713 forwards five
zeros to BF66EF; that decodes current CRT handler 109DD64. A nonnull handler
may return, after which the original caller continues; the default route
invokes Watson. Do not replace these sites with unconditional throws or
early returns. Existing raw containers already use an explicit returning
invalid-parameter binding. Current host CRT, Win32 API and string-pool
exception boundaries remain explicit; native SEH/ABI equivalence is not
claimed by this document.

## Ready source packets and remaining integration limits

1. **Raw FileStore date plus package leaves.** Own new
   `native_vfs_date_leaf_providers` source/header and its report. Implement
   complete BE5C80, BE5A50, BE54D0, BB9D50 and BBB640, with actual header
   comparator 443D00 and existing returning CRT boundary. This is ready over
   actual caller-supplied provider/tree storage; it neither constructs a
   `FileStore` projection nor needs stream ownership or archive parsing.
2. **Raw physical date/path reader.** Own new `native_physical_file_date`
   source/header and report. Implement complete BF3A80, BF39C0, BF3970,
   BF3F70 and the small physical-tree readers BD92C0/BDA260/BF36D0/BD9860.
   Include actual-header adapters for 435C40, 449AF0, 467CF0, 4261A0,
   425F40 and the reached CString construction/assignment operations,
   preserving the documented current fields and cleanup states. Share the
   443D00 adapter with packet 1 through primary integration rather than
   independently editing its metadata or duplicating the implementation.
   Existing actual resize, substring, normalize and pool providers are
   usable; the object/`std::string` projections are not raw adapters.
   This is ready for bounded implementation of the complete reader,
   including nonempty indexed storage. Constructing/populating that index
   is a separate unresolved owner/writer packet, not a reason to omit the
   reader branch. Preserve the unchecked time-conversion result.
3. **Actual date-entry integration.** Own new `native_vfs_file_date`
   source/header and report. Once packets 1 and 2 exist, integrate full
   BDD340, its actual D683B0 visitor operations and native mount traversal,
   including BD97E0. Bind each current selected provider slot to the real
   complete operations above. Use the existing actual-header BEE690, not
   a guarded boolean normalization wrapper. It remains blocked until those
   concrete children exist. A full general BDD0A0 export serving its other
   callers also needs their distinct visitor profiles; this date-created
   visitor domain does not close every VFS operation.

These file suggestions are disjoint candidate packets, not new leases or
source claims. No date-provider callback placeholder, physical-only mount
assumption, empty-index substitute or general VFS rewrite is supplied.
