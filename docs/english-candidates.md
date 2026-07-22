# English candidates and local translations

This downstream branch adds optional English behavior to the native Pinyin
engine. The upstream behavior remains unchanged when the new options are
`False` or when the optional translation file is absent.

## Fuzzy English

```ini
FuzzyEnglishEnabled=True
FuzzyEnglishMinLength=5
FuzzyEnglishMaxCandidates=3
FuzzyEnglishPromote=False
```

The native spell addon is still used. When a lowercase ASCII composition is
long enough to look like a possible English word but the normal Pinyin
`englishNess()` gate would not request spell hints, the addon asks the native
spell dictionary for a bounded set of nearby words. Only close edit-distance
matches are retained (with adjacent transpositions treated as one edit), and
`FuzzyEnglishMaxCandidates` caps the number shown. The engine may inspect a few
additional hints internally so a one-candidate
limit is not mistaken for a unique correction. Fuzzy candidates remain after
the normal Pinyin candidate by default. `FuzzyEnglishPromote=True` promotes a
uniquely close match; it is intentionally opt-in because alphabetic Xiaohe
codes are ambiguous.

`FuzzyEnglishMinLength` gates both lookup and promotion. Short words therefore
remain under the native behavior. In Shuangpin mode, a composition made entirely
of valid two-key syllables is never auto-promoted; its English correction can
still be shown after Chinese candidates. Common complete Xiaohe inputs such as
`hcde`, `doge`, and `detese` are therefore not displaced by arbitrary English
suggestions.

## Translation candidates

Set:

```ini
EnglishTranslationEnabled=True
EnglishTranslationCandidateLimit=1
EnglishTranslationShowSource=True
```

Then install a UTF-8 tab-separated file at the normal Fcitx5 user data path:

```text
~/.local/share/fcitx5/pinyin/english-translation.dict
```

Format:

```text
# lowercase-or-mixed-English-word<TAB>preferred Chinese definition
persistent	持久的；持续存在的
kubernetes	容器编排平台
```

The lookup is case-insensitive. The first definition for a duplicate word is
kept. The file is optional and is loaded only while translations are enabled;
a config reload reloads it. Restart Fcitx5 after an external updater replaces
the file.

For an English candidate, the addon can add an immediately adjacent candidate
whose committed text is only the Chinese definition. Its comment identifies the
source word, for example:

```text
1. persistent
2. 持久的；持续存在的    ← persistent 的中文释义
```

Selecting the second candidate commits only the Chinese text and resets the
Pinyin context. It does not add a bogus Pinyin word to LibIME's learning
history.

The translation data is deliberately not bundled in this source repository.
It may be generated from an independently licensed local English–Chinese
source such as ECDICT. Generated data should be kept out of Git.
