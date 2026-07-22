# English candidates and concise local meanings

This downstream branch adds optional English behavior to the native Pinyin
engine. Upstream candidate behavior remains unchanged when the new options are
`False`.

## Fuzzy English

```ini
FuzzyEnglishEnabled=True
FuzzyEnglishMinLength=4
FuzzyEnglishMaxCandidates=3
FuzzyEnglishPromote=True
```

The native spell addon is still used. The addon requests a bounded set of
nearby words for eligible lowercase ASCII compositions. Only close
optimal-string-alignment matches are retained, so an adjacent transposition
such as `pythno -> python` counts as one edit. The engine may inspect a few
additional hints internally so a one-candidate display limit is not mistaken
for a unique correction.

In Shuangpin mode, a composition made entirely of valid two-key syllables is
not promoted by a static rule. Its English candidate can still be shown after
Chinese candidates. Common complete Xiaohe inputs such as `hcde`, `doge`, and
`detese` are therefore not displaced by arbitrary English suggestions.

## Adaptive English preference

```ini
AdaptiveEnglishEnabled=True
AdaptiveEnglishThreshold=3
```

Committing a complete ASCII composition literally with the configured raw-input
key (normally Enter) records one explicit English preference. Selecting its
English spell candidate also records a preference. Once the threshold is
reached, an exact dictionary word is promoted even if the input is also a valid
Shuangpin code. For example, repeated literal commits can change:

```text
1. 错的
2. code  (n. 代码)
```

into:

```text
1. code  (n. 代码)
2. 错的
```

Selecting a full Chinese candidate removes one preference point, so the choice
can adapt in both directions. Scores are bounded and stored separately from
LibIME data at:

```text
~/.local/share/fcitx5/pinyin/english-preference.history
```

The history is a small text file capped at 4096 entries. It never modifies
`user.dict` or `user.history`.

## Concise English meanings

Set:

```ini
EnglishTranslationEnabled=True
```

Then install a UTF-8 tab-separated file at:

```text
~/.local/share/fcitx5/pinyin/english-translation.dict
```

Format:

```text
# english<TAB>part-of-speech<TAB>concise Chinese meaning
user	n.	用户
persistent	adj.	持久的
```

The meaning is a non-committed candidate comment, not a separate candidate:

```text
1. user        (n. 用户)
2. persistent  (adj. 持久的)
```

Selecting these candidates commits only `user` or `persistent`. The lookup is
case-insensitive, the first duplicate is kept, and legacy two-column dictionary
lines are accepted without a part of speech. The file is loaded only while
meanings are enabled; restart Fcitx5 after an external updater replaces it.

Translation data is deliberately not bundled in this source repository. It may
be generated from an independently licensed local source such as ECDICT.
Generated data should be kept out of Git.
