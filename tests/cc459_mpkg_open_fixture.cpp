// Focused Win32 fixture for the newly connected native MPKG large-entry route.
// Link bsp_core/bsp_lua511/bsp_zlib121 and the platform libraries used by the
// normal probes, with /MANIFEST:EMBED. Synthetic storage, not a game-run claim.
#include "bsp/native_mpkg_runtime.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/water_weather_cc13.hpp"
#include "bsp/oct18_tail_c.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace {
using U = std::uint32_t;
void require(bool value) { if (!value) throw std::runtime_error("MPKG fixture assertion"); }
U address(const void* value) { return reinterpret_cast<U>(value); }
struct Source {
    U* table;
    long refs = 1;
    U position = 0;
    std::vector<unsigned char> bytes;
};
struct Streams final : bsp::NativeAdoptedSubstreamDispatch {
    unsigned reads = 0, seeks = 0, zeroes = 0;
    std::uint8_t source_is_open(std::uintptr_t,void*) override { throw std::logic_error("unexpected open query"); }
    U source_seek(std::uintptr_t target,void* raw,U low,U high,U origin) override {
        require(target == 0x00bef540 && high == 0 && origin == 0);
        static_cast<Source*>(raw)->position = low; ++seeks; return low;
    }
    void source_read(std::uintptr_t target,void* raw,void* out,U size,U* actual) override {
        auto& source = *static_cast<Source*>(raw);
        require(target == 0x00bef590 && source.position + size <= source.bytes.size());
        std::memcpy(out,source.bytes.data()+source.position,size);
        source.position += size; ++reads; if (actual) *actual = size;
    }
    void source_write(std::uintptr_t,void*,const void*,U,U*) override { throw std::logic_error("unexpected write"); }
    void source_zero_reference(std::uintptr_t target,void* raw,std::uintptr_t table) override {
        require(target == 0x00bd30e0 && table == address(static_cast<Source*>(raw)->table));
        require(static_cast<Source*>(raw)->refs == 0); ++zeroes;
    }
};
struct Services final : bsp::NativeMpkgDirectoryServices, bsp::NativeMpkgArchiveServices {
    Source& reopened;
    void* manager;
    unsigned opens = 0, headers = 0, frees = 0;
    explicit Services(Source& source,void* current) : reopened(source),manager(current) {}
    std::uint64_t source_length(std::uintptr_t,void*) override { throw std::logic_error("unexpected length"); }
    void* allocate_00bf55be(U bytes) override { require(bytes == 30); ++headers; return std::malloc(bytes); }
    void free_00bf65ac(void* bytes) override { ++frees; std::free(bytes); }
    bsp::NativeStringPoolStorage* string_pool_00419cc0() override { throw std::logic_error("unexpected pool"); }
    void return_string_00bd1510(bsp::NativeStringPoolStorage*,void*,U,U) override { throw std::logic_error("unexpected string return"); }
    void* open_manager_00bb99a0(std::uintptr_t target,void* owner,const void* name,U flags) override {
        require(target == 0x00bdf310 && owner == manager && flags == 2);
        const auto* header = static_cast<const U*>(name);
        require(header[0] == 12 && std::strcmp(reinterpret_cast<const char*>(header[1]),"fixture.mpkg") == 0);
        ++opens; return &reopened;
    }
    void delete_stream_00bb9aae(std::uintptr_t,void*,U) override { throw std::logic_error("unexpected deletion"); }
};
}

int main() {
    static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
    std::array<U,13> profile{};
    profile[0] = 0x00bd30e0; profile[7] = 0x00bef540; profile[9] = 0x00bef590;
    Source decoded{profile.data()}, reopened{profile.data()};
    decoded.bytes.resize(64);
    // Entry local header at8, name length4 at+26, extra length2 at+28 ->44.
    decoded.bytes[8+26] = 4; decoded.bytes[8+28] = 2;
    std::array<U,2> manager_table{0,0x00bdf310};
    U* manager_owner = manager_table.data();
    void* volatile manager = &manager_owner;
    Streams streams;
    Services services(reopened,manager);
    bsp::NativeMpkgDirectoryContext directory{bsp::crt_string_storage(),streams,services};
    volatile U object_count = 0, requested_bytes = 0;
    bsp::NativeRetainedMemoryOwnerContext memory{object_count,requested_bytes,nullptr,nullptr};
    bsp::NativeStoredStreamConversionContext conversion{memory,nullptr};
    bsp::NativeMpkgArchiveContext context{manager,conversion,directory,services,nullptr};
    bsp::NativeMpkgArchiveRuntimeOperations operations(context);
    std::array<U,13> archive{};
    std::array<U,9> entry{};
    archive[1] = 12; archive[2] = address("fixture.mpkg"); archive[3] = address(&decoded);
    archive[10] = address(entry.data()); archive[11] = 1; archive[12] = 1;
    entry[0] = 9; entry[1] = address("large.bin"); entry[2] = 8; entry[7] = 0x40001;
    std::array<U,6> provider{}; provider[5] = address(archive.data());
    const U name[2] = {9,address("LARGE.BIN")};
    const U missing[2] = {9,address("other.bin")};
    auto open = [&](const void* wanted,U flags) {
        return bsp::water_weather_cc13::w3_mpkg_provider_open_entry_00bb8e70(
            provider.data(),wanted,flags,[&](void* state,const void* key,U mode) {
                return operations.open_named(state,key,mode);
            });
    };
    require(open(name,1) == nullptr && open(missing,2) == nullptr);
    require(streams.reads == 0 && services.opens == 0);
    void* raw = open(name,2);
    const auto* result = static_cast<const U*>(raw);
    require(result && result[0] == 0x00d68db0 && result[1] == 1);
    require(result[2] == address(&reopened) && result[4] == 44 && result[5] == 0);
    require(result[6] == 44+0x40001 && result[7] == 0 && result[8] == 44 && result[9] == 0);
    require(entry[3] == 1 && entry[4] == 44 && decoded.position == 38 && reopened.position == 44);
    require(streams.reads == 1 && streams.seeks == 2 && services.opens == 1);
    require(services.headers == 1 && services.frees == 1 && decoded.refs == 1 && reopened.refs == 1);
    bsp::oct18_tail_c::MpkgBindings cached{};
    require(bsp::oct18_tail_c::mpkg_entry_offset_00bb8a90(archive.data(),entry.data(),cached) == 44);
    require(streams.reads == 1 && streams.seeks == 2);
    bsp::delete_native_adopted_substream_00bf1240(raw,1,streams);
    require(reopened.refs == 0 && decoded.refs == 1 && streams.zeroes == 1);
    require(object_count == 0 && requested_bytes == 0);
    std::puts("PASS: MPKG provider/named/open/offset/adopted source; flag reject, miss, mixed case, local-header cache, exact range and ownership");
}
