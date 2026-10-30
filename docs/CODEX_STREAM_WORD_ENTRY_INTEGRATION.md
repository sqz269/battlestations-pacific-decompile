# Captured stream-word entry integration

The concrete integration gap is a word-read interface without unrelated string
context fields. The existing BE4320/BE4340 wrappers use only a stream dispatcher;
callers must not invent DB64 storage to satisfy their broader context type.

Dispatch-only overloads preserve the full incoming count-pointer bits as the
initial DWORD, capture the current stream vtable and +24 read target, perform one
two-byte read through the same dispatcher, and return the low WORD. They add no
byte swap, full-read guarantee, tail clearing, retry or numeric code execution.

Captured VFS forwarding must admit only BE4320/BE4340 and pass the same stream,
count pointer and dispatcher. The scanner's nonnull count and the sidecar's null
count are different seed domains. Existing APIs and bodies remain unchanged.

The exact revision is a308d7b2768d33f8b7012a0c402c454aaaa4e28e. Primary and
independent source review accept all additions. Inverse projection preserves
all four previous header/source files. Both strict x86 object checks pass, and
the canonical Release Win32 build passes both existing checks (11.50 seconds).
The existing translation-unit registrations suffice; no CMake change is needed.

This closes no scanner/parser ownership, raw16 growth, loaded-locale or runtime
gate by itself. No stream-word body or game run was invoked by these checks.
