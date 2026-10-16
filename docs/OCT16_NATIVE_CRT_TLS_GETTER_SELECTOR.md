# Actual CRT TLS getter selection in a supplied context

Whole 00C05070 is50 defined bytes and16 current instruction starts. The provisional Source name describes observed selection/decode/cache behavior. The saved LIBCRT_unmatched_00c05070 name and comment remain intact pending primary closeout.

Native takes no arguments, preserves ESI, returns a raw pointer in EAX and uses plain RET. New cdecl Source takes three borrowed references, caller-pop12: the existing NativeCrtPointerDecodeContext, actual encoded getter cell109DE38 and actual current TlsSetValue IAT cellCE20B4. The SAME context already borrows actual E15B00 and current TlsGetValue IAT CE20BC; no shadow binding is introduced.

Capture the first index BEFORE sampling the getter IAT. A nonnull cached pointer returns immediately without encoded-cell/decoder/setter access. A miss reads the encoded word late and passes it to the published actual decoder with the SAME context. Capture the returned pointer. Native discards the decoder's outgoing first-word writeback. Reread CURRENT index before sampling current setter IAT; ignore setter BOOL and return the captured pointer even null.

| Whole entry | Native site | Actual transfer |
| --- | --- | --- |
| 00C05070 | 00C05077 | memory-indirect TlsGetValue through current CE20BC |
| 00C05070 | 00C05089 CALL 00C04FDE | published actual decoder with same added context |
| 00C05070 | 00C05098 | memory-indirect TlsSetValue through current CE20B4 |

Both import-cell identities/instructions are checked against PE imports/live bytes/current typed flow. These are not numeric function-entry claims in the transfer verifier; that verifier checks the one direct decoder row and reports two indirect rows separately. A separate15-byte/four-instruction producer window shows a pending PTD index retained across the zero-argument selector; it does not enlarge whole-body ownership.

This adds no TLS allocation/owner, encoded-pointer default, callback/subclass, replacement CRT, null guard, retry, LastError repair, catch, cleanup, rollback or noexcept adapter. The function-pointer alias is the actual Win32 IAT binding. Supplied module-gate errno/invalid services, bootstrap, PTD initializer, locale and concrete runtime remain open. Existing CurrentSDK and CRT helper/class APIs are unchanged. Source references/private frame/register/flag/fault scheduling, Original CRT/FH3/SEH/heap/binary ABI and gameplay remain unproved.

Preserved immutable readiness: runtime1ec6c34c; separate leaf addendum2b5e0254; authoritative corrected Root13 receipt6b11ab63. Initializer audit0c1b17ef separately records actual lock/SEH/runtime gaps; this leaf does not close them.

Default MSVC Win32 ./scripts/build.ps1 passed all three existing suites in8.36 seconds. They do not directly execute this new API; no new tests/probes/Native execution were added. Full actual CMake I386 object SHA256:76aec0d3d5ca850b891493a0711c65c9c22d09fb9673094a70dbc28a79e5abd0. One60-byte/32-instruction public function, one decoder REL32, two IAT-indirect calls, no helpers/EH/data/BSS/padding. All code, noncode sections and relocation were read; new code is not a binary replacement.

Before build, the accepted secure-move original object was copied byte-identically to local/oct16_secure_memmove_original_before_tls_getter.obj with explicit provenance in its adjacent JSON. Earlier frozen Source/report/receipts remain unchanged. Ignored local/oct16_native_crt_tls_getter_selector_* artifacts retain full evidence/decoded output; the tracked report records exact hashes/scopes/provider fingerprints/checks.
