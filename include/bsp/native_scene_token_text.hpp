#pragma once
#include "bsp/native_scene_token_values.hpp"

namespace bsp {
// 008F0800, stack tokenizer, RET4. Balanced group skip with '=' / ';' value
// skipping. Valid input must terminate the group; original has no outer EOF exit.
void skip_native_scene_group_008f0800(void*, NativeSceneTokenValueContext&);
// Complete ordinary wrappers over borrowed, already-live 838h tokenizer storage.
// Contexts and C++ calling conventions are new Source interfaces, not Original
// register/stack ABIs. The accepted providers retain their current-CRT boundary.
// Text must fit the Original 400h token buffers; no bounds repair is introduced.

// 008D9930: ECX owner, entry stack+4 expected text, AL bool, RET4. Peek before
// comparing the borrowed expected text. Match copies +5 to +405, NUL included,
// then clears +805; mismatch performs 008D8F70 recovery and returns false.
bool expect_native_scene_token_008d9930(void* tokenizer,const char* expected,
    NativeSceneTokenValueContext&);

// 008D9980: ECX owner, EAX owner+5, RET0. Scan the whole current token to NUL;
// accept nonempty text or an empty quoted token only while +80A is zero.
// Success consumes; failure recovers then peeks. The returned pointer always
// borrows the current-token buffer, whose contents can change on later reads.
char* read_native_scene_token_008d9980(void* tokenizer,
    NativeSceneTokenValueContext&);
} // namespace bsp
