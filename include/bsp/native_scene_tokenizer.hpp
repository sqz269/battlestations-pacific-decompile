#pragma once
#include "bsp/native_memory_stream.hpp"
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
// Actual 838h owner. The constructor leaves quoted+04 and padding+80B alone.
// No C++ string/vector, implicit construction, or automatic cleanup.
struct alignas(4) NativeSceneTokenizerStorage { std::byte bytes[0x838]; };
static_assert(sizeof(NativeSceneTokenizerStorage)==0x838);
static_assert(std::is_trivially_default_constructible_v<NativeSceneTokenizerStorage>);

struct NativeSceneTokenizerCalls {
    virtual ~NativeSceneTokenizerCalls()=default;
    virtual void* allocate_00bf55be(std::uint32_t);
    virtual void free_00bf6989(void*);
    virtual void free_00bf65ac(void*);
    virtual std::int64_t stream_size_slot30(void*,NativeRetainedMemoryOwnerContext&);
    virtual void stream_read_slot24(void*,void*,std::uint32_t,std::uint32_t*,NativeRetainedMemoryOwnerContext&);
    virtual void stream_zero_reference_slot0(void*,NativeRetainedMemoryOwnerContext&);
};
struct NativeSceneTokenizerContext {
    NativeStringRawPoolContext& strings;
    NativeRetainedMemoryOwnerContext& memory;
    NativeSceneTokenizerCalls& calls;
    const char* const volatile& whitespace_00e0c940;
    const char* const volatile& default_delimiters_00e0c944;
    const char* unknown_file_00d15fe8;
};
struct NativeSceneTokenizerOperation final {
    enum class Phase { fresh,running,complete,failed,diagnostic_retired };
    Phase phase{Phase::fresh};
    void* owner{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    NativeSceneTokenizerOperation()=default;
    ~NativeSceneTokenizerOperation();
    NativeSceneTokenizerOperation(const NativeSceneTokenizerOperation&)=delete;
    NativeSceneTokenizerOperation& operator=(const NativeSceneTokenizerOperation&)=delete;
    // Caller must resolve retained ownership first; no hidden rollback/replay.
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Complete normal bodies, explicit source contexts rather than original ABI.
// 8D9F20: ECX owner, stack stream/extra delimiters, EAX same, RET8. Retain the
// actual stream; buffer low32(size), ignore read count. Defaults admit D642C0
// streams through their complete source methods; other profiles need a binding.
void* construct_native_scene_tokenizer_008d9f20(void*,void*,const char*,NativeSceneTokenizerContext&,NativeSceneTokenizerOperation&);
// 8D9C30: ECX owner, RET. Delimiters, decremented stream, buffer, raw label.
// Clear stream+828 only; other released headers retain native dangling values.
void destroy_native_scene_tokenizer_008d9c30(void*,NativeSceneTokenizerContext&,NativeSceneTokenizerOperation&);
// 8D9F00: full destructor, optional BF65AC for flags bit0, EAX owner, RET4.
void* delete_native_scene_tokenizer_008d9f00(void*,std::uint32_t,NativeSceneTokenizerContext&,NativeSceneTokenizerOperation&);
// 8D8900/8D8A50: ECX owner, AL current byte, RET / tail jump. Signed cursor
// comparison, zero-based LF counter, independent cached-byte and EOF flags.
char peek_native_scene_byte_008d8900(void*) noexcept;
char advance_native_scene_byte_008d8a50(void*) noexcept;
// 8D8A70: ECX owner, EAX owner+5, RET. Full token/comment scan. Valid input
// must fit the original 400h token buffers; native overflow is not repaired.
char* peek_native_scene_token_008d8a70(void*,NativeSceneTokenizerContext&);
// 8D8960: ECX owner, RET. Copy current token including NUL, clear only cache.
void consume_native_scene_token_008d8960(void*) noexcept;
} // namespace bsp
