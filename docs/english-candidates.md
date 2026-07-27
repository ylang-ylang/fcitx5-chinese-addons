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

## On-demand Chinese-to-English candidates

```ini
ChineseEnglishEnabled=True
ChineseEnglishMaxCandidates=5
ChineseEnglishTrigger=semicolon
```

With an active Pinyin composition, the trigger looks up the currently
highlighted Chinese candidate and temporarily replaces the candidate page with
ordered English words or short phrases. For example, highlight `测试` and press
`;` to show:

```text
test       (英译·测试)
beta       (英译·测试)
```

Space and the normal selection keys choose an English candidate. Press `;` or
Escape to restore the Chinese page; any other non-candidate key restores the
Chinese page before the key is processed. A dictionary miss falls through to
the key's original behavior. Selecting an English result consumes the same
Pinyin or Shuangpin segment as its source candidate and commits only the English
text.

Install the generated UTF-8 dictionary at:

```text
~/.local/share/fcitx5/pinyin/chinese-english.dict
```

Its ordered format is:

```text
# chinese<TAB>English candidate
测试\ttest
测试\tbeta
用户\tuser
```

The maintained local generator combines CC-CEDICT with a reverse index of the
already filtered ECDICT English-to-Chinese meanings, then applies a small
curated override layer. This covers direct words hidden inside explanatory
CC-CEDICT glosses, such as `鹰 -> eagle/hawk`, without importing ECDICT's raw unfiltered rows. Lookup is local and runs only after the trigger:
it first tries the exact candidate, then strips a short chain of productive
particles (`的`, `地`, `得`, `们`, `了`, `过`, `着`, `中`), and finally tries
the longest dictionary span inside the candidate. Thus `可选的` reuses the
entry for `可选`, while exact entries always retain priority. The normal
candidate path performs none of this reverse-translation work.

Because upstream Pinyin uses semicolon for Quick Phrase by default, users who
choose semicolon as the translation trigger should move `QuickPhraseKey` to an
unused key. This setup uses `grave` while leaving its stroke-filter key empty.

## Concise English meanings

Set:

```ini
EnglishTranslationEnabled=True
EnglishTranslationMaxMeanings=3
```

`EnglishTranslationMaxMeanings` accepts 1 through 3. It is a maximum: words
with fewer useful groups stay short, while polysemous words can expose up to
three complete ordered groups.

Then install a UTF-8 tab-separated file at:

```text
~/.local/share/fcitx5/pinyin/english-translation.dict
```

Format:

```text
# english<TAB>label<TAB>concise Chinese meanings
user	n.	用户
patch	计.	补丁/修补
patch	n.	片/补缀
patch	v.	补缀/掩饰
```

The meaning is a non-committed candidate comment, not a separate candidate:

```text
user        (n. 用户)
persistent  (adj. 持久的)
patch       (计. 补丁/修补；n. 片/补缀；v. 补缀/掩饰)
```

Selecting these candidates commits only the English word. The lookup is
case-insensitive. Repeated rows for one word are collected in file order;
identical rows are ignored, and legacy two-column lines are accepted without a
label. The file is loaded only while meanings are enabled; restart Fcitx5 after
an external updater replaces it.

Translation data is deliberately not bundled in this source repository. The
maintained local generator combines ECDICT's labelled and general senses with
strict, explicitly domain-labelled CC-CEDICT reverse matches. Generated data
must be kept out of Git and retain the source licenses; the combined local
dictionary is CC BY-SA 4.0.
