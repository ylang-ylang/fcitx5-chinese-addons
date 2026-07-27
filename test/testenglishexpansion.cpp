/*
 * SPDX-FileCopyrightText: 2026-2026 ylang-ylang
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "../im/pinyin/englishexpansion.h"
#include <fcitx-utils/log.h>
#include <sstream>

using namespace fcitx;

int main() {
    std::istringstream input(R"DICT(
# source, candidate, display label
configuration	config	缩·常用
configuration	cfg	缩·代码
CONFIGURATION	config	duplicate
configuration	configure	派生·动
as soon as possible	ASAP	缩·短语
application programming interface	API	缩·短语
)DICT");

    EnglishExpansionDictionary dictionary;
    FCITX_ASSERT(dictionary.load(input));
    FCITX_ASSERT(dictionary.size() == 3);

    const auto *configuration = dictionary.lookup(" Configuration ");
    FCITX_ASSERT(configuration);
    FCITX_ASSERT(configuration->size() == 3);
    FCITX_ASSERT((*configuration)[0].value == "config");
    FCITX_ASSERT((*configuration)[0].label == "缩·常用");
    FCITX_ASSERT((*configuration)[1].value == "cfg");
    FCITX_ASSERT((*configuration)[2].value == "configure");

    const auto *asap = dictionary.lookup("AS  soon\tas possible ");
    FCITX_ASSERT(asap);
    FCITX_ASSERT(asap->front().value == "ASAP");
    FCITX_ASSERT(dictionary.hasPhraseContinuation("as"));
    FCITX_ASSERT(dictionary.hasPhraseContinuation("as soon as"));
    FCITX_ASSERT(!dictionary.hasPhraseContinuation("as soon as possible"));
    FCITX_ASSERT(dictionary.isPhrasePrefix("as soon as possible"));
    FCITX_ASSERT(!dictionary.isPhrasePrefix("as usual"));

    auto completions = dictionary.completePhrase("as soon", 2);
    FCITX_ASSERT(completions.size() == 1);
    FCITX_ASSERT(completions.front().phrase == "as soon as possible");
    FCITX_ASSERT(completions.front().hint == "ASAP");
    FCITX_ASSERT(dictionary.completePhrase("application", 0).empty());

    dictionary.clear();
    FCITX_ASSERT(dictionary.empty());
    FCITX_ASSERT(!dictionary.lookup("configuration"));
    return 0;
}
