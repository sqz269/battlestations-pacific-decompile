#pragma once

#include <cstdint>

namespace bsp {
class NativeStringStorage;
class SingletonLifetimeDomain;
struct NativeStringPoolStorage;

// Library/service boundaries of BEE390 and BDB970. The lowercase argument is
// sign-extended from a byte, including -128..-1; do not coerce it to unsigned.
// The supplied library must retain its current locale behavior. The two getter
// calls in BDB970 repeat the native getter and can throw independently.
class NativePathCanonicalizerServices {
public:
    virtual ~NativePathCanonicalizerServices() = default;
    virtual void* allocate_scratch_00bf55be(std::uint32_t bytes) = 0;
    virtual int lowercase_00bf9611(int signed_byte) = 0;
    virtual void free_scratch_00bf65ac(void* scratch) = 0;
    virtual NativeStringPoolStorage* string_pool_00419cc0() = 0;
    virtual void return_string_00bd1510(NativeStringPoolStorage*, void* block,
        std::uint32_t bytes, std::uint32_t unused_one) = 0;
};

// Actual canonical pool/publication/gate plus source CRT allocation/free.
// lower is a library binding, not an ASCII-only replacement for native tolower.
class NativePathCanonicalizerRuntimeServices final : public NativePathCanonicalizerServices {
public:
    using Lowercase = int (__cdecl*)(int);
    NativePathCanonicalizerRuntimeServices(NativeStringPoolStorage* volatile&,
        volatile std::uint32_t& returns_disabled_01090aa4,
        SingletonLifetimeDomain&, Lowercase) noexcept;
    // Same services over the application's actual raw01090AA0 publication.
    // Borrow its canonical domain; do not create a parallel semantic manager.
    NativePathCanonicalizerRuntimeServices(NativeStringPoolStorage* volatile&,
        volatile std::uint32_t& returns_disabled_01090aa4,
        void* volatile& actual_manager_publication_01090aa0, Lowercase) noexcept;
    void* allocate_scratch_00bf55be(std::uint32_t bytes) override;
    int lowercase_00bf9611(int signed_byte) override;
    void free_scratch_00bf65ac(void*) noexcept override;
    NativeStringPoolStorage* string_pool_00419cc0() override;
    void return_string_00bd1510(NativeStringPoolStorage*, void*, std::uint32_t,
        std::uint32_t) noexcept override;
private:
    NativeStringPoolStorage* volatile& publication_;
    volatile std::uint32_t& returns_disabled_;
    SingletonLifetimeDomain* lifetime_{};
    void* volatile* actual_manager_{};
    Lowercase lower_;
};

struct NativePathCanonicalizerContext {
    NativeStringStorage& strings;
    NativePathCanonicalizerServices& services;
    const char* null_source_0109db91;
};

// Complete BEE390 body over actual eight-byte input/output headers. Original:
// ECX output, EDX input, EAX output, RET0. New C++ interface, not original ABI.
// Copies to the first NUL, then traverses the reloaded header count; these are
// deliberately distinct bounds. No NUL/high-byte validation or length clamp.
// 260-byte uninitialized stack scratch when initial length<256, otherwise a
// wrapping length+1 heap request; heap release occurs only after output ctor.
// No cleanup is added on failure, no input/output alias retain is added.
void* canonicalize_native_path_00bee390(void* output, const void* input,
    NativePathCanonicalizerContext&);

// Complete BDB970 normal body and source representation of its EH state order.
// Original ignores incoming ECX/EDX, consumes one C-string with RET4, no stable
// result. Construct source, canonicalize output, release source then output.
// Canonicalization failure releases source; failure returning source releases
// output. Output-return failure has no further cleanup. Original FH3/SEH ABI
// and simultaneous cleanup-failure behavior are not claimed by this interface.
void canonicalize_discard_native_path_00bdb970(const char* source,
    NativePathCanonicalizerContext&);
} // namespace bsp
