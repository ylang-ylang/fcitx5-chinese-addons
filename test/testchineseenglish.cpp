#include "../im/pinyin/chineseenglish.h"
#include <fcitx-utils/log.h>
#include <sstream>

using namespace fcitx;

int main() {
    std::istringstream input(R"DICT(
# chinese<TAB>ordered English candidate
测试	test
测试	examine
测试	test
 用户 	 user 
invalid
extra	field	ignored
empty	
)DICT");

    ChineseEnglishDictionary dictionary;
    FCITX_ASSERT(dictionary.load(input));
    FCITX_ASSERT(dictionary.size() == 2);

    const auto *testing = dictionary.lookup("测试");
    FCITX_ASSERT(testing);
    FCITX_ASSERT(testing->size() == 2);
    FCITX_ASSERT((*testing)[0] == "test");
    FCITX_ASSERT((*testing)[1] == "examine");

    const auto *user = dictionary.lookup("用户");
    FCITX_ASSERT(user);
    FCITX_ASSERT(user->size() == 1);
    FCITX_ASSERT((*user)[0] == "user");
    FCITX_ASSERT(!dictionary.lookup("不存在"));

    dictionary.clear();
    FCITX_ASSERT(dictionary.empty());
    return 0;
}
