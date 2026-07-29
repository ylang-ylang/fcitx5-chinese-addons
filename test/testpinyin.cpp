/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "testdir.h"
#include "testfrontend_public.h"
#include <algorithm>
#include <cstdint>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/event.h>
#include <fcitx-utils/eventdispatcher.h>
#include <fcitx-utils/eventloopinterface.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/macros.h>
#include <fcitx-utils/standardpaths.h>
#include <fcitx-utils/testing.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/candidateaction.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodgroup.h>
#include <fcitx/inputmethodmanager.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace fcitx;

namespace {

std::unique_ptr<EventSourceTime> endTestEvent;
void testPunctuationPart2(Instance *instance);

int findCandidate(InputContext *ic, std::string_view word) {
    auto candList = ic->inputPanel().candidateList();
    for (int i = 0; i < candList->toBulk()->totalSize(); i++) {
        const auto &candidate = candList->toBulk()->candidateFromAll(i);
        if (candidate.text().toString() == word) {
            return i;
        }
    }
    return -1;
}

int findCandidateOrDie(InputContext *ic, std::string_view word) {
    auto index = findCandidate(ic, word);
    FCITX_ASSERT(index >= 0) << "Failed to find candidate " << word;
    return index;
}

void findAndSelectCandidate(InputContext *ic, std::string_view word) {
    auto candList = ic->inputPanel().candidateList();
    candList->candidate(findCandidateOrDie(ic, word)).select(ic);
}

void sendControlSpace(AddonInstance *testfrontend, InputContext *ic) {
    for (int i = 0; i < 2; i++) {
        testfrontend->call<ITestFrontend::keyEvent>(ic->uuid(),
                                                    Key("Control_L"), false);
        testfrontend->call<ITestFrontend::keyEvent>(
            ic->uuid(), Key("Control+space"), false);
        testfrontend->call<ITestFrontend::keyEvent>(ic->uuid(),
                                                    Key("Control+space"), true);
        testfrontend->call<ITestFrontend::keyEvent>(
            ic->uuid(), Key("Control+Control_L"), true);
        ic->reset();
    }
}

void setup(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *pinyin = instance->addonManager().addon("pinyin", true);
        FCITX_ASSERT(pinyin);
        auto defaultGroup = instance->inputMethodManager().currentGroup();
        defaultGroup.inputMethodList().clear();
        defaultGroup.inputMethodList().push_back(
            InputMethodGroupItem("keyboard-us"));
        defaultGroup.inputMethodList().push_back(
            InputMethodGroupItem("pinyin"));
        defaultGroup.inputMethodList().push_back(
            InputMethodGroupItem("shuangpin"));
        defaultGroup.setDefaultInputMethod("");
        instance->inputMethodManager().setGroup(std::move(defaultGroup));
    });
}

void testBasic(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *pinyin = instance->addonManager().addon("pinyin");
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("俺");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("ni");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("ni");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("你hao");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("`"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(
            uuid, Key(FcitxKey_BackSpace), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("s"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("1"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Return"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("KP_Enter"),
                                                    false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        // Make a partial selection, we do search because the data might change.
        findAndSelectCandidate(ic, "你");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Return"), false);

        // Test switch input method.
        testfrontend->call<ITestFrontend::pushCommitExpectation>("nihao");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        sendControlSpace(testfrontend, ic);

        testfrontend->call<ITestFrontend::pushCommitExpectation>("你hao");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        // Make a partial selection, we do search because the data might change.
        findAndSelectCandidate(ic, "你");
        sendControlSpace(testfrontend, ic);

        RawConfig config;
        config.setValueByPath("SwitchInputMethodBehavior",
                              "Commit default selection");
        pinyin->setConfig(config);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        auto sentence =
            ic->inputPanel().candidateList()->candidate(0).text().toString();
        testfrontend->call<ITestFrontend::pushCommitExpectation>(sentence);
        sendControlSpace(testfrontend, ic);

        config.setValueByPath("SwitchInputMethodBehavior", "Clear");
        pinyin->setConfig(config);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        sendControlSpace(testfrontend, ic);
    });
}

void testSelectByChar(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("g"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("g"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("z"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("u"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("b"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("g"), false);

        testfrontend->call<ITestFrontend::pushCommitExpectation>("你好主病");
        findAndSelectCandidate(ic, "你好");
        auto candidateIdx = findCandidateOrDie(ic, "公主");
        ic->inputPanel().candidateList()->toBulkCursor()->setGlobalCursorIndex(
            candidateIdx);
        // With default config, this should select "主".
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("]"), false);
        findAndSelectCandidate(ic, "病");
    });
}

void testUppercase(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        testfrontend->call<ITestFrontend::pushCommitExpectation>("Apple");

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("A"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("l"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("e"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"), false);

        testfrontend->call<ITestFrontend::pushCommitExpectation>("iPhone");

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("P"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("e"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"), false);
    });
}

void testEnglishFuzzyTranslation(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *pinyin = instance->addonManager().addon("pinyin");
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "shuangpin", true);

        const auto type = [&](std::string_view input) {
            for (const auto key : input) {
                testfrontend->call<ITestFrontend::keyEvent>(
                    uuid, Key(std::string(1, key)), false);
            }
        };
        const auto firstCandidate = [&]() {
            return ic->inputPanel()
                .candidateList()
                ->toBulk()
                ->candidateFromAll(0)
                .text()
                .toString();
        };
        const auto hasNonAscii = [](std::string_view text) {
            return std::ranges::any_of(text, [](char byte) {
                return static_cast<unsigned char>(byte) >= 0x80;
            });
        };

        RawConfig config;
        config.setValueByPath("ShuangpinProfile", "Xiaohe");
        config.setValueByPath("Fuzzy/PartialSp", "False");
        config.setValueByPath("FuzzyEnglishEnabled", "False");
        config.setValueByPath("AdaptiveEnglishEnabled", "False");
        config.setValueByPath("EnglishTranslationEnabled", "False");
        pinyin->setConfig(config);

        // Disabling all English extensions preserves upstream behavior.
        type("persistant");
        const auto disabledEnglishIndex = findCandidate(ic, "persistent");
        if (disabledEnglishIndex >= 0) {
            FCITX_ASSERT(ic->inputPanel()
                             .candidateList()
                             ->toBulk()
                             ->candidateFromAll(disabledEnglishIndex)
                             .comment()
                             .empty());
        }
        ic->reset();

        config.setValueByPath("FuzzyEnglishEnabled", "True");
        config.setValueByPath("FuzzyEnglishMinLength", "32");
        config.setValueByPath("FuzzyEnglishMaxCandidates", "3");
        config.setValueByPath("FuzzyEnglishPromote", "True");
        config.setValueByPath("AdaptiveEnglishEnabled", "True");
        config.setValueByPath("AdaptiveEnglishThreshold", "2");
        config.setValueByPath("EnglishTranslationEnabled", "True");
        config.setValueByPath("EnglishTranslationMaxMeanings", "3");
        pinyin->setConfig(config);

        // The minimum length also gates promotion of native lowercase hints.
        type("persistant");
        FCITX_ASSERT(firstCandidate() != "persistent");
        ic->reset();

        config.setValueByPath("FuzzyEnglishMinLength", "4");
        pinyin->setConfig(config);
        type("persistant");

        const auto persistentIndex = findCandidateOrDie(ic, "persistent");
        FCITX_ASSERT(persistentIndex == 0);
        const auto &persistent =
            ic->inputPanel().candidateList()->toBulk()->candidateFromAll(
                persistentIndex);
        FCITX_ASSERT(persistent.comment().toString() == "(adj. 持久的)");
        FCITX_ASSERT(findCandidate(ic, "持久的") < 0);
        testfrontend->call<ITestFrontend::pushCommitExpectation>("persistent");
        persistent.select(ic);
        FCITX_ASSERT(ic->inputPanel().preedit().toString().empty());

        // Adjacent English transpositions count as one bounded edit.
        type("pythno");
        FCITX_ASSERT(firstCandidate() == "python");
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() == "(n. Python 编程语言)");
        ic->reset();

        // A short exact English word is compact and unambiguous.
        type("user");
        FCITX_ASSERT(firstCandidate() == "user");
        FCITX_ASSERT(findCandidateOrDie(ic, "云用户") > 0);
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() == "(n. 用户)");
        ic->reset();

        // Meaning comments contain one to three complete, ordered groups.
        type("patch");
        FCITX_ASSERT(firstCandidate() == "patch");
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() ==
                     "(计. 补丁/修补；n. 片/补缀；v. 补缀/掩饰)");
        ic->reset();
        config.setValueByPath("EnglishTranslationMaxMeanings", "1");
        pinyin->setConfig(config);
        type("patch");
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() == "(计. 补丁/修补)");
        ic->reset();
        config.setValueByPath("EnglishTranslationMaxMeanings", "2");
        pinyin->setConfig(config);
        type("patch");
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() == "(计. 补丁/修补；n. 片/补缀)");
        ic->reset();
        config.setValueByPath("EnglishTranslationMaxMeanings", "3");
        pinyin->setConfig(config);

        // Enabling fuzzy English must not displace complete Xiaohe codes.
        type("hcde");
        FCITX_ASSERT(firstCandidate() == "好的");
        ic->reset();
        type("doge");
        FCITX_ASSERT(firstCandidate() == "多个");
        ic->reset();
        type("yuxylm");
        FCITX_ASSERT(hasNonAscii(firstCandidate()));
        ic->reset();
        type("detese");
        FCITX_ASSERT(hasNonAscii(firstCandidate()));
        FCITX_ASSERT(findCandidateOrDie(ic, "detest") > 0);
        ic->reset();

        // Repeated raw commits are explicit feedback and override the strict
        // Shuangpin protection. Choosing Chinese later removes one reward.
        type("code");
        FCITX_ASSERT(hasNonAscii(firstCandidate()));
        FCITX_ASSERT(findCandidateOrDie(ic, "code") > 0);
        for (int i = 0; i < 2; i++) {
            testfrontend->call<ITestFrontend::pushCommitExpectation>("code");
            testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Return"),
                                                        false);
            if (i == 0) {
                type("code");
            }
        }
        type("code");
        FCITX_ASSERT(firstCandidate() == "code");
        FCITX_ASSERT(findCandidateOrDie(ic, "云代码") > 0);
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() == "(n. 代码)");

        auto *bulk = ic->inputPanel().candidateList()->toBulk();
        const auto chineseIndex = findCandidateOrDie(ic, "错的");
        FCITX_ASSERT(chineseIndex > 0);
        const auto &chinese = bulk->candidateFromAll(chineseIndex);
        testfrontend->call<ITestFrontend::pushCommitExpectation>(
            chinese.text().toString());
        chinese.select(ic);
        type("code");
        FCITX_ASSERT(hasNonAscii(firstCandidate()));
        ic->reset();

        RawConfig resetConfig;
        resetConfig.setValueByPath("FuzzyEnglishEnabled", "False");
        resetConfig.setValueByPath("FuzzyEnglishPromote", "False");
        resetConfig.setValueByPath("AdaptiveEnglishEnabled", "False");
        resetConfig.setValueByPath("EnglishTranslationEnabled", "False");
        pinyin->setConfig(resetConfig);
    });
}

void testEnglishExpansionAndPhrase(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *pinyin = instance->addonManager().addon("pinyin");
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        const auto type = [&](std::string_view input) {
            for (const auto key : input) {
                testfrontend->call<ITestFrontend::keyEvent>(
                    uuid, Key(std::string(1, key)), false);
            }
        };
        const auto selectCandidateCursor = [&](std::string_view word) {
            const auto index = findCandidateOrDie(ic, word);
            ic->inputPanel()
                .candidateList()
                ->toBulkCursor()
                ->setGlobalCursorIndex(index);
        };
        const auto beginAsSoonAsPossible = [&]() {
            type("as");
            selectCandidateCursor("as");
            testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"),
                                                        false);
            FCITX_ASSERT(ic->inputPanel().auxDown().toString() ==
                         "[英文短语] as");
            type("soon");
            testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"),
                                                        false);
            FCITX_ASSERT(findCandidate(ic, "as soon as possible") >= 0);
            type("as");
            testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"),
                                                        false);
            type("possible");
        };

        RawConfig config;
        config.setValueByPath("FuzzyEnglishEnabled", "True");
        config.setValueByPath("FuzzyEnglishMinLength", "4");
        config.setValueByPath("FuzzyEnglishPromote", "True");
        config.setValueByPath("EnglishExpansionEnabled", "True");
        config.setValueByPath("EnglishPhoneticEnabled", "True");
        config.setValueByPath("EnglishExpansionMaxCandidates", "10");
        config.setValueByPath("EnglishExpansionTrigger", "semicolon");
        config.setValueByPath("EnglishPhraseEnabled", "True");
        config.setValueByPath("ChineseEnglishTrigger", "semicolon");
        config.setValueByPath("QuickPhraseKey", "grave");
        pinyin->setConfig(config);

        // A full English candidate opens a temporary page where conventional
        // abbreviations precede derivational and inflectional forms.
        type("configuration");
        selectCandidateCursor("configuration");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        FCITX_ASSERT(ic->inputPanel().auxDown().toString() ==
                     "[英扩] configuration");
        FCITX_ASSERT(findCandidateOrDie(ic, "config") == 0);
        FCITX_ASSERT(findCandidateOrDie(ic, "cfg") == 1);
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() == "(缩·常用)");

        // The trigger toggles back without modifying the English source.
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        FCITX_ASSERT(findCandidate(ic, "configuration") >= 0);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        testfrontend->call<ITestFrontend::pushCommitExpectation>("config");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"), false);
        FCITX_ASSERT(ic->inputPanel().preedit().empty());

        // A selected English word with a phonetic remains queryable even when
        // it has no abbreviation, derivation, or inflection entry.
        type("persistent");
        selectCandidateCursor("persistent");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        FCITX_ASSERT(ic->inputPanel().auxDown().toString() ==
                     "[英扩] persistent　音标 /pəˈsɪstənt/");
        FCITX_ASSERT(findCandidateOrDie(ic, "persistent") == 0);
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() == "(原词)");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Escape"), false);
        ic->reset();

        type("decide");
        selectCandidateCursor("decide");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        FCITX_ASSERT(findCandidateOrDie(ic, "decision") == 0);
        FCITX_ASSERT(findCandidateOrDie(ic, "decisive") == 1);
        FCITX_ASSERT(findCandidate(ic, "deciding") >= 0);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Escape"), false);
        ic->reset();

        // Space keeps only a known abbreviation phrase in preedit. Semicolon
        // then replaces the entire phrase without surrounding-text deletion.
        beginAsSoonAsPossible();
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        FCITX_ASSERT(ic->inputPanel().auxDown().toString() ==
                     "[英扩] as soon as possible");
        FCITX_ASSERT(findCandidateOrDie(ic, "ASAP") == 0);
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() == "(缩·短语)");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("ASAP");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"), false);

        // Return retains the long phrase, while diverging from every known
        // prefix falls back to ordinary word-and-space commit behavior.
        beginAsSoonAsPossible();
        testfrontend->call<ITestFrontend::pushCommitExpectation>(
            "as soon as possible");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Return"), false);

        type("as");
        selectCandidateCursor("as");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"), false);
        type("usual");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("as usual ");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"), false);

        RawConfig resetConfig;
        resetConfig.setValueByPath("FuzzyEnglishEnabled", "False");
        resetConfig.setValueByPath("FuzzyEnglishPromote", "False");
        resetConfig.setValueByPath("EnglishExpansionEnabled", "False");
        pinyin->setConfig(resetConfig);
    });
}

void testOnDemandChineseEnglish(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *pinyin = instance->addonManager().addon("pinyin");
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        const auto type = [&](std::string_view input) {
            for (const auto key : input) {
                testfrontend->call<ITestFrontend::keyEvent>(
                    uuid, Key(std::string(1, key)), false);
            }
        };

        RawConfig config;
        config.setValueByPath("ChineseEnglishEnabled", "True");
        config.setValueByPath("EnglishPhoneticEnabled", "True");
        config.setValueByPath("ChineseEnglishMaxCandidates", "5");
        config.setValueByPath("ChineseEnglishTrigger", "semicolon");
        config.setValueByPath("QuickPhraseKey", "grave");
        pinyin->setConfig(config);

        type("ceshi");
        const auto testingIndex = findCandidateOrDie(ic, "测试");
        ic->inputPanel().candidateList()->toBulkCursor()->setGlobalCursorIndex(
            testingIndex);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        FCITX_ASSERT(ic->inputPanel().auxDown().toString() == "[英译] 测试");
        FCITX_ASSERT(findCandidateOrDie(ic, "test") == 0);
        FCITX_ASSERT(findCandidateOrDie(ic, "examine") == 1);
        FCITX_ASSERT(findCandidate(ic, "测试") < 0);
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulk()
                         ->candidateFromAll(0)
                         .comment()
                         .toString() == "(英译·测试)");

        // The trigger toggles back and restores the highlighted source.
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        FCITX_ASSERT(findCandidateOrDie(ic, "测试") == testingIndex);
        FCITX_ASSERT(ic->inputPanel()
                         .candidateList()
                         ->toBulkCursor()
                         ->globalCursorIndex() == testingIndex);

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Escape"), false);
        FCITX_ASSERT(findCandidate(ic, "测试") >= 0);

        // Selecting the temporary candidate consumes the same Pinyin segment
        // but commits only the English text.
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        testfrontend->call<ITestFrontend::pushCommitExpectation>("test");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"), false);
        FCITX_ASSERT(ic->inputPanel().preedit().empty());

        // Lookup starts from the Chinese candidate, so Xiaohe input uses the
        // same trigger and consumes the original Shuangpin segment length.
        config.setValueByPath("ShuangpinProfile", "Xiaohe");
        pinyin->setConfig(config);
        instance->setCurrentInputMethod(ic, "shuangpin", true);
        type("ceui");
        const auto shuangpinTestingIndex = findCandidateOrDie(ic, "测试");
        ic->inputPanel().candidateList()->toBulkCursor()->setGlobalCursorIndex(
            shuangpinTestingIndex);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        testfrontend->call<ITestFrontend::pushCommitExpectation>("test");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("space"), false);
        FCITX_ASSERT(ic->inputPanel().preedit().empty());

        // Productive particles use a lexical fallback instead of requiring a
        // generated entry for every surface form.
        instance->setCurrentInputMethod(ic, "pinyin", true);
        type("kexuande");
        const auto optionalIndex = findCandidateOrDie(ic, "可选的");
        ic->inputPanel().candidateList()->toBulkCursor()->setGlobalCursorIndex(
            optionalIndex);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        FCITX_ASSERT(ic->inputPanel().auxDown().toString() ==
                     "[英译] 可选的 → 可选");
        FCITX_ASSERT(findCandidateOrDie(ic, "available") == 0);
        FCITX_ASSERT(findCandidateOrDie(ic, "optional") == 1);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("Escape"), false);
        ic->reset();

        // A dictionary miss falls through to the ordinary punctuation path.
        type("nihao");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("你好");
        testfrontend->call<ITestFrontend::pushCommitExpectation>("；");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(";"), false);
        FCITX_ASSERT(ic->inputPanel().preedit().empty());

        RawConfig resetConfig;
        resetConfig.setValueByPath("ChineseEnglishEnabled", "False");
        resetConfig.setValueByPath("QuickPhraseKey", "semicolon");
        pinyin->setConfig(resetConfig);
    });
}

void testForget(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        auto *candidateList = ic->inputPanel().candidateList().get();
        const auto &cand = candidateList->candidate(0);
        auto *actionable = candidateList->toActionable();
        FCITX_ASSERT(actionable);
        FCITX_ASSERT(actionable->hasAction(cand));
        auto actions = actionable->candidateActions(cand);
        FCITX_ASSERT(!actions.empty());
        FCITX_ASSERT(actions[0].id() == 0);
        actionable->triggerAction(cand, 0);
        FCITX_ASSERT(ic->inputPanel().candidateList());
    });
}

void testActionInStrokeFilter(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        // Target ppp for 彡
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        auto *candidateList = ic->inputPanel().candidateList().get();
        findCandidateOrDie(ic, "彡");
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("`"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        candidateList = ic->inputPanel().candidateList().get();
        FCITX_ASSERT(findCandidate(ic, "彡") < 0);
        int index = findCandidateOrDie(ic, "䫠");
        const auto &cand = candidateList->candidate(index);
        auto *actionable = candidateList->toActionable();
        FCITX_ASSERT(actionable);
        FCITX_ASSERT(actionable->hasAction(cand));
        auto actions = actionable->candidateActions(cand);
        FCITX_ASSERT(!actions.empty());
        FCITX_ASSERT(actions[0].id() == 0);
        actionable->triggerAction(cand, 0);
        FCITX_ASSERT(ic->inputPanel().candidateList());
    });
}

void testPinyinTabFilter(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *pinyin = instance->addonManager().addon("pinyin");
        FCITX_ASSERT(pinyin);
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("x"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        findCandidateOrDie(ic, "西安");
        auto *tabbed = ic->inputPanel().candidateList()->toTabbed();
        FCITX_ASSERT(tabbed);
        auto actionSpan = tabbed->tabActions();
        std::vector<CandidateAction> actions{actionSpan.begin(),
                                             actionSpan.end()};
        std::vector<std::string> names;
        names.reserve(actions.size());
        for (const auto &action : actions) {
            names.push_back(action.text());
        }
        auto indexOf = [&names](std::string_view name) {
            auto it = std::ranges::find(names, name);
            FCITX_ASSERT(it != names.end());
            return std::distance(names.begin(), it);
        };
        const auto xiAction = actions[indexOf("xi")];
        const auto singleAction = actions[indexOf("单字")];
        const auto strokeAction = actions[indexOf("笔画")];

        auto checkedActionsAre = [tabbed](std::initializer_list<int> ids) {
            std::unordered_set<int> checkedIds;
            for (const auto &action : tabbed->tabActions()) {
                if (action.isChecked()) {
                    checkedIds.insert(action.id());
                }
            }
            return checkedIds == std::unordered_set<int>{ids};
        };

        FCITX_ASSERT(actions[0].text() == "xian");
        tabbed->triggerTabAction(actions[0].id());
        FCITX_ASSERT(findCandidate(ic, "西安") < 0);
        FCITX_ASSERT(checkedActionsAre({actions[0].id()}));

        // Trigger same action again should uncheck it.
        tabbed->triggerTabAction(actions[0].id());
        FCITX_ASSERT(findCandidate(ic, "西安") >= 0);
        FCITX_ASSERT(checkedActionsAre({}));

        FCITX_ASSERT(xiAction.text() == "xi");
        tabbed->triggerTabAction(xiAction.id());
        FCITX_ASSERT(findCandidate(ic, "西安") >= 0);
        FCITX_ASSERT(checkedActionsAre({xiAction.id()}));

        // Pinyin and single char can be checked at the same time.
        FCITX_ASSERT(singleAction.text() == "单字");
        tabbed->triggerTabAction(singleAction.id());
        FCITX_ASSERT(findCandidate(ic, "西安") < 0);
        FCITX_ASSERT(checkedActionsAre({xiAction.id(), singleAction.id()}));

        // Trigger single char action again should only uncheck itself.
        tabbed->triggerTabAction(singleAction.id());
        FCITX_ASSERT(findCandidate(ic, "西安") >= 0);
        FCITX_ASSERT(checkedActionsAre({xiAction.id()}));

        tabbed->triggerTabAction(singleAction.id());
        FCITX_ASSERT(findCandidate(ic, "西安") < 0);
        FCITX_ASSERT(checkedActionsAre({xiAction.id(), singleAction.id()}));

        // Trigger pinyin action again should only uncheck itself.
        tabbed->triggerTabAction(xiAction.id());
        FCITX_ASSERT(findCandidate(ic, "西安") < 0);
        FCITX_ASSERT(checkedActionsAre({singleAction.id()}));

        // Stroke action should not uncheck single char action.
        tabbed->triggerTabAction(strokeAction.id());
        FCITX_ASSERT(findCandidate(ic, "西安") < 0);
        FCITX_ASSERT(checkedActionsAre({}));

        actionSpan = tabbed->tabActions();
        actions = {actionSpan.begin(), actionSpan.end()};
        FCITX_ASSERT(actions.size() >= 2);
        FCITX_ASSERT(actions[actions.size() - 2].isSeparator());
        FCITX_ASSERT(actions.back().text() == "返回");
        tabbed->triggerTabAction(actions.back().id());
        FCITX_ASSERT(findCandidate(ic, "西安") < 0);
        FCITX_ASSERT(checkedActionsAre({singleAction.id()}));

        // Trigger single char action again should uncheck it.
        tabbed->triggerTabAction(singleAction.id());
        FCITX_ASSERT(findCandidate(ic, "西安") >= 0);
        FCITX_ASSERT(checkedActionsAre({}));
    });
}

void testPinyinTabFilterWithSeparator(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *pinyin = instance->addonManager().addon("pinyin");
        FCITX_ASSERT(pinyin);
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("x"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("'"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("'"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("a"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        findAndSelectCandidate(ic, "西");
        auto *tabbed = ic->inputPanel().candidateList()->toTabbed();
        FCITX_ASSERT(tabbed);
        auto actionSpan = tabbed->tabActions();
        FCITX_ASSERT(std::ranges::none_of(actionSpan, [](const auto &a) {
            return a.text().starts_with('\'');
        }));
    });
}

void testPin(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("t"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("o"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("g"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("y"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("i"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("n"), false);
        auto index1 = findCandidateOrDie(ic, "同音");
        auto index2 = findCandidateOrDie(ic, "痛饮");
        const auto oldIndex1 = index1;
        const auto oldIndex2 = index2;
        FCITX_INFO() << "同音:" << index1 << " "
                     << "痛饮:" << index2;
        {
            auto *candidateList = ic->inputPanel().candidateList().get();
            // Pin the one that is after.
            const auto &cand =
                candidateList->candidate(std::max(index1, index2));
            auto *actionable = candidateList->toActionable();
            FCITX_ASSERT(actionable);
            FCITX_ASSERT(actionable->hasAction(cand));
            auto actions = actionable->candidateActions(cand);
            FCITX_ASSERT(!actions.empty());
            FCITX_ASSERT(std::ranges::any_of(
                actions, [](const auto &a) { return a.id() == 1; }));
            FCITX_ASSERT(!std::ranges::any_of(
                actions, [](const auto &a) { return a.id() == 2; }));
            // This is pin action, 痛饮 should be pined to head.
            actionable->triggerAction(cand, 1);
        }
        index1 = findCandidateOrDie(ic, "同音");
        index2 = findCandidateOrDie(ic, "痛饮");
        FCITX_INFO() << "同音:" << index1 << " "
                     << "痛饮:" << index2;
        FCITX_ASSERT(index1 != oldIndex1);
        FCITX_ASSERT(index2 != oldIndex2);
        FCITX_ASSERT(std::min(index1, index2) == 0);

        {
            auto *candidateList = ic->inputPanel().candidateList().get();
            const auto &candNew = candidateList->candidate(index2);
            auto *actionable = candidateList->toActionable();
            FCITX_ASSERT(actionable);
            FCITX_ASSERT(actionable->hasAction(candNew));
            auto actions = actionable->candidateActions(candNew);
            FCITX_ASSERT(!actions.empty());
            // Check if deletable action is there.
            FCITX_ASSERT(std::ranges::any_of(
                actions, [](const auto &a) { return a.id() == 2; }));
            // This is delete custom phrase action, 痛饮 should be pined to
            // head.
            actionable->triggerAction(candNew, 2);
        }
        index1 = findCandidateOrDie(ic, "同音");
        index2 = findCandidateOrDie(ic, "痛饮");
        FCITX_INFO() << "同音:" << index1 << " "
                     << "痛饮:" << index2;
        FCITX_ASSERT(index1 == oldIndex1);
        FCITX_ASSERT(index2 == oldIndex2);
    });
}

void testQuickPhraseTrigger(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("w"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("w"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("w"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("."), false);
        FCITX_ASSERT(ic->inputPanel().preedit().toString() == "www.");

        ic->reset();
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("b"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("b"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("s"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("."), false);
        FCITX_ASSERT(ic->inputPanel().preedit().toString() == "bbs.");

        ic->reset();
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("u"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("s"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("e"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("r"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("@"), false);
        FCITX_ASSERT(ic->inputPanel().preedit().toString() == "user@");

        ic->reset();
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("t"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("t"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("p"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(":"), false);
        FCITX_ASSERT(ic->inputPanel().preedit().toString() == "http:");

        ic->reset();
        // htt: shouldn't trigger.
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("h"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("t"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("t"), false);

        FCITX_ASSERT(ic->inputPanel().candidateList());
        FCITX_ASSERT(!ic->inputPanel().candidateList()->empty());
        const auto firstCandidate =
            ic->inputPanel().candidateList()->candidate(0).text().toString();
        testfrontend->call<ITestFrontend::pushCommitExpectation>(
            firstCandidate);
        testfrontend->call<ITestFrontend::pushCommitExpectation>("：");

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key(":"), false);
        FCITX_ASSERT(ic->inputPanel().preedit().toString() == "");
    });
}

void testVQuickPhraseTrigger(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");

        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("v"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("."), false);
        FCITX_ASSERT(ic->inputPanel().preedit().toString() == "v.");

        instance->setCurrentInputMethod(ic, "shuangpin", true);
        ic->reset();
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("V"), false);
        testfrontend->call<ITestFrontend::keyEvent>(uuid, Key("."), false);
        FCITX_ASSERT(ic->inputPanel().preedit().toString() == "V.");
    });
}

void testPunctuation(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        testfrontend->call<ITestFrontend::pushCommitExpectation>("。");
        FCITX_ASSERT(testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("."), false));
        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("1"), false));
        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("."), false));
        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("1"), false));
        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("."), false));
        // This is cancel last eng.
        testfrontend->call<ITestFrontend::pushCommitExpectation>("。");
        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("BackSpace"), false));

        auto event = instance->eventLoop().addTimeEvent(
            CLOCK_MONOTONIC, now(CLOCK_MONOTONIC) + 2000000, 0,
            [instance](EventSourceTime *event, uint64_t) {
                testPunctuationPart2(instance);
                delete event;
                return true;
            });
        (void)event.release();
    });
}

void testPunctuationPart2(Instance *instance) {
    instance->eventDispatcher().schedule([instance]() {
        auto *testfrontend = instance->addonManager().addon("testfrontend");
        auto uuid =
            testfrontend->call<ITestFrontend::createInputContext>("testapp");
        auto *ic = instance->inputContextManager().findByUUID(uuid);
        FCITX_ASSERT(ic);
        instance->setCurrentInputMethod(ic, "pinyin", true);

        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("1"), false));
        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("."), false));
        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("space"), false));
        // This should not cancel last eng.
        FCITX_ASSERT(!testfrontend->call<ITestFrontend::sendKeyEvent>(
            uuid, Key("BackSpace"), false));

        endTestEvent = instance->eventLoop().addTimeEvent(
            CLOCK_MONOTONIC, now(CLOCK_MONOTONIC) + 2000000, 0,
            [instance](EventSourceTime *, uint64_t) {
                instance->exit();
                return true;
            });
    });
}

} // namespace

int main() {
    setupTestingEnvironment(
        TESTING_BINARY_DIR, {"bin"},
        {TESTING_BINARY_DIR "/test", TESTING_BINARY_DIR "/im",
         TESTING_BINARY_DIR "/modules", TESTING_SOURCE_DIR "/modules",
         StandardPaths::fcitxPath("pkgdatadir")});
    // fcitx::Log::setLogRule("default=5,table=5,libime-table=5");
    char arg0[] = "testpinyin";
    char arg1[] = "--disable=all";
    char arg2[] = "--enable=testim,testfrontend,pinyin,punctuation,"
                  "pinyinhelper,spell,quickphrase";
    char *argv[] = {arg0, arg1, arg2};
    fcitx::Log::setLogRule("default=5,pinyin=5");
    Instance instance(FCITX_ARRAY_SIZE(argv), argv);
    instance.addonManager().registerDefaultLoader(nullptr);
    setup(&instance);
    testBasic(&instance);
    testSelectByChar(&instance);
    testUppercase(&instance);
    testEnglishFuzzyTranslation(&instance);
    testEnglishExpansionAndPhrase(&instance);
    testOnDemandChineseEnglish(&instance);
    testForget(&instance);
    testActionInStrokeFilter(&instance);
    testPinyinTabFilter(&instance);
    testPinyinTabFilterWithSeparator(&instance);
    testPin(&instance);
    testQuickPhraseTrigger(&instance);
    testVQuickPhraseTrigger(&instance);
    testPunctuation(&instance);
    instance.exec();
    endTestEvent.reset();
    return 0;
}
