#include "bsp/native_vfs_search_defaults.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_search_groups.hpp"
#include "bsp/native_vfs_extension_prefix.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
struct Header { std::uint32_t length{}; char* data{}; };
static_assert(sizeof(Header) == 8);
struct Owner {
    Header header{};
    ActualNativeStringPoolStorage& strings;
    explicit Owner(ActualNativeStringPoolStorage& storage) : strings(storage) {}
    ~Owner() noexcept {
        if (header.data) destroy_native_string_header_0041dd20(&header, strings);
    }
    void resize_copy(const char* literal, std::uint32_t length) {
        resize_native_string_header_0041dd40(&header, strings, length, true);
        if (header.data) std::memcpy(header.data, literal, header.length + 1u);
    }
    void assign(const char* literal) {
        construct_native_string_cstring_0041e870(&header, literal, strings);
    }
};
} // namespace

void register_native_vfs_search_defaults_00738360(void* volatile& actual_manager_publication_0109ceec,
    ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& callbacks) {
    // 01: original CALL 00738401 -> 00be25e0; ECX reload 007383f6 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("dds", 3u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_extension_00be25e0(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 02: original CALL 007384d2 -> 00be25e0; ECX reload 007384bd <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("tga", 3u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_extension_00be25e0(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 03: original CALL 007385a1 -> 00be2600; ECX reload 00738596 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("textures\\", 9u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 04: original CALL 00738670 -> 00be2600; ECX reload 0073865b <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("models\\textures\\", 16u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 05: original CALL 0073873f -> 00be2600; ECX reload 00738734 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("models\\textures\\noseart\\", 24u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 06: original CALL 0073880e -> 00be2600; ECX reload 007387f9 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("models\\gui\\map\\units\\", 21u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 07: original CALL 007388dd -> 00be2600; ECX reload 007388d2 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("models\\gui\\map\\icons\\", 21u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 08: original CALL 007389ac -> 00be2600; ECX reload 00738997 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("interface\\textures\\mainmenu\\", 28u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 09: original CALL 00738a7b -> 00be2600; ECX reload 00738a70 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("interface\\textures\\", 19u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 10: original CALL 00738b4a -> 00be2600; ECX reload 00738b35 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Particles\\Textures\\", 19u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 11: original CALL 00738c19 -> 00be2600; ECX reload 00738c0e <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Particles\\Textures\\anims\\fragments\\", 35u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 12: original CALL 00738ce8 -> 00be2600; ECX reload 00738cd3 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\Flares\\Textures\\", 24u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 13: original CALL 00738db7 -> 00be2600; ECX reload 00738dac <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Fonts\\", 6u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 14: original CALL 00738e86 -> 00be2600; ECX reload 00738e71 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Weather\\", 8u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 15: original CALL 00738f55 -> 00be2600; ECX reload 00738f4a <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Terrain\\", 8u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 16: original CALL 00739024 -> 00be2600; ECX reload 0073900f <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\", 8u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 17: original CALL 007390f3 -> 00be2600; ECX reload 007390e8 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\Coast\\", 14u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 18: original CALL 007391c2 -> 00be2600; ECX reload 007391ad <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\WaterTracer\\", 20u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 19: original CALL 00739291 -> 00be2600; ECX reload 00739286 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\Foam\\", 13u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 20: original CALL 00739360 -> 00be2600; ECX reload 0073934b <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\Caustics\\", 17u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 21: original CALL 0073942f -> 00be2600; ECX reload 00739424 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\Lightning\\", 18u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 22: original CALL 007394fe -> 00be2600; ECX reload 007394e9 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\Traceline\\", 18u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 23: original CALL 007395cd -> 00be2600; ECX reload 007395c2 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\Foliage\\", 16u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 24: original CALL 0073969c -> 00be2600; ECX reload 00739687 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("Effects\\postprocess\\", 20u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 25: original CALL 0073976b -> 00be2600; ECX reload 00739760 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("interface\\textures\\terkep\\", 26u);
        Owner group(strings);
        group.resize_copy("textures", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 26: original CALL 0073983a -> 00be1480; ECX reload 00739825 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("interface\\", 10u);
        Owner group(strings);
        group.resize_copy("lua", 3u);
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 27: original CALL 00739909 -> 00be1480; ECX reload 007398fe <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("models\\", 7u);
        Owner group(strings);
        group.resize_copy("mmod", 4u);
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 28: original CALL 007399d8 -> 00be1480; ECX reload 007399c3 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("fonts\\", 6u);
        Owner group(strings);
        group.resize_copy("dat", 3u);
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 29: original CALL 00739aa7 -> 00be1480; ECX reload 00739a9c <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("textures\\", 9u);
        Owner group(strings);
        group.resize_copy("dat", 3u);
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 30: original CALL 00739b76 -> 00be1480; ECX reload 00739b61 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\bin\\", 13u);
        Owner group(strings);
        group.resize_copy("shbin", 5u);
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 31: original CALL 00739c45 -> 00be25e0; ECX reload 00739c3a <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shfx", 4u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_extension_00be25e0(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 32: original CALL 00739d14 -> 00be2600; ECX reload 00739cff <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\", 9u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 33: original CALL 00739de3 -> 00be2600; ECX reload 00739dd8 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\plane\\", 15u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 34: original CALL 00739eb2 -> 00be2600; ECX reload 00739e9d <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\ship\\", 14u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 35: original CALL 00739f81 -> 00be2600; ECX reload 00739f76 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\common\\", 16u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 36: original CALL 0073a050 -> 00be2600; ECX reload 0073a03b <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\particles\\", 19u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 37: original CALL 0073a11f -> 00be2600; ECX reload 0073a114 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\terrain\\", 17u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 38: original CALL 0073a1ee -> 00be2600; ECX reload 0073a1d9 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\ocean\\", 15u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 39: original CALL 0073a2bd -> 00be2600; ECX reload 0073a2b2 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\lights\\", 16u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 40: original CALL 0073a38c -> 00be2600; ECX reload 0073a377 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\postprocess\\", 21u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 41: original CALL 0073a45b -> 00be2600; ECX reload 0073a450 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaderfx\\gui\\", 13u);
        Owner group(strings);
        group.resize_copy("shaderfx", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 42: original CALL 0073a52a -> 00be1480; ECX reload 0073a515 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaders\\", 8u);
        Owner group(strings);
        group.resize_copy("pso", 3u);
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 43: original CALL 0073a5f9 -> 00be1480; ECX reload 0073a5ee <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("shaders\\", 8u);
        Owner group(strings);
        group.resize_copy("vso", 3u);
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 44: original CALL 0073a6c8 -> 00be25e0; ECX reload 0073a6b3 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("mshd", 4u);
        Owner group(strings);
        group.resize_copy("fshaders", 8u);
        register_native_vfs_search_extension_00be25e0(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 45: original CALL 0073a797 -> 00be2600; ECX reload 0073a78c <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("fshaders\\", 9u);
        Owner group(strings);
        group.resize_copy("fshaders", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 46: original CALL 0073a866 -> 00be2600; ECX reload 0073a851 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("fshaders\\plane\\", 15u);
        Owner group(strings);
        group.resize_copy("fshaders", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 47: original CALL 0073a935 -> 00be2600; ECX reload 0073a92a <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("fshaders\\ship\\", 14u);
        Owner group(strings);
        group.resize_copy("fshaders", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 48: original CALL 0073aa04 -> 00be2600; ECX reload 0073a9ef <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("fshaders\\common\\", 16u);
        Owner group(strings);
        group.resize_copy("fshaders", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 49: original CALL 0073aad3 -> 00be2600; ECX reload 0073aac8 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("fshaders\\particles\\", 19u);
        Owner group(strings);
        group.resize_copy("fshaders", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 50: original CALL 0073aba2 -> 00be2600; ECX reload 0073ab8d <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("fshaders\\terrain\\", 17u);
        Owner group(strings);
        group.resize_copy("fshaders", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 51: original CALL 0073ac71 -> 00be2600; ECX reload 0073ac66 <- 0109ceec
    {
        Owner value(strings);
        value.resize_copy("fshaders\\ocean\\", 15u);
        Owner group(strings);
        group.resize_copy("fshaders", 8u);
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 52: original CALL 0073acf6 -> 00be1480; ECX reload 0073ace1 <- 0109ceec
    {
        Owner value(strings);
        value.assign("effects\\postprocess\\");
        Owner group(strings);
        group.assign("pel");
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 53: original CALL 0073ad7b -> 00be1480; ECX reload 0073ad6b <- 0109ceec
    {
        Owner value(strings);
        value.assign("effects\\postprocess\\");
        Owner group(strings);
        group.assign("pfx");
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 54: original CALL 0073ae00 -> 00be1480; ECX reload 0073adf5 <- 0109ceec
    {
        Owner value(strings);
        value.assign("effects\\postprocess\\");
        Owner group(strings);
        group.assign("pfv");
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 55: original CALL 0073ae85 -> 00be1480; ECX reload 0073ae70 <- 0109ceec
    {
        Owner value(strings);
        value.assign("Weather\\");
        Owner group(strings);
        group.assign("raw");
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 56: original CALL 0073af0a -> 00be1480; ECX reload 0073aefa <- 0109ceec
    {
        Owner value(strings);
        value.assign("particles\\");
        Owner group(strings);
        group.assign("pes");
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 57: original CALL 0073af8f -> 00be25e0; ECX reload 0073af84 <- 0109ceec
    {
        Owner value(strings);
        value.assign("fsb");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_extension_00be25e0(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 58: original CALL 0073b014 -> 00be2600; ECX reload 0073afff <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\events\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 59: original CALL 0073b099 -> 00be2600; ECX reload 0073b089 <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\engines\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 60: original CALL 0073b11e -> 00be2600; ECX reload 0073b113 <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\explosions\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 61: original CALL 0073b1a3 -> 00be2600; ECX reload 0073b18e <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\weapons\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 62: original CALL 0073b228 -> 00be2600; ECX reload 0073b218 <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\messages\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 63: original CALL 0073b2ad -> 00be2600; ECX reload 0073b2a2 <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\messages\\warning");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 64: original CALL 0073b332 -> 00be2600; ECX reload 0073b31d <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\environment\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 65: original CALL 0073b3b7 -> 00be2600; ECX reload 0073b3a7 <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\music\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 66: original CALL 0073b43c -> 00be2600; ECX reload 0073b431 <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\muzzle\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 67: original CALL 0073b4c1 -> 00be2600; ECX reload 0073b4ac <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\impact\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 68: original CALL 0073b546 -> 00be2600; ECX reload 0073b536 <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\splash\\");
        Owner group(strings);
        group.assign("sound");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 69: original CALL 0073b5cb -> 00be25e0; ECX reload 0073b5c0 <- 0109ceec
    {
        Owner value(strings);
        value.assign("fev");
        Owner group(strings);
        group.assign("events");
        register_native_vfs_search_extension_00be25e0(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 70: original CALL 0073b650 -> 00be2600; ECX reload 0073b63b <- 0109ceec
    {
        Owner value(strings);
        value.assign("sound\\events\\");
        Owner group(strings);
        group.assign("events");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 71: original CALL 0073b6d5 -> 00be1480; ECX reload 0073b6c5 <- 0109ceec
    {
        Owner value(strings);
        value.assign("movies");
        Owner group(strings);
        group.assign("bik");
        register_native_vfs_extension_prefix_00be1480(actual_manager_publication_0109ceec, &group.header, &value.header, strings,
            callbacks);
    }
    // 72: original CALL 0073b75a -> 00be25e0; ECX reload 0073b74f <- 0109ceec
    {
        Owner value(strings);
        value.assign("mpak");
        Owner group(strings);
        group.assign("mpaks");
        register_native_vfs_search_extension_00be25e0(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 73: original CALL 0073b7df -> 00be2600; ECX reload 0073b7ca <- 0109ceec
    {
        Owner value(strings);
        value.assign("mpak\\classes\\");
        Owner group(strings);
        group.assign("mpaks");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 74: original CALL 0073b864 -> 00be2600; ECX reload 0073b854 <- 0109ceec
    {
        Owner value(strings);
        value.assign("mpak\\scenes\\");
        Owner group(strings);
        group.assign("mpaks");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 75: original CALL 0073b8e9 -> 00be2600; ECX reload 0073b8de <- 0109ceec
    {
        Owner value(strings);
        value.assign("mpak\\global\\");
        Owner group(strings);
        group.assign("mpaks");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 76: original CALL 0073b96e -> 00be2600; ECX reload 0073b959 <- 0109ceec
    {
        Owner value(strings);
        value.assign("mpak_pc\\classes\\");
        Owner group(strings);
        group.assign("mpaks");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 77: original CALL 0073b9f3 -> 00be2600; ECX reload 0073b9e3 <- 0109ceec
    {
        Owner value(strings);
        value.assign("mpak_pc\\scenes\\");
        Owner group(strings);
        group.assign("mpaks");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
    // 78: original CALL 0073ba78 -> 00be2600; ECX reload 0073ba6d <- 0109ceec
    {
        Owner value(strings);
        value.assign("mpak_pc\\global\\");
        Owner group(strings);
        group.assign("mpaks");
        register_native_vfs_search_directory_00be2600(actual_manager_publication_0109ceec, &group.header, &value.header, strings, callbacks);
    }
}
} // namespace bsp
