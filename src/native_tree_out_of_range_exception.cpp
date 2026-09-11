#include "bsp/native_tree_out_of_range_exception.hpp"

namespace bsp {

NativeLegacyExceptionStorage& copy_native_tree_out_of_range_00441760(
    NativeLegacyExceptionStorage& destination,
    const NativeLegacyExceptionStorage& source) {
    copy_native_legacy_logic_error_004118d0(destination, source);
    destination.native_vtable_00 = 0x00d6926c;
    return destination;
}

void destroy_native_tree_out_of_range_004412b0(
    NativeLegacyExceptionStorage& owner) noexcept {
    // The complete native cleanup has the same member/base operations as
    // 00411780, including its returning-free continuation and current fields.
    destroy_native_legacy_logic_error_00411780(owner);
}

} // namespace bsp
