#pragma once
#include <set>
#include <string>
#include <vector>

std::vector<std::string_view> SplitIntoWords(std::string_view text);

using TransparentStringSet = std::set<std::string, std::less<>>;

template <typename StringContainer>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string, std::less<>> non_empty_strings;
    for (std::string_view str : strings) {
        if (!str.empty()) {
            non_empty_strings.emplace(str);
        }
    }
    return non_empty_strings;
}