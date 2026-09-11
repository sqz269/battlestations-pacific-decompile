#include "bsp/xlive_pipe_framing.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>

#pragma comment(lib, "advapi32.lib")

namespace bsp {
namespace {
using Tables = XLivePipeProtocolTables;
using Value = XLivePipeEncodedValue;
using Wide = XLivePipeEncodedWideValue;
using Globals = XLivePipeFrameGlobals;
using Byte = std::uint8_t;
using U32 = std::uint32_t;
using I32 = std::int32_t;
constexpr I32 signed_bits(U32 n) noexcept {
    return n < 0x80000000u ? static_cast<I32>(n)
        : static_cast<I32>(static_cast<std::int64_t>(n) - 0x100000000ll);
}
constexpr I32 unexpected = signed_bits(0x8000ffffu);
constexpr I32 invalid_handle = signed_bits(0x80070006u);
constexpr I32 invalid_argument = signed_bits(0x80070057u);
constexpr I32 failure = signed_bits(0x80004005u);
U32 row(const Byte* rows, I32 index) {
    const auto offset = static_cast<std::ptrdiff_t>(index) * 2;
    const U32 value = U32(rows[offset]) | (U32(rows[offset + 1]) << 8);
    return value & 0x8000u ? value | 0xffff0000u : value;
}
std::ptrdiff_t output_index(I32 index, I32 displacement) {
    return index == 0 ? 0 : signed_bits(U32(index) - U32(displacement));
}
template<class... Values> void lock_values(Globals& g, Values&... values) {
    enter_xlive_pipe_global_00a5f94f(g.acquisition_lock_f8b778);
    (values.lock->enter(), ...);
    leave_xlive_pipe_global_00a5f95b(g.acquisition_lock_f8b778);
}
template<class... Values> void unlock_values(Values&... values) {
    // Reload every pointer, in native input order, including repeated aliases.
    (values.lock->leave(), ...);
}
void pair(const Byte* a, const Byte* b, Byte* out, U32 r1, U32 r2,
    I32 na, I32 nb, I32 no, const Tables& t) {
    transform_xlive_pipe_pair_00a5fe80(a, b, out,
        t.bytes(r1, std::size_t(na) * 2), t.bytes(r2, std::size_t(na) * 2),
        0, na, nb, no, t);
}
} // namespace

void transform_xlive_pipe_seed_bits_00a5fa70(const Byte* seed, const Byte* encoded,
    Byte* out, const Byte* r1, const Byte* r2, I32 displacement,
    I32 na, I32 nb, I32 no, const Tables& t) {
    U32 first = 0, second = 0;
    const I32 count = (std::min)({na, nb, no});
    for (I32 i = 0; i < count; ++i) {
        const U32 bit = i < 8 ? 0 : (seed[(i - 8) / 8] >> ((i - 8) & 7)) & 1u;
        first = t.word(0xd260d0, (encoded[i] & 3u) | row(r1, i)
                | ((first >> 12) & 0x1e0u) | (bit << 2))
            ^ t.word(0xd26950, (first >> 6) & 0x3fu)
            ^ t.word(0xd268d0, (first >> 12) & 0x1fu)
            ^ t.word(0xd26a50, first & 0x3fu);
        second = t.word(0xd26b50,
                ((((first & 0xe00000u) >> 7) | (second & 0x1e0000u)) >> 8) | row(r2, i))
            ^ t.word(0xd2ebd0, (second >> 6) & 0x3fu)
            ^ t.word(0xd2eb50, (second >> 12) & 0x1fu)
            ^ t.word(0xd2ecd0, second & 0x3fu);
        out[output_index(i, displacement)] = Byte((second >> 21) & 3u);
        if (i == 0 && displacement != 0)
            second = t.word(0xd53a08, ((first & 0xe00000u) >> 15) | row(r2, 0));
    }
}
void transform_xlive_pipe_packed_bits_00a5fd15(const Byte* a, const Byte* b,
    Byte* out, const Byte* r1, const Byte* r2, I32 displacement,
    I32 na, I32 nb, I32 no, const Tables& t) {
    const auto cleared = U32(no / 8) - 1u;
    std::memset(out, 0, cleared);
    U32 first = 0, second = 0;
    const I32 count = (std::min)({na, nb, no});
    for (I32 i = 0; i < count; ++i) {
        first = t.word(0xd382d0, (b[i] & 3u) | row(r1, i)
                | ((a[i] & 3u) << 2) | ((first >> 16) & 0x1f80u))
            ^ t.word(0xd404d0, (first >> 8) & 0xffu)
            ^ t.word(0xd402d0, (first >> 16) & 0x7fu)
            ^ t.word(0xd408d0, first & 0xffu);
        second = t.word(0xd40cd0,
                ((((first >> 21) & 0x700u) | (second & 0x3800u)) >> 3) | row(r2, i))
            ^ t.word(0xd42cf0, (second >> 4) & 0xfu)
            ^ t.word(0xd42cd0, (second >> 8) & 7u)
            ^ t.word(0xd42d30, second & 0xfu);
        if (i >= 8) {
            const I32 bit = i == 8 ? 0 : signed_bits(U32(i) - U32(displacement) - 8u);
            out[bit / 8] |= Byte(((second >> 14) & 1u) << (U32(bit % 8) & 31u));
        }
        if (i == 0 && displacement != 0) std::memset(out, 0, cleared);
    }
}

namespace {
// Native ECX=raw destination, RET. These are byte copies, not semantic numbers.
void copy_00a5ebe5(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd25dc8, 72), 72); }
void copy_00a5ec07(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd25e10, 72), 72); }
void copy_00a5ec29(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd25e58, 40), 40); }
void copy_00a5ec4b(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd25e80, 40), 40); }
void copy_00a5ec6d(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd25ea8, 72), 72); }
void copy_00a5ec8f(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd25ef0, 72), 72); }
void copy_00a5ecb1(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd25f38, 72), 72); }
void copy_00a5ecd3(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd25f80, 72), 72); }
void copy_00a5ecf5(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd25fc8, 72), 72); }
void copy_00a5ed39(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd26038, 40), 40); }
void copy_00a5ed5b(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd26060, 72), 72); }
void copy_00a5ed7d(Byte* out, const Tables& t) { std::memcpy(out, t.bytes(0xd260a8, 40), 40); }

void mix_00a601aa(const Wide& a, Byte* out, const Tables& t) {
    pair(a.bytes.data(), t.bytes(0xd554e8, 72), out, 0xd55530, 0xd555c0, 72, 72, 72, t);
}
void combine_00a601d5(const Byte* a, const Wide& b, Wide& out, const Tables& t) {
    pair(a, b.bytes.data(), out.bytes.data(), 0xd55650, 0xd556e0, 72, 72, 72, t);
}
void combine_00a6017c(const Byte* a, const Wide& b, Wide& out, const Tables& t) {
    pair(a, b.bytes.data(), out.bytes.data(), 0xd553c8, 0xd55458, 72, 72, 72, t);
}
void mix_00a6030b(const Wide& a, const Wide& b, Byte* out, const Tables& t) {
    pair(a.bytes.data(), b.bytes.data(), out, 0xd55cc8, 0xd55d58, 72, 72, 72, t);
}
void combine_00a60339(const Byte* a, const Wide& b, Wide& out, const Tables& t) {
    pair(a, b.bytes.data(), out.bytes.data(), 0xd55de8, 0xd55e78, 72, 72, 72, t);
}
void mix_00a60367(const Value& a, const Value& b, Byte* out, const Tables& t) {
    pair(a.bytes.data(), b.bytes.data(), out, 0xd55f08, 0xd55f58, 40, 40, 40, t);
}
void combine_00a60395(const Byte* a, const Value& b, Value& out, const Tables& t) {
    pair(a, b.bytes.data(), out.bytes.data(), 0xd55fa8, 0xd55ff8, 40, 40, 40, t);
}
void seed_00a60203(U32 low, U32 high, Byte* out, const Tables& t) {
    const U32 words[2]{low, high}; // x86 little endian, matching consecutive stack arguments
    transform_xlive_pipe_seed_bits_00a5fa70(reinterpret_cast<const Byte*>(words),
        t.bytes(0xd55770, 72), out, t.bytes(0xd557b8, 144), t.bytes(0xd55848, 144),
        0, 72, 72, 72, t);
}
void combine_00a6022b(const Byte* a, const Wide& b, Wide& out, const Tables& t) {
    pair(a, b.bytes.data(), out.bytes.data(), 0xd558d8, 0xd55968, 72, 72, 72, t);
}
void mix_00a602b2(const Value& a, Byte* out, const Tables& t) {
    pair(a.bytes.data(), t.bytes(0xd55b60, 40), out, 0xd55b88, 0xd55bd8, 40, 40, 40, t);
}
void combine_00a602dd(const Byte* a, const Value& b, Value& out, const Tables& t) {
    pair(a, b.bytes.data(), out.bytes.data(), 0xd55c28, 0xd55c78, 40, 40, 40, t);
}
void mix_00a60527(const Wide& a, const Byte* b, Byte* out, const Tables& t) {
    pair(a.bytes.data(), b, out, 0xd56828, 0xd568b8, 72, 72, 72, t);
}
void mix_00a60551(const Value& a, Byte* out, const Tables& t) {
    // Native signed minimum is40: the initialized last32 output bytes survive.
    pair(a.bytes.data(), t.bytes(0xd56948, 72), out, 0xd56970, 0xd569c0, 40, 72, 72, t);
}
void combine_00a6057c(const Byte* a, const Byte* b, Byte* out, const Tables& t) {
    pair(a, b, out, 0xd56a10, 0xd56aa0, 72, 72, 72, t);
}
void mix_00a605a2(const Value& a, const Value& b, Byte* out, const Tables& t) {
    pair(a.bytes.data(), b.bytes.data(), out, 0xd56b30, 0xd56b80, 40, 40, 40, t);
}
void pack_00a605d0(const Byte* a, const Byte* b, Byte* out, const Tables& t) {
    transform_xlive_pipe_packed_bits_00a5fd15(a, b, out,
        t.bytes(0xd56bd0, 80), t.bytes(0xd56c20, 80), 0, 40, 40, 40, t);
}
void mix_00a605f6(const Wide& a, const Wide& b, Byte* out, const Tables& t) {
    pair(a.bytes.data(), b.bytes.data(), out, 0xd56c70, 0xd56d00, 72, 72, 72, t);
}
void pack_00a60624(const Byte* a, const Byte* b, Byte* out, const Tables& t) {
    transform_xlive_pipe_packed_bits_00a5fd15(a, b, out,
        t.bytes(0xd56d90, 144), t.bytes(0xd56e20, 144), 0, 72, 72, 72, t);
}
void extend_00a5f09f(const Value& a, Byte* out, const Globals& g, const Tables& t) {
    std::array<Byte, 72> scratch;
    copy_00a5ec07(scratch.data(), t);
    mix_00a60551(a, scratch.data(), t);
    combine_00a6057c(scratch.data(), g.raw_f8b8f8.data(), out, t);
}
void mix_00a5f2ec(const Wide& a, const Value& b, Byte* out, const Globals& g, const Tables& t) {
    std::array<Byte, 72> scratch;
    copy_00a5ebe5(scratch.data(), t);
    extend_00a5f09f(b, scratch.data(), g, t);
    mix_00a60527(a, scratch.data(), out, t);
}
bool negative_00a5f0e8(const Value& a, const Value& b, const Tables& t) {
    std::array<Byte, 40> first, second;
    std::array<Byte, 4> packed;
    copy_00a5ec29(first.data(), t); copy_00a5ec4b(second.data(), t);
    mix_00a605a2(a, b, first.data(), t);
    pack_00a605d0(first.data(), second.data(), packed.data(), t);
    return (packed[3] & 0x80u) != 0;
}
bool negative_00a5f144(const Wide& a, const Wide& b, const Tables& t) {
    std::array<Byte, 72> first, second;
    std::array<Byte, 8> packed;
    copy_00a5ec6d(first.data(), t); copy_00a5ecb1(second.data(), t);
    mix_00a605f6(a, b, first.data(), t);
    pack_00a60624(first.data(), second.data(), packed.data(), t);
    return (packed[7] & 0x80u) != 0;
}
void transform_00a5ee0d(Wide& a, Wide& out, Globals& g, const Tables& t) {
    lock_values(g, a, out);
    std::array<Byte, 72> scratch;
    copy_00a5ecd3(scratch.data(), t); mix_00a601aa(a, scratch.data(), t);
    combine_00a601d5(scratch.data(), out, out, t);
    unlock_values(a, out);
}
void transform_00a5efa1(Wide& a, Wide& b, Wide& out, Globals& g, const Tables& t) {
    lock_values(g, a, b, out);
    std::array<Byte, 72> scratch;
    copy_00a5ed5b(scratch.data(), t); mix_00a6030b(a, b, scratch.data(), t);
    combine_00a60339(scratch.data(), out, out, t);
    unlock_values(a, b, out);
}
void transform_00a5f397(Wide& a, Value& b, Wide& out, Globals& g, const Tables& t) {
    lock_values(g, a, b, out);
    std::array<Byte, 72> scratch;
    copy_00a5ec8f(scratch.data(), t); mix_00a5f2ec(a, b, scratch.data(), g, t);
    combine_00a6017c(scratch.data(), out, out, t);
    unlock_values(a, b, out);
}
void transform_00a5f020(Value& a, Value& b, Value& out, Globals& g, const Tables& t) {
    lock_values(g, a, b, out);
    std::array<Byte, 40> scratch;
    copy_00a5ed7d(scratch.data(), t); mix_00a60367(a, b, scratch.data(), t);
    combine_00a60395(scratch.data(), out, out, t);
    unlock_values(a, b, out);
}
void transform_00a5ee78(U32 low, U32 high, Wide& out, const Tables& t) {
    out.lock->enter(); // This helper does not take the acquisition lock.
    std::array<Byte, 72> scratch;
    copy_00a5ecf5(scratch.data(), t); seed_00a60203(low, high, scratch.data(), t);
    combine_00a6022b(scratch.data(), out, out, t);
    out.lock->leave();
}
void transform_00a5ef36(Value& a, Value& out, Globals& g, const Tables& t) {
    lock_values(g, a, out);
    std::array<Byte, 40> scratch;
    copy_00a5ed39(scratch.data(), t); mix_00a602b2(a, scratch.data(), t);
    combine_00a602dd(scratch.data(), out, out, t);
    unlock_values(a, out);
}
bool bad_owner(XLivePipeTransport* p) {
    return !p || reinterpret_cast<std::uintptr_t>(p) == ~std::uintptr_t(0);
}
void destroy_temporary(Wide& temporary) {
    if (temporary.lock) temporary.lock->destroy(1);
}
void copy_dwords_forward(void* destination, const void* source, U32 count) {
    auto* out = static_cast<Byte*>(destination);
    const auto* in = static_cast<const Byte*>(source);
    // REP MOVSD copies each DWORD before advancing; preserve overlapping input.
    for (U32 i = 0; i < count; ++i) {
        U32 word;
        std::memcpy(&word, in + i * 4u, 4);
        std::memcpy(out + i * 4u, &word, 4);
    }
}
} // namespace

bool compare_xlive_pipe_value_00a5f25c(Value& a, Value& b, Globals& g, const Tables& t) {
    lock_values(g, a, b);
    const bool negative = negative_00a5f0e8(a, b, t);
    unlock_values(a, b);
    return !negative;
}
bool compare_xlive_pipe_wide_value_00a5f2a4(Wide& a, Wide& b, Globals& g, const Tables& t) {
    lock_values(g, a, b);
    const bool negative = negative_00a5f144(a, b, t);
    unlock_values(a, b);
    return !negative;
}
I32 send_xlive_pipe_capacity_00a5f204(XLivePipeNativeState& s, U32 n, U32* out) {
    if (!s.protocol_00) return failure;
    if (n > 0x3b8u) return signed_bits(0x80070008u);
    *out = n + 72u;
    return 0;
}
I32 receive_xlive_pipe_capacity_00a5f230(XLivePipeNativeState& s, U32 n, U32* out) {
    if (!s.protocol_00) return failure;
    if (n > 0x3b8u) return signed_bits(0x80070008u);
    *out = n + 72u;
    return 0;
}
I32 send_xlive_pipe_capacity_00a5df96(XLivePipeTransport* p, U32 n, U32* out) {
    if (bad_owner(p)) return invalid_handle;
    if (!out) return invalid_argument;
    return send_xlive_pipe_capacity_00a5f204(p->native, n, out);
}
I32 receive_xlive_pipe_capacity_00a5dfce(XLivePipeTransport* p, U32 n, U32* out) {
    if (bad_owner(p)) return invalid_handle;
    if (!out) return invalid_argument;
    return receive_xlive_pipe_capacity_00a5f230(p->native, n, out);
}
I32 encode_xlive_pipe_frame_00a5f6c0(XLivePipeNativeState& owner, void* buffer,
    U32* capacity, XLiveIpcEncodeCallback callback, void* context, Globals& g,
    const Tables& t, XLivePipeFrameSystemHost& system) {
    auto* const p = static_cast<XLivePipeProtocolNativeState*>(owner.protocol_00);
    if (!p) return failure;
    if (*capacity < 72u) return unexpected;
    U32 payload = *capacity - 72u;
    auto* frame = static_cast<Byte*>(buffer);
    if (callback) {
        const I32 result = callback(frame + 72, &payload, context);
        if (result < 0) return result;
        if (payload > *capacity - 72u) return unexpected;
    } else payload = 0;
    if (compare_xlive_pipe_value_00a5f25c(p->key_7c, g.key_f8badc, g, t)) {
        transform_00a5f397(p->pending_30, p->seeded_04, p->pending_30, g, t);
        transform_00a5efa1(p->pending_30, g.value_f8b9a0, p->pending_30, g, t);
        copy_dwords_forward(frame, p->pending_30.bytes.data(), 18);
    } else {
        // Typed boundary acquires defined preimage before owning the native lock.
        const auto preimage = system.temporary_wide_preimage();
        Wide temporary{system.create_value_lock(), preimage};
        transform_00a5ee0d(g.value_f8b858, temporary, g, t);
        transform_00a5efa1(temporary, g.value_f8b9a0, temporary, g, t);
        copy_dwords_forward(frame, temporary.bytes.data(), 18);
        destroy_temporary(temporary);
    }
    return 0;
}
I32 decode_xlive_pipe_frame_00a5f7c5(XLivePipeNativeState& owner, const void* buffer,
    U32 bytes, XLiveIpcDecodeCallback callback, void* context, Globals& g,
    const Tables& t, XLivePipeFrameSystemHost& system) {
    auto* const p = static_cast<XLivePipeProtocolNativeState*>(owner.protocol_00);
    if (!p) return failure;
    if (bytes < 72u) return unexpected;
    const auto* frame = static_cast<const Byte*>(buffer);
    if (callback) {
        const I32 result = callback(frame + 72, bytes - 72u, context);
        if (result < 0) return result;
    }
    if (compare_xlive_pipe_value_00a5f25c(p->key_7c, g.key_f8badc, g, t)) {
        Wide temporary;
        temporary.lock = system.create_value_lock();
        copy_dwords_forward(temporary.bytes.data(), frame, 18);
        transform_00a5efa1(p->pending_30, g.value_f8b9a0, p->pending_30, g, t);
        transform_00a5f397(p->pending_30, p->seeded_04, p->pending_30, g, t);
        const bool rejected = compare_xlive_pipe_wide_value_00a5f2a4(temporary, p->pending_30, g, t);
        destroy_temporary(temporary);
        return rejected ? unexpected : 0;
    }
    copy_dwords_forward(p->seeded_04.bytes.data(), frame, 10);
    transform_00a5f020(p->seeded_04, g.value_f8bba0, p->seeded_04, g, t);
    auto random = system.random_output_preimage();
    system.set_last_error(0);
    if (!system.random_bytes(p->provider_00, random)) {
        const I32 error = signed_bits(system.get_last_error());
        if (error < 0) return error;
    }
    if (!random.defined.all())
        throw std::runtime_error("A5F7C5 would consume undefined CryptGenRandom output/preimage");
    U32 low, high;
    std::memcpy(&low, random.bytes.data(), 4);
    std::memcpy(&high, random.bytes.data() + 4, 4);
    transform_00a5ee78(low, high, p->pending_30, t);
    transform_00a5ef36(p->key_7c, p->key_7c, g, t);
    return 0;
}
I32 encode_xlive_pipe_frame_00a5e055(XLivePipeTransport* p, void* buffer, U32* size,
    XLiveIpcEncodeCallback callback, void* context, Globals& g, const Tables& t,
    XLivePipeFrameSystemHost& system) {
    if (bad_owner(p)) return invalid_handle;
    if (p->native.mode_04 != 0 || (!callback && context)) return invalid_argument;
    return encode_xlive_pipe_frame_00a5f6c0(p->native, buffer, size, callback, context, g, t, system);
}
I32 decode_xlive_pipe_frame_00a5e09e(XLivePipeTransport* p, const void* buffer, U32 size,
    XLiveIpcDecodeCallback callback, void* context, Globals& g, const Tables& t,
    XLivePipeFrameSystemHost& system) {
    if (bad_owner(p)) return invalid_handle;
    if (p->native.mode_04 != 0 || (!callback && context)) return invalid_argument;
    return decode_xlive_pipe_frame_00a5f7c5(p->native, buffer, size, callback, context, g, t, system);
}

std::array<Byte, 72> Win32XLivePipeFrameSystemHost::temporary_wide_preimage() {
    return preimages_.temporary_wide_preimage();
}
XLivePipeRandomOutput Win32XLivePipeFrameSystemHost::random_output_preimage() {
    return preimages_.random_output_preimage();
}
XLivePipeValueLock* Win32XLivePipeFrameSystemHost::create_value_lock() {
    return create_xlive_pipe_value_lock_00a5fa5a();
}
void Win32XLivePipeFrameSystemHost::set_last_error(DWORD e) { SetLastError(e); }
DWORD Win32XLivePipeFrameSystemHost::get_last_error() { return GetLastError(); }
bool Win32XLivePipeFrameSystemHost::random_bytes(HCRYPTPROV p, XLivePipeRandomOutput& out) {
    const bool success = CryptGenRandom(p, 8, out.bytes.data()) != FALSE;
    if (success) out.defined.set();
    return success;
}
I32 ReconstructedXLivePipeFramingHost::send_capacity_00a5df96(void* p, U32 n, U32& out) {
    return send_xlive_pipe_capacity_00a5df96(static_cast<XLivePipeTransport*>(p), n, &out);
}
I32 ReconstructedXLivePipeFramingHost::receive_capacity_00a5dfce(void* p, U32 n, U32& out) {
    return receive_xlive_pipe_capacity_00a5dfce(static_cast<XLivePipeTransport*>(p), n, &out);
}
I32 ReconstructedXLivePipeFramingHost::encode_00a5e055(void* p, void* b, U32& n,
    XLiveIpcEncodeCallback callback, void* context) {
    return encode_xlive_pipe_frame_00a5e055(static_cast<XLivePipeTransport*>(p), b, &n,
        callback, context, globals_, tables_, system_);
}
I32 ReconstructedXLivePipeFramingHost::decode_00a5e09e(void* p, const void* b, U32 n,
    XLiveIpcDecodeCallback callback, void* context) {
    return decode_xlive_pipe_frame_00a5e09e(static_cast<XLivePipeTransport*>(p), b, n,
        callback, context, globals_, tables_, system_);
}
} // namespace bsp
