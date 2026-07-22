/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "englishtranslation.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

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

} // namespace

std::string EnglishTranslationDictionary::normalize(std::string_view word) {
    std::string result;
    result.reserve(word.size());
    for (const auto character : word) {
        const auto byte = static_cast<unsigned char>(character);
        if (byte >= 'A' && byte <= 'Z') {
            result.push_back(static_cast<char>(byte - 'A' + 'a'));
        } else {
            result.push_back(character);
        }
    }
    return result;
}

bool EnglishTranslationDictionary::load(std::istream &input) {
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
        if (separator == std::string_view::npos) {
            continue;
        }
        const auto word = trim(lineView.substr(0, separator));
        const auto translation = trim(lineView.substr(separator + 1));
        if (word.empty() || translation.empty()) {
            continue;
        }

        auto normalized = normalize(word);
        if (normalized.empty()) {
            continue;
        }
        // Keep the first definition in case an updater accidentally emits a
        // duplicate.  This makes output deterministic across updates.
        translations_.try_emplace(std::move(normalized), translation);
    }
    return static_cast<bool>(input) || input.eof();
}

void EnglishTranslationDictionary::clear() { translations_.clear(); }

std::string EnglishTranslationDictionary::lookup(std::string_view word) const {
    auto iter = translations_.find(normalize(word));
    if (iter == translations_.end()) {
        return {};
    }
    return iter->second;
}

} // namespace fcitx
