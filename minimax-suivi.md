# Suivi d'apprentissage — Minimax

## Objectif
Mettre en pratique l'algorithme **minimax** à travers un exercice progressif.

## Exercice retenu : le morpion (Tic-Tac-Toe)

L'exercice canonique pour apprendre minimax.

Pourquoi c'est le bon choix pour débuter :
- **Arbre minuscule**, explorable en entier (≤ 9! = 362 880 feuilles, en réalité bien moins). Pas besoin de profondeur limitée ni d'heuristique : on descend jusqu'aux fins de partie réelles → minimax dans sa forme pure.
- **Scores simples** : +1 (victoire IA), -1 (défaite), 0 (nul). Aucune heuristique à inventer.
- **Vérifiable** : un minimax correct est **imbattable** → on ne le bat jamais (au mieux match nul). Test de correction immédiat.
- **Récursion observable** : grille petite, on peut logguer l'arbre et les scores remontés.

## Progression prévue (difficulté croissante)

1. **Morpion — minimax nu**, profondeur complète.
2. **Morpion + élagage alpha-bêta** sur le *même* code → résultat identique, beaucoup moins de nœuds explorés. (Garder les deux concepts bien séparés.)
3. **Puissance 4 (Connect Four)** → arbre trop gros pour être exploré entièrement → introduction obligatoire d'une **profondeur limitée + fonction d'évaluation heuristique** (alignements de 2/3, contrôle du centre…). Vrai passage à l'échelle.
4. **Échecs / dames** → version « pour de vrai », gros morceau (génération des coups légaux…), à réserver pour plus tard.

## Lancer le projet
Ne pas ouvrir `morpion.html` en double-clic (`file://` → origine de sécurité isolée, blocages divers). Servir via un serveur HTTP local :

```bash
python3 -m http.server 8000
```

Puis ouvrir <http://localhost:8000/morpion.html>.

## Méthode
Commencer par le morpion en **deux temps** : d'abord minimax nu ; une fois que ça marche et qu'on ne le bat jamais, ajouter l'alpha-bêta sur le même code.

## Point de vigilance
Piège classique du morpion en minimax : **l'alternance min/max**. La fonction s'appelle récursivement en inversant le joueur — on *maximise* à son tour, on *minimise* au tour de l'adversaire. Si l'IA joue n'importe comment, c'est presque toujours là que ça cloche.

## Statut
- [x] Morpion — minimax nu ✅ *(IA imbattable)*
- [x] Morpion — alpha-bêta ✅ *(IA imbattable, ~8,9× moins de nœuds)*
- [x] Puissance 4 — profondeur + heuristique ✅ *(IA jouable et difficile à battre)*
- [ ] Échecs / dames (plus tard)

### Avancement détaillé — Morpion
Plomberie en place (jeu jouable) :
- [x] Affichage de la grille (`draw()` → `<table>`, injection dans `#game`)
- [x] Gestion des clics par **event delegation** sur le conteneur (`data-x`/`data-y` + `closest('td')`)
- [x] Pose des pions + alternance des joueurs (`play()`, `canPlay()`)
- [x] `win()` → retourne le gagnant (`'O'`/`'X'`) ou `null` (8 alignements)
- [x] `isFull()` + `isOver()` (gère le cas du match nul)

Minimax (fait ✅) :
- [x] Séparation logique / affichage : `draw()` sorti de `play()` (la simulation ne touche plus au DOM)
- [x] `clone()` via `Object.create(Morpion.prototype)` + `structuredClone(this.cells)` — on ne clone **que l'état**, pas le `container` (le DOM n'est pas clonable → erreur `HTMLDivElement could not be cloned`)
- [x] Cas de base / feuilles : `100 - depth` (X gagne), `-100 + depth` (O gagne), `0` (nul). Le `depth` fait **préférer les victoires rapides**.
- [x] Minimax récursif avec alternance max/min : `X` maximise, `O` minimise (`this.joueur` = à qui de jouer)
- [x] `bestMove()` séparé de `minmax()` : `minmax` ne renvoie **qu'un nombre** (le score), `bestMove` renvoie le coup `{x, y}`
- [x] IA branchée après le coup du joueur (dans `attachEvents`), annonce victoire **et** match nul (`announceIfOver()`)

### Bugs rencontrés (et leçons)
- **`structuredClone(this)` plante** car `Morpion` contient `this.container` (un nœud DOM, non clonable). → cloner uniquement l'état.
- **`return` dans un `forEach` ne retourne rien** à la fonction englobante (la valeur est jetée). → un minimax se fait avec une **boucle `for`**, pas `forEach`.
- **Réutiliser un seul clone pour tous les coups** = les coups s'accumulent. → un **clone neuf par coup** simulé.
- **Mélanger score et coup dans un seul retour** (`[score, x, y]`) provoquait `number is not iterable` : les feuilles renvoyaient un nombre, le reste un tableau. → **deux fonctions** (`minmax` = score, `bestMove` = coup).

### Simplifications / refacto
- Swap joueur lisible : `this.joueur === 'O' ? 'X' : 'O'` (au lieu du calcul via `charCodeAt`)
- Helper `emptyCells()` → dédoublonne les boucles de `minmax` et `bestMove` (`for (const {x,y} of ...)`)
- Gestion de fin de partie unifiée dans `announceIfOver()` (victoire + match nul)
- **Choix pédagogique** : `win()` / `isFull()` gardés en **2D explicite** (`c[0][0]`, `c[1][2]`…) plutôt que `.flat()` + indices `0..8`. Plus verbeux, mais on *voit* chaque alignement → plus clair pour apprendre. (`.flat()` aplatit un niveau d'imbrication : `[[a,b],[c,d]]` → `[a,b,c,d]`.)

### Instrumentation — comparer l'exploration (pour préparer l'α-β)
Objectif : **mesurer le gain de l'élagage**, pas changer le résultat.
- `Morpion.nodes` : compteur **statique** (partagé par tous les clones), incrémenté à chaque appel de `minmax()` = une grille testée.
- `measure(scoreMethod)` : remet le compteur à 0, lance `bestMove(scoreMethod)`, chronomètre avec **`performance.now()`** (haute précision, monotone, à utiliser **par différence**) et renvoie `{ move, nodes, timeMs }`. Renvoie `null` si la méthode n'existe pas encore → le bloc α-β s'activera tout seul.
- `bestMove(scoreMethod)` : générique, prend le **nom** de la fonction de score (`'minmax'` ou `'minmaxAB'`).
- UI : **une seule grille + deux panneaux de stats** (`#stats-mm`, `#stats-ab`), `drawStats()` / `renderStatsBox()`.

### Décision de conception : pourquoi UNE grille (et pas deux)
**L'α-β donne EXACTEMENT le même coup que le minimax** (même valeur). Ce n'est pas un meilleur algo de décision, juste le **même** avec un élagage. Donc deux grilles afficheraient la **même partie** → inutile. Ce qui diffère, c'est uniquement le **nombre de grilles explorées** → d'où **deux blocs de stats** côte à côte. (Nuance : en cas d'égalité de score entre plusieurs coups, le coup *retenu* peut différer selon l'ordre de parcours, mais les deux restent optimaux.)

### Rappels de notation (notes d'apprentissage)
- `100` dans le score = **arbitraire**, mais doit être **> profondeur max (9)** pour que le `- depth` (préférer les victoires rapides) n'inverse jamais l'ordre `victoire > nul > défaite`.
- `±Infinity` = bornes de départ de `best` : valeur « la pire possible » → garantit que le **premier vrai score** la batte. (`Infinity` n'est pas le plus grand nombre fini — ça, c'est `Number.MAX_VALUE` — mais une valeur spéciale > à tout nombre réel.)

### Alpha-bêta — fait ✅
`Morpion.prototype.minmaxAB(depth = 0, alpha = -Infinity, beta = Infinity)` : même structure que `minmax`, mais on **coupe** une branche dès que `alpha >= beta` (l'autre joueur ne la laissera jamais passer).
- **Mesure** : **61 044 → 6 833 grilles** au 1er coup, soit **~8,9×** moins de nœuds. Même coup joué que le minimax → IA toujours imbattable.
- **Mise à jour des bornes** dans la boucle : `X` relève `alpha = Math.max(alpha, best)`, `O` abaisse `beta = Math.min(beta, best)`.
- **Coupure** : `if (alpha >= beta) break;` — **`break`, pas `return`** (on renvoie le `best` déjà calculé).
- **Propagation** : l'appel récursif passe `minmaxAB(depth + 1, alpha, beta)` — sans ça, chaque enfant repart avec les bornes par défaut et ne coupe jamais.

### Compteurs par algo — routage par nom de propriété
Deux compteurs statiques : `Morpion.nodes` (minimax) et `Morpion.nodesAB` (α-β). `measure(scoreMethod)` doit reset/lire **le bon**.
- Table de routage : `Morpion.COMPTEURS = { minmax: 'nodes', minmaxAB: 'nodesAB' }`.
- Dans `measure` : `const compteur = Morpion.COMPTEURS[scoreMethod]; Morpion[compteur] = 0; ... nodes: Morpion[compteur]`.
- **Leçon JS** : on ne « passe pas un compteur par référence » — les **primitives sont par valeur**. L'indirection se fait via le **nom de propriété** (`Morpion[compteur]` ⇔ `Morpion.nodesAB`), pas via la variable. `Morpion` (objet) est partagé par référence, ses propriétés sont mutables.
- L'objet renvoyé par `measure` garde un champ **`nodes`** pour les deux algos (homogène → `renderStatsBox` lit `stats.nodes` sans connaître l'algo). À ne pas confondre avec le compteur `Morpion.nodesAB`.

### Bugs rencontrés (α-β)
- **Copier-coller depuis `minmax`** : `minmaxAB` incrémentait `Morpion.nodes` (au lieu de `nodesAB`) et rappelait `other.minmax()` (au lieu de `other.minmaxAB()`) → récursion qui retombe sur le minimax nu, compteurs faussés.
- **`this.statsAB.nodesAB`** dans `attachEvents` : l'objet stats n'a pas de champ `nodesAB` (juste `nodes`) → `total += undefined` = `NaN`. Corrigé en `this.statsAB.nodes`.

### Nuance : pas d'élagage à la racine
`bestMove` appelle `other[scoreMethod]()` **sans arguments** (pour rester générique entre `minmax` et `minmaxAB`) → chaque coup du 1er étage repart avec `alpha/beta` par défaut. L'élagage ne joue donc **pas entre les coups racines**, seulement *à l'intérieur* de chaque sous-arbre. Gain maximal possible (mais non retenu, pour garder `bestMove` générique) : threader `alpha` dans `bestMove`.

## Puissance 4 (Connect Four) — FAIT ✅

Démarré par **copie de `morpion.js`** (renommé `Puissance4`, un seul algo `minmaxAB`). Objectif de l'étape : arbre trop gros pour être exploré entièrement → passage obligé à une **profondeur limitée + fonction d'évaluation heuristique**. **Résultat : IA jouable et difficile à battre** — c'est le saut conceptuel central du projet (le morpion descendait toujours jusqu'aux feuilles, jamais d'heuristique).

### Plomberie (fait)
- [x] Plateau **6 lignes × 7 colonnes** : `cells[y][x]`, `y` = ligne (0..5), `x` = colonne (0..6). Gravité = fixer la colonne, descendre la ligne.
- [x] `play(x)` prend **une colonne** (un nombre) ; part de `let y = 5` (bas du plateau) et remonte à la 1ʳᵉ case libre.
- [x] `canPlay(x)` : boucle `for` (pas `forEach` — piège du `return` du morpion), bornes `x < 7`.
- [x] `availableColumns()` remplace `emptyCells()` : liste des **colonnes** jouables (≤ 7), pas des 42 cases → c'est ce qui borne le facteur de branchement.

### `win()` réécrit autour de `nbAlignes(j, nb)` — le vrai morceau de plomberie
`win()` ne teste plus 8 alignements en dur : il appelle `nbAlignes('O', 4)` puis `nbAlignes('X', 4)`.
- **`nbAlignes(j, nb)`** balaie les 4 directions (→ horizontal, ↑ vertical, ↗, ↖) et cherche `nb` pions identiques de `j` alignés. Bornes **généralisées en fonction de `nb`** : `x <= 7-nb`, `y >= nb-1`, `x >= nb-1` (au lieu des `3`/`4` codés en dur du morpion) → la **même fonction sert pour 4 (victoire) ET 2/3 (heuristique)**.
- **Détecteur, pas compteur** : `return j` au **premier** alignement trouvé (malgré le nom « nb »). Donc `evaluate` ne sait pas *combien* il y en a — une menace pèse autant que trois. Suffisant pour ce 1er jet.
- **`nb === 4`** → victoire pleine, on renvoie `j` direct. **`nb < 4`** → on n'accepte l'alignement que s'il a **au moins une extrémité immédiatement libre** (case vide juste avant/après la fenêtre) = un alignement *encore complétable*. (`nbVide` testé en deux temps avant d'être remplacé par ce `nb === 4`.)

### LE concept neuf : profondeur limitée + heuristique
`minmaxAB(depth, alpha, beta)` — ordre des tests **important** :
```
win() → ±10000 ∓ depth   (fins de partie réelles d'abord ; depth = préférer les victoires rapides)
isFull() → 0
depth >= MAX_DEPTH → this.evaluate()   (cutoff heuristique APRÈS les terminaux)
```
- **`evaluate()`** renvoie un **score signé depuis un point de vue fixe** (X positif, O négatif), pas « le joueur courant » — parce que X maximise / O minimise :
  ```js
  if (this.nbAlignes('X', 3)) score += 1000;
  if (this.nbAlignes('X', 2)) score += 100;
  if (this.nbAlignes('O', 3)) score -= 1000;
  if (this.nbAlignes('O', 2)) score -= 100;
  ```
- **Échelle** : victoire (10000) doit rester **>> heuristique** pour qu'une vraie victoire domine toujours.
- **`MAX_DEPTH`** : testé à 10 (très fort mais 1er coup lent), puis **ramené à 5** (bon compromis jouable/réactif). À profondeur faible, c'est **la qualité de `evaluate()` qui compte le plus**.

### Bugs rencontrés (et leçons) — Puissance 4
- **Sens de boucle inversé dans `win()`** : `for (y=5; y<=0; y--)` (et `y<=3`) → condition fausse d'entrée, la boucle **ne tourne jamais**. Seules les victoires verticales étaient détectées. → `y>=0` / `y>=3`. *(pendant du piège `return`-dans-`forEach` : faire matcher le **sens de la condition** avec le sens de l'itération.)*
- **Bornes codées en dur pour `nb=4`** dans 3 des 4 directions (`y>=3`, `x<4`…) : faux dès qu'on appelle `nbAlignes` avec `nb=2/3`. → généraliser en `nb-1` / `7-nb`.
- **Indice d'extrémité droite** : la case juste après une fenêtre `[x .. x+nb-1]` est `x+nb`, pas `x+nb+1`.
- **Noms `x`/`y` inversés dans `draw`** : la boucle externe (les lignes) était nommée `x`. Marchait par hasard car le handler lisait `data-y`. → renommé `(ligne, y)` / `(valeur, x)`, `data-x` = colonne, et le handler lit `dataset.x`.
- **`bestMove` plantait** : `for (const y of availableColumns())` puis `other.play(x)` avec `x` jamais déclaré → `ReferenceError`. Et `move = {x, y}` (forme morpion) au lieu de `move = x` (une colonne). 
- **L'IA « calculait mais ne jouait pas »** : le handler faisait `this.play(result.move.x)` — or `move` est un **nombre** (colonne), donc `.x` = `undefined` → `play(undefined)` ne pose rien. → `this.play(result.move)`.
- **Colonne 0 falsy** : `if (result && result.move)` ignore le coup quand l'IA choisit la colonne 0 (`0` est *falsy*). → `result.move !== null`.

### Refacto `nbAlignes` → vecteurs `(dx, dy)` — fait ✅
`nbAlignes` (~95 lignes, **4 blocs copiés-collés** ne différant que par les indices de parcours + le calcul d'extrémité libre) éclatée en **3 fonctions** (~30 lignes) pilotées par une table de directions.
- **`Puissance4.DIRECTIONS = [[1,0],[0,-1],[1,-1],[-1,-1]]`** (→ ↑ ↗ ↖). Une direction = un vecteur `(dx, dy)`. **Un seul sens par orientation suffit** : la fenêtre démarrant depuis *chaque* case, balayer `→` couvre déjà les lectures droite-à-gauche → 4 vecteurs, pas 8.
- **`inBounds(x, y)`** : test de bord **centralisé** (`0..6`, `0..5`). Les 3 bugs d'indices listés plus haut (sens de boucle, bornes en dur `nb=4`, `x+nb` vs `x+nb+1`) ne peuvent plus se reproduire par direction.
- **`aligne(j, nb, x, y, dx, dy)`** : la fenêtre de `nb` cases depuis `(x,y)` est-elle pleine de `j` ? (et si `nb < 4`, bordée d'au moins une extrémité vide → encore complétable). Dernière case = `(x+dx*(nb-1), y+dy*(nb-1))`, vérifiée via `inBounds` avant de lire.
- **`compteAlignes(j, nb)`** : triple boucle `lignes × colonnes × DIRECTIONS`, incrémente un total. **Vrai compteur** (≠ l'ancien détecteur qui renvoyait au 1er trouvé).
- **Effets en cascade** : `win()` devient `compteAlignes('O', 4) > 0` ; `evaluate` passe à `1000 * compteAlignes(...)` (au lieu de `if (...) 1000`) → l'IA distingue 1 menace de 3 → **meilleure à profondeur égale**. (Coche la 1ʳᵉ ligne du « Reste optionnel ».)
- **Leçon** : la duplication par copier-coller *fabrique* des bugs (chaque bloc réintroduit ses propres bornes). Paramétrer la variation (ici le déplacement) par des **données** (`(dx, dy)`) plutôt que par du code dupliqué → un seul endroit à corriger.
- Vérifié hors DOM (node, `Object.create(prototype)` + `cells` posées à la main) : victoires H/V/diag détectées, `.XXX.` complétable = 1, `OXXXO` bouché = 0.
- Limites inchangées (acceptables) : alignement **à trou** (`X.XX`) non vu ; fenêtres légèrement chevauchantes dans le comptage.

### Move ordering + threading `alpha` à la racine — fait ✅
Deux optimisations **qui ne changent pas le résultat** (même coup joué), seulement le nombre de nœuds explorés → permettent de pousser `MAX_DEPTH`.

**Move ordering (centre → bords).** `availableColumns()` renvoie désormais les colonnes triées par une table statique `Puissance4.ORDRE_COLONNES = [3, 2, 4, 1, 5, 0, 6]` (`.filter(x => canPlay(x))`).
- *Pourquoi le centre d'abord* : la colonne 3 participe au plus grand nombre d'alignements → souvent le meilleur coup. L'explorer en 1er resserre `alpha`/`beta` plus tôt → l'α-β coupe davantage de branches sœurs.
- *Effet* : gain de l'α-β qui tend vers `b^(d/2)` (ordre parfait) au lieu de `b^d` (mauvais ordre) — la **racine carrée** du nombre de nœuds. Ordre **statique** ≠ parfait → on capture une bonne partie, pas tout (~1-2 plis gagnés).
- *Propagation gratuite* : `minmaxAB` et `bestMove` itèrent déjà sur `availableColumns()` → une seule ligne change, l'ordre se répand partout.

**Threading `alpha` dans `bestMove`.** Avant, `bestMove` appelait `minmaxAB()` **sans bornes** → chaque sous-arbre racine repartait à `±Infinity` et n'élaguait pas contre les coups racines déjà évalués (la « nuance : pas d'élagage à la racine »).
- *Fix* : maintenir `alpha`/`beta` dans la boucle racine, appeler `minmaxAB(0, alpha, beta)`, resserrer après chaque coup (`maximise → alpha = max(...)`, sinon `beta = min(...)`).
- *Subtilité clé* : à la racine, `beta` reste `+Infinity` (pas de parent) → `alpha >= beta` ne se déclenche **jamais entre coups racines**, donc **pas de `break`** ici. Le gain n'est **pas** de couper des coups racines mais de **threader la borne vers le bas** : chaque sous-arbre enfant hérite d'un `alpha` serré et coupe ses branches sous ce seuil.
- *Combo* : avec le move ordering, le bon coup racine (colonne 3) est exploré en 1er → `alpha` serré dès la 1ʳᵉ itération → les colonnes suivantes héritent d'une borne tendue. C'est ce qui rend `MAX_DEPTH` élevé réellement abordable.
- **`MAX_DEPTH` poussé à 7** (était 5) une fois ces deux optims en place.

### Reste optionnel (améliorations, non bloquantes)
- ~~**Comptage au lieu de détection** dans `evaluate`~~ → fait avec le refacto `compteAlignes`.
- ~~**Move ordering** (explorer le centre d'abord)~~ → fait (`ORDRE_COLONNES`) + threading `alpha` à la racine.
- **Bonus centre** dans `evaluate` (différent du move ordering : ici on *score* l'occupation du centre, pas l'ordre d'exploration).
- ~~Nettoyage : `Puissance4.nodesAB` (l.7) déclaré mais mort~~ → supprimé ; la ligne 7 déclare désormais le vrai compteur `Puissance4.nodes`.
