#pragma once

// The "<Table>=nil\n" class-table resets. BSP_Game_LoadMissionScene (004DFB70) calls
// eleven of them back to back right after luaPrecacheUnits (004E0A9B..004E0ACD), and
// BSP_EngineMovie_Stop (0045F600) calls the same set plus the bare RET 004708A0
// (0045F6C1..0045F6F8). Each body is the same five instructions, for example 004B1890:
//
//   MOV EAX,[00E188A8]; MOV ECX,[EAX+1A08h]; MOV ECX,[ECX+4]
//   PUSH 2; PUSH 0; PUSH 0; PUSH <text>; CALL 006B8AD0; RET
//
// that is, BSP_LuaMachine_RunString(text, sink = 0, error = 0, mode = 2) on the mission
// Lua machine at [[00E188A8]+1A08h]+4, with no null test of the machine. 006B8AD0 is an
// inline strlen (006B8AE0..006B8AE7) and 006B89F0(text, length, text, sink, error, mode)
// (006B8AE9..006B8AFD), so the text is also the chunk name.
//
// MissionLuaRunString is that one call. The bodies that take it (004B1890, 004AF930,
// 004A7120, 00474200, 00476410, 006D74B0, 00869D20) used to take their segment's whole
// host interface, which no host implements; they only ever needed this method. The
// machine itself is the implementer's: a host binds it to its mission Lua state.
// Hypothesis names; the evidence is the listing above.

namespace bsp {

struct MissionLuaRunString {
    virtual ~MissionLuaRunString() = default;
    // 006B8AD0(text, 0, 0, mode) on [[00E188A8]+1A08h]+4. Every reset passes null for
    // both sinks, so the interface has no parameters for them.
    virtual void run_string_006b8ad0(const char* text, int mode) = 0;
};

// The mode every reset pushes (PUSH 2).
inline constexpr int kClassTableResetRunMode = 2;

}  // namespace bsp
