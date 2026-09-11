// Native evidence and bounded-domain limits: docs/OPTIONS_TOKEN_READER.md.
#include "bsp/options_token_reader.hpp"

#include <utility>

namespace bsp {

OptionsTokenReader::OptionsTokenReader(std::string text)
    : lexer_(std::move(text), ";")
{
}

const SceneToken& OptionsTokenReader::peek()
{
    return lexer_.peek();
}

SceneToken OptionsTokenReader::consume()
{
    return lexer_.next();
}

bool OptionsTokenReader::at_end()
{
    return recovered_end_ || lexer_.at_end();
}

void OptionsTokenReader::fail(OptionsTokenError error)
{
    last_error_ = error;
    ++failures_;
    recovered_end_ = lexer_.recover_after_failed_read();
}

std::string OptionsTokenReader::read_string(bool& ok)
{
    const SceneToken& token = peek();
    ok = !recovered_end_ && !token.text.empty();
    if (!ok) {
        fail(OptionsTokenError::expected_nonempty_string);
        return token.text;
    }
    last_error_ = OptionsTokenError::none;
    return consume().text;
}

std::int32_t OptionsTokenReader::read_int(bool& ok)
{
    std::int32_t value = 0;
    ok = scene_scan_int(peek().text, value);
    if (!ok) {
        fail(OptionsTokenError::expected_integer);
        return 0;
    }
    last_error_ = OptionsTokenError::none;
    consume();
    return value;
}

bool OptionsTokenReader::read_bool(bool& ok)
{
    std::int32_t value = 0;
    ok = scene_scan_int(peek().text, value) && (value == 0 || value == 1);
    if (!ok) {
        fail(OptionsTokenError::expected_boolean);
        return false;
    }
    last_error_ = OptionsTokenError::none;
    consume();
    return value == 1;
}

} // namespace bsp
