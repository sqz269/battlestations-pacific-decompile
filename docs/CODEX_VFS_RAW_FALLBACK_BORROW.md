# Retained VFS fallback borrow

GameNativeVfsRawServices appends the exact actual_empty_name_0109cef0 pointer
already supplied to and validated by the retained VFS runtime. Existing fields
keep their order. The view copies this pointer from the same runtime inputs
used by name, enumeration and block routes; it creates no storage or provider.

The application owns the byte and passes its address when constructing the
runtime. Every consumer must keep that application, runtime and input backing
alive through its operations and any retained failure state. This role is
distinct from the E17654 fallback and must not be replaced by a new empty literal.

The borrow is metadata only: no getter, I/O, mutation, native call or startup
activation occurs. Independent source review accepts the exact change. The
combined Win32 Release build/checks pass (2/2, 11.47 seconds) at 74b8f2617.
Raw BDEF90 and parser composition remain separate.
