/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _PINYIN_ENGLISHEXPANSION_H_
#define _PINYIN_ENGLISHEXPANSION_H_

#include <cstddef>
#include <istream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fcitx {

struct EnglishExpansionCandidate {
    std::string value;
    std::string label;
};

struct EnglishPhraseCompletion {
    std::string phrase;
    std::string hint;
};

/**
 * Local, on-demand English lexical expansion data.
 *
 * Each non-comment line contains:
 *
 *     source<TAB>candidate<TAB>label
 *
 * Sources may be a word or a phrase. Lookup is ASCII-case-insensitive and
 * collapses whitespace. Candidate spelling and case are preserved. The same
 * representation supplies both the semicolon expansion page and deterministic
 * completion of known multi-word abbreviation phrases.
 */
class EnglishExpansionDictionary {
public:
    bool load(std::istream &input);

    void clear();
    bool empty() const { return expansions_.empty(); }
    size_t size() const { return expansions_.size(); }

    const std::vector<EnglishExpansionCandidate> *
    lookup(std::string_view source) const;

    // True only when another word can follow prefix in a known source phrase.
    bool hasPhraseContinuation(std::string_view prefix) const;
    // True for either a proper phrase prefix or a complete multi-word source.
    bool isPhrasePrefix(std::string_view prefix) const;

    std::vector<EnglishPhraseCompletion> completePhrase(std::string_view prefix,
                                                        size_t maximum) const;

    static std::string normalize(std::string_view text);

private:
    std::unordered_map<std::string, std::vector<EnglishExpansionCandidate>>
        expansions_;
    std::unordered_map<std::string, std::vector<std::string>> phrasePrefixes_;
};

} // namespace fcitx

#endif // _PINYIN_ENGLISHEXPANSION_H_
