// Execute only the five verified, call-free math routines in this test process.
// No game process, assets, DLL loading, global addresses, or startup code involved.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "bsp/math.hpp"
#include "seed_reference.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

class Reference {
public:
    void* code;
    template<std::size_t N> explicit Reference(const unsigned char (&bytes)[N]) {
        code = VirtualAlloc(nullptr, N, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!code) throw std::runtime_error("VirtualAlloc failed");
        std::memcpy(code, bytes, N);
        DWORD old = 0;
        if (!VirtualProtect(code, N, PAGE_EXECUTE_READ, &old) ||
            !FlushInstructionCache(GetCurrentProcess(), code, N)) {
            VirtualFree(code, 0, MEM_RELEASE);
            throw std::runtime_error("Could not prepare native reference");
        }
    }
    ~Reference() { VirtualFree(code, 0, MEM_RELEASE); }
    Reference(const Reference&) = delete;
    Reference& operator=(const Reference&) = delete;
};

void binary_op(void* code, bsp::Vec3d* destination, const bsp::Vec3d* a, const bsp::Vec3d* b) {
    __asm {
        mov eax, destination
        mov ecx, a
        mov edx, b
        call code
    }
}
void scale(void* code, bsp::Vec3d* destination, double factor) {
    __asm {
        push dword ptr factor[4]
        push dword ptr factor[0]
        mov eax, destination
        call code
    }
}
double length(void* code, const bsp::Vec3d* value) {
    double result;
    __asm {
        mov eax, value
        call code
        fstp result
    }
    return result;
}
bool same(const bsp::Vec3d& a, const bsp::Vec3d& b) {
    return std::memcmp(&a, &b, sizeof(a)) == 0;
}

int main() {
    using namespace bsp;
    Reference cross(ref_00401c20), subtract(ref_00401cb0), scaling(ref_00401cd0),
              squared(ref_00401cf0), absolute(ref_00401170);
    auto native_abs = reinterpret_cast<float (__stdcall*)(float)>(absolute.code);
    int comparisons = 0;
    int failures = 0;
    auto check = [&](bool ok) { ++comparisons; if (!ok) ++failures; };
    // Integer-derived fixtures are exactly representable in x87 and SSE.
    // This intentionally does not establish general FP / ABI equivalence.
    for (int i = -32; i <= 32; ++i) {
        const double n = i;
        const Vec3d a{n, n + 2, n - 3}, b{7 - n, n + 5, 2 * n};
        Vec3d expected{}, actual{};
        binary_op(cross.code, &expected, &a, &b);
        cross_reversed_00401c20(actual, a, b);
        check(same(expected, actual));
        expected = actual = a;
        binary_op(cross.code, &expected, &expected, &b);
        cross_reversed_00401c20(actual, actual, b);
        check(same(expected, actual));
        expected = actual = b;
        binary_op(cross.code, &expected, &a, &expected);
        cross_reversed_00401c20(actual, a, actual);
        check(same(expected, actual));
        binary_op(subtract.code, &expected, &a, &b);
        subtract_reversed_00401cb0(actual, a, b);
        check(same(expected, actual));
        expected = actual = a;
        scale(scaling.code, &expected, -2.0);
        scale_00401cd0(actual, -2.0);
        check(same(expected, actual));
        check(length(squared.code, &a) == length_squared_00401cf0(a));
        const float f = static_cast<float>(i) * 0.5f;
        const float reference = native_abs(f), rebuilt = abs_00401170(f);
        check(std::memcmp(&reference, &rebuilt, sizeof(float)) == 0);
    }
    std::cout << comparisons << " native-reference comparisons; " << failures << " failures\n";
    return failures ? 1 : 0;
}
