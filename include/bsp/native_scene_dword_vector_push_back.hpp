#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw DWORD-vector push-back requires MSVC Win32.
#endif

namespace bsp {
struct NativeSceneDwordVectorInsertOneFrame;

// Stable metadata is disjoint from actual cells/payload. actual_parameter4
// binds the LIVE stacked value-pointer DWORD at native entry P+4. Its contents
// are read late on either branch, after a reached fallback diagnostic returns.
// Child metadata and private output8 are needed only on a reached fallback.
// Actual described cells/payload may alias where every reached access/release
// remains valid; Source/native argument homes have no implicit coincidence.
struct NativeSceneDwordVectorPushBackFrame {
    const void* actual_parameter4;
    void* actual_private_output8;
    const NativeSceneDwordVectorInsertOneFrame* child_frame;
};

// Whole0046C450 [105B/44ops]. Native ECX vector16, one live stacked value-
// pointer, RET4; no common semantic EAX result. Capture begin once, retain full
// unsigned SAR2 size/capacity decision, and on fast path reread current end
// BEFORE late parameter/source loads, store there, then publish captured end+4.
// Fallback captures current end before a returning begin>end diagnostic, then
// reads CURRENT value pointer and stores One child arguments in native push
// order: value, captured end, captured tree, private output. Ignore child EAX.
void push_back_native_scene_dword_vector_0046c450(void* actual_vector16,
    const NativeSceneDwordVectorPushBackFrame&);

// Child is the real full One/N provider. One owns its reached nested N backing.
// Fast path reads neither child metadata nor private output. No nominal element
// ownership, capacity clamp, fallback-only projection, callback or stub is added.
// New raw Source interface; current CRT/heap policy, native private-frame/fault/
// register/stack/FH3/SEH/binary ABI, application and game proof remain separate.
} // namespace bsp
