#include "bsp/warning_messages.hpp"

#include <stdexcept>
#include <utility>

namespace bsp {
namespace {

// Native LuaObjects are tracked stack values. Reuse the existing host's
// registry-reference adaptation and release every borrowed iteration result.
class WarningLuaReference final {
public:
    WarningLuaReference(GuiLuaHost& host, GuiLuaRef reference)
        : host_(host), reference_(reference) {}
    ~WarningLuaReference() { host_.release(reference_); }
    WarningLuaReference(const WarningLuaReference&) = delete;
    WarningLuaReference& operator=(const WarningLuaReference&) = delete;
    const GuiLuaRef& get() const noexcept { return reference_; }
private:
    GuiLuaHost& host_;
    GuiLuaRef reference_;
};

void require_table(GuiLuaHost& lua, const GuiLuaRef& table) {
    if (lua.type_of(table) != GuiLuaType::Table) {
        // Host diagnostic for violation of the native lua_next precondition.
        throw std::invalid_argument("warning data iteration requires a Lua table");
    }
}

std::string required_string(GuiLuaHost& lua, const GuiLuaRef& value) {
    const char* text = lua.to_string(value);
    if (text == nullptr) {
        // The native strlen loops dereference this pointer without a check.
        throw std::invalid_argument("warning data requires a string-convertible value");
    }
    return text; // native strlen, including its embedded-NUL truncation
}

template<class Visit>
void each_warning_lua_pair(GuiLuaHost& lua, const GuiLuaRef& table, Visit visit) {
    require_table(lua, table);
    GuiLuaRef key{}, value{};
    bool restart = true;
    while (lua.next(table, key, value, restart)) {
        restart = false;
        WarningLuaReference own_key(lua, key);
        WarningLuaReference own_value(lua, value);
        visit(key, value);
    }
}

} // namespace

void load_warning_message_tree_0097f570(WarningMessageMap& destination,
    GuiLuaHost& lua, const GuiLuaRef& table, const std::string& prefix) {
    std::vector<std::string> values;
    each_warning_lua_pair(lua, table, [&](const GuiLuaRef& key, const GuiLuaRef& value) {
        if (lua.type_of(value) == GuiLuaType::Table) {
            const std::string nested_prefix = prefix + required_string(lua, key);
            load_warning_message_tree_0097f570(destination, lua, value, nested_prefix);
        } else {
            values.push_back(required_string(lua, value));
        }
    });
    if (!values.empty()) {
        destination[prefix] = std::move(values); // 008EC460 then 00506C80
    }
}

void load_warning_escape_tree_00979730(WarningEscapeTable& destination,
    GuiLuaHost& lua, const GuiLuaRef& table, const std::string& prefix) {
    each_warning_lua_pair(lua, table, [&](const GuiLuaRef& key, const GuiLuaRef& value) {
        const std::string nested_prefix = prefix + required_string(lua, key);
        if (lua.type_of(value) == GuiLuaType::Table) {
            load_warning_escape_tree_00979730(destination, lua, value, nested_prefix);
        } else {
            const char* text = lua.to_string(value);
            destination.replacements[nested_prefix] = text == nullptr ? "" : text;
        }
    });
}

void load_warning_escape_table_00979990(WarningMessageTables& manager,
    WarningEscapeTable& destination, GuiLuaHost& lua, const GuiLuaRef& table) {
    each_warning_lua_pair(lua, table, [&](const GuiLuaRef& key, const GuiLuaRef& value) {
        const std::string prefix = required_string(lua, key);
        manager.escape_prefixes += prefix; // 00979A5E..00979A8C
        destination.prefixes += prefix; // 00979A91..00979AAB
        load_warning_escape_tree_00979730(destination, lua, value, prefix);
    });
}

void load_warning_tables_009870a0(WarningMessageTables& destination,
    GuiLuaHost& lua, const GuiLuaRef& warnings) {
    require_table(lua, warnings);
    WarningLuaReference escapecharacters(lua, lua.get_by_name(warnings, "escapecharacters"));
    require_table(lua, escapecharacters.get());
    {
        WarningLuaReference entity(lua, lua.get_by_name(escapecharacters.get(), "entity"));
        load_warning_escape_table_00979990(destination, destination.entity, lua, entity.get());
    }
    {
        WarningLuaReference section(lua, lua.get_by_name(escapecharacters.get(), "playerunit_section"));
        load_warning_escape_table_00979990(destination, destination.playerunit_section, lua, section.get());
    }
    WarningLuaReference messages(lua, lua.get_by_name(warnings, "messages"));
    load_warning_message_tree_0097f570(destination.messages, lua, messages.get(), "");
}

bool warning_message_id_known_0096dbb0(
    const WarningMessageTables& tables, std::string_view id) {
    return tables.messages.find(id) != tables.messages.end();
}

} // namespace bsp
