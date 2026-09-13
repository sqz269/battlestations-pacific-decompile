# Qualification-preserving reference adoption

Address: 0054D510. Related reuse candidate: 00484620.

The canonical `adopt_sound_reference_0054d510` now accepts a `void* volatile&` overload. The existing `void*&` overload adds qualification and delegates to that single implementation, then returns the original lvalue. A genuinely volatile handle cell therefore needs no cast that strips its qualification.

The existing behavior is retained: capture the old pointer once; when nonnull, release it through the required `VoiceReferenceHost`, clear the same cell after that callback, then adopt the supplied replacement without retaining it. Same-pointer adoption still releases first. The new overload does not supply reference ownership, atomic decrement or terminal destruction; those remain the existing host's complete contract. Volatile access adds no synchronization guarantee.

The active X effect-provider packet compares 00484620 with the complete native 0054D510 body and supplies its actual-owner adapter. That byte/graph proof and its original-byte fixture are separate from this source-interface change. W records the combined Win32/CTest result; native exceptions, stack aliasing and wider concurrency behavior remain unproved.
