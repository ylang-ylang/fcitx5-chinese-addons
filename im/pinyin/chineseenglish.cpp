/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "chineseenglish.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace fcitx {

namespace {

std::string trim(std::string_view value) {
    while (!value.empty() &&
           std::isspace(static_cast<unsigned char>(value.front()))) {
        value.remove_prefix(1);
    }
    while (!value.empty() &&
           std::isspace(static_cast<unsigned char>(value.back()))) {
        value.remove_suffix(1);
    }
    return std::string(value);
}

std::vector<size_t> utf8Offsets(std::string_view value) {
    std::vector<size_t> result{0};
    for (size_t offset = 0; offset < value.size();) {
        const auto lead = static_cast<unsigned char>(value[offset]);
        size_t length = 0;
        if (lead < 0x80) {
            length = 1;
        } else if ((lead & 0xe0) == 0xc0) {
            length = 2;
        } else if ((lead & 0xf0) == 0xe0) {
            length = 3;
        } else if ((lead & 0xf8) == 0xf0) {
            length = 4;
        } else {
            return {};
        }
        if (offset + length > value.size()) {
            return {};
        }
        for (size_t index = 1; index < length; index++) {
            if ((static_cast<unsigned char>(value[offset + index]) & 0xc0) !=
                0x80) {
                return {};
            }
        }
        offset += length;
        result.push_back(offset);
    }
    return result;
}

bool isAttributiveParticle(std::string_view value) {
    return value == "的" || value == "地" || value == "得";
}

} // namespace

bool ChineseEnglishDictionary::load(std::istream &input) {
    clear();

    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const auto lineView = std::string_view(line);
        if (lineView.empty() || lineView.front() == '#') {
            continue;
        }

        const auto separator = lineView.find('\t');
        if (separator == std::string_view::npos ||
            lineView.find('\t', separator + 1) != std::string_view::npos) {
            continue;
        }
        auto chinese = trim(lineView.substr(0, separator));
        auto english = trim(lineView.substr(separator + 1));
        if (chinese.empty() || english.empty()) {
            continue;
        }

        auto &candidates = translations_[std::move(chinese)];
        if (std::ranges::find(candidates, english) == candidates.end()) {
            candidates.push_back(std::move(english));
        }
    }
    return static_cast<bool>(input) || input.eof();
}

void ChineseEnglishDictionary::clear() { translations_.clear(); }

size_t ChineseEnglishDictionary::StringHash::operator()(
    std::string_view value) const noexcept {
    return std::hash<std::string_view>{}(value);
}

const std::vector<std::string> *
ChineseEnglishDictionary::lookup(std::string_view chinese) const {
    auto iter = translations_.find(chinese);
    if (iter == translations_.end()) {
        return nullptr;
    }
    return &iter->second;
}

ChineseEnglishMatch
ChineseEnglishDictionary::lookupBest(std::string_view chinese) const {
    if (const auto *exact = lookup(chinese)) {
        return {exact, std::string(chinese)};
    }

    struct SuffixRule {
        std::string_view suffix;
        size_t minimumBaseCharacters;
    };
    static constexpr std::array suffixRules{
        SuffixRule{"的", 1}, SuffixRule{"地", 1}, SuffixRule{"得", 1},
        SuffixRule{"们", 1}, SuffixRule{"了", 1}, SuffixRule{"过", 1},
        SuffixRule{"着", 1}, SuffixRule{"中", 2},
    };

    // Chinese particles are productive and usually omitted from dictionary
    // headwords. Strip a short chain so forms such as 可选的 and 运行中的 can
    // reuse the lexical entries 可选 and 运行 without generating every form.
    auto normalized = chinese;
    for (size_t pass = 0; pass < 3; pass++) {
        bool stripped = false;
        for (const auto &[suffix, minimumBaseCharacters] : suffixRules) {
            if (!normalized.ends_with(suffix) ||
                normalized.size() == suffix.size()) {
                continue;
            }
            const auto base =
                normalized.substr(0, normalized.size() - suffix.size());
            const auto offsets = utf8Offsets(base);
            if (offsets.empty() || offsets.size() - 1 < minimumBaseCharacters) {
                continue;
            }
            normalized = base;
            stripped = true;
            if (const auto *entry = lookup(normalized)) {
                return {entry, std::string(normalized)};
            }
            break;
        }
        if (!stripped) {
            break;
        }
    }

    // The trigger is explicit and Pinyin candidates are capped at 35
    // characters, so longest-span fallback costs at most a few hundred O(1)
    // hash lookups. Prefer a same-length span followed by 的/地/得, which is
    // commonly the lexical adjective or adverb the user wants to translate.
    const auto offsets = utf8Offsets(normalized);
    if (offsets.size() <= 3) {
        return {};
    }
    const size_t characters = offsets.size() - 1;
    for (size_t length = characters - 1; length >= 2; length--) {
        ChineseEnglishMatch firstMatch;
        for (size_t start = 0; start + length <= characters; start++) {
            const auto end = start + length;
            const auto span = normalized.substr(offsets[start],
                                                offsets[end] - offsets[start]);
            const auto *entry = lookup(span);
            if (!entry) {
                continue;
            }
            ChineseEnglishMatch match{entry, std::string(span)};
            if (end < characters) {
                const auto following = normalized.substr(
                    offsets[end], offsets[end + 1] - offsets[end]);
                if (isAttributiveParticle(following)) {
                    return match;
                }
            }
            if (!firstMatch) {
                firstMatch = std::move(match);
            }
        }
        if (firstMatch) {
            return firstMatch;
        }
    }
    return {};
}

} // namespace fcitx
