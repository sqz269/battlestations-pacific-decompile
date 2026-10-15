#pragma once

#include "bsp/native_scene_tokenizer.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene numeric lookahead requires MSVC Win32.
#endif

namespace bsp {

// Two complete ordinary bodies through new C++ interfaces. Names are
// descriptive hypotheses, not recovered symbols or Original binary ABIs.
// Borrow genuine already-live838h tokenizer storage and its SAME retained
// accepted context, stream/buffer, canonical raw-string and delimiter owners.
// Original ECX is the tokenizer; no stack arguments; full EAX is0 or1; RET0.
//
// Original PUSH ECX creates the ignored four-byte scan destination BEFORE
// raw008D8A70. Preserve those initial bits without evaluating them as float.
// Peek may scan/cache the next token; neither wrapper calls consume/advance.
// Scan the returned CURRENT token with fixed exact immutable format text:
// CE3A34 "%d" or CE4334 "%f". These Source literals preserve the format
// roles/text; they do not claim the Original numeric addresses or globals.
// Return scan-assignment-count ==1, including prefix conversions; the temporary
// value is never read. No whole-token check, trailing-text rule or recovery.
//
// std::sscanf is the explicit linked HOST CRT boundary for OriginalBF7533.
// Admit stable valid NUL-terminated ASCII tokens fitting the Original400h
// buffers, C locale, ordinary decimal prefixes whose converted values fit int
// or finite binary32 respectively, and normal successful provider execution.
// A token with no numeric assignment returns false. No alias, overflow, trim,
// concurrent mutation/reentry, fault, provider throw or exceptional unwind.
// Keep every existing raw-tokenizer lifetime/provider precondition. No model
// host, caller-selectable format or new callback/process startup is supplied.
// Historical scanf/locale/FP/error-state parity, arbitrary exceptional values,
// Original register/class/FS/EH/private CRT, startup and game remain unproven.

// Whole [008D8F10,008D8F33),35B/13 instructions, two direct calls.
bool next_native_scene_token_is_int_008d8f10(void* actual_live_tokenizer,
    NativeSceneTokenizerContext& context);

// Whole [008D8F40,008D8F63),35B/13 instructions, two direct calls.
// This predicate has no x87 instruction or floating return. It does not
// implement the distinct ST0-valued float reader008D9B40.
bool next_native_scene_token_is_float_008d8f40(void* actual_live_tokenizer,
    NativeSceneTokenizerContext& context);

} // namespace bsp
