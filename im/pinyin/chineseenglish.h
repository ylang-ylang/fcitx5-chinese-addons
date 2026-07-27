/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _PINYIN_CHINESEENGLISH_H_
#define _PINYIN_CHINESEENGLISH_H_

#include <cstddef>
#include <functional>
#include <istream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fcitx {

struct ChineseEnglishMatch {
    const std::vector<std::string> *candidates = nullptr;
    std::string matchedChinese;

    explicit operator bool() const { return candidates != nullptr; }
};

/**
 * An optional, local Chinese-to-English candidate dictionary.
 *
 * The UTF-8 text format contains one ordered candidate per line:
 *
 *     Chinese word<TAB>English word or short phrase
 *
 * Repeated Chinese keys are collected in file order. Empty lines, comments,
 * malformed rows, and duplicate candidate pairs are ignored.
 */
class ChineseEnglishDictionary {
public:
    bool load(std::istream &input);

    void clear();
    bool empty() const { return translations_.empty(); }
    size_t size() const { return translations_.size(); }

    const std::vector<std::string> *lookup(std::string_view chinese) const;
    ChineseEnglishMatch lookupBest(std::string_view chinese) const;

private:
    struct StringHash {
        using is_transparent = void;
        size_t operator()(std::string_view value) const noexcept;
    };

    std::unordered_map<std::string, std::vector<std::string>, StringHash,
                       std::equal_to<>>
        translations_;
};

} // namespace fcitx

#endif // _PINYIN_CHINESEENGLISH_H_
