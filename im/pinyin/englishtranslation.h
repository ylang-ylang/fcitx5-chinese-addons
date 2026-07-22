/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _PINYIN_ENGLISHTRANSLATION_H_
#define _PINYIN_ENGLISHTRANSLATION_H_

#include <cstddef>
#include <istream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace fcitx {

struct EnglishTranslationEntry {
    std::string partOfSpeech;
    std::string translation;

    std::string comment() const;
};

/**
 * A small, optional, local English-to-Chinese word dictionary.
 *
 * The file format is deliberately plain text so that applications can update
 * the data independently from the pinyin addon:
 *
 *     english<TAB>part-of-speech<TAB>preferred Chinese definition
 *
 * Legacy two-column lines are accepted without a part of speech. Empty lines
 * and lines beginning with '#' are ignored. The dictionary is intentionally
 * word-only; sentence translation is outside the scope of the candidate list.
 */
class EnglishTranslationDictionary {
public:
    bool load(std::istream &input);

    void clear();
    bool empty() const { return translations_.empty(); }
    size_t size() const { return translations_.size(); }

    const EnglishTranslationEntry *lookup(std::string_view word) const;

    static std::string normalize(std::string_view word);

private:
    std::unordered_map<std::string, EnglishTranslationEntry> translations_;
};

} // namespace fcitx

#endif // _PINYIN_ENGLISHTRANSLATION_H_
