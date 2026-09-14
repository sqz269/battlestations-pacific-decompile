# Native hierarchy record cleanup

`B88320-B8833F` is a complete 32-byte scalar cleanup wrapper. The live Ghidra
body and installed PE bytes agree (`568bf1e858fefffff644240801740b56b92c020901e8b6f7f8ff8bc65ec20400`). Entry ECX holds the original 84h hierarchy
payload in an 88h pool slot, and the single stack DWORD holds flags. The routine saves the record in
ESI, calls `B88180` to destroy its fields, tests flags bit 0, and only when set
passes the record on the stack with ECX=`109022C` to `B17AF0`. It returns the
original record in EAX and consumes the flags with RET4. It has no direct live
callers or local FH3 handler; a field-destruction exception skips slot return.

The source accepts the actual `NativeStringRawPoolContext` and initialized
`NativeMaterialParameterPool` companion for storage `109022C`, borrowed from
the caller. The existing field destructor and pool return implementations are
used unchanged. The pool's `AllocatorListDomain` remains shared; this wrapper
creates no storage or owner. It performs no pointee release, record rollback,
manager operation, validation, or extra exception handling.

The implementation is a new C++ source interface, not a binary replacement for
the original ECX/RET4 ABI. The strict MSVC Win32 Release build passed with
`/W4 /WX /fp:strict`; its one registered CTest, `reconstructed_math`, passed.
`native_math_differential` was not registered in this clean build. The copied
1090-byte production object matches the uniquely extracted archive member by
SHA-256 (`6cf535183b74d4970c144f1605b1e64ee9a24186891b7e96120210febc7fe8aa`).
Compiler/source/object evidence is retained in `local/hierarchy-cleanup-bg/`.
These checks establish compilation and linkage only; the game path and native
exception machinery are unexercised.
