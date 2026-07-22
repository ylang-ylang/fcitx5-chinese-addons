#include "../im/pinyin/englishtranslation.h"
#include <fcitx-utils/log.h>
#include <sstream>

using namespace fcitx;

int main() {
    std::stringstream input;
    input << R"TEST(
# word<TAB>part of speech<TAB>preferred translation
Persistent	adj.	持久的
TMUX	n.	终端复用器
legacy	旧格式释义
Persistent	n.	这个重复定义不会覆盖前一个
invalid line
)TEST";

    EnglishTranslationDictionary dictionary;
    FCITX_ASSERT(dictionary.load(input));
    FCITX_ASSERT(dictionary.size() == 3);
    const auto *persistent = dictionary.lookup("persistent");
    FCITX_ASSERT(persistent);
    FCITX_ASSERT(persistent->partOfSpeech == "adj.");
    FCITX_ASSERT(persistent->translation == "持久的");
    FCITX_ASSERT(persistent->comment() == "(adj. 持久的)");
    FCITX_ASSERT(dictionary.lookup("PERSISTENT") == persistent);
    const auto *tmux = dictionary.lookup("tmux");
    FCITX_ASSERT(tmux && tmux->comment() == "(n. 终端复用器)");
    const auto *legacy = dictionary.lookup("legacy");
    FCITX_ASSERT(legacy && legacy->comment() == "(旧格式释义)");
    FCITX_ASSERT(!dictionary.lookup("missing"));
    return 0;
}
