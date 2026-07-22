/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _PINYIN_ENGLISHPREFERENCE_H_
#define _PINYIN_ENGLISHPREFERENCE_H_

#include <cstddef>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace fcitx {

/** A bounded, word-only preference history for explicit literal commits. */
class EnglishPreferenceHistory {
public:
    bool load(std::istream &input);
    bool save(std::ostream &output) const;

    void clear();
    bool empty() const { return scores_.empty(); }
    size_t size() const { return scores_.size(); }
    int score(std::string_view input) const;
    bool promoted(std::string_view input, int threshold) const;

    bool reward(std::string_view input, int amount = 1);
    bool penalize(std::string_view input, int amount = 1);

    static std::string normalize(std::string_view input);

private:
    static constexpr int MaxScore = 20;
    static constexpr size_t MaxEntries = 4096;
    std::unordered_map<std::string, int> scores_;
};

} // namespace fcitx

#endif // _PINYIN_ENGLISHPREFERENCE_H_
