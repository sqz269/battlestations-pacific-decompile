#include "bsp/native_lua_log_raw.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstddef>
#include <cstring>
#include <intrin.h>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Lua Log requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 && sizeof(Word) == 4);

// Scalar raw-header access only. Payload copying always uses real BF7680.
Word word(const void* storage, std::size_t offset) noexcept {
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, static_cast<const unsigned char*>(storage) + offset, 4);
    _ReadWriteBarrier();
    return value;
}
void put(void* storage, std::size_t offset, Word value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(static_cast<unsigned char*>(storage) + offset, &value, 4);
    _ReadWriteBarrier();
}
void* address(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

NativeLuaLogRawContext::NativeLuaLogRawContext(NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& feature, const void* prefix, std::uint32_t prefix_bytes)
    : strings_(strings), feature_(feature), prefix_(prefix), prefix_bytes_(prefix_bytes) {
    if (reinterpret_cast<std::uintptr_t>(prefix) != 0x00d112d4u || prefix_bytes != 18u)
        throw std::invalid_argument("raw Log requires actual D112D4/18 metadata");
}
NativeLuaLogRawFrame::NativeLuaLogRawFrame(const NativeLuaLogRawContext& context) noexcept
    : context_(context) {}
void NativeLuaLogRawFrame::fail() noexcept {
    failed_ = true;
    phase_ = Phase::failed;
    progress_.failed = true;
}
[[noreturn]] void NativeLuaLogRawFrame::reject(const char* text) {
    fail();
    throw std::logic_error(text);
}
void NativeLuaLogRawFrame::require_active() {
    if (phase_ != Phase::active || failed_)
        reject("raw Log invocation is not healthy and active");
}
void NativeLuaLogRawFrame::complete() {
    require_active();
    normal_result_ = true;
    progress_.completed = true;
    phase_ = Phase::returned;
}

std::int32_t execute_native_lua_log_0088bec0(
    const NativeLuaLogRawContext& c, NativeLuaLogRawFrame& f) {
    // Rejected inner entry cannot consume or clean the admitted outer attempt.
    if (&f.context_ != &c || f.phase_ != NativeLuaLogRawFrame::Phase::fresh)
        f.reject("raw Log Frame mismatch, reentry or replay");
    f.phase_ = NativeLuaLogRawFrame::Phase::active;
    f.progress_.attempted = true;
    try {
        put(f.scratch_, 0, 0);
        put(f.scratch_, 4, 0);
        f.progress_.resize_entered = true;
        resize_native_string_header_0041dd40(f.scratch_, c.strings_, 17, true);
        f.progress_.resize_returned = true;
        f.data_ = word(f.scratch_, 4);
        f.progress_.data_captured = true;
        f.progress_.captured_data = f.data_;
        if (f.data_ != 0) {
            f.bytes_ = word(f.scratch_, 0) + 1u;
            f.disposition_ = NativeLuaLogRawFrame::BufferDisposition::live;
            f.progress_.length_captured = true;
            f.progress_.captured_bytes = f.bytes_;
        } else {
            f.disposition_ = NativeLuaLogRawFrame::BufferDisposition::null_data;
            f.progress_.null_data = true;
        }
        f.require_active(); // actual return/operands/debt captured BEFORE this Source fence
        if (f.data_ != 0) {
            f.progress_.copy_entered = true;
            f.copy_result_ = copy_native_crt_bytes_00bf7680(
                address(f.data_), c.prefix_, f.bytes_, c.feature_);
            f.progress_.copy_returned = true;
            f.progress_.copy_result = reinterpret_cast<std::uintptr_t>(f.copy_result_);
            f.require_active();
            f.progress_.getter_entered = true;
            f.pool_ = native_string_pool_get_or_create_00419cc0(
                c.strings_.actual_published_01090aa8,
                c.strings_.actual_manager_publication_01090aa0);
            f.progress_.getter_returned = true;
            f.progress_.pool_address = reinterpret_cast<std::uintptr_t>(f.pool_);
            f.require_active(); // do not continue caught reentry from getter into BD
            f.disposition_ = NativeLuaLogRawFrame::BufferDisposition::return_entered;
            f.progress_.block_return_entered = true;
            return_native_string_pool_00bd1510(f.pool_, address(f.data_), f.bytes_,
                c.strings_.actual_small_returns_disabled_01090aa4);
            f.disposition_ = NativeLuaLogRawFrame::BufferDisposition::returned;
            f.progress_.block_return_returned = true; // disposition BEFORE post-return fence
        }
        f.complete(); // own metadata only; NEVER reread the returned header/payload
        return 0;
    } catch (...) {
        f.fail();
        throw; // Native supplies no local action: retain actual partial debt, no cleanup
    }
}
} // namespace bsp
