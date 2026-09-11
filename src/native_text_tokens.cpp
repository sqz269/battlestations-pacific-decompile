#include "bsp/native_text_tokens.hpp"
#include <stdexcept>
#include <utility>

namespace bsp {
NativeTextTokens::NativeTextTokens(std::vector<std::uint8_t> bytes,
    std::string additional_separators) : bytes_(std::move(bytes)),
    separators_(";" + additional_separators) {
    if (additional_separators.find('\0') != std::string::npos)
        throw std::invalid_argument("Native scanner separators require a C string");
}
bool NativeTextTokens::whitespace(std::uint8_t c) noexcept {
    // PTR00e15334 ->00d15f2c: space, TAB, CR, LF, comma. strchr also finds NUL.
    return c == 0 || c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ',';
}
bool NativeTextTokens::separator(std::uint8_t c) const noexcept {
    return c == 0 || separators_.find(static_cast<char>(c)) != std::string::npos;
}
std::uint8_t NativeTextTokens::peek_byte_00bee840() {
    if (!byte_cached_) {
        if (cursor_ == bytes_.size()) {
            eof_ = true;
            current_ = 0;
        } else {
            current_ = bytes_[cursor_++];
            byte_cached_ = true;
            if (current_ == '\n') ++line_;
        }
    }
    return current_;
}
void NativeTextTokens::advance_byte_00bee8c0() {
    previous_ = current_;
    byte_cached_ = false;
    peek_byte_00bee840();
}
void NativeTextTokens::append(std::uint8_t c) {
    if (c == 0) return; // strncat of a one-byte NUL appends no bytes.
    if (token_.size() == 1023)
        throw std::length_error("Native scanner token exceeds its1024-byte buffer");
    token_.push_back(static_cast<char>(c));
}
const std::string& NativeTextTokens::peek_00bee8e0() {
    if (token_cached_) return token_;
    quoted_ = false;
    eof_at_token_start_ = eof_;
    token_.clear();
    peek_byte_00bee840();
    while (!eof_) {
        while (!eof_ && whitespace(peek_byte_00bee840())) advance_byte_00bee8c0();
        if (eof_) break;
        const auto first = current_;
        advance_byte_00bee8c0();
        if (first == '/' && current_ == '/') {
            while (!eof_ && current_ != '\n') advance_byte_00bee8c0();
            advance_byte_00bee8c0();
            continue;
        }
        if (first == '/' && current_ == '*') {
            while (!eof_ && !(previous_ == '*' && current_ == '/')) advance_byte_00bee8c0();
            advance_byte_00bee8c0();
            continue;
        }
        if (separator(first)) {
            append(first);
            break;
        }
        if (first == '"') {
            quoted_ = true;
            while (!eof_) {
                if (peek_byte_00bee840() == '"') {
                    advance_byte_00bee8c0();
                    break;
                }
                append(current_);
                advance_byte_00bee8c0();
            }
            break;
        }
        append(first);
        while (!eof_ && !whitespace(peek_byte_00bee840()) && !separator(current_)) {
            append(current_);
            advance_byte_00bee8c0();
        }
        break;
    }
    token_cached_ = true;
    return token_;
}
void NativeTextTokens::accept_00bee800() {
    previous_token_ = token_;
    token_cached_ = false;
}
void NativeTextTokens::skip_whitespace_00beedb0() {
    if (!eof_) {
        do {
            if (!whitespace(peek_byte_00bee840())) break;
            if (current_ == 0) break; // native strchr("",current).
            advance_byte_00bee8c0();
        } while (!eof_);
    }
    eof_at_token_start_ = eof_;
}
const std::string& NativeTextTokens::read_string_00bef020(bool& success) {
    peek_00bee8e0();
    if (!eof_at_token_start_ && (!token_.empty() || quoted_) && !token_.empty()) {
        accept_00bee800();
        success = true;
        return token_;
    }
    skip_whitespace_00beedb0();
    success = false;
    return peek_00bee8e0();
}
}
