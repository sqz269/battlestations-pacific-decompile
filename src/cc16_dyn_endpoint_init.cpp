#include "bsp/cc16_dyn_endpoint_init.hpp"

namespace bsp::cc16_dyn_endpoint_init {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The native records use Win32 pointers");

U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
volatile U32& word(U32 location) noexcept {
    return *reinterpret_cast<volatile U32*>(location);
}
volatile U32& field(const void* value, U32 offset) noexcept {
    return word(address(value) + offset);
}
volatile std::uint8_t& byte(void* value, U32 offset) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(address(value) + offset);
}
}  // namespace

void release_vector_storage_0040a050(
    void* actual_vector, const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 data = field(actual_vector, 0);
    if (data != 0) {
        crt.free_00bf6989(pointer(data));  // 0040A057
    }
}

void* initialize_endpoint_00c4ce60(
    U32 token, std::uint8_t flag, void* endpoint, const void* bounds,
    const cc15_dyn_live::DynLiveCrt& crt) {
    field(endpoint, 0) = token;                         // 00C4CE7F
    byte(endpoint, 0x1c) = flag;                        // 00C4CE81
    field(endpoint, 0x04) = field(bounds, 0x00);        // 00C4CE84..86
    field(endpoint, 0x08) = field(bounds, 0x04);        // 00C4CE89..8C
    field(endpoint, 0x0c) = field(bounds, 0x08);        // 00C4CE8F..92
    field(endpoint, 0x10) = field(bounds, 0x0c);        // 00C4CE95..98
    field(endpoint, 0x14) = field(bounds, 0x10);        // 00C4CE9B..9E
    field(endpoint, 0x18) = field(bounds, 0x14);        // 00C4CEA1..A6
    field(endpoint, 0x30) = 0;                          // 00C4CEAA
    field(endpoint, 0x34) = 0;                          // 00C4CEAD
    field(endpoint, 0x38) = 0;                          // 00C4CEB0

    // Native state0 begins at CEBA, after vector zeroing. EAX=10 and EBX=0
    // make the unsigned JNC at CEBE unreachable on normal entry.
    try {
        field(endpoint, 0x38) = 10;                    // 00C4CEC2
        const U32 allocated = address(crt.operator_new_00bf55be(40)); // 00C4CEC5
        U32 index = 0;
        if (field(endpoint, 0x34) != 0) {               // 00C4CED1..D4
            U32 output = allocated;
            do {
                if (output != 0) {                     // 00C4CED8..DA
                    word(output) = word(field(endpoint, 0x30) + index * 4u);
                }
                index += 1u;
                output += 4u;
            } while (index < field(endpoint, 0x34));    // live unsigned bound
        }
        const U32 previous = field(endpoint, 0x30);     // 00C4CEEF
        if (previous != 0) {
            crt.free_00bf6989(pointer(previous));       // 00C4CEF7
        }
        field(endpoint, 0x30) = allocated;             // 00C4CEFF
    } catch (...) {
        // EH map E040A0: state0 -> CC823A, which adds30h to the actual
        // endpoint and tail-jumps to 0040A050. No new-buffer cleanup is added.
        release_vector_storage_0040a050(pointer(address(endpoint) + 0x30u), crt);
        throw;
    }
    byte(endpoint, 0x2c) = 0;                           // 00C4CF06
    return endpoint;                                   // 00C4CF0A / RET8
}

}  // namespace bsp::cc16_dyn_endpoint_init
