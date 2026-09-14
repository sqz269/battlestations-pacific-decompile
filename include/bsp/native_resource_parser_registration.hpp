#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct SingletonLifetimeCallbacks;

// Native parser slot4 returns an owned name through a hidden output header.
// The first registration lookup uses its RETURNED pointer; the second call's
// return is ignored and its actual by-value output is consumed. Implementations
// dispatch the captured target without re-reading the parser's table.
class NativeResourceParserNameCalls {
public:
    virtual ~NativeResourceParserNameCalls() = default;
    virtual void* type_name(std::uintptr_t captured_target, void* actual_parser,
        void* actual_output_header) = 0;
};

// Complete367B B80A50. Native ECX raw manager, stack parser, AL0/1, RET4.
// Capture current head AFTER the first getter, BEFORE lookup. Clean the first
// local name without an armed outer EH state; duplicate returns0. Otherwise
// reload current parser/table/slot4 and get the name AGAIN, construct the
// borrowed-parser pair, insert it, clean both pairs and return1 even if that
// second name collides. No parser AddRef/release or rollback is introduced.
std::uint8_t register_native_resource_type_parser_00b80a50(void* actual_manager,
    void* actual_parser, NativeStringRawPoolContext&, const SingletonLifetimeCallbacks&,
    NativeResourceParserNameCalls&);

// Raw parser-map helpers. Their complete instructions and EH metadata match
// the integrated cache specialization after the explicitly audited address
// substitutions. Reuse those actual-storage algorithms; no std::map adapter,
// projected tree, generic parser lifetime or callable numeric table is added.
// Tree+4=head,+8=count. Node1Ch links0/4/8, owned keyC/10, borrowed parser14,
// color18,nil19. Iterator/result writes and current reads remain unchanged.

// B7CA80[82]/B7D590[78]: ECX tree, stacked node, EAX pivot, RET4.
void* rotate_native_resource_parser_right_00b7ca80(void* tree, void* node) noexcept;
void* rotate_native_resource_parser_left_00b7d590(void* tree, void* node) noexcept;
// B7F1B0[112]: ECX node, stacked left,parent,right,pair,color; EAX node, RET14h.
void* construct_native_resource_parser_node_00b7f1b0(void* node, void* left,
    void* parent, void* right, const void* pair, std::uint32_t color,
    NativeStringRawPoolContext&);
// B7F610[114], catch B7F682[21]: incoming ECX unused, same five stack words,
// EAX captured allocation, RET14h. Free captured allocation on construction
// failure, then rethrow; placement-delete action is RET-only401130.
void* allocate_native_resource_parser_node_00b7f610(void* left, void* parent,
    void* right, const void* pair, std::uint32_t color, NativeStringRawPoolContext&);
// B7F340[161]: ECX pair, EDX parser, by-value name2DWORDs, EAX pair, RET8.
// The added input-header pointer exposes its native ownership and alias rules.
void* construct_native_resource_parser_pair_00b7f340(void* pair, void* parser,
    void* owned_input_name, NativeStringRawPoolContext&);
// B7FD30[492]: ECX tree, stack output/side/parent/pair, EAX output, RET10h.
// Bound15555554, actual node allocation/link/rebalance; no rollback.
void* insert_native_resource_parser_at_00b7fd30(void* tree, void* output,
    std::uint32_t side, void* parent, const void* pair, NativeStringRawPoolContext&);
// B80290[276]: ECX tree, stack output/pair, EAX output, RET8. Equivalent key
// preserves existing node/parser; output=node4,inserted BYTE8,owner0.
void* insert_native_resource_parser_unique_00b80290(void* tree, void* output,
    const void* pair, NativeStringRawPoolContext&, const SingletonLifetimeCallbacks&);
// B7E930/B7E950[29 each]: ECX pair, RET. Return current name storage through
// current raw pool; keep stale header and borrowed parser at+8.
void destroy_native_resource_parser_pair_00b7e930(void*, NativeStringRawPoolContext&);
void destroy_native_resource_parser_insert_pair_00b7e950(void*, NativeStringRawPoolContext&);

// New explicit-service C++ ABI. Native FH3/SEH identity/private stack aliases,
// hardware faults, concrete parser name/body dispatch, actual manager/bootstrap
// and gameplay remain separate. Source C++ cleanup follows the recovered maps;
// failure during an unwind cleanup terminates.
} // namespace bsp
