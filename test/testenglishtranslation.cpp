#include "../im/pinyin/englishtranslation.h"
#include <fcitx-utils/log.h>
#include <sstream>

using namespace fcitx;

int main() {
    std::stringstream input;
    input << R"TEST(
# word<TAB>preferred translation
Persistent	持久的；持续存在的
TMUX	终端复用器
Persistent	这个重复定义不会覆盖前一个
invalid line
)TEST";

    EnglishTranslationDictionary dictionary;
    FCITX_ASSERT(dictionary.load(input));
    FCITX_ASSERT(dictionary.size() == 2);
    FCITX_ASSERT(dictionary.lookup("persistent") == "持久的；持续存在的");
    FCITX_ASSERT(dictionary.lookup("PERSISTENT") == "持久的；持续存在的");
    FCITX_ASSERT(dictionary.lookup("tmux") == "终端复用器");
    FCITX_ASSERT(dictionary.lookup("missing").empty());
    return 0;
}
