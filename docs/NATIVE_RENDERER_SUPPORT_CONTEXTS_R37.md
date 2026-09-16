# Renderer resource-support contexts

The physical-buffer and hardware-layout owner contexts now accept the existing `NativeResourceSupportLifetime` adapter. Semantic callers retain their original manager object; raw callers borrow the application's `NativeResourceSupportRawContext`, including the same AA0 and 108FEDC publication cells. The adapter owns neither a manager nor native storage.

`borrows_same_domain` compares identity, not publication values. Its semantic overload matches only the original manager object. Its raw overload matches only the referenced AA0 cell. Two null cells do not match, and copying the adapter preserves the borrowed identity. The existing raw getter also rejects a different resource-support publication cell before allocation or publication.

The stream-clone and mesh-buffer guards now use the semantic identity overload. Their physical-lock diagnostic contexts still require a semantic manager, so a raw resource-support route cannot silently pass those guards. Raw diagnostic lifetime remains a separate prerequisite.

No native routine body, pool, reference count, or native object layout changed. These source context layouts changed and all consumers require rebuilding; they are not binary replacements. No application renderer or generated-model factory is admitted by this change.

Validation: strict Win32 `/MD` build and three existing CTests pass. One extension of the retained raw-support fixture uses a real `GameSingletonHost`, constructs and destroys an actual 2Ch physical index buffer, observes one support registration in the shared raw manager, checks exact-cell identity and adapter copying, then drains the real D62B64 owner. Hardware-layout runtime, raw stream locking, active rendering, native exception identity and gameplay remain unvalidated. See `reports/native_renderer_support_contexts_r37.json` for artifact receipts.
