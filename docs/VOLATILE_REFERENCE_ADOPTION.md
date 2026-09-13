# Qualification-preserving reference adoption

Address: 0054D510. Related reuse candidate: 00484620.

The canonical `adopt_sound_reference_0054d510` now accepts a `void* volatile&` overload. The existing `void*&` overload adds qualification and delegates to that single implementation, then returns the original lvalue. A genuinely volatile handle cell therefore needs no cast that strips its qualification.

The existing behavior is retained: capture the old pointer once; when nonnull, release it through the required `VoiceReferenceHost`, clear the same cell after that callback, then adopt the supplied replacement without retaining it. Same-pointer adoption still releases first. The new overload does not supply reference ownership, atomic decrement or terminal destruction; those remain the existing host's complete contract. Volatile access adds no synchronization guarantee.

The active X effect-provider packet compares 00484620 with the complete native 0054D510 body and supplies its actual-owner adapter. That byte/graph proof and its original-byte fixture are separate from this source-interface change. W records the combined Win32/CTest result; native exceptions, stack aliasing and wider concurrency behavior remain unproved.

## X correction: discarded volatile return

The W nonvolatile delegator discarded the returned volatile reference as an expression. MSVC emitted an additional `MOV ECX,[ESI]` after the final slot store. The delegator now binds the returned reference and discards its address, avoiding that extra access while returning the original nonvolatile lvalue. No qualification is removed.

A retained before/after MSVC Hostx64/x86 `/O2 /W4 /WX` compilation confirms one slot read in both corrected overloads; their complete generated machine bytes agree. The volatile overload bytes are unchanged from W. The source, object files, full listings, compiler-discovered input headers and selected compiler binaries are retained by `local/volatile_adoption_codegen_x/manifest.json`. This is a code generation check of the C++ interfaces; native ABI and gameplay validation remain separate.
