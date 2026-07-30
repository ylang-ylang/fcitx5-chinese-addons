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

## On-demand English expansions and known phrases

```ini
EnglishExpansionEnabled=True
EnglishExpansionMaxCandidates=10
EnglishExpansionTrigger=semicolon
EnglishPhraseEnabled=True
EnglishPhoneticEnabled=True
```

When the highlighted source is an English spell candidate, the trigger opens a
separate local expansion page instead of the Chinese-to-English page. Ordered
candidates may include conventional abbreviations, cross-part-of-speech
families, and inflections:

```text
[英扩] configuration
config          (缩·常用)
cfg             (缩·代码)
configure       (派生·动)
configurational (派生·形)
configurations  (复数)
```

The page title shows one phonetic: that of the selected English source word,
not every expansion candidate. For example, a word without lexical expansions
can still open a minimal page when its dictionary entry has a phonetic:

```text
[英扩] persistent　/pəˈsɪstənt/
persistent      (原词)
```

Space or a selection key commits the expansion. Semicolon or Escape restores
the original English candidate. Normal Pinyin and Shuangpin behavior is used
when the highlighted candidate is not English or neither an expansion nor a
phonetic is available.

The same dictionary identifies a deliberately small set of abbreviation
phrases. Selecting an English word with Space starts phrase composition only
when that word is a prefix of one of those phrases. Subsequent words remain in
preedit while they continue to match the word-boundary prefix index:

```text
as -> as soon -> as soon as -> as soon as possible
```

Pressing semicolon on the completed phrase shows `ASAP (缩·短语)`. Enter commits
the long phrase. A word that diverges from every known prefix commits the text
with its trailing Space and returns to normal input. This bounded behavior does
not use application surrounding-text deletion and does not retain arbitrary
English sentences in composition. Phrase completions such as
`as soon as possible (短语·ASAP)` are also exposed while the prefix is active.

Install the generated UTF-8 data at:

```text
~/.local/share/fcitx5/pinyin/english-expansion.dict
```

Its format is:

```text
# source word or phrase<TAB>candidate<TAB>display label
configuration\tconfig\t缩·常用
decide\tdecision\t派生·名
as soon as possible\tASAP\t缩·短语
```

Lookup is case-insensitive and collapses whitespace. Candidate spelling and
case are preserved. The maintained local generator combines curated one-way
abbreviations, ECDICT inflection families, and the derivation, pertainym, and
participle relations from Open English WordNet 2025. A two-edge closure is
allowed only within that morphological graph and frequency-filtered with
ECDICT; semantic synonym or hypernym links are never imported. No Wiktionary or
WikiMorph data is used.

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
EnglishPhoneticEnabled=True
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
# english<TAB>label<TAB>concise Chinese meanings<TAB>optional phonetic
user	n.	用户	'ju:zә
patch	计.	补丁/修补	pætʃ
patch	n.	片/补缀
patch	v.	补缀/掩饰
```

The meaning is a non-committed candidate comment, not a separate candidate:

```text
user        (n. 用户)
persistent  (adj. 持久的)
patch       (计. 补丁/修补；n. 片/补缀；v. 补缀/掩饰)
```

Selecting these candidates commits only the English word. The optional
phonetic field is shown once in the temporary `[英扩]` page title for the
selected source word; it is not repeated beside every expansion and does not
add text to the committed candidate. It only needs to appear on one row for a
word. Lookup is case-insensitive. Repeated rows for one word are collected in
file order; identical rows are ignored, and legacy two-column lines are
accepted without a label. The file is loaded while meanings or phonetics are
enabled; restart Fcitx5 after an external updater replaces it.

Translation data is deliberately not bundled in this source repository. The
maintained local generator combines ECDICT's labelled and general senses with
strict, explicitly domain-labelled CC-CEDICT reverse matches. Phonetics come
directly from the lexical source; the generator does not maintain a manual
phonetic supplement. Generated data must be kept out of Git and retain the
source licenses; the combined local
dictionary is CC BY-SA 4.0.
