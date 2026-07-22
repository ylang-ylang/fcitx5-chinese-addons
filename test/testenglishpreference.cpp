#include "../im/pinyin/englishpreference.h"
#include <fcitx-utils/log.h>
#include <sstream>

using namespace fcitx;

int main() {
    EnglishPreferenceHistory history;
    FCITX_ASSERT(!history.reward("api"));
    FCITX_ASSERT(!history.reward("hao-de"));
    FCITX_ASSERT(history.reward("Code"));
    FCITX_ASSERT(history.reward("code", 2));
    FCITX_ASSERT(history.score("CODE") == 3);
    FCITX_ASSERT(history.promoted("code", 3));
    FCITX_ASSERT(history.penalize("code"));
    FCITX_ASSERT(!history.promoted("code", 3));
    FCITX_ASSERT(history.score("code") == 2);

    std::stringstream output;
    FCITX_ASSERT(history.save(output));
    EnglishPreferenceHistory loaded;
    FCITX_ASSERT(loaded.load(output));
    FCITX_ASSERT(loaded.score("code") == 2);
    FCITX_ASSERT(!loaded.penalize("missing"));
    FCITX_ASSERT(loaded.penalize("code", 2));
    FCITX_ASSERT(loaded.size() == 0);

    std::stringstream malformed;
    malformed << "# comment\nvalid\t99\nshort\t0\nbad score\t3\n";
    FCITX_ASSERT(loaded.load(malformed));
    FCITX_ASSERT(loaded.score("valid") == 20);
    FCITX_ASSERT(loaded.size() == 1);
    return 0;
}
