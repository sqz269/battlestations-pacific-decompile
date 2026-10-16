#include "bsp/native_input_preset_temp_key_release.hpp"

namespace bsp {

void __fastcall release_native_input_preset_temp_key_0069e090(
    void* completed_actual_key_header, std::uint32_t,
    NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(completed_actual_key_header, strings);
}

} // namespace bsp
