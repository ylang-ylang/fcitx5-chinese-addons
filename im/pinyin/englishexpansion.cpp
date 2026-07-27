/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "englishexpansion.h"
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

std::string EnglishExpansionDictionary::normalize(std::string_view text) {
    std::string result;
    result.reserve(text.size());
    bool pendingSpace = false;
    for (const auto character : text) {
        const auto byte = static_cast<unsigned char>(character);
        if (std::isspace(byte)) {
            pendingSpace = !result.empty();
            continue;
        }
        if (pendingSpace) {
            result.push_back(' ');
            pendingSpace = false;
        }
        if (byte >= 'A' && byte <= 'Z') {
            result.push_back(static_cast<char>(byte - 'A' + 'a'));
        } else {
            result.push_back(character);
        }
    }
    return result;
}

bool EnglishExpansionDictionary::load(std::istream &input) {
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
        const auto secondSeparator = lineView.find('\t', firstSeparator + 1);
        if (secondSeparator == std::string_view::npos) {
            continue;
        }

        auto source = normalize(lineView.substr(0, firstSeparator));
        auto candidate = trim(lineView.substr(
            firstSeparator + 1, secondSeparator - firstSeparator - 1));
        auto label = trim(lineView.substr(secondSeparator + 1));
        if (source.empty() || candidate.empty() || label.empty()) {
            continue;
        }

        auto &candidates = expansions_[source];
        const auto duplicate =
            std::ranges::any_of(candidates, [&](const auto &existing) {
                return existing.value == candidate;
            });
        if (!duplicate) {
            candidates.push_back(EnglishExpansionCandidate{std::move(candidate),
                                                           std::move(label)});
        }
    }

    // Build a word-boundary prefix index for multi-word sources. The complete
    // source is indexed too, so a finished phrase remains editable until the
    // user either expands it or continues with a non-matching word.
    for (const auto &[source, candidates] : expansions_) {
        // Only abbreviation sources participate in automatic phrase capture.
        // WordNet also contains ordinary multi-word lexical entries; retaining
        // those on Space would unexpectedly turn general English into a long
        // composition.
        if (!std::ranges::any_of(candidates, [](const auto &candidate) {
                return candidate.label.starts_with("缩");
            })) {
            continue;
        }
        auto separator = source.find(' ');
        if (separator == std::string::npos) {
            continue;
        }
        while (separator != std::string::npos) {
            auto &phrases = phrasePrefixes_[source.substr(0, separator)];
            if (std::ranges::find(phrases, source) == phrases.end()) {
                phrases.push_back(source);
            }
            separator = source.find(' ', separator + 1);
        }
        auto &phrases = phrasePrefixes_[source];
        if (std::ranges::find(phrases, source) == phrases.end()) {
            phrases.push_back(source);
        }
    }

    return static_cast<bool>(input) || input.eof();
}

void EnglishExpansionDictionary::clear() {
    expansions_.clear();
    phrasePrefixes_.clear();
}

const std::vector<EnglishExpansionCandidate> *
EnglishExpansionDictionary::lookup(std::string_view source) const {
    const auto iter = expansions_.find(normalize(source));
    if (iter == expansions_.end()) {
        return nullptr;
    }
    return &iter->second;
}

bool EnglishExpansionDictionary::hasPhraseContinuation(
    std::string_view prefix) const {
    const auto normalized = normalize(prefix);
    const auto iter = phrasePrefixes_.find(normalized);
    if (iter == phrasePrefixes_.end()) {
        return false;
    }
    return std::ranges::any_of(iter->second, [&](const auto &phrase) {
        return phrase.size() > normalized.size();
    });
}

bool EnglishExpansionDictionary::isPhrasePrefix(std::string_view prefix) const {
    return phrasePrefixes_.contains(normalize(prefix));
}

std::vector<EnglishPhraseCompletion>
EnglishExpansionDictionary::completePhrase(std::string_view prefix,
                                           size_t maximum) const {
    std::vector<EnglishPhraseCompletion> result;
    if (maximum == 0) {
        return result;
    }
    const auto normalized = normalize(prefix);
    const auto iter = phrasePrefixes_.find(normalized);
    if (iter == phrasePrefixes_.end()) {
        return result;
    }
    for (const auto &phrase : iter->second) {
        if (phrase == normalized) {
            continue;
        }
        std::string hint;
        if (const auto *candidates = lookup(phrase);
            candidates && !candidates->empty()) {
            hint = candidates->front().value;
        }
        result.push_back(EnglishPhraseCompletion{phrase, std::move(hint)});
        if (result.size() == maximum) {
            break;
        }
    }
    return result;
}

} // namespace fcitx
