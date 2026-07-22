/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "englishpreference.h"
#include <algorithm>
#include <charconv>
#include <string>
#include <utility>
#include <vector>

namespace fcitx {

std::string EnglishPreferenceHistory::normalize(std::string_view input) {
    if (input.size() < 4 || input.size() > 32) {
        return {};
    }
    std::string result;
    result.reserve(input.size());
    for (const auto character : input) {
        const auto byte = static_cast<unsigned char>(character);
        if (byte >= 'A' && byte <= 'Z') {
            result.push_back(static_cast<char>(byte - 'A' + 'a'));
        } else if (byte >= 'a' && byte <= 'z') {
            result.push_back(character);
        } else {
            return {};
        }
    }
    return result;
}

bool EnglishPreferenceHistory::load(std::istream &input) {
    clear();
    std::string line;
    while (scores_.size() < MaxEntries && std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line.front() == '#') {
            continue;
        }
        const auto separator = line.find('\t');
        if (separator == std::string::npos) {
            continue;
        }
        auto word = normalize(std::string_view(line).substr(0, separator));
        if (word.empty()) {
            continue;
        }
        int value = 0;
        const auto scoreText = std::string_view(line).substr(separator + 1);
        const auto [end, error] = std::from_chars(
            scoreText.data(), scoreText.data() + scoreText.size(), value);
        if (error != std::errc() ||
            end != scoreText.data() + scoreText.size() || value <= 0) {
            continue;
        }
        scores_.try_emplace(std::move(word), std::min(value, MaxScore));
    }
    return static_cast<bool>(input) || input.eof();
}

bool EnglishPreferenceHistory::save(std::ostream &output) const {
    std::vector<std::pair<std::string, int>> entries(scores_.begin(),
                                                     scores_.end());
    std::ranges::sort(entries, {}, &std::pair<std::string, int>::first);
    output << "# Explicit raw-English preference scores.\n";
    for (const auto &[word, value] : entries) {
        output << word << '\t' << value << '\n';
    }
    return static_cast<bool>(output);
}

void EnglishPreferenceHistory::clear() { scores_.clear(); }

int EnglishPreferenceHistory::score(std::string_view input) const {
    const auto normalized = normalize(input);
    const auto iter = scores_.find(normalized);
    return iter == scores_.end() ? 0 : iter->second;
}

bool EnglishPreferenceHistory::promoted(std::string_view input,
                                        int threshold) const {
    return score(input) >= threshold;
}

bool EnglishPreferenceHistory::reward(std::string_view input, int amount) {
    auto normalized = normalize(input);
    if (normalized.empty() || amount <= 0) {
        return false;
    }
    auto iter = scores_.find(normalized);
    if (iter == scores_.end()) {
        if (scores_.size() >= MaxEntries) {
            return false;
        }
        iter = scores_.emplace(std::move(normalized), 0).first;
    }
    const auto next = std::min(MaxScore, iter->second + amount);
    if (next == iter->second) {
        return false;
    }
    iter->second = next;
    return true;
}

bool EnglishPreferenceHistory::penalize(std::string_view input, int amount) {
    const auto normalized = normalize(input);
    auto iter = scores_.find(normalized);
    if (iter == scores_.end() || amount <= 0) {
        return false;
    }
    iter->second -= amount;
    if (iter->second <= 0) {
        scores_.erase(iter);
    }
    return true;
}

} // namespace fcitx
