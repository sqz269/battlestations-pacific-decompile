# Native procedural sampler-cache continuation

Addresses: 00b1a4f0 00b1b4d0

This packet reconstructs the complete normal continuation **B1A51D..B1AA29**
over actual cache storage. **The full B1A4F0 and B1B4D0 entries remain
unimplemented.** Before entering the continuation, the caller must execute
actual BECCD0 on the raw platform captured from current 0109CF04 at B1A50E.
No input name contents may be read or copied before that pump returns. The
receiver passed to the continuation is the cache captured before the pump.
There is no replacement pump callback, projected platform or success token.

| Routine | Original ABI | Coverage |
|---|---|---|
| B1A4F0 | ECX secondary cache; stack name/options/retain-new/load-if-missing; EAX resource; RET10 | Full 1338-byte body analyzed; source continuation starts B1A51D |
| B1B4D0 | ECX complete1Ch owner; stack actual name; EAX resource; RET4 | Full 280-byte/95-instruction body analyzed; source entry unavailable |

B1A4F0 has 446 listed instructions and three unreachable alignment bytes at
B1A58D..B1A58F, skipped by the unconditional B1A58B jump. Linear decoding adds
one alignment instruction. The continuation is 1293 bytes, including that
padding and both native epilogues. All normal field/call schedules are covered;
the C++ interface does not implement native stack/SEH/FH3 delivery. Descriptive
names remain hypotheses. All queries verify the saved `C:/Users/sqz269/bsp.gpr`
program `/battlestationspacific.exe`; no worker Ghidra mutation occurred.

The actual1Ch owner is produced by 4DE4B0, with primary CE7D38 and secondary
CE7D24 at owner+4. Its existing `NativeResourceRecordVectorStorage` starts at
owner+8/cache+4. Counts and capacity are DWORDs; elements are existing actual
2Ch `NativeRenderResourceRecord` values. There is no second cache, projected
ParticleClock or resource reference count. The constructor leaves owner+18
untouched. At each reached dispatch, the source reads the current secondary
identity and current original profile slot: +4 B19E40 name copy, +8 B1B810
factory forwarder, +C 4DDB20 actual resource retain. Other profiles/selectors
raise explicit source-domain errors. D5F088/B30B40 texture loading is separate.

The continuation initializes and copies the requested header only after entry,
arms its cleanup only after initial copy returns, and invokes actual BEE690.
The first scan captures current vector end/base and each alias sentinel. It
checks all aliases, preserving current-sentinel validation before dereference
and advancement. B1A597 is unreachable because its comparison is EAX with
itself. A null resource for a matching alias skips to the next record. A hit
always retains through current+Ch, independently of the retain-new flag.

A miss constructs the hidden result through current+4, copies from the
returned header, releases that actual temporary and normalizes the resolved
name. Requested/resolved comparison uses empty/C-string semantics without
a stored-length equality precheck. If different, the second scan captures
the current vector again and checks only each first alias. Empty-list CRT
validation can return. A first-alias match appends requested through unchanged
4CE6F0 and 4CE780, then performs the original link stores before rereading
record+28. A null matching resource proceeds to creation without later scans.

Only a still-missing result tests load-if-missing. Creation calls actual B1B810
with the resolved header and options; this uses the shared actual singleton
manager/registry and existing B19E90 lookup. It may legitimately return null.
The source constructs the temporary actual2Ch record, preserving its reserved
word, appends resolved, queries actual BDD340 on the current manager captured
at B1A80D and copies five returned words in ascending interleaved order. The
physical date provider independently rereads current 0109CEEC. Requested is
added when different. The record is appended even if its resource is null.
Only this newly created result honors retain-new's low byte. Temporary record
destruction never retains/releases resource+28.

Normal cleanup captures current name data before disarming, then reads current
length for actual owning-pool return. The operation keeps actual headers,
temporary record, created resource, unlinked successful alias allocation and
the persistent B1B810 child available on failure. It neither invents resource
rollback nor changes existing helper EH/orphan-prefix behavior. Explicit
diagnostic cleanup precedes acknowledgment; failed/running frame destruction
terminates. Owners, publications and contexts must remain alive throughout.

B1B4D0's unimplemented full wrapper copies the name, captures its data at
B1B516, lowercases its actual header, constructs pooled `Default`, then calls
B1A4F0 with owner+4 and flags1/1. It cleans current Default data/current length,
then captured first data/current length. B1B400 instead forwards caller options;
both call sites consume RET10. Descriptor sampler type1 obtains the actual
owner at B3B2BA and calls B1B4D0 at B3B2C5 with sampler+18.

The unresolved entry dependency is specific: BECCD0 captures the incoming raw
platform, drains all messages using real Windows/XLive5030, then invokes BECB20
with that captured platform and loading1. BECB20 reads current raw F8ABE8 and
F8BBF4, performs actual A409F0 before reading platform+41/current manager+3E8,
and reaches further raw input operations. Current projected platform/online
interfaces do not establish that binding. This packet's continuation does
not remove that requirement or claim full procedural sampler loading.

Validation and transitive artifact pins are recorded in the companion report
and final local manifest. The focused fixture qualifies any copied original
event prefix to a null actual online manager, using the original guard; it
does not prove the unresolved nonnull A409F0/cursor path. Actual singleton,
resource and pool lifetimes remain in the fixture process arena until exit.
No full raw singleton teardown, private FH3, original C++ ABI, drawing or
gameplay behavior is claimed.

Final validation passed: default Win32 C++17 `/W4 /WX`, both CTests, all52
numeric call rows, three exact original/source output/pool snapshots, the real
message mutation case, source profile-boundary retention/replay and guard77.
The four copied fixture bodies differ only in direct CALL operands.

The executable fixture also relocates CE7D24's three reached function targets
to their concrete ABI bridges; the source's selector view remains byte-identical
to the original profile. Both views are pinned. B1B4D0 is copied and mechanically
checked but is not invoked. Its full source entry remains unavailable.
The fixture explicitly preloads the verified private Microsoft credential DLL
before the matching XLive DLL, following `docs/XLIVE_PRIVATE_RUNTIME.md`.
The initial system-DLL error182 is preserved separately. No installer, account
or SDK initialization ran, and no installed game or Windows file was changed.
