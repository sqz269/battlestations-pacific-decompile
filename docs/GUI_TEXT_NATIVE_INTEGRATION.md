# Retained Text native construction

The Text runtime factory uses the same pooled allocation and canonical widget
owner for its proven eight-byte native identity. The remaining raw Text
payload is not constructed or overlaid with C++ objects. Native material
retention borrows the actual `+04`; the canonical semantic fields remain on
the existing widget and Text lifetime.

Default derived initialization is split at the native section call. The
factory first retains its implementation and lifetime, then enters `AB8530`.
The owner runtime preserves that same constructor admission on failure.
Factory-owned unbound glyph wrappers also stay with the failed allocation;
external page/loader owners must retain their own caller frames.

One `GuiTextSectionOperation` on that lifetime records the shadow Model,
temporary native name, mesh/section/material creators and cleanup state. All
four section callers use this same operation. Native cleanup releases only
the evidenced temporary strings and unconstructed slots; completed creators
are retained on failure. Completed frames may serve later calls, while failed
or active frames cannot restart.

Glyph-buffer acquisition also stays on the same lifetime. This is necessary
because the older local content-prefix object unwinds when a nested factory
throws. The persistent frame exposes completed declaration/vertex/index
acquisitions and rejects retirement or copy after interruption. It does not
add native Text fields, new reference counts or a second hierarchy.

The actual renderer `38/5C/60` calls and hardware layout `40` now compose with
existing native factories and one canonical registry. Real HAL fixtures check
buffers, layout cache reuse, COM release and CPU/hardware pool return. Those
fixtures do not execute complete Text construction. Material renderer `48`
still needs actual effect-cache loading, and native material property readers
must resolve raw Text identity to the same semantic owner. The earlier
default base-constructor callback ordering and native full-payload/string/SEH
ABI remain boundaries. No game validation is claimed.

See the packet reports for exact native call evidence and the combined batch
validation in `reports/gui_text_native_integration.json`.
