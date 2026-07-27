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
可选	available
可选	optional
项目	project
运行	run
invalid
extra	field	ignored
empty	
)DICT");

    ChineseEnglishDictionary dictionary;
    FCITX_ASSERT(dictionary.load(input));
    FCITX_ASSERT(dictionary.size() == 5);

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

    const auto optional = dictionary.lookupBest("可选的");
    FCITX_ASSERT(optional);
    FCITX_ASSERT(optional.matchedChinese == "可选");
    FCITX_ASSERT((*optional.candidates)[0] == "available");
    FCITX_ASSERT((*optional.candidates)[1] == "optional");

    const auto running = dictionary.lookupBest("运行中的");
    FCITX_ASSERT(running);
    FCITX_ASSERT(running.matchedChinese == "运行");
    FCITX_ASSERT((*running.candidates)[0] == "run");

    const auto embedded = dictionary.lookupBest("这是可选的项目");
    FCITX_ASSERT(embedded);
    FCITX_ASSERT(embedded.matchedChinese == "可选");
    FCITX_ASSERT(!dictionary.lookupBest("完全不存在"));

    dictionary.clear();
    FCITX_ASSERT(dictionary.empty());
    return 0;
}
