#include "bsp/cc16_dyn_sap_dispatch.hpp"

#include "bsp/cc16_dyn_sap_endpoints.hpp"
#include "bsp/cc16_dyn_sap_update.hpp"

namespace bsp::cc16_dyn_sap_dispatch {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The recovered raw storage uses Win32 pointers");
U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
volatile U32& word(U32 location) noexcept {
    return *reinterpret_cast<volatile U32*>(location);
}
volatile std::uint8_t& byte(U32 location) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(location);
}
}  // namespace

void update_flagged_endpoint_00c4cbb0(
    void* manager, void* endpoint, const cc15_dyn_live::DynLiveCrt& crt) {
    if (byte(address(endpoint) + 0x2cu) != 0) {
        cc16_dyn_sap_update::update_axis_endpoints_00c4c700(manager, endpoint, 0u, crt);
        cc16_dyn_sap_update::update_axis_endpoints_00c4c700(manager, endpoint, 1u, crt);
        cc16_dyn_sap_update::update_axis_endpoints_00c4c700(manager, endpoint, 2u, crt);
    }
}

void process_pending_endpoints_00c4cbf0(
    void* manager, const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 base = address(manager);
    U32 index = 0;
    if (word(base + 0x198u) != 0) {
        do {
            const U32 data = word(base + 0x194u);
            void* const endpoint = pointer(word(data + index * 4u));
            (void)cc16_dyn_sap_update::append_axis_endpoints_00c40900(manager, endpoint, 0u, crt);
            (void)cc16_dyn_sap_update::append_axis_endpoints_00c40900(manager, endpoint, 1u, crt);
            (void)cc16_dyn_sap_update::append_axis_endpoints_00c40900(manager, endpoint, 2u, crt);
            cc16_dyn_sap_endpoints::update_keys_move_left_00c40b30(endpoint, manager, 0u);
            cc16_dyn_sap_endpoints::update_keys_move_left_00c40b30(endpoint, manager, 1u);
            cc16_dyn_sap_update::update_axis_endpoints_00c4c700(manager, endpoint, 2u, crt);
            ++index;
            byte(address(endpoint) + 0x2cu) = 1u;
        } while (index < word(base + 0x198u));
    }
    word(base + 0x198u) = 0;
}

}  // namespace bsp::cc16_dyn_sap_dispatch
