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

std::string EnglishTranslationMeaning::text() const {
    if (translation.empty()) {
        return {};
    }
    if (label.empty()) {
        return translation;
    }
    return label + " " + translation;
}

std::string EnglishTranslationEntry::comment(size_t maximumMeanings) const {
    if (meanings.empty() || maximumMeanings == 0) {
        return {};
    }

    std::string result = "(";
    const auto count = std::min(maximumMeanings, meanings.size());
    for (size_t index = 0; index < count; index++) {
        if (index != 0) {
            result += "；";
        }
        result += meanings[index].text();
    }
    result += ")";
    return result;
}

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

        const auto firstSeparator = lineView.find('\t');
        if (firstSeparator == std::string_view::npos) {
            continue;
        }
        const auto word = trim(lineView.substr(0, firstSeparator));
        auto value = lineView.substr(firstSeparator + 1);
        const auto secondSeparator = value.find('\t');

        std::string label;
        std::string translation;
        std::string phonetic;
        if (secondSeparator == std::string_view::npos) {
            translation = trim(value);
        } else {
            label = trim(value.substr(0, secondSeparator));
            value.remove_prefix(secondSeparator + 1);
            const auto thirdSeparator = value.find('\t');
            if (thirdSeparator == std::string_view::npos) {
                translation = trim(value);
            } else {
                translation = trim(value.substr(0, thirdSeparator));
                phonetic = trim(value.substr(thirdSeparator + 1));
            }
        }
        if (word.empty() || translation.empty()) {
            continue;
        }

        auto normalized = normalize(word);
        if (normalized.empty()) {
            continue;
        }
        auto &entry = translations_[std::move(normalized)];
        if (entry.phonetic.empty() && !phonetic.empty()) {
            entry.phonetic = std::move(phonetic);
        }
        const auto duplicate =
            std::ranges::any_of(entry.meanings, [&](const auto &meaning) {
                return meaning.label == label &&
                       meaning.translation == translation;
            });
        if (!duplicate) {
            entry.meanings.push_back(EnglishTranslationMeaning{
                std::move(label), std::move(translation)});
        }
    }
    return static_cast<bool>(input) || input.eof();
}

void EnglishTranslationDictionary::clear() { translations_.clear(); }

const EnglishTranslationEntry *
EnglishTranslationDictionary::lookup(std::string_view word) const {
    auto iter = translations_.find(normalize(word));
    if (iter == translations_.end()) {
        return nullptr;
    }
    return &iter->second;
}

} // namespace fcitx
