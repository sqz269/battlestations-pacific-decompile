#include "bsp/resource_parser_map.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
bool ResourceParserMap::NameLess::operator()(const std::string& left,
    const std::string& right) const noexcept {
    //00b7df40/00b80290: a truly empty counted string sorts before every
    // nonempty string, even one whose first byte is NUL. For nonempty strings
    // only the C-string prefix participates in the CRT comparison.
    if (left.empty()) return !right.empty();
    if (right.empty()) return false;
    return _stricmp(left.c_str(), right.c_str()) < 0;
}

ResourceParserMapInsertResult ResourceParserMap::insert(const std::string& name,
    StructuredResourceParser* parser) {
    const auto prior = entries_.find(name);
    if (prior != entries_.end()) return {prior->second, false};
    //00b7fd30 rejects only a new insertion at or above this native count.
    if (entries_.size() >= 0x15555554u)
        throw std::length_error("map/set<T> too long");
    const auto inserted = entries_.emplace(name, parser);
    return {inserted.first->second, inserted.second};
}

StructuredResourceParser* ResourceParserMap::find(const std::string& name) const {
    const auto entry = entries_.find(name);
    return entry == entries_.end() ? nullptr : entry->second;
}

bool ResourceParserMap::erase(const std::string& name) {
    return entries_.erase(name) != 0;
}

void ResourceParserMap::clear() noexcept {
    entries_.clear();
}
}
