#include "bsp/xlive_pipe_protocol.hpp"

#include <algorithm>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>

#pragma comment(lib, "advapi32.lib")

namespace bsp {
static_assert(sizeof(Win32XLivePipeValueLock) == 0x1c);
static_assert(sizeof(XLivePipeEncodedValue) == 0x2c);
static_assert(sizeof(XLivePipeEncodedWideValue) == 0x4c);
static_assert(sizeof(XLivePipeProtocolNativeState) == 0xa8);
static_assert(offsetof(XLivePipeProtocolNativeState, pending_30) == 0x30);
static_assert(offsetof(XLivePipeProtocolNativeState, key_7c) == 0x7c);

Win32XLivePipeValueLock::Win32XLivePipeValueLock() { InitializeCriticalSection(&section_04); }
Win32XLivePipeValueLock::~Win32XLivePipeValueLock() { DeleteCriticalSection(&section_04); }
XLivePipeValueLock* Win32XLivePipeValueLock::destroy(std::uint32_t flags) {
    this->~Win32XLivePipeValueLock();
    if (flags & 1) ::operator delete(this);
    return this;
}
void Win32XLivePipeValueLock::enter() { EnterCriticalSection(&section_04); }
void Win32XLivePipeValueLock::leave() { LeaveCriticalSection(&section_04); }
XLivePipeValueLock* create_xlive_pipe_value_lock_00a5fa5a() {
    auto* storage = ::operator new(sizeof(Win32XLivePipeValueLock));
    return storage ? new (storage) Win32XLivePipeValueLock : nullptr;
}
void enter_xlive_pipe_global_00a5f94f(CRITICAL_SECTION& section) { EnterCriticalSection(&section); }
void leave_xlive_pipe_global_00a5f95b(CRITICAL_SECTION& section) { LeaveCriticalSection(&section); }

namespace {
constexpr std::array<std::uint8_t, 32> expected_digest{
    0xf3,0x45,0x61,0x22,0x84,0x44,0x89,0xdc,0x39,0xce,0xee,0x39,0x3f,0xc4,0xec,0xfa,
    0xdd,0xf7,0x89,0x44,0x9b,0xdd,0xf8,0x9b,0x43,0x4b,0x2c,0xd5,0x5f,0xe5,0xc7,0x7c};
bool matches_tables(const std::uint8_t* bytes, DWORD count) {
    HCRYPTPROV provider = 0;
    if (!CryptAcquireContextW(&provider, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        throw std::runtime_error("Cannot acquire SHA-256 table validation provider");
    HCRYPTHASH hash = 0;
    auto digest = expected_digest;
    DWORD size = static_cast<DWORD>(digest.size());
    const bool ok = CryptCreateHash(provider, CALG_SHA_256, 0, 0, &hash) &&
        CryptHashData(hash, bytes, count, 0) &&
        CryptGetHashParam(hash, HP_HASHVAL, digest.data(), &size, 0);
    if (hash) CryptDestroyHash(hash);
    CryptReleaseContext(provider, 0);
    if (!ok) throw std::runtime_error("Cannot hash original XLive pipe tables");
    return size == digest.size() && digest == expected_digest;
}
std::uint32_t signed_row(const std::uint8_t* rows, std::int32_t index) {
    std::int16_t value;
    std::memcpy(&value, rows + static_cast<std::size_t>(index) * 2, sizeof value);
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(value));
}
std::int32_t output_index(std::int32_t index, std::int32_t displacement) {
    return index == 0 ? 0 : static_cast<std::int32_t>(
        static_cast<std::uint32_t>(index) - static_cast<std::uint32_t>(displacement));
}
}

XLivePipeProtocolTables::XLivePipeProtocolTables(const std::uint8_t* data, std::size_t length)
    : data_(data) {
    if (!data || length != size) throw std::invalid_argument("XLive pipe table extent mismatch");
    if (!matches_tables(data, static_cast<DWORD>(length)))
        throw std::invalid_argument("XLive pipe original table SHA-256 mismatch");
}
const std::uint8_t* XLivePipeProtocolTables::bytes(std::uint32_t va, std::size_t count) const {
    if (va < first_va || va - first_va > size || count > size - (va - first_va))
        throw std::out_of_range("XLive pipe table address outside verified extent");
    return data_ + (va - first_va);
}
std::uint32_t XLivePipeProtocolTables::word(std::uint32_t base, std::uint32_t index) const {
    std::uint32_t result;
    // Native x86 scaled address arithmetic wraps at 32 bits.
    std::memcpy(&result, bytes(base + index * 4u, 4), sizeof result);
    return result;
}

void transform_xlive_pipe_seed_00a5fbc3(const std::uint8_t* left,
    const std::uint8_t* seed, std::uint8_t* output, const std::uint8_t* rows1,
    const std::uint8_t* rows2, std::int32_t displacement, std::int32_t left_size,
    std::int32_t right_size, std::int32_t output_size, const XLivePipeProtocolTables& t) {
    std::uint32_t first = 0, second = 0;
    const auto count = (std::min)((std::min)(left_size, right_size), output_size);
    for (std::int32_t i = 0; i < count; ++i) {
        // MOVSX byte / SAR / AND1 equals this bit extraction for positions0..7.
        const auto bit = i < 8 ? 0u : (seed[(i - 8) / 8] >> ((i - 8) & 7)) & 1u;
        first = t.word(0xd2edd0, ((left[i] & 3u) * 2) | signed_row(rows1, i) |
                ((first >> 12) & 0x3e0) | bit) ^
            t.word(0xd2fe50, (first >> 6) & 0x3f) ^
            t.word(0xd2fdd0, (first >> 12) & 0x1f) ^ t.word(0xd2ff50, first & 0x3f);
        second = t.word(0xd30050, ((((first & 0x1c00000) >> 8) | (second & 0x1e0000)) >> 8) |
                signed_row(rows2, i)) ^
            t.word(0xd380d0, (second >> 6) & 0x3f) ^
            t.word(0xd38050, (second >> 12) & 0x1f) ^ t.word(0xd381d0, second & 0x3f);
        output[output_index(i, displacement)] = static_cast<std::uint8_t>((second >> 21) & 3);
        if (i == 0 && displacement != 0)
            second = t.word(0xd54208, ((first & 0x1c00000) >> 16) | signed_row(rows2, 0));
    }
}
void transform_xlive_pipe_pair_00a5fe80(const std::uint8_t* left,
    const std::uint8_t* right, std::uint8_t* output, const std::uint8_t* rows1,
    const std::uint8_t* rows2, std::int32_t displacement, std::int32_t left_size,
    std::int32_t right_size, std::int32_t output_size, const XLivePipeProtocolTables& t) {
    std::uint32_t first = 0, second = 0;
    const auto count = (std::min)((std::min)(left_size, right_size), output_size);
    for (std::int32_t i = 0; i < count; ++i) {
        first = t.word(0xd42d70, (right[i] & 3u) | signed_row(rows1, i) |
                ((left[i] & 3u) << 2) | ((first >> 16) & 0x1f80)) ^
            t.word(0xd4af70, (first >> 8) & 0xff) ^
            t.word(0xd4ad70, (first >> 16) & 0x7f) ^ t.word(0xd4b370, first & 0xff);
        second = t.word(0xd4b770, ((((first & 0xe0000000) >> 15) | (second & 0x1e0000)) >> 8) |
                signed_row(rows2, i)) ^
            t.word(0xd537f0, (second >> 6) & 0x3f) ^
            t.word(0xd53770, (second >> 12) & 0x1f) ^ t.word(0xd538f0, second & 0x3f);
        output[output_index(i, displacement)] = static_cast<std::uint8_t>((second >> 21) & 3);
        if (i == 0 && displacement != 0)
            second = t.word(0xd54a08, ((first & 0xe0000000) >> 23) | signed_row(rows2, 0));
    }
}
void copy_xlive_pipe_seed_constant_00a5ebc3(std::uint8_t* output, const XLivePipeProtocolTables& t) {
    std::memcpy(output, t.bytes(0xd25d9c, 40), 40);
}
void copy_xlive_pipe_key_constant_00a5ed17(std::uint8_t* output, const XLivePipeProtocolTables& t) {
    std::memcpy(output, t.bytes(0xd26010, 40), 40);
}
void mix_xlive_pipe_seed_00a60123(const XLivePipeEncodedValue& source, std::uint32_t seed,
    std::uint8_t* output, const XLivePipeProtocolTables& t) {
    transform_xlive_pipe_seed_00a5fbc3(source.bytes.data(), reinterpret_cast<const std::uint8_t*>(&seed),
        output, t.bytes(0xd55288, 80), t.bytes(0xd552d8, 80), 0, 40, 40, 40, t);
}
void combine_xlive_pipe_seed_00a6014e(const std::uint8_t* left,
    const XLivePipeEncodedValue& right, XLivePipeEncodedValue& output, const XLivePipeProtocolTables& t) {
    transform_xlive_pipe_pair_00a5fe80(left, right.bytes.data(), output.bytes.data(),
        t.bytes(0xd55328, 80), t.bytes(0xd55378, 80), 0, 40, 40, 40, t);
}
void mix_xlive_pipe_key_00a60259(const XLivePipeEncodedValue& source,
    std::uint8_t* output, const XLivePipeProtocolTables& t) {
    transform_xlive_pipe_pair_00a5fe80(source.bytes.data(), t.bytes(0xd559f8, 40), output,
        t.bytes(0xd55a20, 80), t.bytes(0xd55a70, 80), 0, 40, 40, 40, t);
}
void combine_xlive_pipe_key_00a60284(const std::uint8_t* left,
    const XLivePipeEncodedValue& right, XLivePipeEncodedValue& output, const XLivePipeProtocolTables& t) {
    transform_xlive_pipe_pair_00a5fe80(left, right.bytes.data(), output.bytes.data(),
        t.bytes(0xd55ac0, 80), t.bytes(0xd55b10, 80), 0, 40, 40, 40, t);
}
void initialize_xlive_pipe_seed_00a5ed9f(XLivePipeEncodedValue& source,
    std::uint32_t seed, XLivePipeEncodedValue& output, CRITICAL_SECTION& global,
    const XLivePipeProtocolTables& tables) {
    enter_xlive_pipe_global_00a5f94f(global);
    source.lock->enter();
    output.lock->enter();
    leave_xlive_pipe_global_00a5f95b(global);
    std::array<std::uint8_t, 40> temporary;
    copy_xlive_pipe_seed_constant_00a5ebc3(temporary.data(), tables);
    mix_xlive_pipe_seed_00a60123(source, seed, temporary.data(), tables);
    combine_xlive_pipe_seed_00a6014e(temporary.data(), output, output, tables);
    source.lock->leave();
    output.lock->leave();
}
void initialize_xlive_pipe_key_00a5eecb(XLivePipeEncodedValue& source,
    XLivePipeEncodedValue& output, CRITICAL_SECTION& global, const XLivePipeProtocolTables& tables) {
    enter_xlive_pipe_global_00a5f94f(global);
    source.lock->enter();
    output.lock->enter();
    leave_xlive_pipe_global_00a5f95b(global);
    std::array<std::uint8_t, 40> temporary;
    copy_xlive_pipe_key_constant_00a5ed17(temporary.data(), tables);
    mix_xlive_pipe_key_00a60259(source, temporary.data(), tables);
    combine_xlive_pipe_key_00a60284(temporary.data(), output, output, tables);
    source.lock->leave();
    output.lock->leave();
}

XLivePipeProtocolNativeState* Win32XLivePipeProtocolSystemHost::allocate_context() {
    // Default initialization is deliberate: value initialization would erase
    // the actual allocation bytes before the provider can observe them.
    std::unique_ptr<XLivePipeProtocolNativeState> context(new XLivePipeProtocolNativeState);
    const auto preimage = preimages_.context_allocation_preimage(*context);
    std::memcpy(context.get(), preimage.data(), preimage.size());
    return context.release(); // provider failure frees this still-unpublished allocation
}
void Win32XLivePipeProtocolSystemHost::free_context(XLivePipeProtocolNativeState* value) noexcept { delete value; }
XLivePipeValueLock* Win32XLivePipeProtocolSystemHost::create_value_lock() { return create_xlive_pipe_value_lock_00a5fa5a(); }
void Win32XLivePipeProtocolSystemHost::set_last_error(DWORD value) { SetLastError(value); }
DWORD Win32XLivePipeProtocolSystemHost::get_last_error() { return GetLastError(); }
bool Win32XLivePipeProtocolSystemHost::acquire_context(HCRYPTPROV& provider) {
    return CryptAcquireContextW(&provider, nullptr, nullptr, PROV_RSA_FULL, 0xf0000040) != FALSE;
}
bool Win32XLivePipeProtocolSystemHost::random_seed(HCRYPTPROV provider, std::uint32_t& seed) {
    return CryptGenRandom(provider, 4, reinterpret_cast<BYTE*>(&seed)) != FALSE;
}
void Win32XLivePipeProtocolSystemHost::release_context(HCRYPTPROV provider) { static_cast<void>(CryptReleaseContext(provider, 0)); }
XLivePipeProtocolNativeState* construct_xlive_pipe_protocol_00a5f336(
    XLivePipeProtocolNativeState& context, XLivePipeProtocolSystemHost& system) {
    context.seeded_04.lock = system.create_value_lock();
    context.pending_30.lock = system.create_value_lock();
    context.key_7c.lock = system.create_value_lock();
    return &context;
}
void destroy_xlive_pipe_protocol_members_00a5f1d8(XLivePipeProtocolNativeState& context) {
    if (context.key_7c.lock) context.key_7c.lock->destroy(1);
    if (context.pending_30.lock) context.pending_30.lock->destroy(1);
    if (context.seeded_04.lock) context.seeded_04.lock->destroy(1);
}
void initialize_xlive_pipe_protocol_00a5f416(void*& slot, XLivePipeProtocolGlobals& globals,
    const XLivePipeProtocolTables& tables, XLivePipeProtocolSystemHost& system) {
    auto* context = system.allocate_context();
    if (context) {
        construct_xlive_pipe_protocol_00a5f336(*context, system);
        system.set_last_error(0);
        bool success = false;
        if (!system.acquire_context(context->provider_00)) {
            static_cast<void>(system.get_last_error());
            context->provider_00 = 0;
        } else {
            system.set_last_error(0);
            std::uint32_t seed;
            if (system.random_seed(context->provider_00, seed)) {
                initialize_xlive_pipe_seed_00a5ed9f(globals.seed_f8b8cc, seed, context->seeded_04,
                    globals.acquisition_lock_f8b778, tables);
                initialize_xlive_pipe_key_00a5eecb(globals.key_f8badc, context->key_7c,
                    globals.acquisition_lock_f8b778, tables);
                success = true;
            } else static_cast<void>(system.get_last_error());
        }
        if (!success) {
            if (context->provider_00) system.release_context(context->provider_00);
            destroy_xlive_pipe_protocol_members_00a5f1d8(*context);
            system.free_context(context);
            context = nullptr; // A5F4D5 XOR ESI,ESI hidden by false no-return
        }
    }
    slot = context;
}
void destroy_xlive_pipe_protocol_00a5f371(void* protocol, XLivePipeProtocolSystemHost& system) {
    auto* context = static_cast<XLivePipeProtocolNativeState*>(protocol);
    if (!context) return;
    if (context->provider_00) system.release_context(context->provider_00);
    destroy_xlive_pipe_protocol_members_00a5f1d8(*context);
    system.free_context(context);
}
void ReconstructedXLivePipeProtocolHost::initialize_protocol_00a5f416(void*& slot) {
    initialize_xlive_pipe_protocol_00a5f416(slot, globals_, tables_, system_);
}
void ReconstructedXLivePipeProtocolHost::destroy_protocol_00a5f371(void* protocol) {
    destroy_xlive_pipe_protocol_00a5f371(protocol, system_);
}
} // namespace bsp
