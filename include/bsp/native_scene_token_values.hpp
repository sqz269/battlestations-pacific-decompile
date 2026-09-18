#pragma once
#include "bsp/native_scene_tokenizer.hpp"
#include <cstdint>

namespace bsp {
// The game entries call BF7533 with the borrowed CE3A34 "%d" format and an
// actual initialized local DWORD. The default is the linked host CRT's sscanf;
// this is an explicit CRT boundary, not a reconstruction of VS2005 _input_l.
// Original-CRT overflow, locale and error-state equivalence remain unproven.
struct NativeSceneTokenValueCalls {
    virtual ~NativeSceneTokenValueCalls()=default;
    virtual int scan_decimal_00bf7533(const char* text,const char* format,
        std::int32_t* actual_result);
};
struct NativeSceneTokenValueContext {
    NativeSceneTokenizerContext& tokenizer;
    NativeSceneTokenValueCalls& calls;
    const char* decimal_format_00ce3a34;
    const char* recovery_stop_00d15f34; // Original immutable empty string.
};

// Full game-body schedules over the actual 838h storage. These source entries
// add contexts; they are not original binary ABIs. The byte output is allowed
// to alias owner fields: its stores occur at the native points in the schedule.
// 8D8F70: ECX owner, RET. Skip only following whitespace until a stop byte or
// physical EOF; retain cached token; copy current +809 into token EOF +80A.
void recover_native_scene_whitespace_008d8f70(void*,NativeSceneTokenValueContext&);
// 8D8E50: ECX owner, stack byte* ok, AL result, RET4. Non-consuming conversion;
// preserve ((scan==1 && value==0) || value==1), including failed-scan local bits.
bool peek_native_scene_bool_008d8e50(void*,std::uint8_t*,NativeSceneTokenValueContext&);
// 8D99F0: ECX owner, stack byte* ok, EAX token pointer, RET4. Reject empty
// quoted strings too. Failure recovers, stores false, then peeks again.
char* read_native_scene_nonempty_string_008d99f0(void*,std::uint8_t*,NativeSceneTokenValueContext&);
// 8D9A80: same args, AL result, RET4. Consume only if CURRENT *ok is nonzero;
// otherwise recover, store false again and return false.
bool read_native_scene_bool_008d9a80(void*,std::uint8_t*,NativeSceneTokenValueContext&);
// 8D9AD0: same args, EAX signed result, RET4. Write conversion success BEFORE
// copying/consuming the token. Failure recovers, stores false again, returns0.
std::int32_t read_native_scene_int_008d9ad0(void*,std::uint8_t*,NativeSceneTokenValueContext&);
} // namespace bsp
