# Independent review: raw Text glyph-buffer supplier

Reviewed historical Source `93cbff9705e248dacca0edb2978f580fb98b3dba` and the owner's correction `6896c65e34f796e3a7912390ce3293bd9205aa49`. Both were merged into the reviewer's existing named worktree solely to review exact committed files. Only this document and its report are review-owned; no address or Source ownership was claimed.

**One P1 finding in93cb is fixed in6896. No other blocking Source issue was found in this bounded review.** The corrected supplier is ready for the primary integrator's common build/review. This does not admit an actual caller, changed Text/content path, frontend activation, Original ABI or runtime parity.

## Fixed finding

At93cb, `NativeGuiTextGlyphBuffersStorageFrame::~NativeGuiTextGlyphBuffersStorageFrame` (`src/native_gui_text_glyph_buffers_storage.cpp:91..93`) called `std::terminate()` whenever `requires_process_retention()` returned true. The operation's outer catch made failure sticky, but there was no public completion/disposition path to change a failed phase. Therefore even an admission error before native entry, or an ordinary propagated failure whose native graph was retained by the caller, inevitably terminated when its borrowed metadata frame left scope. The existing Mesh/stream/declaration owner contracts do not require this extra terminal policy.

The minimal fix is verified in6896: the frame destructor is default/no-op, the four-line CPP destructor definition is removed, and the explicit caller retention query/contract remains. No cleanup, retry, reset, new acknowledgment or failure policy was added. Byte comparison proves that the whole operation, declaration catch, factory/publication/import/return schedule and all helper implementations are unchanged. The corrected owner document/report state the qualification and preserve both historical93cb Source pins separately.

## Listing-to-Source review

The full new126-line header and189-line CPP at93cb were reviewed against all110 saved native instructions of `AB8400..AB852D` (302 bytes, unused incoming ECX, capacity/Mesh stack DWORDs, RET8). The exact correction diff was then reviewed. The following contracts match the listing within the existing Source provider qualifications:

| Native phase | Verified Source behavior |
| --- | --- |
|8426..8449 | Reached actual8B header zeros, real raw41DD40(16,1), captured current nonnull data then current length+1, mapped literal bytes. Resize/copy have no name cleanup arm. |
|8451..8465 | First current renderer, observed current38/B317E0 through genuine raw-header declaration/cache services, SAME canonical declaration registration. |
|8467..848B | Name state disarmed before explicit raw pool return. Header pointer/length are consumed by the existing real41DD20/raw-pool composition; no normal-release retry or declaration rollback. |
|8490..84AA | Second renderer read, capacity home read ONCE late, wrapping unsigned4*n, existing current5C vertex factory. |
|84AC..84B4 | Current Mesh home read after factory, SAME registry companion resolved to `NativeMeshReference`, exact `&reference.storage()==captured_identity`, then genuine B73BB0(mesh,0,vertex). No raw-to-typed Mesh cast. |
|84B9..84E2 | CE2220 target captured ONCE after vertex publication; use that same captured target for vertex then declaration returns before index factory. At returned zero use existing canonical actual+04/current-profile terminal; no second decrement. |
|84E4..850B | Third renderer read, wrapping6*captured capacity, current60 index factory(format65), separately reload current Mesh home after factory, B73B70, then index return using the SAME captured decrement. |
|Returned terminals | Metadata records retain identity/status; no raw storage access after possible terminal retirement. Existing registry metadata locking does not supply native lifetime synchronization. |

Domain checks use the SAME renderer publication reference, existing registry, declaration/string raw AA8/AA4/AA0 references, physical/pool/synchronization/profile and stream providers. The new context owns no renderer, stream, Mesh, declaration cache or count. Callback/lifetime validity and disjoint retained invocation backing remain caller proof.

## Failure and null-result qualifications

The current38 inner catch preserves the trigger in `name_cleanup_trigger`, disarms the name flag and invokes real raw header cleanup once. If cleanup itself throws, the outer catch records and propagates that cleanup exception; the original trigger remains separately retained. This is ordinary Source exception transport, explicitly qualified by the owner. Native FH3/SEH/private-unwind identity or secondary-exception termination is not proved by the saved handler shape. This review does not request new termination to imitate that unproved behavior.

Null factory returns and unsupported profiles are inherited Source-domain limits. The existing declaration adapter already throws `bad_alloc` for a null raw declaration; the stream adapters already throw for null creators before canonical registration. Native AB8400 instead later dereferences its result, so null/fault behavior is not claimed equivalent. The new supplier mirrors that established supported nonnull domain, with no fake stream, unknown-target success or no-op fallback.

The existing canonical zero dispatcher resolves the SAME companion, verifies actual+04 aliasing and uses its already-established `noexcept` current-profile terminal. This qualification remains inherited; this packet neither adds a new terminal nor proves arbitrary throwing native current00 behavior. Actual Graph/provider retention remains mandatory through every unresolved effect.

## Exact evidence audit

All35 unchanged supplier/tool/config pins match their declared immutable Git blob OID, LF content SHA256 and byte count at BOTH revisions. Both raw shared export SHA/length pins match. Both new93cb Source pins and both corrected6896 pins independently match Git LF bytes. The corrected document/report also match exact6896 Git content, and historical93cb pins remain unchanged in the corrected report. Working-copy CRLF normalization was not silently applied to raw export hashes. Full OID/SHA/length receipts are in the review report.

Corrected Source:
- Header: `d80b39a0dab6f1297098fc185cd833de0f928799b2903c8bacdce2384bd4444f`,6701 LF bytes.
- CPP: `729018eaa935d606d6ea233a856b6b2e598183bdfabb83974a0ee75b565a6c94`,10097 LF bytes.

Only static source/listing, exact Git/file pin verification, correction comparison and diff checks were performed. No build, test, probe, game/Original/reconstructed execution, process action, Source/CMake/ledger edit or Ghidra read/write was performed by this review. Existing immutable build/runtime receipts were not used to validate this unbuilt new supplier. Root's common MSVC Win32 build remains pending.

Actual changed ABAED0/ABA8D0, nonempty layout/glyph recursion, used caller custody, widget factories and frontend activation remain outside this packet and unclosed. New Native/vendor/duplicate-body/CG/runtime/ABI/gameplay/activation credit is zero.
