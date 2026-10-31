#include "bsp/native_lua_variant_scalar_setters.hpp"

#include <climits>
#include <cstring>
#include <intrin.h>
#include <limits>
#include <type_traits>
#include <xmmintrin.h>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Result = NativeLuaVariantRetirementResult;
static_assert(CHAR_BIT == 8);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(Word) == 4 && sizeof(std::int32_t) == 4 && sizeof(int) == 4);
static_assert(std::is_same_v<std::int32_t, int>);
static_assert(std::is_same_v<std::make_unsigned_t<std::int32_t>, Word>);
static_assert((std::numeric_limits<std::int32_t>::min)() == (-2147483647 - 1));
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);

void store_word(void* receiver, Word offset, Word value) noexcept {
    const Word address = static_cast<Word>(reinterpret_cast<std::uintptr_t>(receiver)) + offset;
    _ReadWriteBarrier();
    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(address)),
        &value, sizeof value);
    _ReadWriteBarrier();
}
} // namespace

Result set_native_lua_variant_signed_number_008869f0(
    void* actual_variant, const volatile Word& actual_input_home,
    NativeStringRawPoolContext& strings, NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup) {
    void* const captured_receiver = actual_variant;
    const Result child = clear_native_lua_variant_00886920_cpp_cleanup(
        captured_receiver, strings, operation, cleanup);
    if (child != Result::completed) return child;

    _ReadWriteBarrier();
    const Word captured_input = actual_input_home;
    _ReadWriteBarrier();
    std::int32_t signed_input;
    std::memcpy(&signed_input, &captured_input, sizeof signed_input);
    const __m128 converted = _mm_cvtsi32_ss(_mm_setzero_ps(), signed_input);
    float scalar;
    _mm_store_ss(&scalar, converted);
    Word payload_bits;
    std::memcpy(&payload_bits, &scalar, sizeof payload_bits);
    // Private Source scratch anchors conversion completion before tag store.
    // It is not the caller HOME, an owner/Frame, or a Native register claim.
    volatile Word conversion_completed_bits = payload_bits;
    _ReadWriteBarrier();
    const Word captured_payload = conversion_completed_bits;
    store_word(captured_receiver, 4u, 0u);
    store_word(captured_receiver, 8u, captured_payload);
    return Result::completed;
}

NativeLuaVariantPointerSetResult set_native_lua_variant_pointer_word_00886b30(
    void* actual_variant, const volatile Word& actual_input_home,
    NativeStringRawPoolContext& strings, NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup) {
    void* const captured_receiver = actual_variant;
    const Result child = clear_native_lua_variant_00886920_cpp_cleanup(
        captured_receiver, strings, operation, cleanup);
    if (child != Result::completed) return {child, std::nullopt};

    _ReadWriteBarrier();
    const Word captured_input = actual_input_home;
    _ReadWriteBarrier();
    store_word(captured_receiver, 4u, 4u);
    store_word(captured_receiver, 8u, captured_input);
    return {Result::completed, std::optional<Word>{captured_input}};
}

} // namespace bsp
