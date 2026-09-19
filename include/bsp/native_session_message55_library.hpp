#pragma once
#include "bsp/native_session_message55.hpp"

namespace bsp {
// Complete104B game value-copy constructor: ECX destination, stack source,
// RET4/EAX destination. It zeros the string header, preserves byte9, copies
// flag/handle/pointer, then performs three separate x87 FLD/FSTP pairs.
NativeMessage55Receive* copy_construct_native_message55_receive_008e0950(
    NativeMessage55Receive*,const NativeMessage55Receive*,NativeStringRawPoolContext&);
// Complete35B value destructor: native stack pointer, RET4. Release only the
// owned string; retain header and payload bits. Explicit context changes ABI.
void destroy_native_message55_receive_008ddfe0(NativeMessage55Receive*,NativeStringRawPoolContext&);

// Concrete binding to the installed MSVC STL, not a port of its implementation.
// Win32, iterator-debug-level0 layouts: list has head/count at0/4, vector has
// begin/end/capacity at0/4/8; each is placed after the native retained DWORD.
// Allocators use the existing actual singleton allocation/free provider. The
// caller and this provider must use the same raw storage ownership contract.
// Other iterator-debug layouts fail explicitly at construction. Context must
// outlive every operation; thread-local scoped binding restores nested callers.
class NativeMessage55StandardLibrary final : public NativeMessage55LibraryCalls {
public:
    explicit NativeMessage55StandardLibrary(NativeStringRawPoolContext&);
    NativeMessage55Node* sentinel_008db560(NativeMessage55List*) override;
    NativeMessage55Node* sentinel_008db4f0(NativeMessage55List*) override;
    void resize_entries_008e0ec0(NativeMessage55List*,std::uint32_t,void*) override;
    void resize_received_008e17c0(NativeMessage55Vector*,std::uint32_t,const NativeMessage55Receive&) override;
    void destroy_received_008de660(NativeMessage55Receive*,NativeMessage55Receive*,NativeMessage55Vector*,NativeSessionMessage55*) override;
    void destroy_entries_008db580(NativeMessage55List*) override;
    void unwind_entries_008dbab0(NativeMessage55List*) override;
private:
    NativeStringRawPoolContext* strings_;
};
} // namespace bsp
