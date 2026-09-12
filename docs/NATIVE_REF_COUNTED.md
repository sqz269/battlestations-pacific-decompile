# Raw reference-counted deletion entries

Addresses: 00BD30E0, 00BD30F0.

The common raw device root now has reusable source entries. Existing recovered
names are retained. BD30E0 takes ECX owner and no stack arguments, returns with
RET, skips null, otherwise captures the current profile/slot04 target and calls
the scalar deleter with flags1. It does not decrement a reference count. The
new source interface requires a concrete captured-profile deletion provider;
the caller cannot pass its own flags or treat arbitrary source vtables as native.

BD30F0 is exactly MOV DWORD PTR[ECX],CEB130; RET. It neither frees storage nor
checks null nor changes the reference count or other fields. The source body
uses the same Win32 instructions and native profile identity; that identity is
not a callable source C++ vtable. Original whole-object/SEH/ABI compatibility and
arbitrary virtual target dispatch are not established.

The native complete byte spans are14 and7 bytes; saved listings and live bytes
were checked. BD30EB is an indirect EDX call: vtable+4 was captured atBD30E6 and
flags1 pushed atBD30E9. The provider preserves this target identity even if later
host lookup has side effects. Device-family fixtures validate use through their
actual source scalar bodies; this initial shared leaf is build validation only.
