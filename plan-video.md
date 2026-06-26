# Plan vidéo — Puissance 4 minimax : du JS au C++

> Document de travail, étoffé au fur et à mesure.
> Synopsis : **1 motivations · 2 version 1 en JS · 3 version 2 en C++ · 4 verdict**
> Détails techniques de chaque étape : voir [`minimax-suivi.md`](minimax-suivi.md).

## Fil rouge

Une seule idée porte les 4 parties et donne la chute :

> **force ≈ profondeur atteinte × qualité de l'évaluation**

Motivations, JS, C++ et verdict ne sont que 4 angles sur cette équation. Le C++ ne
touche pas à l'heuristique : il achète de la **profondeur**. C'est ça, le verdict.

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
- **Payoff** : `MAX_DEPTH` poussé à **10** (vs 7 en JS), temps encore acceptable.
- 🎬 **Démo** : même position que la démo JS, mais profondeur 10.

## 4 — Verdict (retour à l'équation)

- **Le chiffre qui tue** : à **profondeur égale**, nœuds/ms JS vs C++ ; et **profondeur max
  atteignable en ~X ms** (7 vs 10). Démonstration empirique de `force = profondeur × éval` :
  le C++ n'a pas changé `evaluate`, il a acheté **3 plis**.
- **La nuance qui surprend** : à profondeur 10, l'IA C++ joue déjà très bien **même avec
  `evaluate ≡ 0`** — la profondeur a saturé la tactique. « J'ai débranché l'heuristique, elle
  gagne quand même. »
- **Coût honnête** : C++ = compilation, mémoire, setup Qt, itération lente. JS = prototypage
  immédiat, l'heuristique se règle en rechargeant la page.
- **Conclusion utilisable** : prototyper l'algo + l'heuristique en JS (boucle de feedback
  courte), porter en C++ pour la profondeur/perf. Deux **étapes**, pas deux concurrents.
- 🎬 **Clôture** : retour au plan d'ouverture (les deux apps), annonce de la suite — confronter
  l'IA C++ au solveur parfait de gamesolver.org.

---

## À capturer avant de tourner (sinon le verdict reste qualitatif)

Les deux UIs affichent déjà nœuds + temps. Sur **une même position** :

- [ ] JS et C++ à **profondeur égale** (ex. 7) → écart de nœuds / ms.
- [ ] Temps du **1er coup C++ à profondeur 10**.
- [ ] (option) C++ profondeur 10 avec `evaluate ≡ 0` vs avec heuristique → différence de jeu.
