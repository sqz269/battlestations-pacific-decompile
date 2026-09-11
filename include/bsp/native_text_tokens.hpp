#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {
// Value projection of the 828h scanner used by language descriptors. Byte input
// is supplied from the native mode32h VFS open. Strings, cursor and ownership
// replace native buffers/stream ABI. Tokens above1023 bytes throw, not overflow.
class NativeTextTokens {
public:
    explicit NativeTextTokens(std::vector<std::uint8_t> bytes,
        std::string additional_separators = {});
    const std::string& peek_00bee8e0();
    void accept_00bee800();
    const std::string& read_string_00bef020(bool& success);
    void skip_whitespace_00beedb0();
    bool quoted() const noexcept { return quoted_; }
    bool eof_at_token_start() const noexcept { return eof_at_token_start_; }
    std::uint32_t line() const noexcept { return line_; }
    const std::string& previous_token() const noexcept { return previous_token_; }
private:
    std::uint8_t peek_byte_00bee840();
    void advance_byte_00bee8c0();
    void append(std::uint8_t);
    bool separator(std::uint8_t) const noexcept;
    static bool whitespace(std::uint8_t) noexcept;
    std::vector<std::uint8_t> bytes_;
    std::size_t cursor_{};
    std::string separators_;
    std::string token_, previous_token_;
    std::uint8_t current_{}, previous_{};
    bool token_cached_{}, byte_cached_{}, eof_{}, eof_at_token_start_{}, quoted_{};
    std::uint32_t line_{};
};
}
