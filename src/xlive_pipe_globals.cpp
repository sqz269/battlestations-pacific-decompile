#include "bsp/xlive_pipe_globals.hpp"

#include <cstring>
#include <new>
#include <stdexcept>

#pragma comment(lib, "advapi32.lib")

namespace bsp {
XLivePipeGlobalsPreimage original_xlive_pipe_loader_preimage() noexcept { return {}; }

XLivePipeGlobalData::XLivePipeGlobalData(const std::uint8_t* data, std::size_t length) : data_(data) {
    if (!data || length != size) throw std::invalid_argument("XLive pipe global data extent mismatch");
    constexpr std::array<std::uint8_t, 32> expected{
        0x20,0x43,0x6e,0xe5,0x7a,0xd0,0xc1,0x79,0xe4,0xcd,0xe3,0xff,0xd8,0x0e,0xa7,0x5b,
        0x53,0x33,0x9d,0xea,0x4d,0x50,0x0c,0xda,0x75,0x76,0x91,0x77,0x17,0x6e,0xae,0x21};
    HCRYPTPROV provider = 0;
    if (!CryptAcquireContextW(&provider, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        throw std::runtime_error("Cannot acquire global data validation provider");
    HCRYPTHASH hash = 0;
    auto digest = expected;
    DWORD count = static_cast<DWORD>(digest.size());
    const bool ok = CryptCreateHash(provider, CALG_SHA_256, 0, 0, &hash) &&
        CryptHashData(hash, data, static_cast<DWORD>(length), 0) &&
        CryptGetHashParam(hash, HP_HASHVAL, digest.data(), &count, 0);
    if (hash) CryptDestroyHash(hash);
    CryptReleaseContext(provider, 0);
    if (!ok) throw std::runtime_error("Cannot hash original XLive pipe global data");
    if (count != digest.size() || digest != expected)
        throw std::invalid_argument("XLive pipe global data SHA-256 mismatch");
}
const std::uint8_t* XLivePipeGlobalData::bytes(std::uint32_t va, std::size_t count) const {
    if (va < first_va || va - first_va > size || count > size - (va - first_va))
        throw std::out_of_range("XLive pipe fixed source outside verified extent");
    return data_ + (va - first_va);
}

XLivePipeGlobalsOwner::XLivePipeGlobalsOwner(const XLivePipeGlobalsPreimage& image)
    : value_f8b9a0(image.value_f8b9a0), value_f8b858(image.value_f8b858),
      seed_f8b8cc(image.seed_f8b8cc), value_f8bba0(image.value_f8bba0),
      key_f8badc(image.key_f8badc), raw_f8b8f8(image.raw_f8b8f8) {
    std::memcpy(&acquisition_storage_, image.acquisition.data(), image.acquisition.size());
}
Win32XLivePipeValueLock& XLivePipeGlobalsOwner::construct_acquisition_object() {
    return *new (&acquisition_storage_) Win32XLivePipeValueLock;
}
Win32XLivePipeValueLock& XLivePipeGlobalsOwner::acquisition_object() {
    return *std::launder(reinterpret_cast<Win32XLivePipeValueLock*>(&acquisition_storage_));
}
CRITICAL_SECTION& XLivePipeGlobalsOwner::acquisition_lock_f8b778() { return acquisition_object().native_section(); }
XLivePipeProtocolGlobals XLivePipeGlobalsOwner::protocol_globals() {
    return {acquisition_lock_f8b778(), seed_f8b8cc, key_f8badc};
}

namespace {
template<class Value>
Value* construct_value(Value& output, const std::uint8_t* input, XLivePipeGlobalsStartupHost& host) {
    output.lock = host.create_value_lock_00a5fa5a();
    for (std::size_t i = 0; i < output.bytes.size(); ++i) output.bytes[i] = input[i];
    return &output;
}
}
XLivePipeEncodedValue* construct_xlive_pipe_seed_global_00a60015(
    XLivePipeEncodedValue& output, const std::uint8_t* input, XLivePipeGlobalsStartupHost& host) { return construct_value(output, input, host); }
XLivePipeEncodedValue* construct_xlive_pipe_key_global_00a5ffcf(
    XLivePipeEncodedValue& output, const std::uint8_t* input, XLivePipeGlobalsStartupHost& host) { return construct_value(output, input, host); }
XLivePipeEncodedValue* construct_xlive_pipe_frame_value_00a5fff2(
    XLivePipeEncodedValue& output, const std::uint8_t* input, XLivePipeGlobalsStartupHost& host) { return construct_value(output, input, host); }
XLivePipeEncodedWideValue* construct_xlive_pipe_frame_value_00a60038(
    XLivePipeEncodedWideValue& output, const std::uint8_t* input, XLivePipeGlobalsStartupHost& host) { return construct_value(output, input, host); }
XLivePipeEncodedWideValue* construct_xlive_pipe_frame_value_00a5ffac(
    XLivePipeEncodedWideValue& output, const std::uint8_t* input, XLivePipeGlobalsStartupHost& host) { return construct_value(output, input, host); }

void initialize_xlive_pipe_raw_constant_00a60095(std::uint8_t* output, const XLivePipeProtocolTables& tables) {
    const auto* input = tables.bytes(0xd55230, 72);
    for (std::size_t i = 0; i < 72; ++i) output[i] = input[i];
}
std::uint8_t* construct_xlive_pipe_raw_global_00a600ab(
    std::uint8_t* output, const std::uint8_t* input, const XLivePipeProtocolTables& tables) {
    initialize_xlive_pipe_raw_constant_00a60095(output, tables);
    for (std::size_t i = 0; i < 72; ++i) output[i] = input[i];
    return output;
}

void cleanup_xlive_pipe_acquisition_00ce099a(XLivePipeGlobalsOwner& state) { state.acquisition_object().~Win32XLivePipeValueLock(); }
void cleanup_xlive_pipe_frame_value_00ce09a4(XLivePipeGlobalsOwner& state) {
    if (state.value_f8b9a0.lock) state.value_f8b9a0.lock->destroy(1);
}
void cleanup_xlive_pipe_frame_value_00ce09b5(XLivePipeGlobalsOwner& state) {
    if (state.value_f8b858.lock) state.value_f8b858.lock->destroy(1);
}
void cleanup_xlive_pipe_seed_00ce09c6(XLivePipeGlobalsOwner& state) {
    if (state.seed_f8b8cc.lock) state.seed_f8b8cc.lock->destroy(1);
}
void cleanup_xlive_pipe_frame_value_00ce09d7(XLivePipeGlobalsOwner& state) {
    if (state.value_f8bba0.lock) state.value_f8bba0.lock->destroy(1);
}
void cleanup_xlive_pipe_key_00ce09e8(XLivePipeGlobalsOwner& state) {
    if (state.key_f8badc.lock) state.key_f8badc.lock->destroy(1);
}

int initialize_xlive_pipe_acquisition_global_00cd6e16(XLivePipeGlobalsOwner& state, XLivePipeGlobalsStartupHost& host) {
    state.construct_acquisition_object();
    return host.register_cleanup_00bf6ff5(0xce099a, &cleanup_xlive_pipe_acquisition_00ce099a, state);
}
int initialize_xlive_pipe_frame_global_00cd6e2c(XLivePipeGlobalsOwner& state, const XLivePipeGlobalData& data, XLivePipeGlobalsStartupHost& host) {
    construct_xlive_pipe_frame_value_00a60038(state.value_f8b9a0, data.bytes(0xe12ac8, 72), host);
    return host.register_cleanup_00bf6ff5(0xce09a4, &cleanup_xlive_pipe_frame_value_00ce09a4, state);
}
int initialize_xlive_pipe_frame_global_00cd6e47(XLivePipeGlobalsOwner& state, const XLivePipeGlobalData& data, XLivePipeGlobalsStartupHost& host) {
    construct_xlive_pipe_frame_value_00a5ffac(state.value_f8b858, data.bytes(0xe12b10, 72), host);
    return host.register_cleanup_00bf6ff5(0xce09b5, &cleanup_xlive_pipe_frame_value_00ce09b5, state);
}
int initialize_xlive_pipe_seed_global_00cd6e62(XLivePipeGlobalsOwner& state, const XLivePipeGlobalData& data, XLivePipeGlobalsStartupHost& host) {
    construct_xlive_pipe_seed_global_00a60015(state.seed_f8b8cc, data.bytes(0xe12b58, 40), host);
    return host.register_cleanup_00bf6ff5(0xce09c6, &cleanup_xlive_pipe_seed_00ce09c6, state);
}
int initialize_xlive_pipe_frame_global_00cd6e7d(XLivePipeGlobalsOwner& state, const XLivePipeGlobalData& data, XLivePipeGlobalsStartupHost& host) {
    construct_xlive_pipe_frame_value_00a5fff2(state.value_f8bba0, data.bytes(0xe12b80, 40), host);
    return host.register_cleanup_00bf6ff5(0xce09d7, &cleanup_xlive_pipe_frame_value_00ce09d7, state);
}
int initialize_xlive_pipe_key_global_00cd6e98(XLivePipeGlobalsOwner& state, const XLivePipeGlobalData& data, XLivePipeGlobalsStartupHost& host) {
    construct_xlive_pipe_key_global_00a5ffcf(state.key_f8badc, data.bytes(0xe12ba8, 40), host);
    return host.register_cleanup_00bf6ff5(0xce09e8, &cleanup_xlive_pipe_key_00ce09e8, state);
}
std::uint8_t* initialize_xlive_pipe_raw_global_00cd6f03(XLivePipeGlobalsOwner& state,
    const XLivePipeGlobalData& data, const XLivePipeProtocolTables& tables) {
    return construct_xlive_pipe_raw_global_00a600ab(state.raw_f8b8f8.data(), data.bytes(0xe12cb8, 72), tables);
}
std::array<int, 6> initialize_xlive_pipe_globals(XLivePipeGlobalsOwner& state,
    const XLivePipeGlobalData& data, const XLivePipeProtocolTables& tables, XLivePipeGlobalsStartupHost& host) {
    std::array<int, 6> results;
    results[0] = initialize_xlive_pipe_acquisition_global_00cd6e16(state, host);
    results[1] = initialize_xlive_pipe_frame_global_00cd6e2c(state, data, host);
    results[2] = initialize_xlive_pipe_frame_global_00cd6e47(state, data, host);
    results[3] = initialize_xlive_pipe_seed_global_00cd6e62(state, data, host);
    results[4] = initialize_xlive_pipe_frame_global_00cd6e7d(state, data, host);
    results[5] = initialize_xlive_pipe_key_global_00cd6e98(state, data, host);
    initialize_xlive_pipe_raw_global_00cd6f03(state, data, tables);
    return results;
}
} // namespace bsp
