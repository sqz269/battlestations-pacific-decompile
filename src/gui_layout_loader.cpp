#include "bsp/gui_layout_loader.hpp"

#include <cctype>
#include <cstdlib>

namespace bsp {
namespace {

// __stricmp's ASCII folding. The native comparison is the CRT's, which folds
// with the current locale; every literal it compares against is ASCII, so the
// difference cannot show on the type table.
char fold(char c) noexcept {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

bool iequals(std::string_view a, std::string_view b) noexcept {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (fold(a[i]) != fold(b[i])) {
            return false;
        }
    }
    return true;
}

// The order 00AA2490 tests, which is not the order of the ids. The sizes are
// the ECX immediates at each 00AA6560 case target +19h.
const GuiWidgetClass kClasses[] = {
    {GuiWidgetType::Icon, "Icon", 0x138, 0x00aa1410},
    {GuiWidgetType::Text, "Text", 0x1f4, 0x00aa1380},
    {GuiWidgetType::Group, "Group", 0x0ec, 0x00aa12f0},
    {GuiWidgetType::Progbar, "Progbar", 0x140, 0x00aa14a0},
    {GuiWidgetType::Line, "Line", 0x104, 0x00aa1530},
    {GuiWidgetType::Scrollbar, "Scrollbar", 0x0fc, 0x00aa15c0},
    {GuiWidgetType::Grid, "Grid", 0x124, 0x00aa1650},
    {GuiWidgetType::Model, "Model", 0x11c, 0x00aa16e0},
    {GuiWidgetType::Movie, "Movie", 0x130, 0x00aa1770},
    {GuiWidgetType::Listbox, "Listbox", 0, 0x00a9b7e0},
    {GuiWidgetType::AnimIcon, "AnimIcon", 0x144, 0x00aa2340},
    {GuiWidgetType::Curve, "Curve", 0x140, 0x00aa1800},
    {GuiWidgetType::Sound, "Sound", 0x108, 0x00aa1890},
    {GuiWidgetType::Button, "Button", 0x0fc, 0x00aa4440},
    {GuiWidgetType::ClipBox, "ClipBox", 0x10c, 0x00aa1920},
    {GuiWidgetType::Section, "Section", 0x12c, 0x00aa19b0},
    {GuiWidgetType::FrameBox, "FrameBox", 0x11c, 0x00aa1a40},
};

// The descriptor rows of 00AAA710, in call order. Every offset is the widget
// field address the native frame stages; every literal address is the pointer
// pushed with tag 0.
const GuiPropertyDescriptor kBaseProperties[] = {
    {"Pos", GuiValueTag::Vec3, 0x0c, 0x00cf168c},
    {"Size", GuiValueTag::Vec2, 0x20, 0x00cff278},
    {"Pivot", GuiValueTag::Vec2, 0x18, 0x00d5c228},
    {"Scale", GuiValueTag::Vec2, 0x28, 0x00ce60ac},
    {"Rotate", GuiValueTag::Float, 0x48, 0x00d5c220},
    {"Color", GuiValueTag::Color, 0x50, 0x00ce93f8},
    {"LowColor", GuiValueTag::Color, 0xa4, 0x00d5c1f4},
    {"HighColor", GuiValueTag::Color, 0xb4, 0x00d5c1ec},
    {"BlendFactor", GuiValueTag::Float, 0xc4, 0x00d5c1fc},
    {"WideScreenAlign", GuiValueTag::String, 0xe0, 0x00ce92e4},
    {"Visible", GuiValueTag::Bool, 0xe4, 0x00d5c1e4},
    {"MouseBlock", GuiValueTag::Bool, 0x84, 0x00d5c1d8},
    {"MouseHit", GuiValueTag::Bool, 0x78, 0x00d5c1cc},
};

// ---------------------------------------------------------------------------
// Reading one property out of an evaluated table
// ---------------------------------------------------------------------------

// A Lua number reaching a float field. The script side is a double; the field
// is a float, so the narrowing here is the one the native store performs.
float to_float(const GuiValue& value, float fallback) noexcept {
    if (value.kind() == GuiValue::Kind::Number) {
        return static_cast<float>(value.number());
    }
    return fallback;
}

// Lua truth as the native bool field sees it: the byte is written from the
// script's boolean, and any other kind leaves the default in place.
bool to_bool(const GuiValue& value, bool fallback) noexcept {
    if (value.kind() == GuiValue::Kind::Boolean) {
        return value.boolean();
    }
    return fallback;
}

// The vector properties arrive as array-part tables: Pos is three entries,
// Pivot/Size/Scale two, the colours four. A table that is too short leaves the
// remaining lanes at their defaults, which is what the native reader does when
// the visitor cannot fill a lane.
void read_lanes(const GuiValue& value, float* out, std::size_t count) noexcept {
    if (!value.is_table()) {
        return;
    }
    const GuiTable& table = *value.table();
    const std::size_t n = table.array.size() < count ? table.array.size() : count;
    for (std::size_t i = 0; i < n; ++i) {
        out[i] = to_float(table.array[i], out[i]);
    }
}

GuiWideScreenAlign align_from_string(const GuiValue* value) noexcept {
    // 00AAAB27 compares "Left" (00CE92E4) and 00AAAB48 compares "Right"
    // (00CE92DC) through 00425850, whose stack argument the decompiler drops.
    // Anything else, a missing key included, leaves 0.
    if (value == nullptr || value->kind() != GuiValue::Kind::String) {
        return GuiWideScreenAlign::None;
    }
    if (iequals(value->string(), "Left")) {
        return GuiWideScreenAlign::ShiftNegativeX;
    }
    if (iequals(value->string(), "Right")) {
        return GuiWideScreenAlign::ShiftPositiveX;
    }
    return GuiWideScreenAlign::None;
}

// ---------------------------------------------------------------------------
// The static page-table parser
// ---------------------------------------------------------------------------

struct Token {
    enum class Kind {
        End, LBrace, RBrace, LBracket, RBracket, Comma, Equals, String, Number,
        Name,
    };
    Kind kind{Kind::End};
    std::string text;
    double number{0.0};
    std::size_t offset{0};
};

class Lexer {
public:
    explicit Lexer(std::string_view text) : text_(text) {}

    bool next(Token& out, std::string& error) {
        skip_trivia();
        out = Token{};
        out.offset = pos_;
        if (pos_ >= text_.size()) {
            out.kind = Token::Kind::End;
            return true;
        }
        const char c = text_[pos_];
        switch (c) {
            case '{': ++pos_; out.kind = Token::Kind::LBrace; return true;
            case '}': ++pos_; out.kind = Token::Kind::RBrace; return true;
            case '[': ++pos_; out.kind = Token::Kind::LBracket; return true;
            case ']': ++pos_; out.kind = Token::Kind::RBracket; return true;
            case ',':
            case ';': ++pos_; out.kind = Token::Kind::Comma; return true;
            case '=': ++pos_; out.kind = Token::Kind::Equals; return true;
            default: break;
        }
        if (c == '"') {
            return lex_string(out, error);
        }
        if (c == '-' || (c >= '0' && c <= '9')) {
            return lex_number(out, error);
        }
        if (c == '_' || std::isalpha(static_cast<unsigned char>(c)) != 0) {
            const std::size_t start = pos_;
            while (pos_ < text_.size()) {
                const char d = text_[pos_];
                if (d != '_' && std::isalnum(static_cast<unsigned char>(d)) == 0) {
                    break;
                }
                ++pos_;
            }
            out.kind = Token::Kind::Name;
            out.text = std::string(text_.substr(start, pos_ - start));
            return true;
        }
        error = "unexpected character at offset " + std::to_string(pos_);
        return false;
    }

private:
    void skip_trivia() {
        while (pos_ < text_.size()) {
            const char c = text_[pos_];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                ++pos_;
                continue;
            }
            if (c == '-' && pos_ + 1 < text_.size() && text_[pos_ + 1] == '-') {
                while (pos_ < text_.size() && text_[pos_] != '\n') {
                    ++pos_;
                }
                continue;
            }
            return;
        }
    }

    bool lex_string(Token& out, std::string& error) {
        const std::size_t start = ++pos_;  // past the opening quote
        while (pos_ < text_.size() && text_[pos_] != '"') {
            if (text_[pos_] == '\n') {
                error = "unterminated string at offset " + std::to_string(start);
                return false;
            }
            ++pos_;
        }
        if (pos_ >= text_.size()) {
            error = "unterminated string at offset " + std::to_string(start);
            return false;
        }
        out.kind = Token::Kind::String;
        out.text = std::string(text_.substr(start, pos_ - start));
        ++pos_;  // past the closing quote
        return true;
    }

    bool lex_number(Token& out, std::string& error) {
        const std::size_t start = pos_;
        if (text_[pos_] == '-') {
            ++pos_;
        }
        bool digits = false;
        while (pos_ < text_.size()) {
            const char d = text_[pos_];
            if (d >= '0' && d <= '9') {
                digits = true;
                ++pos_;
                continue;
            }
            if (d == '.' || d == 'e' || d == 'E') {
                ++pos_;
                continue;
            }
            if ((d == '+' || d == '-') && pos_ > start &&
                (text_[pos_ - 1] == 'e' || text_[pos_ - 1] == 'E')) {
                ++pos_;
                continue;
            }
            break;
        }
        if (!digits) {
            error = "malformed number at offset " + std::to_string(start);
            return false;
        }
        const std::string body(text_.substr(start, pos_ - start));
        out.kind = Token::Kind::Number;
        out.text = body;
        out.number = std::strtod(body.c_str(), nullptr);
        return true;
    }

    std::string_view text_;
    std::size_t pos_{0};
};

class Parser {
public:
    explicit Parser(std::string_view text) : lexer_(text) {}

    bool parse_chunk(GuiTable& out, std::string& error) {
        if (!advance(error)) {
            return false;
        }
        while (current_.kind != Token::Kind::End) {
            if (!parse_statement(out, error)) {
                return false;
            }
        }
        return true;
    }

private:
    bool advance(std::string& error) { return lexer_.next(current_, error); }

    bool expect(Token::Kind kind, const char* what, std::string& error) {
        if (current_.kind != kind) {
            error = std::string("expected ") + what + " at offset " +
                    std::to_string(current_.offset);
            return false;
        }
        return advance(error);
    }

    // stmt := Name ('[' key ']')* '=' value
    bool parse_statement(GuiTable& out, std::string& error) {
        if (current_.kind != Token::Kind::Name) {
            error = "expected a statement at offset " +
                    std::to_string(current_.offset);
            return false;
        }
        const std::string root = current_.text;
        if (!advance(error)) {
            return false;
        }
        std::vector<std::string> path;
        while (current_.kind == Token::Kind::LBracket) {
            std::string key;
            if (!parse_bracket_key(key, error)) {
                return false;
            }
            path.push_back(key);
        }
        if (!expect(Token::Kind::Equals, "'='", error)) {
            return false;
        }
        GuiValue value;
        if (!parse_value(value, error)) {
            return false;
        }
        if (root != kGuiScreenTableName) {
            return true;  // a helper global; the visitor never sees it
        }
        if (path.empty()) {
            if (value.is_table()) {
                out = *value.table();
            }
            return true;
        }
        assign_path(out, path, std::move(value));
        return true;
    }

    bool parse_bracket_key(std::string& key, std::string& error) {
        if (!advance(error)) {  // past '['
            return false;
        }
        if (current_.kind == Token::Kind::String ||
            current_.kind == Token::Kind::Name ||
            current_.kind == Token::Kind::Number) {
            key = current_.text;
        } else {
            error = "expected a key at offset " + std::to_string(current_.offset);
            return false;
        }
        const bool numeric = current_.kind == Token::Kind::Number;
        if (!advance(error)) {
            return false;
        }
        if (!expect(Token::Kind::RBracket, "']'", error)) {
            return false;
        }
        numeric_key_ = numeric;
        return true;
    }

    // A subscripted assignment writes one path into the page table, creating
    // the intermediate tables the script relies on already existing.
    static void assign_path(
        GuiTable& root, const std::vector<std::string>& path, GuiValue value) {
        GuiTable* cursor = &root;
        std::vector<std::shared_ptr<GuiTable>> owned;
        for (std::size_t i = 0; i + 1 < path.size(); ++i) {
            GuiTable* next = nullptr;
            for (auto& entry : cursor->named) {
                if (entry.first == path[i] && entry.second.is_table()) {
                    // The stored table is shared and const; replace it with a
                    // mutable copy so the deeper write lands in the page.
                    auto copy = std::make_shared<GuiTable>(*entry.second.table());
                    next = copy.get();
                    entry.second = GuiValue(std::shared_ptr<const GuiTable>(copy));
                    break;
                }
            }
            if (next == nullptr) {
                auto fresh = std::make_shared<GuiTable>();
                next = fresh.get();
                cursor->named.emplace_back(
                    path[i], GuiValue(std::shared_ptr<const GuiTable>(fresh)));
            }
            cursor = next;
        }
        for (auto& entry : cursor->named) {
            if (entry.first == path.back()) {
                entry.second = std::move(value);
                return;
            }
        }
        cursor->named.emplace_back(path.back(), std::move(value));
    }

    bool parse_value(GuiValue& out, std::string& error) {
        switch (current_.kind) {
            case Token::Kind::LBrace: {
                auto table = std::make_shared<GuiTable>();
                if (!parse_table(*table, error)) {
                    return false;
                }
                out = GuiValue(std::shared_ptr<const GuiTable>(table));
                return true;
            }
            case Token::Kind::String: {
                out = GuiValue(current_.text);
                return advance(error);
            }
            case Token::Kind::Number: {
                out = GuiValue(current_.number);
                return advance(error);
            }
            case Token::Kind::Name: {
                const std::string& word = current_.text;
                if (word == "true") {
                    out = GuiValue(true);
                } else if (word == "false") {
                    out = GuiValue(false);
                } else if (word == "nil") {
                    out = GuiValue();
                } else {
                    error = "unsupported expression '" + word + "' at offset " +
                            std::to_string(current_.offset);
                    return false;
                }
                return advance(error);
            }
            default:
                break;
        }
        error = "expected a value at offset " + std::to_string(current_.offset);
        return false;
    }

    bool parse_table(GuiTable& out, std::string& error) {
        if (!advance(error)) {  // past '{'
            return false;
        }
        while (true) {
            if (current_.kind == Token::Kind::RBrace) {
                return advance(error);
            }
            if (current_.kind == Token::Kind::Comma) {
                if (!advance(error)) {
                    return false;
                }
                continue;
            }
            if (current_.kind == Token::Kind::End) {
                error = "unterminated table";
                return false;
            }
            std::string key;
            bool keyed = false;
            bool numeric = false;
            if (current_.kind == Token::Kind::LBracket) {
                if (!parse_bracket_key(key, error)) {
                    return false;
                }
                if (!expect(Token::Kind::Equals, "'='", error)) {
                    return false;
                }
                keyed = true;
                numeric = numeric_key_;
            } else if (current_.kind == Token::Kind::Name && peek_is_equals()) {
                key = current_.text;
                if (!advance(error) ||
                    !expect(Token::Kind::Equals, "'='", error)) {
                    return false;
                }
                keyed = true;
            }
            GuiValue value;
            if (!parse_value(value, error)) {
                return false;
            }
            // An integer key is an array entry: 00AAA710 skips those, and the
            // vector properties read their lanes out of exactly this list.
            if (keyed && !numeric) {
                out.named.emplace_back(key, std::move(value));
            } else {
                out.array.push_back(std::move(value));
            }
        }
    }

    // One-token lookahead, needed only to tell `Name =` from a bare value.
    bool peek_is_equals() {
        Lexer probe = lexer_;
        Token token;
        std::string ignored;
        return probe.next(token, ignored) && token.kind == Token::Kind::Equals;
    }

    Lexer lexer_;
    Token current_{};
    bool numeric_key_{false};
};

}  // namespace

// ---------------------------------------------------------------------------

const GuiValue* GuiTable::find(std::string_view key) const noexcept {
    for (const auto& entry : named) {
        if (entry.first == key) {
            return &entry.second;
        }
    }
    return nullptr;
}

const GuiWidgetClass* gui_widget_class_table(std::size_t& count) noexcept {
    count = sizeof(kClasses) / sizeof(kClasses[0]);
    return kClasses;
}

const GuiWidgetClass* gui_widget_class(GuiWidgetType type) noexcept {
    for (const auto& row : kClasses) {
        if (row.type == type) {
            return &row;
        }
    }
    return nullptr;
}

GuiWidgetType gui_widget_type_for_key_00aa2490(std::string_view key) noexcept {
    // 004BCB80 with 00CE7890 ('_'): the last separator, or none at all, in
    // which case the whole key is the suffix.
    const std::size_t cut = key.rfind('_');
    const std::string_view suffix =
        cut == std::string_view::npos ? key : key.substr(cut + 1);
    for (const auto& row : kClasses) {
        if (iequals(suffix, row.suffix)) {
            return row.type;
        }
    }
    return GuiWidgetType::None;
}

std::string gui_page_model_path_00aa5840(std::string_view page_name) {
    return std::string(page_name) + ".mmod";
}

std::string gui_page_script_path_00ac5600(std::string_view script_name) {
    return "interface/" + std::string(script_name) + ".lua";
}

const GuiPropertyDescriptor* gui_base_property_descriptors(
    std::size_t& count) noexcept {
    count = sizeof(kBaseProperties) / sizeof(kBaseProperties[0]);
    return kBaseProperties;
}

bool parse_gui_page_table(
    std::string_view text, GuiTable& out, std::string& error) {
    out = GuiTable{};
    error.clear();
    Parser parser(text);
    return parser.parse_chunk(out, error);
}

void bind_widget_properties_00aaa710(
    const GuiTable& table, GuiLayoutWidget& widget, bool is_page_root,
    bool widescreen_enabled) noexcept {
    GuiWidgetTransform& xf = widget.transform;

    if (const GuiValue* value = table.find("Pos")) {
        float lanes[3] = {xf.position.x, xf.position.y, xf.position.z};
        read_lanes(*value, lanes, 3);
        xf.position.x = lanes[0];
        xf.position.y = lanes[1];
        xf.position.z = lanes[2];
    }
    if (const GuiValue* value = table.find("Size")) {
        float lanes[2] = {xf.size.width, xf.size.height};
        read_lanes(*value, lanes, 2);
        xf.size.width = lanes[0];
        xf.size.height = lanes[1];
    }
    if (const GuiValue* value = table.find("Pivot")) {
        float lanes[2] = {xf.pivot_x, xf.pivot_y};
        read_lanes(*value, lanes, 2);
        xf.pivot_x = lanes[0];
        xf.pivot_y = lanes[1];
    }
    if (const GuiValue* value = table.find("Scale")) {
        float lanes[2] = {xf.scale_x, xf.scale_y};
        read_lanes(*value, lanes, 2);
        xf.scale_x = lanes[0];
        xf.scale_y = lanes[1];
    }
    if (const GuiValue* value = table.find("Rotate")) {
        xf.rotate = to_float(*value, xf.rotate);
    }
    if (const GuiValue* value = table.find("Color")) {
        read_lanes(*value, widget.color, 4);
    }
    xf.alpha = widget.color[3];  // +5Ch is the alpha lane of the same property
    if (const GuiValue* value = table.find("LowColor")) {
        read_lanes(*value, widget.low_color, 4);
    }
    if (const GuiValue* value = table.find("HighColor")) {
        read_lanes(*value, widget.high_color, 4);
    }
    if (const GuiValue* value = table.find("BlendFactor")) {
        widget.blend_factor = to_float(*value, widget.blend_factor);
    }

    xf.widescreen_align = align_from_string(table.find("WideScreenAlign"));

    // 00AAABA6: the authored X is cached at +8h before the fixup, so a later
    // layout pass recomputes +0Ch from the same source this read used.
    xf.authored_x = xf.position.x;
    xf.position.x = widescreen_local_x(
        xf.authored_x, xf.widescreen_align, widescreen_enabled, xf.position.x);

    // 00AAABD6: vtable +5Ch is the type tag, and the page root (1) skips
    // Visible entirely, which leaves it at the constructor's value.
    if (!is_page_root) {
        if (const GuiValue* value = table.find("Visible")) {
            widget.visible = to_bool(*value, widget.visible);
        }
    }

    if (const GuiValue* value = table.find("MouseBlock")) {
        xf.mouse_block = to_bool(*value, false);
    } else {
        xf.mouse_block = false;
    }
    if (xf.mouse_block) {
        // 00AAAC84: a blocking widget is hit-testable without a lookup.
        xf.mouse_hit = true;
    } else if (const GuiValue* value = table.find("MouseHit")) {
        xf.mouse_hit = to_bool(*value, false);
    } else {
        xf.mouse_hit = false;
    }
}

GuiLayoutWidget* find_child_by_name_00aa7e00(
    GuiLayoutWidget& parent, std::string_view name) noexcept {
    for (const auto& child : parent.children) {
        if (child->node_id == 0) {
            continue;  // 00AA7E31: a child with no scene node is skipped
        }
        if (iequals(child->key, name)) {
            return child.get();
        }
    }
    return nullptr;
}

GuiLayoutWidget* find_descendant_by_name(
    GuiLayoutWidget& parent, std::string_view name) noexcept {
    if (GuiLayoutWidget* direct = find_child_by_name_00aa7e00(parent, name)) {
        return direct;
    }
    for (const auto& child : parent.children) {
        if (GuiLayoutWidget* hit = find_descendant_by_name(*child, name)) {
            return hit;
        }
    }
    return nullptr;
}

void build_widget_children_00aaa710(
    const GuiTable& table, GuiLayoutWidget& widget, GuiLayoutHost& host) {
    for (const auto& entry : table.named) {
        const GuiWidgetType type = gui_widget_type_for_key_00aa2490(entry.first);
        if (type == GuiWidgetType::None) {
            continue;  // 00AAAD7F: not a widget key, so it stayed a property
        }
        auto child = std::make_unique<GuiLayoutWidget>();
        child->key = entry.first;
        child->type = type;
        child->transform.type_id = static_cast<std::int32_t>(type);
        child->node_id = host.create_widget_node(entry.first);

        child->parent = &widget;
        child->transform.parent = &widget.transform;
        if (child->node_id != 0 && widget.node_id != 0) {
            host.set_node_parent(child->node_id, widget.node_id);
        }
        GuiLayoutWidget& stored = *child;
        widget.transform.children.push_back(&stored.transform);
        widget.children.push_back(std::move(child));

        host.on_widget_constructed(stored);
        if (entry.second.is_table()) {
            const GuiTable& body = *entry.second.table();
            stored.source = &body;
            bind_widget_properties_00aaa710(
                body, stored, false, host.widescreen_enabled());
            build_widget_children_00aaa710(body, stored, host);
        }
        host.on_widget_loaded(stored);
    }
}

GuiLayoutPage* GuiPageRegistry::find_00aa3140(std::string_view name) noexcept {
    for (const auto& page : pages_) {
        if (iequals(page->name, name)) {
            return page.get();
        }
    }
    return nullptr;
}

GuiLayoutPage* GuiPageRegistry::register_00aa52a0(
    std::unique_ptr<GuiLayoutPage> page) {
    GuiLayoutPage* stored = page.get();
    // 00AA5310: the scan stops at the first page whose priority is strictly
    // greater, so equal priorities keep the order they were registered in.
    auto slot = pages_.begin();
    while (slot != pages_.end() && (*slot)->priority <= stored->priority) {
        ++slot;
    }
    pages_.insert(slot, std::move(page));
    return stored;
}

GuiLayoutPage* load_gui_page_00aa5840(
    GuiPageRegistry& registry, GuiLayoutHost& host, const std::string& name,
    std::int32_t screen_flag, bool add_reference) {
    static_cast<void>(screen_flag);  // reaches screen +120h; meaning unrecovered

    if (GuiLayoutPage* existing = registry.find_00aa3140(name)) {
        if (add_reference) {
            ++existing->reference_count;  // 00AA5990, through 00CE221C
        }
        return existing;
    }

    auto page = std::make_unique<GuiLayoutPage>();
    page->name = name;

    // 00AA58B8: the model branch is taken only when the VFS already has the
    // name. When it does not, 00AA58FA builds a plain 188h-byte object instead
    // and the page still loads; the model is a backdrop, not the layout.
    const std::string model_path = gui_page_model_path_00aa5840(name);
    if (host.vfs_name_exists(model_path)) {
        page->model_backed = host.instantiate_page_model(model_path);
    }

    auto root = std::make_unique<GuiLayoutWidget>();
    root->key = name;
    root->type = GuiWidgetType::Screen;
    root->transform.type_id = static_cast<std::int32_t>(GuiWidgetType::Screen);
    root->node_id = host.create_widget_node(name);

    const std::shared_ptr<const GuiTable> table = host.evaluate_page_script(name);
    if (table) {
        page->script_evaluated = true;
        root->source = table.get();
        bind_widget_properties_00aaa710(
            *table, *root, true, host.widescreen_enabled());
        if (const GuiValue* priority = table->find("Priority")) {
            if (priority->kind() == GuiValue::Kind::Number) {
                page->priority = static_cast<std::int32_t>(priority->number());
            }
        }
        build_widget_children_00aaa710(*table, *root, host);
    }
    page->root = std::move(root);
    // The page keeps the table alive: every widget's `source` points into it.
    page->script_table = table;
    return registry.register_00aa52a0(std::move(page));
}

}  // namespace bsp
