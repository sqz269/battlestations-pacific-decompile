# Native cleanup-call register primitive

This packet implements the complete `C16898` primitive as a naked MSVC Win32 entry containing only CALL EAX; RET (`FF D0 C3`). The source interface is `bsp::call_native_crt_cleanup_00c16898()` in `native_crt_local_cleanup_call.cpp/.hpp`.

Accepted CP `1d81dafac62a5e3284c9e3da1a68bc4c53a5af4e`, report SHA256 `a95fb176d9c4215f3425e264f2511f90444eaba0ba4ed75df793903be6cb1c8c`, establishes the complete body and both native caller contracts. All 523 original CP artifacts are retained unchanged, including accepted BU/NLG/frame evidence. The source base is exact candidate `75379b3383780740f61409c4223004a0fc3fd7d1`.

The original entry has no stack arguments. EAX supplies the actual executable cleanup target; EBP is the inherited native funclet frame. ECX and every other register arrive from the caller. C0DBC4 explicitly supplies ECX=1 after its actual NLG notification. C167C9 instead supplies the scope's enclosing level in ECX, which actual NLG preserves; it reloads only EAX before calling this primitive. No constant argument or generic callback signature is inferred.

If entry ESP is P, CALL EAX writes its return word at P-4. A balanced normal cleanup return permits the final RET to resume the primitive's caller. The entry does not set up, save, restore or normalize registers or flags. Actual cleanup effects propagate; exceptional or nonlocal exit does not promise execution of the final RET. The interface supplies no cleanup body, native frame, FS chain, cookie, checker, NLG state, validation, catch or rollback owner.

This primitive does not implement the 70-byte C0DC54 handler or close its native checker/recursive local-unwind dependencies. Exact instruction bytes are separate from enclosing native caller/frame/exception/gameplay validity. Calling the declaration from arbitrary C++ does not establish EAX or the inherited frame.

The strict MSVC Win32 build passed with unchanged source/header/CMake inputs, eight live/disk seed matches and both existing math CTests. The final `.text$mn` contains exactly three bytes `FF D0 C3`, matching the complete original PE, fresh live bytes and accepted CP evidence. It has no relocations, undefined external providers or nonempty writable sections. The 741-byte current object equals its uniquely extracted archive member; an independent scan of all 1,261 real archive COFF members finds one definition. The source/header read tlogs, strict compiler command, resolved `/Fo` output and write tlog bind these bytes to this build.

The accompanying report and complete compiler retention record preserve these checks and two full SHA256/SHA512 inventories. No build failed. No new tests, probes or cleanup/native/game execution were performed; the existing math checks do not validate cleanup targets or enclosing native exception behavior. C16898 remains the sole source primitive added by this packet.
