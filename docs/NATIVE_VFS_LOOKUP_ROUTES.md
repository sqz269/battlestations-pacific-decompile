# Native VFS lookup routes

This packet reconstructs the BDD440 membership route and BDD600 mutable-name probe using actual manager, mount, provider, profile and pooled-string storage. It contains eleven complete native entries, 1324 bytes, including a dedicated implementation of the 672-byte BDD0A0 traversal qualified to the D68398 and D683E8 visitors. The existing date traversal and its source record remain separate. Names are hypotheses; the new C++ interfaces are not original binary ABI or SEH replacements and have not been game validated.

| Native entry | Complete bytes | Original ABI and role |
| --- | --- | --- |
| BDD440 | 223 | ECX captured manager, stack name, RET4, raw AL result byte; membership |
| BDD600 | 211 | ECX captured manager, stack mutable name, RET4, AL0/1; normalize/probe/replace |
| BDD0A0 | 672 | ECX manager, stack name/visitor, RET8, no specified result; qualified traversal |
| BD90D0 | 29 | ECX visitor, stack mount payload/name, RET8; provider +10 callback |
| BD90F0 | 4 | ECX visitor, RET, AL=current byte+4 without normalization |
| BDBC00 | 76 | ECX visitor, stack payload/name, RET8; owned-name assignment then provider +18 |
| BDB5D0 | 4 | ECX visitor, RET, AL=current byte+4 without normalization |
| BDB5E0 | 88 | ECX visitor, RET; release current owned string, reset base identity |
| BD8FE0 | 7 | ECX visitor, RET; distinct unwind-only base reset to D68380 |
| BB8640 | 5 | ECX and stack name unused, RET4, AL0; MPKG +18 |
| BBA080 | 5 | ECX and stack name unused, RET4, AL0; MSAR +18 |

`NativeVfsLookupRouteContext` borrows the actual D68398 and D683E8 three-word profile storage plus the existing physical context. Stack visitors retain their original numeric identities. Every reached +4 callback and +8 stop dispatch reads the current identity and current table word. The two callback words BD90D0/BDBC00 and two raw-byte getters BD90F0/BDB5D0 have concrete implementations; no provider callback object or synthetic profile is substituted. Each selected word still requires its actual compatible object layout: BDBC00 needs a live owning header at +8, and no extra storage is supplied for an incompatible profile mutation. Other identities or slot words raise an explicit `invalid_argument` source boundary. That boundary is not original native failure behavior or a general BDD0A0 reconstruction.

The observed profiles are D68398 = {BD9F50, BD90D0, BD90F0} and D683E8 = {BDBC50, BDBC00, BDB5D0}. Their scalar-destructor words are not reached by these stack routes. D68398 has an identity DWORD and a result byte at +4. D683E8 adds an owning length/data pair at +8/+C. Construction writes only the result byte at +4, leaving its three padding bytes alone. The separate BD8FE0 and existing BD90B0 are byte-identical base resets, but their native addresses and unwind edges remain distinct.

BD90D0 reads payload+8, then the current provider table+10, calls that concrete provider and stores returned AL at visitor+4. BDBC00 first assigns the suffix into the visitor's current string at +8. Only after that assignment does it reread payload+8 and the current provider table+18. The provider receives the mutable owned header; BDBC00 then stores its returned AL. It neither adopts a returned string pointer nor preserves a snapshot of the old provider or name. Both raw getters return the stored byte unchanged; BDD0A0 tests nonzero, while BDD600 explicitly returns zero or one.

| Concrete provider | +10 membership | +18 mutable-name probe |
| --- | --- | --- |
| D689E8 FileStore | BE5C00 actual primary tree contains | BE5C40 contains, leaves name unchanged |
| D64390 MPKG | BB8E80 -> separate archive-state pointer -> BB8E00 | BB8640 returns zero without reading arguments |
| D643C4 MSAR | BBA710 -> inline vector BBA650 | BBA080 returns zero without reading arguments |
| D69168 physical | BF3F70 complete physical lookup | BF39C0 checks then replaces with BF3970 path |

These methods use the existing complete raw lookup leaves and physical index/path implementations. Native provider tables are read at the call site and selected by current function word; no claim is made that all roots are physical. The base provider's purecall routes are outside this supported domain. MPKG and MSAR state are supplied by the caller under their established layouts; this packet constructs no archive, tree, vector, stream or provider owner and makes no normal-startup claim for latent MSAR.

BDD440 copies the query into a fresh raw header before any cleanup state is armed. After a successful initial copy, it owns the name, normalizes that temporary and constructs D68398. After traversal it captures the current name pointer, captures raw result AL, resets the visitor to D68380, disarms both scopes and returns the captured name storage through the actual pool using the current length. Its caller's name is not changed. FH3 E007AC/E0079C owns name cleanup CC6320 -> 41DD20, then visitor cleanup CC6328 -> BD90B0; unwinding reverses that order. No cleanup is invented for a failed initial copy.

BDD600 normalizes the caller's actual header before constructing D683E8 or arming visitor cleanup. It initializes identity, result byte and both owning name words, then traverses the captured manager. A nonzero result copies the current visitor string back to the caller, preserving identity, resize and the native load order: current destination length, destination data, then visitor data. It disarms the visitor before the normal destructor call on both exits. FH3 E0080C/E00804 owns CC6360 -> BDB5E0 throughout traversal and output assignment; it owns neither the caller's input header nor a visitor before construction.

BDB5E0 captures current visitor+0C data, reads current length+8 if nonnull, returns that allocation through the actual current pool, and only then stores D68380. It does not zero the released header or result byte. Its own FH3 E0035C/E00354 maps to CC6010, which reloads the saved object and invokes the distinct BD8FE0 base reset if release unwinds. The source retains that edge while documenting the inherited `noexcept` release limitation; no native-throw equivalence is claimed.

The dedicated BDD0A0 retains the complete current date implementation's verified traversal and temporary ownership. It sets passed manager+18 to FFFFFFFF before its initial name copy, walks the actual tree at manager+3C/head+40 in current link order, handles strict prefix-length and slash-boundary checks, uses returned substring pointers, makes the callback name copy, releases a conditional suffix before invoking the current visitor, rereads the current stop slot after callback work, releases the callback copy before stopping or advancing, and releases the main copy last. Current head capture precedes returning owner validation; the shared complete BD97E0 iterator handles every link and post-CRT continuation. Prefix-comparison temporary cleanup is only on the native normal path; no extra unwind scope is added. The full traversal FH3 span E00724 and CC62C0 funclets are refreshed alongside the new visitor maps.

All owning string operations use `ActualNativeStringPoolStorage`, its current publication/gate and canonical lifetime domain. Existing complete source providers cover raw 426060/425F40 copy/assignment, 41DD40 resize, 41DD20 destruction, BEE690 normalization, 469840 substring, 435C40 comparison, FileStore/tree/MPKG/MSAR membership, physical path/index lookup and actual pool allocation/return. Physical lookup/name replacement here do not perform the separate BF3A80 date route's current-manager+78 gate. Supplying the physical context does not add that date-only behavior to these routes.

Inherited boundaries remain explicit: C++ exception/CRT interfaces instead of original SEH; omission of a zero-byte `memcpy` in the raw string layer; `noexcept` release; source rejection of unsupported current selectors; and the caller's obligation to supply valid actual storage and string ownership. No extra normalization, pointer/capacity validation or conversion exception is added. Unusable nonempty strings and unsynchronized concurrent mutation are not promised portable C++ behavior. BB8640/BBA080 themselves perform no reads even with unusable provider/name arguments.

The ignored focused comparison executes 23 unchanged original bodies, 2230 bytes, covering every new entry and existing lookup/search/comparison/iterator dependencies. Original external string, pool, CRT and physical call targets bridge to the exact linked complete providers. Membership and replacement compositions compare returned bytes, all 2048 arena bytes and all actual pool bytes before the live critical section at 8AD484. Cases include ordered MPKG/MSAR/FileStore routes, misses, prefix boundaries, normalization, empty prefixes, real physical-file success/miss and a returning CRT handler that repairs the current head after nil-node advance. Direct checks cover raw 80 result bytes, the distinct base reset, no-read false leaves and BDB5E0 header/pool effects. Fixed native visitor profile addresses are reserved before the relocated code arena. Physical slot bridges are per-route calling adapters; no new provider behavior is supplied by the fixture.

The report freezes the guarded original spans, current source inputs, complete archive object provenance, probe and strict-build artifacts. This is composition evidence, not independent original execution of every pool/physical/string dependency, a throwing FH3 fixture, mutation testing of all visitor slots, archive startup proof or gameplay validation. Primary integration owns source registration and additional BDD0A0 symbol/source metadata; the generic Ghidra traversal name and existing date record must be retained. Shared date, renderer, node, getter and alias-list files are unchanged.


## Primary main-library integration

All entries are registered in main. Strict MSVC Win32 compilation, both existing
CTests and all eight native seeds passed.

The primary checked 51 worker pins, twenty-four literal current files and
53 fresh guarded spans totaling 4,397 bytes. The unchanged actual main-library
fixture passed twenty-four route pairs, one separate destructor pool pair,
two raw-byte getter pairs, two no-read false-leaf pairs and one explicit
base-reset pair. All eleven owned original bodies execute across these checks.
Twenty-three original bodies totaling 2,230 bytes are installed; the existing
BD90B0 dependency is not called because the normal reset is inlined, and its
throwing EH edge is checked statically. BD8FE0 is compared by a direct call;
its separate EH edge is also static evidence.

The comparisons include 51,200 arena bytes and 227,459,300 actual pool-prefix
bytes across the twenty-five route/destructor pairs, plus the sixteen-byte
base-reset header and scalar results. The real physical fixture file was
removed. Native profile tables contain translated code addresses while source
selectors retain original words; those setup tables are outside arena byte
comparison. Shared full rebuilt string/pool/normalization/physical/CRT bridges
do not establish independent original-provider runtime equivalence.

Eleven exact main archive members and 289 complete COFF sections were checked:
24,270 bytes and 990 relocations. Nine separate owned entries are retained;
BD90D0 and BDBC00 are inlined into the complete traversal. Entire source
objects are frozen. No runtime code postimages or throwing native EH case
was added. Five missing saved functions were defined from complete bytes.
The new qualified BDD0A0 source record supplements the existing date-specific
record and keeps its generic Ghidra name and earlier evidence.

The actual main library SHA256 is `afebfdb70e5013aa459ff7bdd15a27db70cc2e28126a6ce3229e43375a5f68ac`.
The read-only primary bundle is `local/vfs_lookup_routes_primary/`, seal
`405a8090cb00d69f85f3ba1a7b68bfe2cff0204af4908c64a4650cbf046c8c18`. Reviewed evidence was appended to preserved
Ghidra names/comments, saved, exported and registered in the sharded ledger.
Full original caller ABI and gameplay remain unvalidated.
