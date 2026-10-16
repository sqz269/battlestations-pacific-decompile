# Actual CRT secure movement in a supplied callee domain

The whole ordinary _memmove_s at 00BF67A7 is reconstructed as move_native_crt_secure_bytes_00bf67a7. Fresh complete typed metadata, all 91 PE/live bytes and all 43 current instruction starts cover 00BF67A7..00BF6801 inclusive. The established library name is preserved. The earlier readiness brief's 39-instruction count was corrected to 43 before Source work; four physical CALL instructions reach three distinct targets.

Native is cdecl(D,Dmax,S,uint32 count), four stacked words, plain RET, errno_t in EAX. Source adds the actual stable canonical feature-word reference and the existing NativeCrtStricmpCallees reference: six caller-pop words instead of four. There is no native entry/frame/register/flag ABI claim. Native argument-slot reloads become Source value arguments, with count explicitly captured before validation.

Zero count returns zero before buffer validation or access to the supplied services or feature cell. Nonzero count validates destination before source, then compares unsigned capacity against the captured count. Null pointers select 0x16; insufficient capacity selects 0x22. The actual errno getter precedes a volatile write of the selected constant and a five-zero invalid-parameter invocation. Only a normal handler return yields the cached error, independent of handler EAX or subsequent errno changes. The wrapper adds no buffer clearing, catch, cleanup, noexcept, retry, rollback or extra validation.

Valid movement uses the published actual move_native_crt_bytes_00bf87e0 with the same canonical 0109EEA4 reference, ignores its returned destination, and returns zero. Its DF=0, valid nonwrapping extent, active-frame exclusion and actual vector-provider/SSE2 preconditions remain required where reached. Zero/error paths do not sample that feature word.

The existing NativeCrtStricmpCallees supplies only its errno-cell and five-DWORD invalid-parameter methods here. Its locale method is never invoked; no locale flag, new subclass/callback, default adapter or CRT owner is introduced. No concrete implementation was found in src/include/tests. The actual errno/TLS/getPTD owner and encoded-handler/Watson chain remain open runtime dependencies. Their existing fragment, tag-only and qualified child records are not promoted by this wrapper.

The earlier CurrentSDK memmove_s class method and checked-DWORD helper remain unchanged, including their separate reports. This new body recovers Original validation and return control for an explicitly supplied callee domain; it is not a new SDK fallback or an executable Original CRT closure.

## Owned Native transfer claims

- 00BF67BA CALL 00BFFB8B: actual errno service on the null-input path.
- 00BF67C9 CALL 00BF66EF: the shared Native five-zero invalid-parameter site.
- 00BF67DF CALL 00BFFB8B: actual errno service on the insufficient-capacity path.
- 00BF67F4 CALL 00BF87E0: actual movement after validation.

These are four owned sites, with no duplicate graph-walker or producer rows. Source has five emitted CALL instructions because the compiler keeps two separate invalid-method sites.

## Build and emitted object

The default MSVC Win32 build passed all three existing suites in 7.77 seconds; those suites do not execute this API. No new tests, probes or Native execution were added. The actual CMake object is 971 bytes: one public CODE section of 133 bytes / 60 instructions, one REL32 to the accepted movement symbol and four virtual method invocations through existing slots 4/8. All code, noncode sections and relocations were read. There are no helper functions, padding, embedded code data, BSS or EH sections.

The zero-count RET precedes D/S/capacity/service/feature loads. The unsigned valid arm alone loads the feature pointer. Both emitted errno stores precede their handlers; error literals are returned only after normal return. Compiler caching of D/S and three private zero pushes before each errno store are qualified changes from Original stack scheduling. Only reached C++ and public volatile effects are claimed.

Preserved evidence and exact hashes are in reports/oct16_native_crt_secure_memmove.json and the ignored native/COFF/build/frozen receipts. Original errno/TLS/handler ownership, heap integration, private-frame/argument aliases, fault/SEH/FH3 behavior, drop-in binary ABI and gameplay remain unproved.
