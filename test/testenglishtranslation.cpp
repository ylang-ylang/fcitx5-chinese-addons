#include "../im/pinyin/englishtranslation.h"
#include <fcitx-utils/log.h>
#include <sstream>

using namespace fcitx;

int main() {
    std::stringstream input;
    input << R"TEST(
# word<TAB>label<TAB>concise translation<TAB>optional phonetic
Persistent	adj.	持久的	pəˈsɪstənt
patch	计.	补丁/修补	ˈpætʃ
patch	n.	片/补缀
patch	v.	补缀/掩饰
patch	v.	补缀/掩饰
TMUX	n.	终端复用器
legacy	旧格式释义
invalid line
)TEST";

    EnglishTranslationDictionary dictionary;
    FCITX_ASSERT(dictionary.load(input));
    FCITX_ASSERT(dictionary.size() == 4);
    const auto *persistent = dictionary.lookup("persistent");
    FCITX_ASSERT(persistent);
    FCITX_ASSERT(persistent->meanings.size() == 1);
    FCITX_ASSERT(persistent->meanings[0].label == "adj.");
    FCITX_ASSERT(persistent->meanings[0].translation == "持久的");
    FCITX_ASSERT(persistent->phonetic == "pəˈsɪstənt");
    FCITX_ASSERT(persistent->comment(3) == "(adj. 持久的)");
    FCITX_ASSERT(dictionary.lookup("PERSISTENT") == persistent);

    const auto *patch = dictionary.lookup("PATCH");
    FCITX_ASSERT(patch);
    FCITX_ASSERT(patch->meanings.size() == 3);
    FCITX_ASSERT(patch->phonetic == "ˈpætʃ");
    FCITX_ASSERT(patch->comment(1) == "(计. 补丁/修补)");
    FCITX_ASSERT(patch->comment(2) == "(计. 补丁/修补；n. 片/补缀)");
    FCITX_ASSERT(patch->comment(3) ==
                 "(计. 补丁/修补；n. 片/补缀；v. 补缀/掩饰)");

    const auto *tmux = dictionary.lookup("tmux");
    FCITX_ASSERT(tmux && tmux->comment(3) == "(n. 终端复用器)");
    FCITX_ASSERT(tmux->phonetic.empty());
    const auto *legacy = dictionary.lookup("legacy");
    FCITX_ASSERT(legacy && legacy->comment(3) == "(旧格式释义)");
    FCITX_ASSERT(!dictionary.lookup("missing"));
    return 0;
}
