# Plan vidéo — Puissance 4 minimax : du JS au C++

> Document de travail, étoffé au fur et à mesure.
> Synopsis : **1 motivations · 2 version 1 en JS · 3 version 2 en C++ · 4 verdict · 5 confrontation au solveur**
> Détails techniques de chaque étape : voir [`minimax-suivi.md`](minimax-suivi.md).

## Fil rouge

Une seule idée porte les 4 parties et donne la chute :

> **force ≈ profondeur atteinte × qualité de l'évaluation**

Les cinq parties ne sont que des angles sur cette même équation. Le C++ ne
touche pas à l'heuristique : il achète de la **profondeur**. C'est ça, le verdict.

## Intention — le cadrage honnête (les 3 messages)

⚠️ **Le but n'est PAS de créer l'algorithme parfait.** Vidéo pédagogique, pas démo de prouesse.
Trois messages, et c'est tout :

1. **Le choix du langage compte.** *JS* : rapide à écrire, **sans risque** (pas de crash mémoire),
   boucle de feedback courte — mais on est vite **borné par l'interpréteur**. *C++* : plus puissant,
   moins facile, **plus de crashs possibles** (segfault, mémoire), mais ouvre les **optimisations
   mémoire & vitesse** → donc la **profondeur**. Le langage est un *levier de profondeur*.

2. **Les bonnes idées algo comptent autant.** Au-delà du langage, des idées comme la **table de
   transposition** font descendre **encore plus bas, sans souci** : de **7 (JS) à 12** plis. Deux
   leviers distincts pour la même équation — le **langage** ET l'**algo**.

3. **Attention aux faux positifs — leçon d'humilité.** Notre heuristique reste une **évaluation**
   du meilleur coup, pas une vérité : elle peut se tromper **avec aplomb** (faux positifs), et
   **elle ne battra jamais un algo qui explore le chemin complet** (solveur parfait, jeu résolu).
   Plus de profondeur + de bonnes idées *rapprochent* du parfait — **sans jamais l'atteindre**.

---

## 1 — Les motivations (le « pourquoi »)

- **Objectif de fond** : minimax en pratique sur une progression (morpion → Puissance 4
  → awalé → échecs). Le morpion descend toujours jusqu'aux feuilles ; le **Puissance 4
  est le premier saut conceptuel** → arbre trop gros → **profondeur limitée + heuristique
  obligatoires**.
- **Pourquoi reprogrammer en C++** (cœur de la vidéo) : pas pour changer l'IA, mais pour
  **descendre plus profond dans le même temps** → IA plus forte *sans toucher à
  l'heuristique*. Poser ici l'équation `force = profondeur × éval`.
- **Motivation secondaire honnête** : apprendre C++/Qt, découvrir `constexpr`, le bitboard,
  make/undo. Le dire franchement rend le verdict crédible.
- 🎬 **Hook d'ouverture** : les deux apps côte à côte (navigateur vs fenêtre Qt), même jeu —
  « même algo, deux mondes — à la fin on saura ce que le C++ rapporte vraiment ».

## 2 — Version 1 en JS (le prototype qui pense vite)

- **Le moteur** : `minmaxAB(depth, alpha, beta)`, profondeur limitée `MAX_DEPTH`, ordre des
  tests (terminaux `win/isFull` **avant** le cutoff heuristique).
- **Évaluation par contenu de fenêtre** 👤 *(à expliquer moi-même)* : 69 fenêtres de 4,
  `nbX/nbO`, fenêtre morte si 2 camps, menaces 3→1000 / 2→100, **bonus centre** (départage,
  poids volontairement petit).
- **Série d'optims JS** (montée en puissance) :
  - α-β + **move ordering** centre→bords + **threading `alpha`** racine → `MAX_DEPTH` à 7.
  - **fenêtres pré-calculées** : sortir la géométrie du point chaud (1× au chargement).
  - **`Uint8Array` plat** : cloner un nœud = `slice()` de 42 octets (prépare la révélation C++).
- **Stats à l'écran** (grilles testées, temps, MAX_DEPTH) : c'est l'**instrument de mesure**
  du verdict.
- 🎬 **Démo** : une partie, la box de stats qui bouge.

## 3 — Version 2 en C++ (le même cerveau, mais qui pense profond)

Angle : *« je ne réinvente pas l'IA, je la fais descendre plus bas »*. Trois choses que le
C++ permet et que le JS ne pouvait pas :

- **`constexpr` — géométrie calculée à la *compilation*** 👤 *(concept neuf, bon moment
  pédago)*. En JS le mieux était « 1× au chargement » ; en C++ le tableau des 69 fenêtres est
  figé **dans le binaire**. Montrer le **truc du `throw`** : compte faux → ça **ne compile pas**.
- **make/undo au lieu de cloner** : `height[]` + `undo()` remplacent `slice()`/`structuredClone`.
  L'optim qui paie le plus dans le point chaud.
- **`win()` incrémental** : seul le dernier coup peut créer un alignement → ≤13 fenêtres au
  lieu de 69 (`FENETRES_PAR_CASE`, figé au build).
- **Bitboard** (6 `unsigned short`, 2 bits/case) + **stats par signaux** (`aiMoved`) +
  `Q_ASSERT` vs garde silencieuse — en B-roll.
- **Table de transposition** : ne pas recalculer un sous-arbre déjà vu (*transposition* = même
  position atteinte par un autre ordre de coups). Clé **exacte compacte 49 bits** (`position + mask`).
  L'**idée algo** qui pousse la profondeur au-delà du seul gain de langage.
- **Payoff** : `MAX_DEPTH` **7 (JS) → 10 (C++) → 12 (C++ + TT)**, temps encore acceptable. Deux
  leviers : le **langage** (7→10) ET l'**idée algo** (10→12).
- 🎬 **Démo** : même position que la démo JS, mais profondeur 10.

## 4 — Verdict (retour à l'équation)

- **Le chiffre qui tue** : à **profondeur égale**, nœuds/ms JS vs C++ ; et **profondeur max
  atteignable en ~X ms** (7 vs 10). Démonstration empirique de `force = profondeur × éval` :
  le C++ n'a pas changé `evaluate`, il a acheté **3 plis**.
- **La nuance qui surprend** : à profondeur 10, l'IA C++ joue déjà très bien **même avec
  `evaluate ≡ 0`** — la profondeur a saturé la tactique. « J'ai débranché l'heuristique, elle
  gagne quand même. »
- **Leçon d'humilité (faux positifs)** : notre heuristique n'est qu'une **évaluation** du meilleur
  coup, pas une vérité — elle peut se tromper avec aplomb, et **ne battra jamais un algo qui voit le
  chemin complet** (solveur parfait). Profondeur + bonnes idées *rapprochent* du parfait, sans l'atteindre.
- **Coût honnête** : C++ = compilation, mémoire, setup Qt, itération lente. JS = prototypage
  immédiat, l'heuristique se règle en rechargeant la page.
- **Conclusion utilisable** : prototyper l'algo + l'heuristique en JS (boucle de feedback
  courte), porter en C++ pour la profondeur/perf. Deux **étapes**, pas deux concurrents.
- 🎬 **Clôture** : retour au plan d'ouverture (les deux apps), annonce de la suite — confronter
  l'IA C++ au solveur parfait de gamesolver.org.

## 5 — Confrontation au solveur parfait (l'épreuve de vérité)

L'adversaire : **<https://connect4.gamesolver.org>** (solveur de Pascal Pons), l'**oracle
parfait** — le Puissance 4 est *résolu* depuis 1988. C'est le crash-test ultime de l'équation :
mon IA voit ~10 plis + une heuristique bricolée ; lui voit **jusqu'au bout**.

- **La prémisse fausse à casser d'entrée** : « ça finira nul ». Non — le Puissance 4 résolu,
  c'est **le 1er joueur qui gagne** en jeu parfait (en ouvrant au centre). L'issue est fixée
  par *qui commence*, pas par un nul. Et **mon IA n'est pas parfaite** → face à l'oracle,
  l'écart se voit forcément.

- **Le premier essai (résultat réel)** 🎬 : mon IA ouvre **au centre** — donc elle part d'une
  position **théoriquement gagnante** — et elle **perd**. C'est *le* moment fort : elle tenait
  la victoire et l'a lâchée. Au Puissance 4 résolu, **un seul coup sous-optimal** fait basculer
  la position de gagnée à perdue, et l'oracle **ne la rend jamais**. L'ouverture (centre forcé)
  est bonne par construction → l'erreur est **plus loin** : soit l'horizon de 10 trop court pour
  voir le piège, soit l'éval a mal jugé. *Illustration directe de « imbattable = jeu résolu,
  pas heuristique » et de `force = profondeur × éval`.*

- **Le contenu qui tue — l'analyse coup par coup** : rejouer la séquence dans le solveur et
  trouver **le coup exact où le score bascule** de gagnant à perdant. La phrase de la vidéo :
  « *mon IA a joué N coups parfaits, puis a lâché la victoire au coup N+1 — le voici, et voilà
  pourquoi son heuristique s'est trompée* ». Chiffrable : **nombre de coups optimaux avant la
  1ʳᵉ erreur**.

- **L'outil** 👤 : l'API (non documentée mais publique) `GET …/solve?pos=<colonnes 1-indexées>`
  renvoie un **score par colonne** (signe = issue, magnitude = vitesse). On l'appelle depuis un
  **petit script Node** (hors navigateur → contourne le **CORS**), 1 requête/coup, poliment.
  Détails techniques + format de réponse : voir [`minimax-suivi.md`](minimax-suivi.md).

- **Les deux scénarios à filmer** :
  - *Mon IA ouvre (centre)* → sait-elle **convertir** une position gagnante face à une défense
    parfaite ? (le 1er essai dit : non — c'est le segment pédago).
  - *Le solveur ouvre* → il gagne (parfait), mon IA part perdue → **combien de temps tient-elle**,
    à quel coup craque-t-elle ?

- 🎬 **Mise en scène possible** : segment 1 = vs solveur parfait (démo pédago, l'instant du
  basculement). Segment 2 (spectacle) = vs une IA à difficulté réglable (play4row niveau moyen),
  match plus disputé.

---

## À capturer avant de tourner (sinon le verdict reste qualitatif)

### Les réglages live de l'app C++ (la « table de mixage » des mesures)

Trois contrôles ajoutés dans la barre du bas pilotent le moteur sans recompiler. **Chaque
changement relance la partie de zéro ET vide la table de transposition** → toute mesure part
d'un état neuf, aucun cache résiduel ne fausse le 1er coup. Idem pour « qui commence ».

- **Spinbox Profondeur (1–15)** — le levier *profondeur* de l'équation `force = profondeur × éval`.
- **Checkbox `evaluate = 0`** — débranche l'heuristique (le minimax ne voit plus que les
  feuilles terminales de son horizon). Le mat valant 100 000 ≫ éval max (~69 180), l'IA voit
  toujours les mats : c'est ce qui rend la démo « je débranche l'éval et ça gagne quand même »
  honnête.
- **Checkbox Table de transposition** — active/désactive la TT pour isoler ce que l'*idée algo*
  rapporte, à profondeur égale.

Mapping contrôle → mesure :

| Mesure à capturer | Réglage |
|---|---|
| JS vs C++ à profondeur égale (nœuds/ms) | spinbox = **7** sur les deux apps |
| Temps du 1er coup C++ à profondeur 10 | spinbox = **10** |
| Profondeur seule vs heuristique | spinbox = **10** + cocher **`evaluate = 0`** |
| Ce que rapporte la TT | profondeur fixe, comparer **TT cochée / décochée** |

### La checklist

Les deux UIs affichent déjà nœuds + temps. Sur **une même position** :

- [ ] JS et C++ à **profondeur égale** (ex. 7) → écart de nœuds / ms.
- [ ] Temps du **1er coup C++ à profondeur 10**.
- [ ] (option) C++ profondeur 10 avec `evaluate ≡ 0` vs avec heuristique → différence de jeu.
- [ ] **TT cochée vs décochée** à profondeur égale → effondrement des nœuds (le levier *idée algo*).

Pour la partie 5 (confrontation au solveur) :

- [ ] **Séquence des colonnes** de la partie perdue (mon IA ouvre au centre) — à logger
      (`qDebug() << col;`) puis rejouer.
- [ ] Sortie du **script Node** d'analyse coup-par-coup : colonne jouée vs optimale, **coup du
      basculement** gagnant→perdant, **nb de coups optimaux avant la 1ʳᵉ erreur**.
- [ ] (option) Partie où **le solveur ouvre** : à quel coup mon IA craque.
