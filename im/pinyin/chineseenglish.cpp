/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "chineseenglish.h"
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

const std::vector<std::string> *
ChineseEnglishDictionary::lookup(std::string_view chinese) const {
    auto iter = translations_.find(std::string(chinese));
    if (iter == translations_.end()) {
        return nullptr;
    }
    return &iter->second;
}

} // namespace fcitx
