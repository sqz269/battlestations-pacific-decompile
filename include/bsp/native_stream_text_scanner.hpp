#pragma once
#include "bsp/native_string.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
class NativeVfsRuntimeBindings;
struct alignas(4) NativeStreamTextScannerStorage { std::byte bytes[0x828]; };
static_assert(sizeof(NativeStreamTextScannerStorage)==0x828);

// Original virtual targets are captured from the current raw owner/table at
// each call site. They are identities, never executable addresses in source.
struct NativeStreamTextScannerCalls {
    virtual ~NativeStreamTextScannerCalls()=default;
    virtual void* open(std::uint32_t entry,void* manager,const void* raw_name,std::uint32_t flags)=0;
    virtual void read(std::uint32_t entry,void* stream,void* destination,std::uint32_t requested,std::uint32_t* actual)=0;
    virtual void zero_reference(std::uint32_t entry,void* stream,std::uint32_t captured_table)=0;
    virtual void* allocate_00bf55be(std::uint32_t);
    virtual void free_00bf65ac(void*);
};
// Concrete composition with the application's existing raw VFS routes. This
// borrows its manager/providers/streams; no second VFS or buffered projection.
class NativeStreamTextScannerVfsCalls final : public NativeStreamTextScannerCalls {
public:
    explicit NativeStreamTextScannerVfsCalls(NativeVfsRuntimeBindings&) noexcept;
    void* open(std::uint32_t,void*,const void*,std::uint32_t) override;
    void read(std::uint32_t,void*,void*,std::uint32_t,std::uint32_t*) override;
    void zero_reference(std::uint32_t,void*,std::uint32_t) override;
private:
    NativeVfsRuntimeBindings& bindings_;
};
struct NativeStreamTextScannerContext {
    NativeStringRawPoolContext& strings;
    NativeStreamTextScannerCalls& calls;
    void* const volatile& actual_vfs_0109ceec;
    const char* const volatile& whitespace_00e15334;
    const char* const volatile& default_delimiters_00e15338;
    const char* recovery_stop_00d15f34;
};
// The inlined reads in BEE8E0/BEEDB0 use four/three uninitialized private
// stack DWORDs. Caller supplies independent preimages for each invocation;
// the source copies these into locals and retains slot reuse within that call.
// Real admitted VFS reads write actual before it is tested. BEE840 instead
// initializes its own local with incoming ECX/owner bits, as PUSH ECX does.
struct NativeStreamTextStackPreimages {
    std::array<std::uint32_t,4> token_counts;
    std::array<std::uint32_t,3> recovery_counts;
};
struct NativeStreamTextScannerOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    void* owner{};
    void* consumed_filename_header{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    NativeStreamTextScannerOperation()=default;
    ~NativeStreamTextScannerOperation();
    NativeStreamTextScannerOperation(const NativeStreamTextScannerOperation&)=delete;
    NativeStreamTextScannerOperation& operator=(const NativeStreamTextScannerOperation&)=delete;
    void acknowledge_diagnostic_cleanup() noexcept; // Caller resolves ownership first.
};
// BEF2E0: native ECX owner, stack by-value8h filename / extra delimiters,
// EAX owner, RET0C. The explicit source filename HEADER is consumed/released
// on success and left dangling, matching the native by-value parameter. Caller
// must pass its separately owned argument copy and never release it again.
// Zero full828h; copy name; current VFS slot4 mode32; release old stream AFTER
// open, adopt returned stream without retain; separators; release argument.
void* construct_native_stream_text_scanner_00bef2e0(void*,void* consumed_filename_header,const char*,NativeStreamTextScannerContext&,NativeStreamTextScannerOperation&);
// BEF220 full raw-pool/stream composition. Original existing sound interface
// remains available. Clear owned delimiter/stream fields, retain dead name.
void destroy_native_stream_text_scanner_00bef220(void*,NativeStreamTextScannerContext&,NativeStreamTextScannerOperation&);
char peek_native_stream_text_byte_00bee840(void*,NativeStreamTextScannerContext&);
char advance_native_stream_text_byte_00bee8c0(void*,NativeStreamTextScannerContext&);
void accept_native_stream_text_token_00bee800(void*) noexcept;
char* peek_native_stream_text_token_00bee8e0(void*,NativeStreamTextScannerContext&,const NativeStreamTextStackPreimages&);
void skip_native_stream_text_whitespace_00beedb0(void*,NativeStreamTextScannerContext&,const NativeStreamTextStackPreimages&);
char* read_native_stream_text_string_00bef020(void*,std::uint8_t* ok,NativeStreamTextScannerContext&,const NativeStreamTextStackPreimages&);
// New source interfaces over actual storage; original stack/ABI/FH3/SEH and
// token-buffer overflow are not reproduced. Tokens must fit the 400h buffers.
} // namespace bsp
