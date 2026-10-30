#include "bsp/native_locale_uppercase.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#include <cstring>
#include <cwctype>
#include <intrin.h>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(std::uint16_t) == 2);
static_assert(sizeof(std::wint_t) == 2);

using Address = std::uint32_t;

Address address_of(const void* value) {
    return static_cast<Address>(reinterpret_cast<std::uintptr_t>(value));
}

// Numeric unsigned arithmetic models Native SUB/ADD/scale wrap, not C++
// pointer subtraction or signed overflow. All reached addresses remain caller
// proofs. memcpy avoids typed raw-object/alignment assumptions. Compiler fences
// preserve the explicit operand epochs; they supply no thread synchronization.
template<class T> T load(Address address) {
    _ReadWriteBarrier();
    T value;
    std::memcpy(&value,
        reinterpret_cast<const void*>(static_cast<std::uintptr_t>(address)), sizeof(value));
    _ReadWriteBarrier();
    return value;
}

void store_word(Address address, std::uint16_t value) {
    _ReadWriteBarrier();
    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(address)),
        &value, sizeof(value));
    _ReadWriteBarrier();
}

std::uint32_t sar_one(std::uint32_t bits) {
    return (bits >> 1u) | (bits & 0x80000000u);
}

std::uint32_t extent(Address end, Address begin) {
    return sar_one(end - begin);
}

} // namespace

std::uint16_t uppercase_native_locale_code_unit_00a9eba0(
    const void* actual_manager_4040, std::uint16_t code_unit) {
    const Address manager = address_of(actual_manager_4040);
    std::uint32_t index = 0;
    for (;;) {
        const Address lower = load<Address>(manager + 0x4024u); // 00A9EBB0
        if (lower == 0 || index >= extent(load<Address>(manager + 0x4028u), lower)) {
            // 00A9EC1D..26: qualified current Source CRT, not Original C0391C.
            return static_cast<std::uint16_t>(std::towupper(static_cast<std::wint_t>(code_unit)));
        }
        // 00A9EBC8..DA reuses captured ECX and reloads end. The real Source
        // handler can return; do not replace this with throw or assume noreturn.
        if (lower == 0 || index >= extent(load<Address>(manager + 0x4028u), lower)) {
            invoke_native_invalid_parameter_00bf6713();
        }
        const Address current_lower = load<Address>(manager + 0x4024u); // 00A9EBDF
        if (load<std::uint16_t>(current_lower + index * 2u) == code_unit) {
            const Address upper = load<Address>(manager + 0x4034u); // 00A9EBF0
            if (upper == 0 || index >= extent(load<Address>(manager + 0x4038u), upper)) {
                invoke_native_invalid_parameter_00bf6713(); // 00A9EC08
            }
            const Address current_upper = load<Address>(manager + 0x4034u); // 00A9EC0D
            return load<std::uint16_t>(current_upper + index * 2u);
        }
        index += 1u;
    }
}

void uppercase_native_locale_wide_header_00a9ec30(
    const void* actual_manager_4040, void* actual_wide_header8) {
    const Address header = address_of(actual_wide_header8);
    std::uint32_t index = 0;
    if (load<std::uint32_t>(header) == 0) return; // 00A9EC39..3D
    do {
        const Address data = load<Address>(header + 4u); // CURRENT, 00A9EC40
        const auto code_unit = load<std::uint16_t>(data + index * 2u);
        const Address destination = data + index * 2u; // captured EDI, 00A9EC47
        const auto mapped = uppercase_native_locale_code_unit_00a9eba0(
            actual_manager_4040, code_unit);
        index += 1u; // 00A9EC52, before store to the captured destination
        store_word(destination, mapped); // 00A9EC55, no post-call data resample
    } while (index < load<std::uint32_t>(header)); // CURRENT, 00A9EC58
}

} // namespace bsp
