#include "bsp/native_renderer_record_destroy.hpp"
#include "bsp/native_renderer_container_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
template<class T> volatile T& field(void* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(reinterpret_cast<std::uint32_t>(base) + offset);
}
template<auto Reserve>
void destroy_records(void* header) {
    if (static_cast<std::int32_t>(field<std::uint32_t>(header, 8)) < 0)
        Reserve(header, 0, 0);
    while (static_cast<std::int32_t>(field<std::uint32_t>(header, 4)) > 0)
        field<std::uint32_t>(header, 4) = field<std::uint32_t>(header, 4) - 1u;
    void* const captured_data = field<void*>(header, 0);
    field<std::uint32_t>(header, 4) = 0;
    singleton_lifetime_free(captured_data);
}
} // namespace

void __fastcall destroy_native_renderer_records16_00b29ba0(void* header) {
    destroy_records<reserve_native_renderer_records16_00b228b0>(header);
}
void __fastcall destroy_native_renderer_records20_00b29be0(void* header) {
    destroy_records<reserve_native_renderer_records20_00b22940>(header);
}
void __fastcall destroy_native_renderer_records24_00b29c20(void* header) {
    destroy_records<reserve_native_renderer_records24_00b229d0>(header);
}
void __fastcall destroy_native_renderer_records40_00b29c60(void* header) {
    destroy_records<reserve_native_renderer_records40_00b22a70>(header);
}

} // namespace bsp
