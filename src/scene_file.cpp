// The .scn scene-file reader. See docs/SCENE_FILE_READER.md.
// Addresses: 0046df00, 0046cf40, 00469bf0, 00467e10, 00469e40, 0046a9f0,
//            0046aab0, 008d9cf0, 008d8a70, 008d8960, 008d9930, 008d9980,
//            008d9ad0, 008d9b40.
#include "bsp/scene_file.hpp"
#include "bsp/plane_pose_commit.hpp"
#include "bsp/oct18_rec.hpp"
#include "bsp/oct18_tail_b.hpp"
#include "bsp/support_seg61_cc13.hpp"
#include "bsp/wire_goal2_chain115_switches.hpp"
#include "bsp/wire_goal2_chain145_switches.hpp"
#include "bsp/wire_goal2_chain202_switches.hpp"
#include "bsp/wire_goal4_chain258_switches.hpp"
#include "bsp/wire_goal4_chain319_switches.hpp"
#include "bsp/wire_goal4_chain284_switches.hpp"
#include "bsp/wire_goal4_chain307_switches.hpp"
#include "bsp/wire_goal4_chain311_switches.hpp"
#include "bsp/wire_goal4_chain321_switches.hpp"
#include "bsp/wire_goal4_chain330_switches.hpp"
#include "bsp/wire_goal4_chain361_switches.hpp"
#include "bsp/wire_goal4_chain425_switches.hpp"
#include "bsp/band_0080_cc13.hpp"
#include "bsp/cc13_sweepA.hpp"
#include "bsp/entity_identity.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_scene_property_record_type2_string_storage.hpp"
#include "bsp/native_scene_typed_property_providers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cmath>

#include <algorithm>
#include <array>
#include <cctype>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp::oct18_tail_b {

// 008EE670: ECX=nullable text, RET, EAX=00F88C50 scratch. The source API
// supplies that actual scratch explicitly; forward aliasing/read/write order
// is retained. Preconditions: terminated input, no lone trailing backslash,
// and sufficient scratch storage, matching the native valid domain.
// Chain 258 moves the single existing body from leftovers_cc13.cpp into
// bsp_core, where the actual scene parser and its existing tests can use it.
// Evidence: bsp.py show 008ee670, complete exported body; no arithmetic/ABI
// change. SceneQuotedRunView supplies guarded tokens and reader-owned scratch
// instead of adopting the native global buffer's aliasing/reentry contract.
char* decode_scene_fragment_008ee670(const char* input, char* scratch) {
    char* output = scratch;
    if (input != nullptr) {
        while (*input != '\0') {
            if (*input == '\\') {
                const char escaped = input[1];
                input += 2;
                if (escaped == '\\') *output++ = '\\';
                else if (escaped == 'n') *output++ = '\n';
                else if (escaped == 'q') *output++ = '"';
            } else *output++ = *input++;
        }
    }
    *output = '\0';
    return scratch;
}

} // namespace bsp::oct18_tail_b

namespace bsp {
namespace {

bool equal_insensitive(const std::string& a, const char* b) noexcept
{
    // 00438e10, the case-insensitive compare every keyword test goes through.
    std::size_t i = 0;
    for (; i < a.size(); ++i) {
        const unsigned char lhs = static_cast<unsigned char>(a[i]);
        const unsigned char rhs = static_cast<unsigned char>(b[i]);
        if (rhs == 0) {
            return false;
        }
        if (std::tolower(lhs) != std::tolower(rhs)) {
            return false;
        }
    }
    return b[i] == 0;
}

std::string format_error(int line, const char* what, const std::string& got)
{
    char buffer[192];
    std::snprintf(buffer, sizeof buffer, "line %d: expected %s, got \"%.80s\"", line, what,
        got.c_str());
    return std::string(buffer);
}

// Cap matching the native's own give-up point only in spirit: the native keeps
// reporting, but a reconstruction that never terminates is worse than one that
// stops. Documented in docs/SCENE_FILE_READER.md as a deliberate divergence.
constexpr std::size_t kMaxRecoveredErrors = 256;

class Cursor {
public:
    Cursor(SceneLexer& lexer, std::vector<std::string>& errors) : lexer_(lexer), errors_(errors) {}

    SceneLexer& lexer() noexcept { return lexer_; }

    bool at(const char* text)
    {
        const SceneToken& t = lexer_.peek();
        return !t.is_end() && equal_insensitive(t.text, text);
    }

    bool at_end() { return lexer_.peek().is_end(); }

    // 008d9930: consume only on a match; otherwise record and leave in place.
    bool expect(const char* text)
    {
        const SceneToken& t = lexer_.peek();
        if (equal_insensitive(t.text, text)) {
            lexer_.next();
            return true;
        }
        record(format_error(t.line, text, t.text));
        return false;
    }

    // 008d9980: any non-empty token; an empty unquoted token is end of input.
    std::string read_token()
    {
        const SceneToken& t = lexer_.peek();
        if (t.is_end()) {
            record(format_error(t.line, "a token", t.text));
            return std::string();
        }
        return lexer_.next().text;
    }

    std::int32_t read_int()
    {
        const SceneToken& t = lexer_.peek();
        std::int32_t value = 0;
        if (!scene_scan_int(t.text, value)) {
            record(format_error(t.line, "an integer", t.text));
            return 0;
        }
        lexer_.next();
        return value;
    }

    float read_float()
    {
        bool ok = false;
        return read_float(ok);
    }

    // 008D9B40 BSP_SceneTokenizer_ReadFloat, __thiscall(ECX = tokenizer, bool* ok), RET 4,
    // ST0 = value. Listing read whole (39 instructions, cc13-P4): 008D9B43..008D9B64
    // sscanf(peek 008D8A70, "%f" 00CE4334, &value) == 1; 008D9B69 the result byte to
    // *ok; on success 008D9B6D..008D9B89 copy the current token (+5h) to the previous-
    // token buffer (+405h) and clear +805h (the consume), FLD value; else 008D9B96
    // 008D8F70 (the "expected a float" report), FLDZ, *ok = 0. scene_scan_float stands
    // in for the CRT sscanf "%f" (scene_file's existing projection).
    float read_float(bool& ok)
    {
        const SceneToken& t = lexer_.peek();
        float value = 0.0f;
        ok = scene_scan_float(t.text, value);
        if (!ok) {
            record(format_error(t.line, "a float", t.text));
            return 0.0f;
        }
        lexer_.next();
        return value;
    }

    bool exhausted() const noexcept { return errors_.size() >= kMaxRecoveredErrors; }

private:
    void record(std::string message)
    {
        if (errors_.size() < kMaxRecoveredErrors) {
            errors_.push_back(std::move(message));
        }
    }

    SceneLexer& lexer_;
    std::vector<std::string>& errors_;
};

// kWireGoal2Chain115Bound: the view 008EFB00 (read_quoted_runs) runs over in
// this reader. The tokenizer is this lexer: peek().quoted is the +4h byte
// 008D8A70 leaves, next() is 008D9980's read. The builder is one std::string
// (009502E0 empties it, 0094FF10 appends, 009500A0 hands back the joined
// text); it owns no native storage, so clear and the unwind destructors have
// nothing to release. The decode collaborator keeps this reader's own S rule
// and source-binding guards (the inline loop's). Chain 258 uses the single
// reconstructed 008EE670 body, now in bsp_core, over the guarded fragment.
class SceneQuotedRunView final : public oct18_rec::QuotedRunView {
public:
    explicit SceneQuotedRunView(SceneLexer& lexer) noexcept : lexer_(lexer) {}

    void reset() override { value_.clear(); }
    bool peek_quoted() override { return lexer_.peek().quoted; }
    const char* read_token() override
    {
        fragment_ = lexer_.next();
        // The native cached/read buffers are 400h (+5h/+405h, cache +805h).
        // Overflow, embedded NUL and a trailing single escape are outside
        // this source binding, as in the inline loop.
        const std::string& text = fragment_.text;
        if (text.size() >= 0x400 || text.find('\0') != std::string::npos) {
            throw std::runtime_error("unsupported quoted scene string fragment");
        }
        for (std::size_t i = 0; i < text.size();) {
            if (text[i++] != '\\') {
                continue;
            }
            if (i == text.size()) {
                throw std::runtime_error("unsupported trailing scene string escape");
            }
            ++i;
        }
        return text.c_str();
    }
    const char* decode(const char* token) override
    {
        if constexpr (kWireGoal4Chain258Bound) {
            // 008EFB00's 008EE670 call: token is exactly read_token()'s real
            // fragment. Its <400h, NUL-free, paired-escape guards above imply
            // at most 3FFh output bytes plus NUL; escapes never expand text.
            return oct18_tail_b::decode_scene_fragment_008ee670(token, scratch_.data());
        } else {
        decoded_.clear();
        for (const char* p = token; *p != '\0';) {
            const char ch = *p++;
            if (ch != '\\') {
                decoded_.push_back(ch);
                continue;
            }
            const char escape = *p++;   // read_token admitted only paired escapes
            if (escape == '\\') decoded_.push_back('\\');
            else if (escape == 'n') decoded_.push_back('\n');
            else if (escape == 'q') decoded_.push_back('"');
            // All other backslash pairs emit nothing in 008EE670.
        }
        return decoded_.c_str();
        }
    }
    void append(const char* decoded) override { value_ += decoded; }
    char* join() override { return value_.data(); }
    void clear() override {}
    void destroy_reader() noexcept override {}
    void destroy_base() noexcept override {}

private:
    SceneLexer& lexer_;
    SceneToken fragment_;
    std::array<char, 0x400> scratch_{};
    std::string decoded_;
    std::string value_;
};

// kWireGoal2Chain145Bound: Seg61Host for the text writer 008EFB00 builds on
// its stack, over the C++ heap. 00BF681B (the 80Ch chunk) and 00BF55BE (the
// joined text, which 008F5D0A/008F5D24 release with _free 00BF6989) are
// malloc, failing as operator new does; 00BF65AC is free. The writer's slot 0
// is 00950240 (disk dword [00D162CC] = 00950240h); a chunk's slot 0 is
// 00950380 ([00D19B74]), whose body is 00950350's stores then operator delete.
// With the indent depth 0 that 008EFB00 stores, 0094FDC0 writes nothing, so
// no other host call is reachable from 009502E0, 0094FF10, 009500A0 and
// 00950320; the rest throw.
class SceneTextChunkWriterHost final : public support_seg61_cc13::Seg61Host {
public:
    std::uint32_t& global_u32(std::uint32_t) override { unreachable("global_u32"); }
    void* image_address(std::uint32_t) override { unreachable("image_address"); }
    void* operator_new_00bf55be(std::uint32_t bytes) override { return allocate(bytes); }
    void* operator_new_00bf681b(std::uint32_t bytes) override { return allocate(bytes); }
    void operator_delete_00bf65ac(void* block) override { std::free(block); }
    void free_00bf6989(void* block) override { std::free(block); }
    void* string_pool_00419cc0() override { unreachable("string_pool_00419cc0"); }
    void string_pool_return_00bd1510(void*, void*, std::uint32_t, std::int32_t) override
    {
        unreachable("string_pool_return_00bd1510");
    }
    void* native_string_copy_construct_00426060(void*, const void*) override
    {
        unreachable("native_string_copy_construct_00426060");
    }
    void writer_write_slot0(support_seg61_cc13::Raw writer, const void* data,
        std::uint32_t length) override
    {
        support_seg61_cc13::scene_text_chunk_list_write_00950240(writer, data, length, *this);
    }
    void object_delete_slot0(void* object) override
    {
        support_seg61_cc13::scene_text_chunk_destruct_00950350(
            support_seg61_cc13::Raw{static_cast<std::uint8_t*>(object)}, *this);
        std::free(object);
    }
    void itoa_00bf9010(std::int32_t, char*, std::int32_t) override { unreachable("itoa_00bf9010"); }
    const char* format_static_00439ac0(const char*, std::uint32_t) override
    {
        unreachable("format_static_00439ac0");
    }
    void format_string256_004b8490(char*, const char*, double) override
    {
        unreachable("format_string256_004b8490");
    }
    std::uint32_t create_file_a_ce2260(const char*, std::uint32_t, std::uint32_t, void*,
        std::uint32_t, std::uint32_t, void*) override
    {
        unreachable("create_file_a_ce2260");
    }
    std::int32_t write_file_ce2290(std::uint32_t, const void*, std::uint32_t, std::uint32_t*,
        void*) override
    {
        unreachable("write_file_ce2290");
    }
    void entity_pose_refresh_00414db0(support_seg61_cc13::Raw) override
    {
        unreachable("entity_pose_refresh_00414db0");
    }
    void dyn_world_for_each_body_transform_006fa6d0(void*, const void*) override
    {
        unreachable("dyn_world_for_each_body_transform_006fa6d0");
    }
    void show_last_system_error_box_004531f0(const char*) override
    {
        unreachable("show_last_system_error_box_004531f0");
    }

private:
    static void* allocate(std::uint32_t bytes)
    {
        void* const block = std::malloc(bytes);
        if (block == nullptr) {
            throw std::bad_alloc();
        }
        return block;
    }
    [[noreturn]] static void unreachable(const char* call)
    {
        throw std::logic_error(std::string("scene text chunk writer: unmodelled ") + call);
    }
};

// kWireGoal2Chain145Bound: the view 008EFB00 runs over with the image's own
// builder. The tokenizer and decode collaborators are SceneQuotedRunView's
// (this lexer and the inline loop's guards); the builder is the 20h-byte
// chunk-list writer 008EFB00 constructs on its stack, driven by segment 61's
// bodies: 009502E0 reset, 0094FF10 write, 009500A0 join, 00950320 clear. The
// unwind 008EF030 (state 0, funclet 00CA4700) is the vtable store and
// 00950320; state 1 (00CA4708) only restores the base vtable. The joined
// buffer is released when the view goes, after the caller has copied it, as
// 008F5CF6's caller frees it with _free once stored.
class SceneQuotedRunChunkView final : public oct18_rec::QuotedRunView {
public:
    explicit SceneQuotedRunChunkView(SceneLexer& lexer) noexcept : runs_(lexer)
    {
        // 008EFB00's writer stores, in the order oct18_rec's raw entry keeps:
        // depth 0, width 2, fill ' ', line not yet indented, vtable 00D162CC,
        // tail, head and total length 0.
        writer().at<std::uint32_t>(0x8) = 0;
        writer().at<std::uint32_t>(0xc) = 2;
        writer().at<std::uint8_t>(0x10) = 0x20;
        writer().at<std::uint8_t>(0x4) = 0;
        writer().at<std::uint32_t>(0x0) = kReaderVtable;
        writer().at<std::uint32_t>(0x18) = 0;
        writer().at<std::uint32_t>(0x14) = 0;
        writer().at<std::uint32_t>(0x1c) = 0;
    }
    ~SceneQuotedRunChunkView() override { std::free(joined_); }
    SceneQuotedRunChunkView(const SceneQuotedRunChunkView&) = delete;
    SceneQuotedRunChunkView& operator=(const SceneQuotedRunChunkView&) = delete;

    void reset() override
    {
        support_seg61_cc13::scene_text_chunk_list_reset_009502e0(writer(), host_);
    }
    bool peek_quoted() override { return runs_.peek_quoted(); }
    const char* read_token() override { return runs_.read_token(); }
    const char* decode(const char* token) override { return runs_.decode(token); }
    void append(const char* decoded) override
    {
        support_seg61_cc13::scene_text_writer_write_string_0094ff10(writer(), decoded, host_);
    }
    char* join() override
    {
        joined_ = support_seg61_cc13::scene_text_chunk_list_join_009500a0(writer(), host_);
        return joined_;
    }
    void clear() override
    {
        writer().at<std::uint32_t>(0x0) = kReaderVtable;
        support_seg61_cc13::scene_text_chunk_list_clear_00950320(writer(), host_);
    }
    void destroy_reader() noexcept override
    {
        try {
            clear();
        } catch (...) {
        }
    }
    void destroy_base() noexcept override {}

private:
    static constexpr std::uint32_t kReaderVtable = 0x00d162ccu;

    support_seg61_cc13::Raw writer() noexcept { return support_seg61_cc13::Raw{writer_}; }

    SceneQuotedRunView runs_;
    SceneTextChunkWriterHost host_;
    alignas(4) std::uint8_t writer_[0x20]{};
    char* joined_{nullptr};
};

// Binds 00438E10 to its existing body; the borrowed token text is produced by
// SceneLexer::peek (008D8A70) over the VFS scene/library stream, not a new token.
struct ScenePropertyTokenCompareAdapter final : cc13_sweepA::SceneTokenEqualsHost {
    std::int32_t compare_insensitive(const char* a, const char* b) override {
        return compare_insensitive_00438e10(a, b);
    }
};

// An unassigned type-2 record, filled by the decoded authored S token. The
// parser consumes its actual owned +0Ch result before this scope retires it.
// Evidence: bsp.py show 008EF1B0 (nullable initial text) and 008F3370 --asm:
// 008F3373 tests tag 2, 008F3394 duplicates the token, 008F3399 publishes +0Ch.
// This Source owner uses the admitted current-CRT allocation/free pair; the
// phase word is data, and no Original vtable/class or register ABI is invoked.
class SceneAssignedStringProperty final {
public:
    explicit SceneAssignedStringProperty(const char* decoded_token) {
        construct_native_scene_property_record_type2_string_storage_008ef1b0(
            record_.data(), nullptr, nullptr);
        assign_native_scene_property_from_token_008f3370(
            record_.data(), decoded_token, game::game_native_string_process());
    }
    ~SceneAssignedStringProperty() {
        singleton_lifetime_free(reinterpret_cast<void*>(
            static_cast<std::uintptr_t>(record_[3])));
    }
    SceneAssignedStringProperty(const SceneAssignedStringProperty&) = delete;
    SceneAssignedStringProperty& operator=(const SceneAssignedStringProperty&) = delete;

    const char* text() const {
        const auto* value = reinterpret_cast<const char*>(
            static_cast<std::uintptr_t>(record_[3]));
        // The successful Source domain requires a copy for the nonnull token.
        if (value == nullptr) throw std::bad_alloc();
        return value;
    }

private:
    // Leave the raw constructor's preserved bytes alone; only its initialized
    // type and owned payload fields are read by this adapter/the assignment.
    std::array<std::uint32_t, native_scene_property_record_type2_string_storage_bytes / 4> record_;
};

ScenePropertyBlock parse_property_body(Cursor& cur);

// The optional "( g1 g2 ... )" group list, then "{ body }". 008f5a00.
void parse_property_section(Cursor& cur, std::vector<std::string>* groups,
    ScenePropertyBlock& out)
{
    if (cur.at("(")) {
        cur.expect("(");
        while (!cur.at(")") && !cur.at_end() && !cur.exhausted()) {
            const std::string name = cur.read_token();
            if (groups != nullptr && !name.empty()) {
                groups->push_back(name);
            }
        }
        cur.expect(")");
    }
    cur.expect("{");
    out = parse_property_body(cur);
}

ScenePropertyBlock parse_property_body(Cursor& cur)
{
    ScenePropertyBlock block;
    while (!cur.at("}")) {
        if (cur.at_end() || cur.exhausted()) {
            break;
        }
        const std::string key = cur.read_token();
        if (cur.at("{")) {
            cur.expect("{");
            // Native 008F672E attaches a missing child before recursion, so an
            // empty body still produces a present child (008F6740).
            block.blocks.emplace_back(key, parse_property_body(cur));
            continue;
        }
        if (!cur.at("=")) {
            // 008f66b9: neither '=' nor '{' - drop the key and resume.
            continue;
        }
        cur.expect("=");
        SceneProperty prop;
        prop.key = key;
        prop.type_letter = cur.read_token();
        // Chain 311: the +4h quoted byte of the token after the letter, as the
        // reference arm tests it at 008F623A before 008D9980 reads the target.
        bool first_value_quoted = false;
        if constexpr (kWireGoal4Chain311Bound) {
            first_value_quoted = cur.lexer().peek().quoted;
        }
        if (equal_insensitive(prop.type_letter, "S") && cur.lexer().peek().quoted) {
            if constexpr (kWireGoal2Chain145Bound) {
                // 008F5CF6 -> 008EFB00 with its chunk-list writer: 009502E0,
                // 0094FF10 and 009500A0 build the value. OFF keeps chain 115's
                // std::string view and the inline run loop below.
                SceneQuotedRunChunkView runs(cur.lexer());
                if constexpr (kWireGoal4Chain284Bound) {
                    const SceneAssignedStringProperty assigned(
                        oct18_rec::read_quoted_runs_008efb00(runs));
                    prop.values.emplace_back(assigned.text());
                } else {
                prop.values.push_back(std::string(oct18_rec::read_quoted_runs_008efb00(runs)));
                }
            } else if constexpr (kWireGoal2Chain115Bound) {
                // 008F5CF6 -> 008EFB00: read_quoted_runs_008efb00 consumes the
                // quoted runs over this lexer; OFF keeps the inline run loop.
                SceneQuotedRunView runs(cur.lexer());
                if constexpr (kWireGoal4Chain284Bound) {
                    const SceneAssignedStringProperty assigned(
                        oct18_rec::read_quoted_runs_008efb00(runs));
                    prop.values.emplace_back(assigned.text());
                } else {
                prop.values.push_back(std::string(oct18_rec::read_quoted_runs_008efb00(runs)));
                }
            } else {
            // 008F5CF6 -> 008EFB00: consume quoted runs by +4h, not token text.
            // 008EE670 decodes each run before the builder appends without a
            // separator. The lexer itself retains raw tokens/backslashes.
            std::string value;
            do {
                const SceneToken fragment = cur.lexer().next();
                // The native cached/read buffers are 400h (+5h/+405h, cache
                // +805h). Overflow, embedded NUL and a trailing single escape
                // are outside this source binding, not safe native recovery.
                if (fragment.text.size() >= 0x400
                    || fragment.text.find('\0') != std::string::npos) {
                    throw std::runtime_error("unsupported quoted scene string fragment");
                }
                for (std::size_t i = 0; i < fragment.text.size();) {
                    const char ch = fragment.text[i++];
                    if (ch != '\\') {
                        value.push_back(ch);
                        continue;
                    }
                    if (i == fragment.text.size()) {
                        throw std::runtime_error("unsupported trailing scene string escape");
                    }
                    const char escape = fragment.text[i++];
                    if (escape == '\\') value.push_back('\\');
                    else if (escape == 'n') value.push_back('\n');
                    else if (escape == 'q') value.push_back('"');
                    // All other backslash pairs emit nothing in 008EE670.
                }
            } while (cur.lexer().peek().quoted);
            prop.values.push_back(std::move(value));
            }
            if constexpr (kWireGoal4Chain307Bound) {
                // 008F5CF6 reads quoted runs before 008F38A0 duplicates the
                // decoded text. Retain presence only for a closed statement;
                // empty quoted text is present, while bare S ';' stays raw.
                prop.has_string = cur.at(";") && prop.values.size() == 1;
            } else {
                // The previous parser retained only the decoded token.
            }
        } else if (equal_insensitive(prop.type_letter, "F") && !cur.at(";")) {
            // Successful explicit F: 008F5C55 reads, 008F5C5A stores binary32.
            // Empty F remains on the raw partial path: a native existing value
            // is preserved when the ephemeral read-success byte is zero.
            const char* locale = std::setlocale(LC_NUMERIC, nullptr);
            if (locale == nullptr || std::string(locale) != "C") {
                throw std::runtime_error("unsupported F numeric locale");
            }
            const SceneToken& token = cur.lexer().peek();
            if (token.is_end() || token.text.size() >= 0x400
                || token.text.find('\0') != std::string::npos) {
                throw std::runtime_error("unsupported F scalar token");
            }
            std::size_t first = 0;
            while (first < token.text.size()
                && std::isspace(static_cast<unsigned char>(token.text[first]))) ++first;
            if (first < token.text.size()
                && (token.text[first] == '+' || token.text[first] == '-')) ++first;
            const bool digit = first < token.text.size()
                && std::isdigit(static_cast<unsigned char>(token.text[first]));
            const bool fraction = first + 1 < token.text.size()
                && token.text[first] == '.'
                && std::isdigit(static_cast<unsigned char>(token.text[first + 1]));
            const bool hex = first + 1 < token.text.size()
                && token.text[first] == '0'
                && (token.text[first + 1] == 'x' || token.text[first + 1] == 'X');
            if ((!digit && !fraction) || hex) {
                throw std::runtime_error("unsupported F decimal prefix");
            }
            float value = 0.0f;
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
            const int converted = std::sscanf(token.text.c_str(), "%f", &value);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
            if (converted != 1 || !std::isfinite(value)) {
                throw std::runtime_error("unsupported F scalar conversion");
            }
            // Modern source CRT only; historical VS2005/FP parity unverified.
            prop.float_value = value;
            prop.has_float = true;
            prop.values.push_back(cur.read_token());
        } else if (equal_insensitive(prop.type_letter, "V3")) {
            // 008F629F..008F62B0 zero all lanes before the first-number guard.
            // Immediate ';' retains a present positive-zero vector, with no
            // scalar conversion. Preserve its empty authored token list.
            prop.has_vector3 = true;
            if (!cur.at(";")) {
                const char* locale = std::setlocale(LC_NUMERIC, nullptr);
                if (locale == nullptr || std::string(locale) != "C") {
                    throw std::runtime_error("unsupported V3 numeric locale");
                }
                for (std::size_t lane = 0; lane < prop.vector3.size(); ++lane) {
                    const SceneToken& token = cur.lexer().peek();
                    if (token.is_end() || token.text.size() >= 0x400
                        || token.text.find('\0') != std::string::npos) {
                        throw std::runtime_error("unsupported V3 scalar token");
                    }
                    // This binding admits ordinary decimal prefixes, not hex,
                    // NaN/Inf, or other CRT forms. Do not change the shared
                    // scene_scan_float decimal-prefix/strtod projection.
                    std::size_t first = 0;
                    while (first < token.text.size()
                        && std::isspace(static_cast<unsigned char>(token.text[first]))) ++first;
                    if (first < token.text.size()
                        && (token.text[first] == '+' || token.text[first] == '-')) ++first;
                    const bool digit = first < token.text.size()
                        && std::isdigit(static_cast<unsigned char>(token.text[first]));
                    const bool fraction = first + 1 < token.text.size()
                        && token.text[first] == '.'
                        && std::isdigit(static_cast<unsigned char>(token.text[first + 1]));
                    const bool hex = first + 1 < token.text.size()
                        && token.text[first] == '0'
                        && (token.text[first + 1] == 'x' || token.text[first + 1] == 'X');
                    if ((!digit && !fraction) || hex) {
                        throw std::runtime_error("unsupported V3 decimal prefix");
                    }
                    float value = 0.0f;
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
                    const int converted = std::sscanf(token.text.c_str(), "%f", &value);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
                    if (converted != 1 || !std::isfinite(value)) {
                        throw std::runtime_error("unsupported V3 scalar conversion");
                    }
                    // Native readers at 008F62C6/62D6/62E6 store binary32.
                    // Modern CRT parity with VS2005 BF7533 is not established.
                    prop.vector3[lane] = value;
                    prop.values.push_back(cur.read_token());
                }
            }
        } else {
            // Other typed/implicit paths remain a partial token scan. Stop at
            // braces as well as ';' so a missing terminator cannot swallow the
            // current closing brace and nest later blocks one level too deep
            // (scene175/scene907; docs/SCENE_FILE_READER.md Corrections).
            // 008F635E -> 0045C440 checks the pending ';'; OFF keeps the raw token scan.
            if constexpr (kWireGoal2Chain202Bound) {
                ScenePropertyTokenCompareAdapter compare;
                while (!cc13_sweepA::scene_token_equals_0045c440(
                           cc13_sweepA::SceneTokenView{cur.lexer().peek().text.c_str()}, ";", compare)
                    && !cur.at("{") && !cur.at("}") && !cur.at_end() && !cur.exhausted()) {
                    prop.values.push_back(cur.read_token());
                }
            } else {
            while (!cur.at(";") && !cur.at("{") && !cur.at("}") && !cur.at_end()
                && !cur.exhausted()) {
                prop.values.push_back(cur.read_token());
            }
            }
        }
        if constexpr (kWireGoal4Chain425Bound) {
            // bsp.py show 008F5A00 --asm, 008F6432..008F64B9: read the
            // count with 008D9AD0, allocate 4*count, and FSTP each 008D9B40
            // return into that buffer before the flags-0 008F3510 insertion.
            // Match the existing V3/V3A finite decimal %f provider; historical
            // VS2005 CRT parity is unproven. Ambiguous/empty/failed forms keep
            // their raw tokens and never enter the native owning projection.
            if (equal_insensitive(prop.type_letter, "FA") && cur.at(";")
                && prop.values.size() >= 2) {
                std::int32_t count = 0;
                const char* const locale = std::setlocale(LC_NUMERIC, nullptr);
                if (locale != nullptr && std::string(locale) == "C"
                    && scene_scan_int(prop.values.front(), count) && count >= 1
                    && count <= 0x3fffffff
                    && static_cast<std::size_t>(count) == prop.values.size() - 1) {
                    std::vector<float> elements(static_cast<std::size_t>(count));
                    bool all = true;
                    for (std::size_t i = 0; i < elements.size() && all; ++i) {
                        const std::string& text = prop.values[i + 1];
                        std::size_t first = 0;
                        while (first < text.size()
                            && std::isspace(static_cast<unsigned char>(text[first]))) ++first;
                        if (first < text.size() && (text[first] == '+' || text[first] == '-')) ++first;
                        const bool digit = first < text.size()
                            && std::isdigit(static_cast<unsigned char>(text[first]));
                        const bool fraction = first + 1 < text.size() && text[first] == '.'
                            && std::isdigit(static_cast<unsigned char>(text[first + 1]));
                        const bool hex = first + 1 < text.size() && text[first] == '0'
                            && (text[first + 1] == 'x' || text[first + 1] == 'X');
                        all = text.size() < 0x400 && text.find('\0') == std::string::npos
                            && (digit || fraction) && !hex;
                        if (!all) break;
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
                        all = std::sscanf(text.c_str(), "%f", &elements[i]) == 1
                            && std::isfinite(elements[i]);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
                    }
                    if (all) {
                        prop.float_array = std::move(elements);
                        prop.has_float_array = true;
                    }
                }
            }
        } else {
            // The prior parser retains only FA's raw tokens.
        }
        if constexpr (kWireGoal4Chain330Bound) {
            // bsp.py disasm-raw 008F65CF: count at 65DE, allocate 12*count
            // at 65F6, expect '(' / ')' at 6617 / 6653, three 008D9B40
            // reads each FSTP binary32 at 6628/6638/6648. 669A hands the
            // owned buffer to 008F3610 with flags 0. Admit only complete,
            // positive-count ordinary decimal forms; preserve raw tokens for
            // declaration-dependent, empty, malformed or failed reads.
            if (equal_insensitive(prop.type_letter, "V3A") && cur.at(";")
                && prop.values.size() >= 6) {
                std::int32_t count = 0;
                const char* const locale = std::setlocale(LC_NUMERIC, nullptr);
                if (locale != nullptr && std::string(locale) == "C"
                    && scene_scan_int(prop.values.front(), count) && count >= 1
                    && count <= 0x15555555
                    && static_cast<std::size_t>(count) == (prop.values.size() - 1) / 5
                    && (prop.values.size() - 1) % 5 == 0) {
                    std::vector<std::array<float, 3>> elements(static_cast<std::size_t>(count));
                    bool all = true;
                    for (std::size_t i = 0; i < elements.size() && all; ++i) {
                        const std::size_t start = 1 + i * 5;
                        all = prop.values[start] == "(" && prop.values[start + 4] == ")";
                        for (std::size_t lane = 0; lane < 3 && all; ++lane) {
                            const std::string& text = prop.values[start + lane + 1];
                            std::size_t first = 0;
                            while (first < text.size()
                                && std::isspace(static_cast<unsigned char>(text[first]))) ++first;
                            if (first < text.size() && (text[first] == '+' || text[first] == '-')) ++first;
                            const bool digit = first < text.size()
                                && std::isdigit(static_cast<unsigned char>(text[first]));
                            const bool fraction = first + 1 < text.size() && text[first] == '.'
                                && std::isdigit(static_cast<unsigned char>(text[first + 1]));
                            const bool hex = first + 1 < text.size() && text[first] == '0'
                                && (text[first + 1] == 'x' || text[first + 1] == 'X');
                            all = text.size() < 0x400 && text.find('\0') == std::string::npos
                                && (digit || fraction) && !hex;
                            if (!all) break;
                            // Same bounded %f provider as V3 above. Historical
                            // VS2005 CRT parity is not established; no float
                            // arithmetic or extended-precision chain is added.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
                            all = std::sscanf(text.c_str(), "%f", &elements[i][lane]) == 1
                                && std::isfinite(elements[i][lane]);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
                        }
                    }
                    if (all) {
                        prop.vector3_array = std::move(elements);
                        prop.has_vector3_array = true;
                    }
                }
            }
        } else {
            // The prior parser retains only V3A's raw tokens.
        }
        // 008F5D8A/5DA1 recognize true/false, then store type-3 byte +0Ch.
        // Keep the raw scan and admit only one literal with a closed statement;
        // native existing-record/empty assignment needs declaration context.
        if (equal_insensitive(prop.type_letter, "B") && cur.at(";")
            && prop.values.size() == 1) {
            const bool is_true = equal_insensitive(prop.values.front(), "true");
            if (is_true || equal_insensitive(prop.values.front(), "false")) {
                prop.boolean_value = static_cast<std::uint8_t>(is_true ? 1 : 0);
                prop.has_boolean = true;
            }
        }
        if constexpr (kWireGoal4Chain319Bound) {
            // bsp.py show 008F5A00 --asm, 008F5E15..008F5FCF: E reads the table
            // token (008D9980), looks it up (0048E960, ECX = [00E1867C]), and
            // only when ':' follows (00467CC0, 00CF00B0) reads the symbol token
            // and calls 008F3A20(key, table, symbol); without ':' nothing is
            // added. Mark the closed one-table/one-symbol form; the native merge
            // decides whether the registry knows both (0048E840 needs a hit).
            if (equal_insensitive(prop.type_letter, "E") && cur.at(";")
                && prop.values.size() == 3 && prop.values[1] == ":"
                && !prop.values[0].empty() && !prop.values[2].empty()) {
                bool bounded = true;
                for (const std::string& token : prop.values) {
                    bounded = bounded && token.size() < 0x400
                        && token.find('\0') == std::string::npos;
                }
                prop.has_enum = bounded;
            }
        } else {
            // The prior parser retains only the raw E tokens.
        }
        // 008F5BB2 reads %d and 008F5BC6/008F3730 store the type-0 DWORD.
        // Keep the existing source CRT provider and raw diagnostics; bare I
        // and failed/context-dependent assignments receive no owning payload.
        if (equal_insensitive(prop.type_letter, "I") && cur.at(";")
            && prop.values.size() == 1 && !prop.values.front().empty()) {
            prop.has_integer = scene_scan_int(prop.values.front(), prop.integer_value);
        }
        if constexpr (kWireGoal4Chain311Bound) {
            // 008F64E5 matches "IA" [00D16514]; unless the next token is ';'
            // (008F64FC, 0045C440 on [00CE5698]) 008F650C reads the count with
            // 008D9AD0, 008F6524 allocates 4*count, and 008F653B reads each
            // element with 008D9AD0 into the buffer; 008F6585 then hands
            // (data, count, flags 0) to 008F3590 and 008F66A6 expects ';'.
            // Admit only a closed statement whose first token is a count n >= 1
            // followed by exactly n tokens, all accepted by the same %d rule.
            // A failed read (native 0 without consuming), a count mismatch or
            // an empty/zero count stays on the raw token path.
            if (equal_insensitive(prop.type_letter, "IA") && cur.at(";")
                && prop.values.size() >= 2) {
                std::int32_t count = 0;
                if (scene_scan_int(prop.values.front(), count) && count >= 1
                    && static_cast<std::size_t>(count) == prop.values.size() - 1) {
                    std::vector<std::int32_t> elements(static_cast<std::size_t>(count));
                    bool all = true;
                    for (std::size_t i = 0; i < elements.size() && all; ++i) {
                        all = scene_scan_int(prop.values[i + 1], elements[i]);
                    }
                    if (all) {
                        prop.integer_array = std::move(elements);
                        prop.has_integer_array = true;
                    }
                }
            }
            // 008F6163..008F6231 consume one of eight reference letters (case-
            // insensitive 00467CC0) and put its kind in EDI; 008F623A tests the
            // peeked token's quoted byte and 008F6245 (008D9980) reads it as
            // the target of 008F3AC0 (word18 0, word08 kind); an unquoted next
            // token gives target "" [00CE3A0C] and is left for 008F66A6. Admit
            // only the closed one-quoted-token form, within the 400h buffer.
            if (scene_reference_kind(prop.type_letter) >= 0 && cur.at(";")
                && prop.values.size() == 1 && first_value_quoted
                && prop.values.front().size() < 0x400
                && prop.values.front().find('\0') == std::string::npos) {
                prop.has_reference = true;
            }
        }
        if constexpr (kWireGoal4Chain321Bound) {
            // bsp.py show 008F5A00 --asm, 008F632D..008F63DF: BIN reads
            // count with 008D9AD0, allocates count bytes, then passes each
            // 008D9980 token in ECX to 008EF6D0. The provider receives the
            // retained buffer, count and flags 0. Admit complete nonempty
            // explicit statements; existing-record/empty forms stay raw.
            if (equal_insensitive(prop.type_letter, "BIN") && cur.at(";")
                && prop.values.size() >= 2) {
                std::int32_t count = 0;
                if (scene_scan_int(prop.values.front(), count) && count >= 1
                    && static_cast<std::size_t>(count) == prop.values.size() - 1) {
                    const bool bounded = std::all_of(prop.values.begin() + 1, prop.values.end(),
                        [](const std::string& token) {
                            return token.size() < 0x400
                                && token.find('\0') == std::string::npos;
                        });
                    if (bounded) {
                        prop.byte_array.reserve(static_cast<std::size_t>(count));
                        for (std::size_t i = 1; i < prop.values.size(); ++i) {
                            prop.byte_array.push_back(cc13_band0080::decode_hex_byte_token_008ef6d0(
                                prop.values[i].c_str()));
                        }
                        prop.has_byte_array = true;
                    }
                }
            }
        } else {
            // The prior parser retains only the raw BIN tokens.
        }
        cur.expect(";");
        block.values.push_back(std::move(prop));
    }
    cur.expect("}");
    return block;
}

// The body of a block this packet did not decode: balance braces and count the
// tokens consumed. Used for SceneBrowserGroups (004694f0) and traffic.
std::size_t skip_balanced_block(SceneLexer& lexer)
{
    std::size_t tokens = 0;
    int depth = 0;
    if (lexer.peek().text == "{") {
        lexer.next();
        depth = 1;
    }
    while (depth > 0 && !lexer.at_end()) {
        const SceneToken t = lexer.next();
        ++tokens;
        if (t.quoted) {
            continue;
        }
        if (t.text == "{") {
            ++depth;
        } else if (t.text == "}") {
            --depth;
        }
    }
    return tokens;
}

// Packet cc9_scene_frame_normalise (docs/SCENE_CONTENTS_HOSTS.md section 18):
// BSP_SceneFile_ReadEntityBlock 0046CF40 rebuilds every authored localframe
// before anything uses it. 0046D168..0046D1A5: s = sqrt(row0 . row0), the sum
// stored float, s = 0 unless it is above 1e-10 [00CE3820]; 0046D1BA: 0085DC80
// on the frame (row 2 the authority); 0046D1BF..0046D222: rows 0..2 each
// multiplied by s and stored float. The translation row is untouched.
// OFF: the frame as read (the earlier host behaviour).
// ON since the USN01 / USN02 / USN04 / USN13 pairs (section 18.4).
constexpr bool kSceneFrameNormaliseBound = true;

void normalise_localframe_0046d168(float m[16])
{
    const double sum = static_cast<double>(m[0]) * m[0] + static_cast<double>(m[1]) * m[1]
        + static_cast<double>(m[2]) * m[2];
    const float stored = static_cast<float>(sum);                   // 0046D184
    float s = 0.0f;                                                  // 0046D1AB XORPS
    if (static_cast<double>(stored) > 1e-10) {                       // 0046D192 FCOMI, JBE
        s = static_cast<float>(std::sqrt(static_cast<double>(stored)));   // 0046D198 00BF7030
    }
    bsp::orthonormalize_pose_matrix_0085dc80(m);                     // 0046D1BA
    const int rows[9] = {0, 1, 2, 4, 5, 6, 8, 9, 10};
    for (int k : rows) m[k] = static_cast<float>(static_cast<double>(m[k]) * s);
}

SceneEntity parse_entity(Cursor& cur)
{
    SceneEntity ent;
    ent.name = cur.read_token();
    cur.expect("(");
    ent.class_name = cur.read_token();
    cur.expect(")");
    cur.expect("{");
    cur.expect("localframe");
    for (int i = 0; i < 16; ++i) {
        ent.frame[i] = cur.read_float();
    }
    cur.expect(";");
    if constexpr (kSceneFrameNormaliseBound) normalise_localframe_0046d168(ent.frame);
    if (cur.at("template")) {
        cur.expect("template");
        ent.template_path = cur.read_token();
        ent.has_template = true;
        cur.expect(";");
    }
    if (cur.at("uid")) {
        cur.expect("uid");
        ent.uid = cur.read_int();
        ent.has_uid = true;
        cur.expect(";");
    }
    cur.expect("properties");
    parse_property_section(cur, &ent.groups, ent.properties);
    while (cur.at("entity") && !cur.exhausted()) {
        cur.expect("entity");
        ent.children.push_back(parse_entity(cur));
    }
    cur.expect("}");
    return ent;
}

void multiply_frames(const float parent[16], const float local[16], float out[16])
{
    // 00413920 composes the child's localframe with the parent's world frame.
    // The 16 floats are a column-major 4x4 with the translation in the last
    // row, matching how the shipped files author them.
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += local[row * 4 + k] * parent[k * 4 + col];
            }
            out[row * 4 + col] = sum;
        }
    }
}

void visit_entity(SceneFileReaderHost& host, const SceneEntity& ent, const float parent[16],
    SceneFilePass pass, std::size_t& visited)
{
    float world[16];
    multiply_frames(parent, ent.frame, world);
    ++visited;
    host.instantiate_entity(ent, world, pass);
    for (const SceneEntity& child : ent.children) {
        visit_entity(host, child, world, pass, visited);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// Key tables
// ---------------------------------------------------------------------------

const SceneKeyRow kSceneTopLevelKeys[5] = {
    {"header", SceneTopLevelKey::Header, 0x00469bf0u, "00ce5994"},
    {"entity", SceneTopLevelKey::Entity, 0x0046cf40u, "00ce5818"},
    {"traffic", SceneTopLevelKey::Traffic, 0x009514b0u, "00ce5890"},
    {"groups", SceneTopLevelKey::Groups, 0x00467e10u, "00ce55e4"},
    {"SceneBrowserGroups", SceneTopLevelKey::SceneBrowserGroups, 0x00469e40u, "00ce56a4"},
};

const SceneHeaderKeyRow kSceneHeaderKeys[4] = {
    {"uniqueID", SceneHeaderKey::UniqueId, "int -> scene record +1098h"},
    {"NextUID", SceneHeaderKey::NextUid, "int, read and discarded"},
    {"properties", SceneHeaderKey::Properties, "property bag published as SceneRootProps"},
    {"precache", SceneHeaderKey::Precache, "property bag; sets scene record +905h"},
};

const SceneEntityKeyRow kSceneEntityKeys[4] = {
    {"localframe", false, "16 floats then ';'"},
    {"template", true, "one token then ';'"},
    {"uid", true, "one int then ';'"},
    {"properties", false, "optional ( groups ) then { body }"},
};

const SceneTerrainShadowKeyRow kSceneTerrainShadowKeys[4] = {
    {"g_StaticShadowTexture", "g_Terrain.g_StaticShadowTexture", true},
    {"ga_StaticShadowShotOffsetX", "g_Terrain.ga_StaticShadowShotOffsetX", false},
    {"ga_StaticShadowShotOffsetZ", "g_Terrain.ga_StaticShadowShotOffsetZ", false},
    {"ga_StaticShadowShotSize", "g_Terrain.ga_StaticShadowShotSize", false},
};

const char* const kSceneRegistrationClassNames[4] = {
    "CommandBuilding",
    "LandVehicle",
    "LandFort",
    "VehicleClass",
};

// ---------------------------------------------------------------------------
// Lexer
// ---------------------------------------------------------------------------

SceneLexer::SceneLexer(std::string text, std::string delimiters)
    : text_(std::move(text)), delims_(std::move(delimiters))
{
}

char SceneLexer::get()
{
    if (pos_ >= text_.size()) {
        return '\0';
    }
    const char c = text_[pos_++];
    if (c == '\n') {
        ++line_;
    }
    return c;
}

char SceneLexer::peek_char() const noexcept
{
    return pos_ < text_.size() ? text_[pos_] : '\0';
}

bool SceneLexer::is_space(char c) const noexcept
{
    if (c == '\0') {
        return false;
    }
    for (const char* p = kSceneWhitespace; *p != 0; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}

bool SceneLexer::is_delim(char c) const noexcept
{
    if (c == '\0') {
        return false;
    }
    return delims_.find(c) != std::string::npos;
}

const SceneToken& SceneLexer::peek()
{
    if (!cached_) {
        cache_ = scan();
        cached_ = true;
    }
    return cache_;
}

SceneToken SceneLexer::next()
{
    peek();
    cached_ = false;
    return cache_;
}

bool SceneLexer::recover_after_failed_read()
{
    peek();
    while (pos_ < text_.size() && is_space(peek_char())) {
        get();
    }
    return pos_ >= text_.size();
}

SceneToken SceneLexer::scan()
{
    for (;;) {
        const char c = get();
        if (c == '\0') {
            SceneToken t;
            t.line = line_;
            return t;
        }
        if (is_space(c)) {
            continue;
        }
        if (c == '/') {
            const char nxt = peek_char();
            if (nxt == '/') {
                while (pos_ < text_.size() && text_[pos_] != '\n') {
                    get();
                }
                continue;
            }
            if (nxt == '*') {
                get();
                // The native previous/current-byte pair initially contains
                // '/' and '*', so even the overlapping terminator in /*/ ends
                // this comment (008d8c60-008d8cba).
                char prev = '*';
                while (pos_ < text_.size()) {
                    const char ch = get();
                    if (prev == '*' && ch == '/') {
                        break;
                    }
                    prev = ch;
                }
                continue;
            }
        }
        if (is_delim(c)) {
            SceneToken t;
            t.text.assign(1, c);
            t.line = line_;
            return t;
        }
        if (c == '"') {
            SceneToken t;
            t.quoted = true;
            t.line = line_;
            while (pos_ < text_.size()) {
                const char ch = get();
                if (ch == '"') {
                    break;
                }
                t.text.push_back(ch);
            }
            return t;
        }
        SceneToken t;
        t.line = line_;
        t.text.push_back(c);
        while (pos_ < text_.size()) {
            const char ch = peek_char();
            if (is_space(ch) || is_delim(ch)) {
                break;
            }
            // 008d8dd2-008d8e3a only checks whitespace and delimiters here;
            // comment openers inside an unquoted token are ordinary text.
            t.text.push_back(get());
        }
        return t;
    }
}

// ---------------------------------------------------------------------------
// Scalar conversions
// ---------------------------------------------------------------------------

#if defined(_MSC_VER)
#pragma warning(push)
// The native reader uses sscanf; the integer-only format has no buffer output.
#pragma warning(disable : 4996)
#endif
bool scene_scan_int(const std::string& text, std::int32_t& out) noexcept
{
    // 008d9ad0 uses %d directly, including CRT whitespace inside a quoted
    // token. Keep output unchanged when conversion fails.
    int value = 0;
    if (std::sscanf(text.c_str(), "%d", &value) != 1) {
        return false;
    }
    out = static_cast<std::int32_t>(value);
    return true;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

int scene_reference_kind(const std::string& letter) noexcept
{
    // Letter strings: R [00CE60EC], RLand [00D1637C], RFort [00D16384],
    // RShip [00D1636C], RPlane [00D16374], RPlnShp [00D16364],
    // RLPlnShp [00D16358], RPath [00D1638C]; kinds from 008F6163..008F6231.
    static constexpr struct { const char* text; int kind; } kLetters[] = {
        {"R", 0}, {"RLand", 1}, {"RFort", 2}, {"RShip", 3},
        {"RPlane", 4}, {"RPlnShp", 5}, {"RLPlnShp", 6}, {"RPath", 7},
    };
    for (const auto& entry : kLetters) {
        if (equal_insensitive(letter, entry.text)) return entry.kind;
    }
    return -1;
}

bool scene_scan_float(const std::string& text, float& out) noexcept
{
    std::size_t i = 0;
    if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
        ++i;
    }
    std::size_t digits = 0;
    while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
        ++i;
        ++digits;
    }
    if (i < text.size() && text[i] == '.') {
        ++i;
        while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
            ++i;
            ++digits;
        }
    }
    if (digits == 0) {
        return false;
    }
    std::size_t j = i;
    if (j < text.size() && (text[j] == 'e' || text[j] == 'E')) {
        ++j;
        if (j < text.size() && (text[j] == '+' || text[j] == '-')) {
            ++j;
        }
        const std::size_t exp_start = j;
        while (j < text.size() && std::isdigit(static_cast<unsigned char>(text[j])) != 0) {
            ++j;
        }
        if (j > exp_start) {
            i = j;
        }
    }
    out = static_cast<float>(std::strtod(text.substr(0, i).c_str(), nullptr));
    return true;
}

// ---------------------------------------------------------------------------
// Block parsers
// ---------------------------------------------------------------------------

const SceneProperty* ScenePropertyBlock::find(const std::string& key) const noexcept
{
    for (const SceneProperty& prop : values) {
        if (equal_insensitive(prop.key, key.c_str())) {
            return &prop;
        }
    }
    return nullptr;
}

SceneHeader parse_scene_header_00469bf0(SceneLexer& lexer, std::vector<std::string>& errors)
{
    Cursor cur(lexer, errors);
    SceneHeader header;
    cur.expect("{");
    while (!cur.at("}") && !cur.at_end() && !cur.exhausted()) {
        const std::string key = cur.read_token();
        if (equal_insensitive(key, "NextUID")) {
            header.next_uid = cur.read_int();
            header.has_next_uid = true;
            cur.expect(";");
        } else if (equal_insensitive(key, "uniqueID")) {
            header.unique_id = cur.read_int();
            header.has_unique_id = true;
            cur.expect(";");
        } else if (equal_insensitive(key, "properties")) {
            parse_property_section(cur, nullptr, header.properties);
            header.has_properties = true;
        } else if (equal_insensitive(key, "precache")) {
            parse_property_section(cur, nullptr, header.precache);
            header.has_precache = true;
        }
        // Any other key is consumed and ignored, as in 00469bf0.
    }
    cur.expect("}");
    return header;
}

SceneEntity parse_scene_entity_0046cf40(SceneLexer& lexer, std::vector<std::string>& errors)
{
    Cursor cur(lexer, errors);
    return parse_entity(cur);
}

std::vector<SceneGroupEntry> parse_scene_groups_00467e10(SceneLexer& lexer,
    std::vector<std::string>& errors)
{
    Cursor cur(lexer, errors);
    std::vector<SceneGroupEntry> out;
    cur.expect("{");
    while (!cur.at("}") && !cur.at_end() && !cur.exhausted()) {
        cur.expect("group");
        cur.expect("{");
        SceneGroupEntry entry;
        cur.expect("GroupName");
        cur.expect("=");
        entry.name = cur.read_token();
        cur.expect("Index");
        cur.expect("=");
        entry.index = cur.read_int();
        cur.expect("}");
        out.push_back(std::move(entry));
    }
    cur.expect("}");
    return out;
}

ScenePropertyBlock parse_scene_property_block_008f5a00(SceneLexer& lexer,
    std::vector<std::string>& errors)
{
    Cursor cur(lexer, errors);
    ScenePropertyBlock block;
    parse_property_section(cur, nullptr, block);
    return block;
}

SceneDocument parse_scene_document(SceneLexer& lexer)
{
    SceneDocument doc;
    Cursor cur(lexer, doc.errors);
    while (!cur.at_end() && !cur.exhausted()) {
        const SceneToken token = lexer.next();
        if (equal_insensitive(token.text, "header")) {
            doc.header = parse_scene_header_00469bf0(lexer, doc.errors);
            doc.has_header = true;
        } else if (equal_insensitive(token.text, "entity")) {
            doc.entities.push_back(parse_entity(cur));
        } else if (equal_insensitive(token.text, "groups")) {
            const std::vector<SceneGroupEntry> groups
                = parse_scene_groups_00467e10(lexer, doc.errors);
            doc.groups.insert(doc.groups.end(), groups.begin(), groups.end());
            doc.has_groups = true;
        } else if (equal_insensitive(token.text, "SceneBrowserGroups")) {
            doc.browser_group_tokens += skip_balanced_block(lexer);
        } else if (equal_insensitive(token.text, "traffic")) {
            ++doc.traffic_blocks;
            skip_balanced_block(lexer);
        }
        // Anything else is ignored, as in 0046df00's top-level loop.
    }
    return doc;
}

SceneDocument parse_scene_document(const std::string& text)
{
    SceneLexer lexer(text);
    return parse_scene_document(lexer);
}

// ---------------------------------------------------------------------------
// Path split
// ---------------------------------------------------------------------------

SceneDatabasePath split_scene_path_0046df00(const std::string& scene_path)
{
    SceneDatabasePath out;
    out.full = scene_path;
    if (scene_path.empty()) {
        return out;  // 0046df00 leaves the stem empty and copies 00ce3a0c ("").
    }
    std::size_t n = scene_path.size();
    while (n != 0) {
        if (scene_path[n - 1] == '.') {
            out.stem = scene_path.substr(0, n - 1);
            return out;
        }
        --n;
    }
    out.stem = scene_path;
    return out;
}

// ---------------------------------------------------------------------------
// Passes
// ---------------------------------------------------------------------------

SceneFilePassFlags scene_pass_flags(SceneFilePass pass) noexcept
{
    SceneFilePassFlags flags;
    flags.instantiate = pass == SceneFilePass::Instantiate;
    flags.registration = pass == SceneFilePass::Registration;
    return flags;
}

SceneFilePass scene_pass_from_flags(bool instantiate, bool registration) noexcept
{
    if (registration) {
        return SceneFilePass::Registration;
    }
    if (instantiate) {
        return SceneFilePass::Instantiate;
    }
    return SceneFilePass::Header;
}

bool scene_pass_runs_entity_loop(SceneFilePass pass) noexcept
{
    return pass != SceneFilePass::Header;
}

bool scene_pass_stops_at_tail_blocks(SceneFilePass pass) noexcept
{
    return pass == SceneFilePass::Registration;
}

SceneFileReadResult run_scene_file_reader_0046df00(SceneFileReaderHost& host,
    const std::string& scene_path, const std::string& override_name, SceneFilePass pass)
{
    SceneFileReadResult result;
    host.clear_pending_references();  // 0046a9f0 on database+14Ch
    result.path = split_scene_path_0046df00(scene_path);

    result.weather_descriptor = host.select_weather_descriptor(scene_path, override_name);
    if (override_name.empty()) {
        for (const SceneTerrainShadowKeyRow& row : kSceneTerrainShadowKeys) {
            if (row.is_string) {
                host.set_terrain_shadow_string(row.target, std::string());
            } else {
                host.set_terrain_shadow_float(row.target, 0.0f);
            }
        }
    }

    std::string text;
    result.opened = host.read_scene_file(scene_path, text);
    SceneLexer lexer(text);
    SceneDocument& doc = result.document;
    Cursor cur(lexer, doc.errors);

    // The header is read by every pass; the tail is where they diverge.
    if (cur.at("header")) {
        cur.expect("header");
        doc.header = parse_scene_header_00469bf0(lexer, doc.errors);
        doc.has_header = true;
        if (doc.header.has_properties) {
            host.publish_scene_root_properties(doc.header.properties);
        }
        if (doc.header.has_precache) {
            result.record.precache_seen = true;
            host.publish_scene_precache(doc.header.precache);
        }
        if (doc.header.has_unique_id) {
            result.record.mission_id = doc.header.unique_id;
            host.set_scene_mission_id(doc.header.unique_id);
        }
    }

    if (!scene_pass_runs_entity_loop(pass)) {
        return result;
    }

    static const float kIdentity[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    bool tail_begun = false;
    bool stop = false;
    while (!stop && !cur.at_end() && !cur.exhausted()) {
        if constexpr (kWireGoal4Chain361Bound) {
            // 0046E9F7..0046EBD8 (bsp.py show 0046DF00 --asm): every arm peeks
            // the keyword (008D8A70) and compares it with 00438E10, in the
            // order entity 00CE5818, traffic 00CE5890, groups 00CE55E4,
            // SceneBrowserGroups 00CE56A4. Only `entity` is consumed here.
            const std::string keyword = lexer.peek().text;
            if (equal_insensitive(keyword, "entity")) {
                lexer.next();  // 0046EA47 008D8960
                // 0046EB0F: 0046CF40 reads the entity block (identity parent
                // frame built at 0046EA4C..0046EB01); visit_entity is its
                // instantiation half (0046C550), as before.
                SceneEntity ent = parse_scene_entity_0046cf40(lexer, doc.errors);
                visit_entity(host, ent, kIdentity, pass, result.entities_visited);
                doc.entities.push_back(std::move(ent));
                continue;
            }
            if (equal_insensitive(keyword, "traffic")) {
                ++doc.traffic_blocks;
                if (scene_pass_stops_at_tail_blocks(pass)) {
                    // 0046EB3D JNZ 0046EBDD: 0095CA10 with the keyword still
                    // pending, then the loop ends.
                    host.skip_traffic_block(lexer);
                    stop = true;
                    continue;
                }
                if (!tail_begun) {
                    host.begin_tail_blocks();  // 0046EB4B 00925F20
                    tail_begun = true;
                }
                // 0046EB5B 009514B0: the block reader expects `traffic` itself.
                host.load_traffic_block(lexer);
                continue;
            }
            if (equal_insensitive(keyword, "groups")) {
                if (scene_pass_stops_at_tail_blocks(pass)) {
                    stop = true;  // 0046EB7E JNZ 0046EBE2, keyword left pending
                    continue;
                }
                if (!tail_begun) {
                    host.begin_tail_blocks();  // 0046EB88 00925F20
                    tail_begun = true;
                }
                // 0046EB95 00467E10. Its first step is its own `groups` expect
                // (00467E15..00467E1C, 008D9930 on 00CE55E4); the recovered
                // body starts at the `{` that follows (scene_file.hpp: block
                // parsers take the keyword consumed), so the expect is here.
                cur.expect("groups");
                const std::vector<SceneGroupEntry> groups
                    = parse_scene_groups_00467e10(lexer, doc.errors);
                doc.groups.insert(doc.groups.end(), groups.begin(), groups.end());
                doc.has_groups = true;
                host.load_groups(doc.groups);
                continue;
            }
            if (equal_insensitive(keyword, "SceneBrowserGroups")) {
                if (scene_pass_stops_at_tail_blocks(pass)) {
                    stop = true;  // 0046EBBC JNZ 0046EBE2, keyword left pending
                    continue;
                }
                if (!tail_begun) {
                    host.begin_tail_blocks();  // 0046EBC6 00925F20
                    tail_begun = true;
                }
                // 0046EBD3 00469E40: the block reader expects
                // `SceneBrowserGroups` (00469E48..00469E4F) itself.
                host.load_browser_groups(lexer);
                continue;
            }
            // DEVIATION (kept from the previous loop): an unknown top-level
            // keyword is consumed and ignored. At 0046EBB4 the image jumps back
            // to 0046E9F5 and peeks the same cached token again, so it never
            // leaves the loop; no shipped scene has such a keyword.
            lexer.next();
        } else {
        const SceneToken token = lexer.next();
        if (equal_insensitive(token.text, "entity")) {
            SceneEntity ent = parse_entity(cur);
            visit_entity(host, ent, kIdentity, pass, result.entities_visited);
            doc.entities.push_back(std::move(ent));
            continue;
        }
        if (equal_insensitive(token.text, "traffic")) {
            ++doc.traffic_blocks;
            if (scene_pass_stops_at_tail_blocks(pass)) {
                host.skip_traffic_block(lexer);  // 0095ca10, then the loop ends
                stop = true;
                continue;
            }
            if (!tail_begun) {
                host.begin_tail_blocks();
                tail_begun = true;
            }
            host.load_traffic_block(lexer);
            continue;
        }
        if (equal_insensitive(token.text, "groups")) {
            if (scene_pass_stops_at_tail_blocks(pass)) {
                stop = true;
                continue;
            }
            if (!tail_begun) {
                host.begin_tail_blocks();
                tail_begun = true;
            }
            const std::vector<SceneGroupEntry> groups
                = parse_scene_groups_00467e10(lexer, doc.errors);
            doc.groups.insert(doc.groups.end(), groups.begin(), groups.end());
            doc.has_groups = true;
            host.load_groups(doc.groups);
            continue;
        }
        if (equal_insensitive(token.text, "SceneBrowserGroups")) {
            if (scene_pass_stops_at_tail_blocks(pass)) {
                stop = true;
                continue;
            }
            if (!tail_begun) {
                host.begin_tail_blocks();
                tail_begun = true;
            }
            host.load_browser_groups(lexer);
            continue;
        }
        // Unknown top-level keyword: ignored, as in the native loop.
        }
    }
    if (!tail_begun) {
        host.end_tail_blocks();  // 00925f20 runs once either way
    }
    if (pass == SceneFilePass::Instantiate) {
        host.resolve_deferred_references();  // 0046aab0
    }
    return result;
}

}  // namespace bsp
