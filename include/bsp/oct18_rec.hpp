#pragma once

#include "bsp/cc13_dyn2.hpp"
#include <cstdint>

// Missing live Dyn/Mit entries from the oct18 reconciliation packet.
// Evidence, original ABIs and limitations: docs/OCT18_REC_RECONCILIATION.md.
// Names are hypotheses. Explicit C++ interfaces are not binary replacements.
namespace bsp::oct18_rec {
using Free = void (__cdecl*)(void*);
using Pool = cc13_dyn2::MitPoolAllocator36x128;

// Original ECX receiver, one/four-byte stack word; RET 4. EDX is ignored.
void __fastcall radix_noop_00403c40(void*, void*, std::uint32_t);
// Original ECX receiver, three stack words; RET 0Ch. EDX is ignored.
void __fastcall terrain_noop_00404540(void*, void*, void*, const void*, const void*);
void __fastcall terrain_noop_00404550(void*, void*, void*, const void*, const void*);
// Original ECX receiver; plain RET.
void __fastcall triangle_noop_00405aa0(void*);

// Original ECX shape, stack output/direction/transform, RET 0Ch. The dummy EDX
// parameter adapts thiscall to MSVC fastcall without moving original stack args.
// Shape vertices occupy 210h..230h; transform is twelve float words.
void __fastcall triangle_support_float_00405bf0(
    const void*, void*, float*, const float*, const float*);
void __fastcall triangle_support_double_00405d80(
    const void*, void*, double*, const double*, const float*);

// Original ECX pool; RET; no lock acquired. Host supplies the original free
// binding. The two pool layouts share the same 38h-byte header.
void pool36_reclaim_empty_pages_00407750(Pool&, Free);
void pool20_reclaim_empty_pages_00407bb0(Pool&, Free);

// Original EAX destination/ECX value, RET; no return value.
void store_word_00408300(std::uint32_t*, std::uint32_t) noexcept;
// Original EAX source, RET; EAX value result.
std::uint32_t load_word_00409d00(const std::uint32_t*) noexcept;

// Original EAX vector (00408E00) / ECX vector (0040AB90), RET. Only word 0 is
// read and conditionally freed; the dangling header is deliberately retained.
void vector_free_00408e00(void* const*, Free);
// Original ECX three-word header, RET. EAX is left at the original receiver.
void* vector_construct_empty_0040ab80(std::uint32_t*) noexcept;
void vector_free_0040ab90(void* const*, Free);

// Complete raw-storage drivers for three formerly partial semantic rows. Calls
// are explicit provider contracts, not stubs or inferred original C++ classes.
using RunLuaString = void (*)(void* machine, const char* text,
                             std::uint32_t, std::uint32_t, std::uint32_t);
void set_xbox_mode_008d44c0(void* settings, std::uint8_t enabled,
    volatile std::uint8_t& mirror_0108ff20, void* volatile& game_00e188a8,
    RunLuaString, const char* command_true, const char* command_false);

struct MissionIterator { void* owner; void* node; };
static_assert(sizeof(MissionIterator) == 8);
using FindMission = void (*)(void* table, MissionIterator* output, const void* key);
using InvalidParameter = void (*)(); // May return, just as the original CRT hook.
bool mission_completed_0090c560(void* table, const void* key,
    FindMission, InvalidParameter);

struct QuotedRunCalls {
    void (*reset_009502e0)(void* writer);
    const char* (*peek_008d8a70)(void* tokenizer);
    const char* (*read_008d9980)(void* tokenizer);
    const char* (*decode_008ee670)(const char* token);
    void (*append_0094ff10)(void* writer, const char* decoded);
    char* (*join_009500a0)(void* writer);
    void (*clear_00950320)(void* writer);
    void (*destroy_reader_008ef030)(void* writer);
    void (*destroy_base_0045c4c0)(void* writer);
};
// Original stdcall tokenizer stack argument, EAX owning result, RET 4. The
// original two-state unwind table is mirrored with host C++ exception handling.
// A thin wrapper: it lays out the 20h-byte stack writer and runs the typed
// overload below over that writer and these calls.
char* read_quoted_runs_008efb00(void* tokenizer, const QuotedRunCalls&);

// The typed view 008EFB00 runs over: its tokenizer (008D8A70 peek, then the
// +4h quoted byte; 008D9980 read), the 008EE670 run decoder and the string
// builder on its stack (009502E0 reset, 0094FF10 append, 009500A0 join, the
// 00D162CCh vtable store with 00950320 clear, and the two unwind destructors
// 008EF030 / 0045C4C0). A host implements it over its own lexer and string.
class QuotedRunView {
public:
    virtual ~QuotedRunView() = default;
    virtual void reset() = 0;                           // 009502E0
    virtual bool peek_quoted() = 0;                     // 008D8A70, [tokenizer+4]
    virtual const char* read_token() = 0;               // 008D9980
    virtual const char* decode(const char* token) = 0;  // 008EE670
    virtual void append(const char* decoded) = 0;       // 0094FF10
    virtual char* join() = 0;                           // 009500A0, EAX
    virtual void clear() = 0;                           // [writer] = 00D162CCh, 00950320
    virtual void destroy_reader() noexcept = 0;         // state 0 funclet 00CA4700
    virtual void destroy_base() noexcept = 0;           // state 1 funclet 00CA4708
};
// 008EFB00's body over the typed view; returns join()'s result.
char* read_quoted_runs_008efb00(QuotedRunView& view);
} // namespace bsp::oct18_rec
