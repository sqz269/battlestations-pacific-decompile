#include "bsp/d3d9_startup.hpp"
#include "bsp/camera_inverse.hpp"
#include "bsp/camera_multiply.hpp"
#include "camera_reference.hpp"
#include <cstdio>
#include <cstring>

bool probe_camera_reference() {
    constexpr auto size = sizeof(camera_reference_bytes);
    auto* code = static_cast<unsigned char*>(VirtualAlloc(nullptr, size,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!code) return false;
    std::memcpy(code, camera_reference_bytes, size);
    for (const auto& patch : camera_reference_patches) {
        const auto value = patch.relative ? patch.target - (patch.offset + 4)
            : static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(code) + patch.target);
        std::memcpy(code + patch.offset, &value, 4);
    }
    DWORD old{};
    if (!VirtualProtect(code, size, PAGE_EXECUTE_READ, &old)
        || !FlushInstructionCache(GetCurrentProcess(), code, size)) {
        VirtualFree(code, 0, MEM_RELEASE); return false;
    }
    using Inverse = float* (__fastcall *)(float*, const float*);
    using Multiply = float* (__fastcall *)(const float*, void*, float*, const float*);
    using Projection = float* (__thiscall *)(float*, float, float, float, float);
    Inverse inverse{}; Multiply multiply{}; Projection projection{};
    auto* entry = code + camera_reference_00b63b30;
    std::memcpy(&inverse, &entry, sizeof(inverse));
    entry = code + camera_reference_00413920;
    std::memcpy(&multiply, &entry, sizeof(multiply));
    entry = code + camera_reference_00b642f0;
    std::memcpy(&projection, &entry, sizeof(projection));
    const bsp::CameraMatrix world{1.2f,1.6f,0,0,-2.4f,1.8f,0,0,0,0,4,0,.13f,-.71f,2.3f,1};
    bsp::CameraMatrix native{}, rebuilt{}, right{};
    inverse(native.data(), world.data());
    bsp::invert_camera_affine_00b63b30(rebuilt, world);
    const bool inverse_equal = std::memcmp(native.data(), rebuilt.data(), sizeof(native)) == 0;
    projection(native.data(), 1.1f, 1.7f, .13f, 713);
    bsp::build_projection_00b642f0(right, 1.1f, 1.7f, .13f, 713);
    const bool projection_equal = std::memcmp(native.data(), right.data(), sizeof(native)) == 0;
    bool multiply_equal = true;
    // One input pair, including the exact aliases promised by the native API.
    for (unsigned alias = 0; alias < 4; ++alias) {
        auto native_left = world, native_right = right, rebuilt_left = world, rebuilt_right = right;
        auto* native_out = alias == 1 || alias == 3 ? &native_left : alias == 2 ? &native_right : &native;
        auto* rebuilt_out = alias == 1 || alias == 3 ? &rebuilt_left : alias == 2 ? &rebuilt_right : &rebuilt;
        const auto* nr = alias == 3 ? &native_left : &native_right;
        const auto* rr = alias == 3 ? &rebuilt_left : &rebuilt_right;
        multiply(native_left.data(), nullptr, native_out->data(), nr->data());
        bsp::multiply_camera_matrices_00413920(*rebuilt_out, rebuilt_left, *rr);
        multiply_equal = multiply_equal && std::memcmp(native_out->data(), rebuilt_out->data(), sizeof(native)) == 0;
    }
    std::printf("Camera native comparison: inverse=%d projection=%d multiply_and_exact_aliases=%d\n",
        inverse_equal, projection_equal, multiply_equal);
    VirtualFree(code, 0, MEM_RELEASE);
    return inverse_equal && projection_equal && multiply_equal;
}
