#pragma once

#include "bsp/scene_file.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace bsp {

enum class OptionsTokenError {
    none,
    expected_nonempty_string,
    expected_integer,
    expected_boolean,
};

// Options-text view of the tokenizer constructed by 008d9f20. The native
// receives a retained stream and buffers its bytes; this interface receives
// those bytes directly. It is not a native-layout or ABI replacement.
class OptionsTokenReader {
public:
    explicit OptionsTokenReader(std::string text);

    const SceneToken& peek();
    SceneToken consume();
    bool at_end();

    // 008d99f0: rejects even an empty quoted string. Failure returns the
    // current token text, sets ok=false, and keeps the token cached.
    std::string read_string(bool& ok);
    // 008d9ad0: sscanf("%d") prefix conversion; failure returns zero.
    std::int32_t read_int(bool& ok);
    // 008d9a80 / 008d8e50: only decimal-prefix values 0 and 1 succeed.
    // Settings read through ReadInt followed by !=0 must use read_int.
    bool read_bool(bool& ok);

    // Failure runs 008d8f70's following-whitespace recovery without consuming
    // the cached token. at_end() can then be true while peek().text is nonempty.
    OptionsTokenError last_error() const noexcept { return last_error_; }
    bool failed() const noexcept { return last_error_ != OptionsTokenError::none; }
    std::size_t failures() const noexcept { return failures_; }

private:
    void fail(OptionsTokenError error);

    SceneLexer lexer_;
    bool recovered_end_{false};
    OptionsTokenError last_error_{OptionsTokenError::none};
    std::size_t failures_{0};
};

} // namespace bsp
