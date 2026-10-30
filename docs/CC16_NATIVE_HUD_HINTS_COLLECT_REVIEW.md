# Native HUD hints collector independent Source review

The Source body in immutable `70f1c6cef32575c5e93b58ef4a566e301e891b63` passes bounded review. The only finding is an evidence wording correction before final admission: the main RawScene Push/One/N entries are ordinary C++, rather than private naked child frames. The actual invalid-parameter bridge is separately naked. No body/order/provider change is needed.

Review baseline is published Main `b7f934b42056325c4d79a5f4b868847cd358b963`. Only this document and its report change. No candidate/source/address/shared/CMake/guard/Ghidra/peer edit, test, executable, target-body, DLL or game execution occurred. One strict object-only compile was authorized and passed; Root retains common registration/build/link admission.

## Complete native and Source dataflow

The original `0054E370..0054E37F` is all `16B/five instructions`: add1Ch to the actual hints receiver, push that ADDRESS, read the current stacked destination pointer, call `004D6790`, and `RET4`. Its five instruction starts, complete bytes, direct call at `0054E378`, and actual `CEE290+24` table word at `CEE2B4` match the preserved original PE. The table word is data evidence, not Source object authentication or a callable Source pointer.

The candidate computes unsigned 32-bit receiver+1Ch, writes that address to the actual child input home, THEN reads the CURRENT destination-pointer home. It builds a borrowed12-byte child frame and directly calls the real `push_back_native_scene_dword_vector_0046C450`. It never reads the page value, skips a null page, acquires a reference, constructs an owner, patches a table, adds a callback/allocator, or calls numeric original code. Existing semantic collectors and callers remain unchanged.

The independent MSVC Win32 strict object is the same complete `57B/20ops` as the author, with the same sole relocation to the actual RawScene Push symbol. Physical dataflow is:

| Source object offset | Proven effect |
| --- | --- |
| `0Bh` | Compute receiver+1Ch ADDRESS. |
| `11h` | Store that address into child home. |
| `15h` | Read CURRENT destination-pointer DWORD after the store. |
| `1Ah/20h/27h` | Populate local child metadata with the same home, actual output8 and actual One metadata. |
| `30h` | Direct call of real RawScene Push; no extra callee or branch. |
| `35h/38h` | Discard eight callee-argument bytes plus twelve local bytes, then plain Source RET. |

The frame metadata is stable and disjoint from the described homes/payload/backing. Within that Source contract, valid home/payload aliases preserve the declared order: if destination home aliases the child home, the current read sees the prior address store. Aliasing the child home with a page-slot/payload likewise remains governed by actual reached accesses. A private/native stack-home coincidence is not created by this metadata API. No early page or destination snapshot replaces a late read.

Only EAX/ECX/EDX, flags and ESP are written in this parent emission. There is no parent page-slot dereference, conditional skip, x87 or SSE operation. This does not establish transitive FP/fault behavior, child register preservation or a Native ABI. Its two-argument C++ interface/plain RET differs from the original ECX/one-stack-argument/RET4 interface.

## Genuine child/provider closure

All 26 retained dependency contents and Git identities match this review baseline. The complete frozen family receipt was verified before use, then independently rechecked against the installed PE: twenty whole native bodies (`848ops`), eleven full comparisons, three EH action/FuncInfo/map records and the actual length-message bytes. Every comparison preserves instructions, widths/registers/stack operands, branch indices and the explicit mapped calls/data tokens.

The mappings include whole Push `004D6790 ->0046C450` (`105B/44ops`), One `004D3B70 ->0046B4A0` (`137B/56ops`), N `004D1B20 ->0046AFB0` (`436B/181ops`), six changed helpers, canonical allocation/length services and common release. No general or fallback-only vector approximation is substituted. Native completed-message cleanup metadata is separately checked; it is not proof of Source/native private action-frame identity.

Current Source Push and One/N are ordinary C++ over actual raw vector16/homes. Fast Push captures begin and current end before the late home/value loads and does not dereference unused One/output backing. Fallback retains its actual current reads, returning diagnostics, output8 and full One/N metadata/backing, including the first append to an empty vector. Source N retains its real argument-cell mutations, arithmetic, progressive aliases and allocation/copy/fill/free/publication order. A null page-pointer DWORD is appended, not skipped.

The real allocation route is the existing canonical `singleton_lifetime_allocate` malloc/new-handler service, paired with `singleton_lifetime_free`. Existing Source length-error construction and completed-message cleanup are concrete. Original/foreign heap pointers, semantic vector/widget casts, fake allocators/release callbacks, new owners and profile-as-authentication are not admitted. Genuine hints/Page/vector/provider lifetimes and matching allocation provenance remain caller proof; this packet selects no application owner or collector.

## Precise evidence text correction

The immutable report's `failure_and_abi_qualification.child` says: `Safe C++ exception propagation across the existing private naked helper frames is not proved by this packet`. That wording overgeneralizes the actual main children.

Replace it with wording equivalent to: **The real RawScene Push/One/N entries are ordinary C++ and retain their existing qualified failure and length-error cleanup contracts. The reached `invoke_native_invalid_parameter_00BF6713` diagnostic bridge is explicitly naked and calls the actual SDK `_invalid_parameter_noinfo`; arbitrary handler C++ exception transport across that bridge is not proved here. Original private-frame/FH3/SEH/fault/register/stack ABI and broader caller behavior remain unproved.** The document's compressed `private/naked diagnostic transport` phrase should be made equally specific.

The diagnostic's actual source has the nine-op zero/five-push/call-SDK/add14h/RET schedule. It promises neither noexcept nor noreturn and does not reproduce the Original encoded-handler/Watson policy. That concrete bridge is distinct from ordinary Push/One/N and from the separate `00438E40` duplicate probe. The latter supplies no HUD child exception or full caller proof. Root may apply this scoped evidence correction without changing the reviewed body.

## Failure, custody and disposition

The parent adds no catch, retry, rollback, cleanup or terminal policy. A failure after its home store leaves that store and every completed child effect observable. The caller must retain the real metadata/homes, hints/Page/vector and child provider/backing graph through every reached callback and failed operation. Empty/reset headers, nominal type names or numeric profile words do not establish completed retirement or valid provenance.

Full pause `004D95F0`, parking `004D6480`, hint advancement `0054E440`, other collectors/current raw registry, genuine Game/screen/Page construction/adoption, full caller exception behavior, Original ABI/FH3/SEH/fault and runtime/gameplay remain separate. Parent/common link and application activation are not claimed.

The review pins all four immutable candidate files, twelve verified author artifacts, the whole family proof and 26 unchanged dependencies. The strict compile records all fifteen actual input files; its whole code/relocation match is independent. No tests or executable were created or run. This adds zero new Native/vendor/duplicate, Original ABI, full caller or activation credit.
