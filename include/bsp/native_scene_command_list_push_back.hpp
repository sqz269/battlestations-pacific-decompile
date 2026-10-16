#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene command list reconstruction requires MSVC Win32.
#endif

namespace bsp {

// Whole 00468350 body: [00468350,004683A7), 87 bytes / 36 instructions.
// Original ABI: ECX is the actual 12-byte list; one stacked DWORD is read
// only after allocation and conditional node initialization; RET 4. There
// is no semantic EAX result. List words are count/head/tail at 0/4/8; the
// allocated 12-byte node words are previous/next/payload at 0/4/8. Role
// names are descriptive and do not establish a nominal payload type.
//
// actual_parameter4 exposes the original entry-ESP+4 cell, rather than an
// eagerly captured payload value. Source argument metadata stays stable
// and disjoint from reached raw storage. The reached DWORD storage must
// support these ordered raw accesses; no blanket nonalias rule is imposed.
// The current CRT allocator service supplies 12 bytes, nonnull or throwing.
// The native null-result branch is retained, including its address-8 store.
//
// Ordinary Source interface only: physical stack/register/fault/private ABI,
// Original CRT heap/handler/exception identity and game behavior are unproved.
void push_back_native_scene_command_list_00468350(
    void* actual_list12, const void* actual_parameter4);

} // namespace bsp
