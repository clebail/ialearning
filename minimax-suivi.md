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
4. **Awalé (Oware)** → on **réutilise 100 % du moteur** (α-β, profondeur, eval, move ordering) ; le seul code neuf est la **génération + application d'un coup** (semaille + capture), et il est *court* car le choix d'un coup est trivial (≤ 6 trous). Bon barreau avant les échecs : la « génération des coups » qui fait peur y est justement la partie facile.
5. **Échecs** → version « pour de vrai », gros morceau (génération des coups *légaux* : roque, prise en passant, promotion, filtrage de l'échec…), à réserver pour plus tard.

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
- [ ] Awalé (Oware) — prochaine étape (réutilise le moteur ; code neuf = semaille + capture)
- [ ] Échecs (plus tard, gros morceau)

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

### Fenêtres pré-calculées — sortir la géométrie du point chaud — fait ✅
Le triple `for` de `compteAlignes` (`lignes × colonnes × DIRECTIONS`) était propre mais **dans le point chaud** : `evaluate` est appelé à chaque feuille de l'arbre et relance `compteAlignes` **4 fois** (X-3, X-2, O-3, O-2) → ce balayage tournait des **millions de fois par coup**. Gêne : on **recalcule une géométrie qui ne change jamais**.
- **Idée** : la liste de toutes les fenêtres de `nb` cases alignées est **fixe** → la calculer **une seule fois** au chargement, pas à chaque appel.
- **`construitFenetres(nb)`** (fonction module, exécutée 1×) : refait le triple `for`, mais produit pour chaque fenêtre ses `cases` (liste de `[x,y]`) + ses extrémités `avant`/`apres` (déjà bornées, `null` si hors plateau). `inBounds`/`aligne` supprimées ; `dansPlateau(x,y)` devient une simple fonction (plus besoin d'être une méthode d'instance, elle ne sert qu'au build).
- **`Puissance4.FENETRES = { 2, 3, 4 }`** : la géométrie figée, indexée par longueur. Comptes : **69** fenêtres de 4 (= le nombre canonique d'alignements au Puissance 4 7×6 → validation), 98 de 3, 131 de 2.
- **`compteAlignes` devient une boucle PLATE** sur `FENETRES[nb]` : « toutes mes cases sont-elles `j` ? » puis test d'extrémité libre. **Plus aucun calcul d'indice à chaud** (`x+dx*i`…), juste des lectures `this.cells[y][x]`.
- **Leçon** : le triple `for` n'a pas été *supprimé*, il a **migré dans le setup** (1 exécution au lieu de millions). Quand une boucle laide est sur le chemin critique, la bonne réponse n'est ni de la laisser, ni de la « fonctionnaliser » (`.flatMap().filter()` alloue dans la boucle chaude !) mais de **sortir l'invariant** : ici, la géométrie fixe du plateau.
- Vérifié hors DOM : résultats **identiques** à la version vecteurs (victoires H/V/diag, `.XXX.` = 1, `OXXXO` = 0).

### Évaluation par CONTENU de fenêtre — corrige le trou + le blocage — fait ✅
Suite logique des fenêtres pré-calculées : puisqu'on a déjà toutes les fenêtres de 4, autant **classer chaque fenêtre par son contenu** au lieu de chercher des alignements contigus + tester une extrémité libre. **Change le comportement** de l'IA (≠ refacto iso-résultat) — c'est une *meilleure* heuristique, à juger en jouant.
- **Bascule conceptuelle** : on arrête de chercher « `nb` pions contigus de `j` ». Pour chaque fenêtre de 4, on compte `nbX` / `nbO`, puis :
  ```js
  if (nbX && nbO) continue;          // fenêtre MORTE : les deux camps présents
  if (nbX === 3) score += 1000;      // 3 pions + 1 vide = grosse menace
  else if (nbX === 2) score += 100;  // amorce
  // ... idem 'O' en négatif
  ```
- **Le trou est géré gratuitement** : `X.XX` = 3 X + 1 vide dans la fenêtre → compté (scoré 1200 au test, **0 avant**). Plus aucune notion de contiguïté.
- **Le blocage devient EXACT** : `if (nbX && nbO) continue` — une fenêtre contenant un pion adverse ne peut plus être complétée, point. L'ancien test « extrémité libre » n'en était qu'une **approximation**. `OXXXO` → 0 (fenêtres mortes).
- **Double menace bien pondérée** : `.XXX.` → 2100 (deux fenêtres de 4 contiennent les 3 X) > une simple menace.
- **Nettoyage en cascade** : `compteAlignes` **supprimée** (win et evaluate font désormais leur propre boucle de fenêtres) ; `construitFenetres` ne prend plus `nb` et **ne stocke plus `avant`/`apres`** ; `FENETRES` redevient un **tableau plat de 69** fenêtres (fini l'indexation par longueur, plus de tables 2/3). `win()` reréécrit en boucle sur `FENETRES` (`cases.every(... === j)`).
- **Leçon** : une bonne structure de données (les fenêtres) ne fait pas que ranger le code — elle **débloque une meilleure logique** (ici : raisonner sur le *contenu* plutôt que sur la *forme* d'un alignement). La simplification et l'amélioration de l'IA sont venues ensemble.

### Bonus centre — heuristique positionnelle — fait ✅
Dernier terme d'`evaluate` : récompenser l'**occupation de la colonne du milieu** (colonne 3).
- **Pourquoi** : une case n'appartient pas au même nombre de fenêtres selon sa position — le centre est traversé par le **maximum** (~13 fenêtres) contre ~3 pour un coin. Un pion central est donc **plus flexible** (participe à plus de menaces potentielles). `evaluate` ne le voyait qu'*indirectement* (via menaces déjà formées) ; le bonus le rend **explicite**, dès le coup posé.
- **Code** : `for (y) { centre = cells[y][3] ; X → +30 ; O → -30 }`, constantes `COLONNE_CENTRE = 3` / `BONUS_CENTRE = 30`.
- **Le point délicat = le poids**. Respecter la hiérarchie `victoire (10000) >> 3 (1000) > 2 (100) >> bonus centre`. `30` est volontairement **petit** : un **départage**, pas une priorité. Trop gros (ex. 200) → l'IA empilerait au centre au lieu de bloquer une vraie menace. Il n'agit que quand les coups sont par ailleurs à égalité — **typiquement en début de partie**, où aucune menace n'existe et où `evaluate` renvoie 0 partout : il sert de boussole quand l'heuristique de menaces est muette.
- **À ne pas confondre avec le move ordering** (les deux aiment le centre, mais) : move ordering = **ordre d'exploration** (accélère l'α-β, *même* coup) ; bonus centre = terme du **score** (change la décision). L'un dit « regarde le centre en 1er », l'autre « le centre vaut plus cher ».
- Vérifié : `X` centre → +30, `O` centre → −30, bord → 0, et menace (+100) + centre (+30) = 130 → la menace domine bien.

### Reste optionnel — épuisé ✅
Toutes les améliorations listées ont été faites :
- ~~**Comptage au lieu de détection** dans `evaluate`~~ → fait (refacto `compteAlignes`, puis évaluation par contenu de fenêtre).
- ~~**Move ordering** (explorer le centre d'abord)~~ → fait (`ORDRE_COLONNES`) + threading `alpha` à la racine.
- ~~**Bonus centre** dans `evaluate`~~ → fait (`BONUS_CENTRE`, poids modéré).
- ~~Nettoyage : `Puissance4.nodesAB` (l.7) déclaré mais mort~~ → supprimé ; la ligne 7 déclare désormais le vrai compteur `Puissance4.nodes`.

**Bilan Puissance 4** : IA jouable et difficile à battre, code d'évaluation propre (fenêtres pré-calculées + contenu), α-β optimisé (move ordering + threading racine). Prochaine étape du projet : **Awalé**.

### Choix « qui commence » (2 radios) — fait ✅
Deux radios au-dessus de la grille (« Moi » / « L'IA »). À chaque changement, **partie neuve**. Si l'IA commence, son 1er coup est **forcé au centre** (colonne 3) — pas de minimax pour l'ouverture (le Puissance 4 est résolu : 1er joueur gagnant au centre).
- **Réutiliser l'instance via `reset(iaCommence)`**, pas un `new Puissance4` par changement : le listener de clic est posé sur `#game` en **délégation** → recréer l'objet **empilerait** un listener par partie (chaque clic déclencherait N handlers). Donc : `attachEvents()` **une seule fois** dans le constructeur, qui délègue ensuite l'init d'état à `reset(false)`.
- `reset` : vide `cells`, `joueur = 'O'` (l'humain), compteurs à 0, `statsAB = null`, redessine. Si `iaCommence` : `joueur = 'X'` puis `play(COLONNE_CENTRE)` (qui rebascule `joueur` vers `'O'` → à l'humain de jouer).
- **Leçon** : avec la délégation d'événements, le handler vit sur le **conteneur** (qui survit aux redraws), pas sur les cases. Donc on **ne réattache jamais** ; on ne fait que réinitialiser l'état. `location.reload()` était exclu : il remettrait les radios à leur défaut, perdant le choix « IA commence ».

### Plateau en tableau plat `Uint8Array(42)` — refacto perf — fait ✅
On remplace le **tableau 2D de chaînes** (`[['','',...], ...]`, 6×7 strings `'O'`/`'X'`/`''`) par un **`Uint8Array(42)` plat**, indexé `y * 7 + x`. Refacto **iso-comportement** (l'IA joue pareil) — uniquement la représentation du plateau change. Motivation : le minimax explore des millions de grilles, chacune **clonée** à chaque nœud → le coût de la copie domine.
- **Encodage entier** : constantes `VIDE = 0`, `HUMAIN = 1` (`'O'`), `IA = 2` (`'X'`), et une table `SYMBOLE = ['', 'O', 'X']` pour reconvertir **entier → caractère à l'affichage uniquement** (`draw`, alerte de victoire). Tout le reste du code raisonne en entiers.
- **Clonage : `structuredClone` → `this.cells.slice()`**. C'est le vrai gain. Cloner un tableau 2D de strings = copie profonde de 6 sous-tableaux + 42 références de chaînes ; cloner un `Uint8Array` = **recopie plate de 42 octets contigus**, sans allocation par case. C'est l'opération du **point chaud** du minimax.
- **`canPlay` accéléré au passage** : la colonne est jouable ssi la case du **haut** est libre → `cells[x] === VIDE` (ligne 0 ⇒ index `0*7+x = x`), un seul accès au lieu de scanner la colonne de bas en haut.
- **Conversions en cascade** (toutes les comparaisons `=== 'X'/'O'/''` → entiers, tous les accès `cells[y][x]` → `cells[y*7+x]`) : `play`, `draw`, `win`, `isFull`, `minmaxAB`, `bestMove`, `evaluate`, `announceIfOver`. `win()` retourne désormais l'**entier** du gagnant (`HUMAIN`/`IA`) → l'alerte fait `SYMBOLE[winner]`. `isFull` se simplifie : `cells.every(c => c !== VIDE)` (plus de double boucle de lignes).
- **Piège du refacto à mi-chemin** : le haut du fichier était converti mais le bas (`win`/`isFull`/`minmaxAB`/`evaluate`/…) lisait encore l'ancien encodage 2D → plateau incohérent (mélange `cells[y][x]` et `cells[y*7+x]`). Leçon : un changement de représentation doit balayer **toutes** les fonctions d'un coup ; un `grep` final sur `\]\[`, `=== 'X'`, `=== 'O'`, `=== ''` confirme qu'il ne reste plus un seul accès à l'ancienne forme (hors commentaires et table `SYMBOLE`).
- **Leçon** : choisir la structure de données pour le **point chaud**. Ici le minimax est dominé par le clonage à chaque nœud — un `Uint8Array` se copie en un `memcpy` de 42 octets, là où le tableau 2D de strings paie allocation + GC à chaque grille explorée.

### Stats : temps de réflexion cumulé + `MAX_DEPTH` affiché — fait ✅
Petits ajouts à la box de stats α-β, dans la foulée du portage C++ (comparer les perfs JS ↔ C++).
- **Cumul du temps de réflexion sur la partie** : `totalTimeMs` (remis à 0 dans `reset`, incrémenté de `statsAB.timeMs` à chaque coup IA). On a déjà le total des grilles testées (`totalAB`) ; on ajoute la **durée cumulée** — `renderStatsBox` prend un paramètre `totalTime` de plus, affiché en bas de la box.
- **`MAX_DEPTH` affiché** dans la box (profondeur de recherche), pour rendre lisible le paramètre qui pilote temps de calcul ↔ force de l'IA.
- **`MAX_DEPTH` : 6 → 7**. Un cran plus profond = IA plus forte, au prix d'un temps de réflexion (et d'un nombre de grilles) en hausse — précisément ce que la box rend désormais visible.

## Awalé (Oware) — prochaine étape (pas encore commencé)

Choisi **à la place des dames** : on **réutilise tout le moteur** (minimax, α-β, profondeur limitée, `evaluate`, move ordering, threading racine) — le seul code neuf est la mécanique du jeu. Et la « génération des coups légaux » (ce qui fait peur pour les échecs) y est **triviale**.

### Pourquoi c'est un bon cran
- Jeu **déterministe, parfait, à somme nulle, sans hasard** → terrain idéal de minimax, comme le Puissance 4.
- **Facteur de branchement ≤ 6** (au plus 6 trous jouables) → arbre encore plus petit que le Puissance 4.
- `coupsJouables()` = « mes trous non vides ». Pas de géométrie, pas d'échec à filtrer, pas de roque/promotion. **Une ligne.**

### Le vrai travail : `play()` (semaille + capture)
La difficulté n'est pas dans le *choix* du coup mais dans son *application* :
1. **Semaille** : prendre les N graines d'un trou, les semer une à une dans les trous suivants (sens antihoraire, en bouclant sur le plateau).
2. **Capture** : si la **dernière** graine tombe dans un trou **adverse** qui passe à **2 ou 3**, on les rafle ; puis on **remonte** tant que les trous adverses précédents font aussi 2 ou 3.
3. Deux règles spéciales : **« nourrir l'adversaire »** (s'il n'a plus de graines, on *doit* jouer un coup qui lui en donne) et le **« grand chelem »** (un coup raflant *toutes* ses graines est interdit / non capturant selon la variante).

### Plomberie & éval (esquisse)
- Plateau : **2 rangées × 6 trous**, 4 graines chacun (48 au total) + **2 greniers** (graines engrangées).
- Fin de partie : un joueur dépasse **24 graines** (majorité), ou ne peut plus jouer.
- `evaluate` : différence des **greniers** d'abord, puis raffinements (graines menacées de capture, trous bien chargés…). Esprit identique à l'eval Puissance 4.

### Point de vigilance anticipé
Comme au Puissance 4 (`bestMove` qui retournait un objet au lieu d'un nombre), bien fixer **ce que représente un coup** : ici un coup = **l'indice d'un trou** (un nombre), pas une structure. Et la **capture remonte en arrière** → attention au sens de boucle (le piège récurrent du projet).

### Notes conceptuelles — force d'une IA de jeu (discussion)
Point de départ (intuition de Corentin) : *« sur les jeux plus gros que le morpion, pour que l'IA soit très forte voire imbattable, tout se passe dans l'heuristique. »* → vrai noyau, mais à nuancer.

- **Noyau juste** : dès que l'arbre est trop gros pour atteindre les feuilles, on s'arrête sur des positions non terminales. La seule chose qui dit si la position est bonne, c'est `evaluate()`. Elle *remplace* le vrai résultat qu'on n'a pas le temps d'aller chercher. Au morpion elle n'existait même pas (on descendait toujours jusqu'au bout).
- **Mais la force est un PRODUIT**, pas l'heuristique seule :
  ```
  force ≈ profondeur atteinte  ×  qualité de l'évaluation
  ```
  Les deux se compensent : éval parfaite + profondeur 1 → se fait piéger en 2-3 coups ; éval médiocre + grosse profondeur → joue bien quand même ; **profondeur infinie → l'éval devient inutile** (on retombe sur le minimax pur du morpion = jeu parfait). L'heuristique est une **béquille pour compenser le manque de profondeur** : plus on voit loin, moins elle compte.
- **Là où rentrent les « optimisations »** (move ordering, tables de transposition, α-β…) : elles **ne changent pas le résultat à profondeur égale**, mais laissent descendre **plus profond dans le même temps** → IA plus forte *sans toucher à l'heuristique*. C'est l'autre moitié du travail (sur les échecs, la plus grosse).
- **« Imbattable » ≠ heuristique** : une heuristique est une approximation, elle peut se tromper → ne rend jamais imbattable. Imbattable = jeu **résolu** (vu jusqu'au bout, comme le morpion). Anecdote : le **Puissance 4 est un jeu résolu** (le 1er joueur gagne en commençant au centre) — résolu par recherche profonde + tables, *pas* par une super heuristique.

### AlphaZero — le prolongement (intuition de Corentin : « remplacer l'heuristique à la main par un réseau appris »)
Intuition correcte et non triviale : c'est *littéralement* l'idée centrale. Trois raffinements au-delà du simple « swap `evaluate()` → réseau » :
1. **Le réseau attaque les DEUX moitiés** de `force = profondeur × éval`. Deux sorties : une **value** (« qui gagne ? » = `evaluate()`) et une **policy** (« quels coups valent le coup d'œil ? » = le **move ordering**).
2. **Abandon de l'α-β pour MCTS** (Monte-Carlo Tree Search) : recherche *guidée par la policy* (concentre les simulations sur les coups prometteurs) au lieu de bornes alpha/beta. La recherche reste là — pilotée par le réseau.
3. **D'où viennent les données ? Self-play.** Personne n'étiquette les positions : AlphaZero **joue contre lui-même** et entraîne le réseau sur le résultat de sa propre recherche MCTS (le joueur « avec recherche » est plus fort que le réseau seul → cible d'apprentissage). Boucle qui se tire vers le haut, depuis zéro connaissance humaine.

**Décision** : on reste sur **minimax α-β à la main** pour l'instant. Piste future (après échecs/dames) évoquée : garder l'α-β du Puissance 4 mais remplacer `evaluate()` bricolé par une petite éval **apprise** — pour goûter à l'idée sans tout le MCTS/self-play.

## Projet vidéo

> **Plan de la vidéo « du JS au C++ »** (synopsis 1 motivations · 2 JS · 3 C++ · 4 verdict) :
> document dédié [`plan-video.md`](plan-video.md), étoffé au fur et à mesure.

### Confronter mon IA Puissance 4 à une IA réputée

Idée : prendre un site réputé pour son IA Puissance 4 et l'opposer à la mienne.

### La prémisse « ça finira toujours nul » est fausse
Le Puissance 4 est un **jeu résolu**, mais pas vers le nul : **avec jeu parfait, le 1er joueur gagne** (en démarrant au centre). L'issue est donc déterminée par **qui commence**, pas par un nul. Et surtout, **mon IA n'est pas parfaite** (α-β profondeur 7 + heuristique = approximation) → face à un adversaire parfait, l'écart se verra. Donc il y a un vrai intérêt : c'est la **démonstration empirique** de la note « force = profondeur × éval » et « imbattable = jeu résolu, pas heuristique ».

### Ce que la confrontation révèle (2 scénarios)
- **Mon IA commence** (centre forcé) → position théoriquement gagnante : sait-elle *convertir* la victoire face à une défense parfaite ? (probable qu'elle lâche le gain → nul/défaite). **Le vrai test.**
- **Le solveur commence** → il gagne (parfait) ; mon IA part d'une position perdue → question : à quel coup elle craque, tient-elle longtemps ?

Contenu fort : à **chaque coup de mon IA**, comparer sa colonne au score optimal du solveur → on voit l'instant précis où l'heuristique se trompe (et chiffrer « combien de coups optimaux avant la 1re erreur »).

### Adversaire retenu : le solveur de Pascal Pons (l'oracle parfait)
**<https://connect4.gamesolver.org>** — LA référence (Puissance 4 résolu depuis 1988). Affiche le **score de chaque colonne** → idéal pour l'analyse coup-par-coup. Alternatives à difficulté réglable (match plus disputé, plus spectaculaire) : **play4row.com/connect-4-ai** (niveaux ; max = parfait via base du jeu résolu), **jsreact.com/connect4** (Random→Master). Mise en scène : segment 1 = vs solveur parfait (démo pédago), segment 2 = vs play4row niveau moyen (spectacle).

### L'API gamesolver.org (non documentée mais publique, testée ✅)
```
GET https://connect4.gamesolver.org/solve?pos=<séquence>
```
- **`pos`** = suite des colonnes jouées, **1-indexées** (gauche `1` … droite `7`), dans l'ordre des coups. Ex. `44` = les deux au centre.
- **Réponse** : `{"pos":"44","score":[-3,-3,-2,1,-2,-3,-3]}` — un score **par colonne** (1→7), **du point de vue du joueur au trait**. **signe** = issue (`>0` gagnant, `0` nul, `<0` perdant) ; **magnitude** = vitesse (plus grand = victoire plus rapide / défaite plus lointaine) ; colonne pleine/illégale ≈ valeur sentinelle `100`.
- Validé : sur `44`, meilleure colonne = la **4** (`+1`) → confirme « centre = coup gagnant du 1er joueur ».
- **Piège CORS** : un `fetch` direct depuis la page navigateur (localhost) sera probablement **bloqué** (c'est leur backend, pas une API ouverte). → appeler **côté serveur** (script Node) ou via un **petit proxy local**. Rester poli : 1 appel/coup, pas de boucle agressive (service gratuit, non documenté → peut casser).
- Pas d'API publique pour play4row / jsreact (scraping fragile, ou jeu à la main).

### Plan retenu
1. **Réécrire le moteur d'IA en C++** pour de meilleures perfs (descendre plus profond dans le même temps → IA plus forte sans toucher à l'heuristique, cf. la note « optimisations »).
2. **Brancher au site dans un second temps** (via l'API gamesolver.org, appelée hors navigateur) pour produire l'analyse coup-par-coup de la vidéo.

### Portage C++ (Qt) — en cours
Projet Qt Widgets sous `cpp/puissance4/` (Qt Creator). Étape 1 du plan vidéo.

**État du plateau — bitboard (fait ✅).** La classe `Puissance4` stocke la grille dans **6 `unsigned short`** (`board[NB_ROW]`), **2 bits par case** (`CELL_BIT_COUNT`) : `0` = vide, `1` = O, `2` = X. Accès via `getCell(col, row)` / `setCell(col, row, value)` avec décalage `CELL_BIT_COUNT * (NB_COL - col - 1)` et masque `0x3`. Lecture/écriture validées.
- *Note* : `setCell` fait `board[row] &= ~newValue` avant le `|=` — efface seulement les bits **posés** par la nouvelle valeur, pas les 2 bits de la case (OK pour écrire sur une case vide ; à revoir si on veut écraser une case déjà occupée).
- **Bornes : garde silencieuse → `Q_ASSERT` (fait ✅).** Les deux fonctions testaient les indices avec un `if` qui renvoyait `0` / ne faisait rien hors plateau. **Inutile** : *tous* les appelants passent déjà des indices valides (`canPlay` valide `col` ; `play` borne `row` via l'invariant « haut vide ⇒ colonne non pleine » ; `FENETRES` est in-bounds par construction ; `paintEvent` boucle dans les bornes). Et **nuisible** : (1) une branche par accès sur `getCell`, *le* point chaud du futur minimax — à rebours de l'objectif perf du port ; (2) ça **masque les bugs** (un `setCell` OOB qui n'écrit rien = pion qui disparaît sans plantage). Remplacé par `Q_ASSERT(col >= 0 && col < NB_COL && row >= 0 && row < NB_ROW)` : **zéro coût en release** (effacé sous `QT_NO_DEBUG`), **plantage net en debug** si un appelant viole la précondition, et le contrat est documenté. Sémantique « OOB → 0/no-op » abandonnée (personne ne s'en servait ; à reconsidérer seulement si un `evaluate` voulait lire des voisins hors plateau — inutile ici grâce aux fenêtres pré-calculées). *Leçon* : une garde défensive qui ne se déclenche jamais en usage correct ne protège de rien — elle ne fait que coûter et cacher. Préférer l'`assert` (contrat vérifié en debug, gratuit en release) à la garde silencieuse.

**Affichage — `WPuissance4::paintEvent` (fait ✅).** Widget custom (`wpuissance4.cpp`) qui peint le plateau au `QPainter` :
- **Ronds, jamais d'ovales** : `cell = qMin(width()/NB_COL, height()/NB_ROW)` → cases carrées, disques aussi gros que possible, grille **centrée** (`offsetX`/`offsetY`).
- **Gravité respectée** : `row 0` = bas du plateau → dessiné en bas via `(NB_ROW - 1 - row)`.
- **Look Puissance 4** : fond gris-bleu **pastel** (`176,182,191`), cases vides = ronds blanc cassé (`245,245,245`), O = rouge pastel (`240,138,138`), X = bleu pastel (`130,170,230`). `Antialiasing` + `setPen(Qt::NoPen)`.
- `setBoard(Puissance4*)` stocke le pointeur et `repaint()`.

**Icône (fait ✅).** `icon.svg` (pastel) → `icon.png` en ressource `.qrc` (`windowIcon`) ; sur macOS le Dock exige le bundle → `icon.icns` via `macx: ICON =` (Info.plist). `win32: RC_ICONS` prévu.

**Génération de coups (fait ✅), optims dès l'écriture :**
- `canPlay(col)` **`O(1)`** : gravité ⇒ jouable ssi case du **haut** vide. Garde `col` obligatoire (`getCell` renvoie 0 hors plateau).
- `availableColumns` : renvoie le **nombre** de colonnes jouables ; **move ordering centre→bords** `COLUMNS_ORDER = {3,2,4,1,5,0,6}` (`static const int[]`).
- `play(col)` : pose sur la 1ʳᵉ case libre **depuis le bas**, swap `player = 3 - player`. **Bug corrigé** : 1ʳᵉ version scannait du haut (`getCell(col, row--)`) → boucle morte → tout pion en `row 4`.

**Optim suivante (avec le moteur) : cache `height[NB_COL]`** → `play`/`canPlay` `O(1)` et surtout prépare `undo()` (make/undo sans cloner le plateau, ≫ `structuredClone` JS). À ajouter avec `minmaxAB`, pas avant.

**Fenêtres `constexpr` + `win()` (fait ✅) — la géométrie calculée À LA COMPILATION.**
Pendant C++ des « fenêtres pré-calculées » du JS, mais poussé d'un cran. En JS le mieux atteignable était « calculer 1× **au chargement** » (`construitFenetres` exécuté une fois à l'init). En C++, le mot-clé **`constexpr`** descend encore : le calcul a lieu **1× À LA COMPILATION** → le binaire contient le tableau déjà figé, **zéro calcul à l'exécution, pas même à l'init**. C'est exactement l'intuition de Corentin « on ne pourrait pas fixer `FENETRES` sans calcul ? » — oui, mais via le compilateur, **sans** taper 276 coordonnées à la main (= 69 fenêtres × 4 cases).
- **`constexpr` = « évaluable à la compilation »** (notion nouvelle pour Corentin). Une fonction/variable `constexpr` que le compilateur *peut* exécuter pendant le build. Ici `construitFenetres()` est `constexpr` et son résultat est affecté à `constexpr std::array<...> FENETRES` → ça **force** l'évaluation au build. Le triple `for` (colonnes × lignes × directions) **n'a pas disparu** : il a migré du *runtime chaud* (millions d'appels via `evaluate`) vers le *compilateur* (une fois).
- **Structures** : `struct Cell { int col; int row; }` (sert aussi de vecteur direction), `struct Fenetre { Cell cases[4]; }`. `FENETRES` = `std::array<Fenetre, 69>` en **portée fichier** (namespace anonyme du `.cpp`), pas un membre — seul `win()`/`evaluate` (dans le `.cpp`) le consomment.
- **Validation AU BUILD — le « truc du `throw` »** : à la fin du générateur, `if (n != NB_FENETRE) throw "...";`. En contexte `constexpr`, **atteindre un `throw` rend l'évaluation non-constante → erreur de compilation**. Donc compte trop **bas** → `throw` atteint → build cassé ; compte trop **haut** → dépassement du `std::array` → build cassé. C'est l'**équivalent compile-time du « recompter 69 »** : si la géométrie est fausse, ça ne **compile pas** (vérifié : forcer `NB_FENETRE=70` → `error: subexpression not valid in a constant expression`).
- **C++17 requis** (le `.pro` a `CONFIG += c++17`) : boucles + mutation de variable locale dans une fonction `constexpr`, et `std::array::operator[]` non-const `constexpr`.
- **Directions** : `{{1,0},{0,1},{1,1},{1,-1}}` (H, V, diag montante, diag descendante). **Un seul sens par orientation** (comme en JS) : chaque fenêtre démarre depuis chaque case → balayer « → » couvre déjà les lectures inverses. Test de bord : il suffit de vérifier la **dernière** case (`col+d*3`).
- **`win()` branchée** (1er consommateur) : boucle plate sur `FENETRES`, `j = 1re case ; si les 4 == j → true`. Forme de la version JS finale (boucle sur `FENETRES`, `cases.every`). Renvoie `bool` (un alignement existe), pas le gagnant.
- **Nettoyage en cascade** : retiré le membre mort `DIRECTIONS` (les vecteurs vivent dans le générateur), les `#define NB_DIRECTION`/`NB_AXE` orphelins, et des vestiges `fenetres[]`/`construitFenetres()` (approche « membre » abandonnée). **Réparé la compilation au passage** : le `.cpp` définissait `construitFenetres()` **absente du `.h`** (erreur) et `win()` était un stub **sans `return`**.
- **Vérifié hors Qt** (g++ `-std=c++17`, shim `QtDebug` vide) : compile sans warning ; table = **69** fenêtres, réparties **24 H / 21 V / 12 montantes / 12 descendantes**, **0** case hors plateau.
- **Leçon** : la leçon JS « sortir l'invariant du point chaud » a un cran de plus en C++ — l'invariant peut sortir jusque **dans le compilateur**. Et `constexpr` permet de garder la **boucle génératrice** (lisible, auto-validante) tout en obtenant un tableau aussi figé qu'un littéral codé en dur — sans le risque de typo d'un littéral à la main (cf. la leçon « la duplication copier-coller fabrique des bugs »).

**Jeu jouable au clic — humain vs humain (fait ✅).** `WPuissance4::mouseReleaseEvent` traduit le clic en coup :
- **`cell`/`offset` factorisés** : la géométrie (`cell = qMin(...)`, `offsetX`/`offsetY`) était calculée *inline* dans `paintEvent` ; sortie dans un helper `cellMetrics(offsetX, offsetY)` **partagé** par le dessin ET le mapping clic→colonne → impossible qu'ils dérivent (si l'un change la taille de case, l'autre suit). Leçon JS « sortir l'invariant » appliquée ici à la *cohérence*, pas à la perf.
- **`columnAt(x)`** : `(x - offsetX) / cell`, avec garde `x < offsetX` (marge gauche) et `col >= NB_COL` (marge droite) → `-1` si hors grille. Un coup = **une colonne (un `int`)**, pas une structure (rappel du piège JS `move = x`).
- **`mouseReleaseEvent`** : ignore si pas de plateau / partie finie / colonne pleine ; sinon `play(col)` (qui alterne déjà `player = 3 - player`) → `repaint()`. **Fin de partie** : `win()` → `QMessageBox` « Le joueur N gagne ! » ; sinon `availableColumns == 0` → « Match nul ! ». Drapeau **`gameOver`** dans le widget gèle les clics ensuite.
- **`release` plutôt que `press`** : clic plus naturel (annulable en relâchant ailleurs). L'IA viendra plus tard remplacer le 2ᵉ joueur via `minmaxAB`.

**Bouton « Rejouer » (fait ✅).** Enchaîner les parties sans relancer l'appli.
- **`Puissance4::reset()`** : l'init (plateau vidé via `memset`, `player = 1`) a été **factorisée** hors du constructeur, qui l'appelle désormais — une seule définition de « partie neuve ».
- **`WPuissance4::reset()`** (slot public) : `board->reset()` + `gameOver = false` (les clics repassent) + `repaint()`.
- **Câblage** : `QPushButton rejouerButton` ajouté au `QVBoxLayout` de `mainwindow.ui` (sous la grille ; politique verticale fixe du bouton → la grille garde l'espace). `connect(rejouerButton, &QPushButton::clicked, wPuissance4, &WPuissance4::reset)` dans le constructeur de `MainWindow` (syntaxe pointeur-de-membre, pas de macro `SLOT`). Le type `WPuissance4` est connu via `ui_mainwindow.h` (custom widget).

**Titre du `QMessageBox` (fait ✅).** Bug visuel : la boîte de fin de partie s'affichait « **Puissance 4<1>** ». Cause : le `QMessageBox` reprenait `"Puissance 4"` comme titre, **identique à celui de la fenêtre principale** → le gestionnaire de fenêtres désambiguïse deux fenêtres homonymes en suffixant `<1>`. Corrigé en donnant un titre **distinct** (et plus parlant) : `"Fin de partie"`. Au passage, `.arg(who)` → `.arg(int(who))` : `who` est un `unsigned char` ; le cast force l'overload **entier** de `QString::arg` (affiche « 1 »/« 2 ») et écarte l'overload `QChar` qui aurait inséré le caractère de contrôle U+0001 (autre `<1>` possible, dans le corps cette fois). *Leçon* : un titre de dialogue doit différer de celui de la fenêtre parente.

**Mise en page — bloc stats + radios « qui commence » (préparé ✅, pas encore alimenté).** On réplique l'UI du JS, en placeholder tant que l'IA n'existe pas.
- **Layout restructuré** : le `QVBoxLayout` central devient un **`QHBoxLayout`** — à gauche la zone de jeu (grille + ligne du bas dans un `QVBoxLayout`), à droite le panneau de stats. La grille passe en `sizePolicy` **Expanding** (+ `minimumSize` 350×300) → elle occupe la place ; le panneau de stats est à **largeur fixe** (220 px).
- **`QGroupBox statsBox`** titré « Stats IA minimax α-β » (= le `<h2>` JS), avec **5 `QLabel` nommés** en placeholder `—` : `statDepth` (MAX_DEPTH), `statNodes` (grilles ce coup), `statTime` (temps ce coup), `statTotal` (cumul grilles), `statTotalTime` (cumul temps). Texte en **rich text** (`<br/>`/`<b>`, détecté par `Qt::AutoText`) pour le « libellé au-dessus, valeur en gras » du JS. Spacer vertical en bas pour pousser les labels en haut. *Branchement futur* : `statNodes->setText(...)` etc. depuis le code quand `minmaxAB` mesurera.
- **Radios « qui commence »** : ligne du bas = `QHBoxLayout` `[ "Qui commence :"  (•) Moi  ( ) L'IA  ──spacer──  [Rejouer] ]`. `radioHuman` (« Moi ») **coché par défaut** (= défaut JS), `radioIA` (« L'IA »). Même parent (`centralwidget`) ⇒ **exclusivité automatique**, pas de `QButtonGroup`. **Non branchés** pour l'instant : « qui commence » n'a de sens qu'avec l'IA (centre forcé en ouverture). À câbler avec le minimax → un `reset` qui, si `radioIA->isChecked()`, fait jouer la colonne centrale (logique du `reset(iaCommence)` JS).

**Ensuite C++** : `evaluate` (contenu de fenêtre : `nbX`/`nbO`, fenêtre morte si 2 camps, bonus centre), `minmaxAB` + `height[]`/`undo`, puis gamesolver.org. *Note `win()` renvoie un `bool`* (alignement existe), pas le gagnant → quand on branchera l'annonce IA il faudra le joueur qui vient de jouer (déjà retourné par `play()`, exploité dans `mouseReleaseEvent`).

**`minimax` + `bestMove` C++ (fait ✅, sauf `evaluate`).** Portage de l'α-β. Trois corrections au passage :
- **Bug `play(c)` au lieu de `play(cols[c])`** (le piège récurrent du projet, déjà vu en JS avec `move = x` / `other.play(x)`). `availableColumns` remplit `cols` avec les colonnes **réellement jouables, en move ordering centre-d'abord** (`COLUMNS_ORDER`), mais la boucle passait l'**indice** `c` à `play`. Double dégât : (1) move ordering perdu → α-β n'élague presque plus ; (2) **incorrect dès qu'une colonne est pleine** — `nbAC < 7`, la boucle joue les colonnes brutes `0..nbAC-1` (dont des `play()` no-op sur colonnes pleines, évaluées comme un vrai coup) et **rate les dernières colonnes**. Fix : `play(cols[c], &row)`, et `bestMove` renvoie `cols[c]` (une colonne), pas `c`.
- **`win()` rendu incrémental** — la grosse optim perf. La version pleine balayait les **69 fenêtres à chaque nœud** ; or **seul le dernier coup peut créer un alignement**. Nouvel overload privé `win(lastCol, lastRow)` qui ne teste que les fenêtres passant par cette case (**≤ 13**, souvent 3-4 au bord). Plomberie : `play()` reçoit un out-param optionnel `int *rowOut` (ligne où le jeton s'est posé, déjà calculée en interne) ; `minimax(depth, alpha, beta, lastCol, lastRow)` propage le dernier coup. Le `win()` **sans argument reste** pour l'UI (`mouseReleaseEvent`), inchangé.
- **Index inverse `FENETRES_PAR_CASE` figé au build** (même esprit `constexpr` que `FENETRES`) : pour chaque case, les indices de ses fenêtres dans `FENETRES`. `MAX_FEN_PAR_CASE = 13` ; si une case dépassait, `idx[nb++]` sortirait du `std::array` → **erreur de compilation** (table auto-vérifiée, comme le `n != 69` de `construitFenetres`). Le fait que ça compile **confirme** la borne 13 (= les 2 cases centrales — rejoint la note « bonus centre : ~13 fenêtres contre ~3 pour un coin »).
- **Défauts inversés** dans le `.h` (`alpha = P4INFINITY, beta = -P4INFINITY`) supprimés : signature désormais entièrement explicite (`lastCol`/`lastRow` n'ont pas de défaut sensé, et `bestMove` passe toujours toutes les bornes).
- **Reste à faire : `evaluate()`** (renvoie encore `0` → IA aveugle à l'horizon, ne voit que les mats ≤ `MAX_DEPTH`). Prochain pas, fait par Corentin. *À la louche : balayer les 69 fenêtres, scorer par contenu (`nbX`/`nbO`, fenêtre morte si 2 camps, bonus centre), du point de vue de PLAYER2 pour rester cohérent avec le signe minimax* — exactement la démarche de l'`evaluate` JS.

**Stats IA branchées (fait ✅).** Les 5 labels placeholder du panneau (cf. « bloc stats préparé ») sont désormais alimentés. Corentin a câblé l'IA dans `mouseReleaseEvent` (joue après le coup humain) et rendu `bestMove()` public ; reste à faire vivre les compteurs.
- **Compteur de nœuds `Puissance4::nodes` (statique).** Remis à 0 en tête de `bestMove`, `nodes++` en tête de `minimax`. **Statique** parce que `minimax` récurse sur des **copies** (`Puissance4 other(*this)`) : un membre d'instance ne remonterait jamais au plateau racine — seul un compteur partagé agrège l'arbre. C'est le pendant C++ de `Morpion.nodes`/`measure()` du JS.
- **Chronométrage** : `QElapsedTimer` autour de `bestMove` seul (la *réflexion*, pas le `play`), `nsecsElapsed()/1e6` → ms haute précision.
- **Découplage par signaux** : les labels appartiennent à `MainWindow` (`ui_mainwindow.h`), pas au widget. `WPuissance4` **émet** `aiMoved(long nodes, double timeMs)` après le coup IA et `statsReset()` dans `reset()` ; `MainWindow` connecte (lambdas) et **présente** — il tient les cumuls `totalNodes`/`totalTime` (RAZ sur `statsReset`), pose `statDepth = MAX_DEPTH` une fois. Le widget ne connaît pas les labels (il signale, MainWindow affiche) — même esprit que « sortir l'invariant », appliqué au **couplage**.
- **`setStatValue(label, valeur)`** (helper anonyme de `mainwindow.cpp`) : ne remplace que la **valeur** après le `<br/>`, en gardant le libellé défini dans le `.ui` comme **source unique** → zéro chaîne française dupliquée dans le `.cpp` (évite le piège « duplication = bug »). Nombres groupés via `QLocale`.
- *Note perf à venir* : `MAX_DEPTH` poussé à **10**. Avec `evaluate()` encore à `0`, le 1er coup explore énormément de nœuds — les stats « Grilles testées » rendront ce coût visible, utile pour calibrer une fois l'heuristique écrite.

**Constat : à `MAX_DEPTH=10`, l'IA joue déjà bien SANS `evaluate()` (evaluate≡0).** Et 10 est le **plafond perf acceptable** (au-delà, 1er coup trop lent). Deux raisons que ça marche quand même — illustration directe de la note « force = profondeur × éval » : ici on a saturé le facteur *profondeur*.
- **L'horizon couvre la tactique** : 10 demi-coups suffisent à voir la plupart des menaces forcées avant qu'elles n'aboutissent. Tout ce qui se décide à ≤10 coups est traité *exactement* (terminaux réels via `win`/`isFull`), pas approximé — l'heuristique ne sert qu'**au-delà** de l'horizon, et à profondeur 10 il reste peu de parties décidées plus loin.
- **Le move ordering centre-d'abord = mini-heuristique gratuite** : quand tous les coups valent 0 (aucune menace dans l'horizon), `bestMove` garde le **premier** à égalité → le plus central via `COLUMNS_ORDER`. C'est le « bonus centre » du JS obtenu sans une ligne d'`evaluate`.
- **Conséquence** : `evaluate` reste un vrai plus (départager les positions lointaines, rendre l'IA forte à profondeur *moindre* → moins de nœuds), mais n'est **plus un prérequis** de jouabilité — la profondeur a pris le relais.

**`evaluate()` C++ branché (fait ✅).** Portage de l'heuristique « par contenu de fenêtre » du JS (cf. plan ligne 375). Boucle plate sur les **69 `FENETRES`** ; pour chaque fenêtre, comptage `nbX`/`nbO`, `continue` si les deux camps sont présents (**fenêtre morte**), sinon `±1000` (trois) / `±100` (deux), du **point de vue de PLAYER2** (cohérent avec le signe du minimax). Bonus centre `±BONUS_CENTRE` sur la colonne 3.
- **Traduction `for (const [x,y] of cases)` → C++** : la déstructuration JS devient un **structured binding** dans un range-based for : `for (const auto &[x, y] : f.cases)`. Marche parce que `Cell` a exactement 2 membres publics (`col`, `row`), liés **dans l'ordre de déclaration** → `x = col`, `y = row`. Le `of` JS devient `:`.
- **Bug évité — itérer sur la bonne chose** : 1ʳᵉ version bouclait sur `FENETRES_PAR_CASE[lastCol][lastRow]` (les ≤13 fenêtres du dernier coup), copié du pattern de `win()`. Faux pour `evaluate` : `win()` peut se limiter au dernier coup (une victoire passe forcément par le pion qu'on vient de poser), mais `evaluate` note **toute la position** de la feuille → doit balayer les **69**. Leçon : un raccourci correct pour la *détection de victoire* ne l'est pas pour l'*évaluation positionnelle*.
- **Bug évité — `continue` sur 1re case vide** : un filtre `if (1re case vide) continue` ratait les fenêtres décalées (`. X X X` → origine vide mais trio menaçant). Inutile de toute façon : une fenêtre vide donne `nbX==nbO==0` → aucune branche de score → `+0` gratuit. Supprimé.
- **Bug évité — ordre des arguments `getCell`** : signature `getCell(col, row)` ; les appels passaient `(row, col)`. Compile (deux `int`) mais fausse l'éval en silence → vérifier l'ordre, pas juste que ça compile.

**Choix « qui commence » + ouverture forcée au centre (C++) (fait ✅).** Câblage des deux radios préparés (cf. ligne 366), pendant C++ du `reset(iaCommence)` JS.
- **Radio → MainWindow, jeu → WPuissance4** : les radios vivent dans `MainWindow` (`ui_mainwindow.h`), la logique de coup dans le widget. Le bouton « Rejouer » passe d'un `connect` direct vers `reset` à un **lambda** : `wPuissance4->reset(); if (radioIA->isChecked()) wPuissance4->aiStart();`. `radioIA` accessible car `MainWindow` hérite de `Ui::MainWindow` en privé.
- **Bloc IA factorisé** : le coup de l'IA (chrono + `bestMove` + `play` + tests fin de partie) était inline dans `mouseReleaseEvent` → extrait en `aiTurn()` (privé), gardé par `if (board == nullptr || gameOver) return`. Réutilisé par le tour normal **et** le 1er coup. Effet de bord positif : corrige un cas où l'IA jouait quand même après un **match nul** (l'ancien code ne faisait pas `return`).
- **Ouverture forcée** : `aiStart()` ne lance **pas** le minimax — il joue directement `play(COLONNE_CENTRE)`. Le Puissance 4 est résolu (1er joueur gagnant au centre) → réflexion inutile et coûteuse sur plateau vide. Ce coup forcé n'émet pas `aiMoved` (rien à mesurer) → les stats restent à « — » jusqu'au 1er vrai coup calculé.

**Confrontation au solveur — premier essai (constat).** IA C++ opposée à un connect4 en ligne : « pas pire » (tient la route). Cohérent avec la note « force = profondeur × éval » — l'α-β profondeur 10 + heuristique fenêtres donne déjà un bon niveau. Reste à formaliser l'analyse coup-par-coup vs gamesolver.org (segment vidéo).

**Polish UI — pulsation de la dernière boule (fait ✅).** Animation `QPainter` : la dernière boule jouée pulse (petit ↔ grand) pour attirer l'œil.
- **Mécanique** : `QTimer` à ~60 fps (`setInterval(16)`) qui appelle `update()` ; `paintEvent` redessine ce seul disque à une échelle réduite. `startPulse(col, row)` mémorise la case — la ligne vient de l'out-param `rowOut` de `play()` (`Puissance4` ne stocke pas `lastCol/lastRow`, le minimax les passe en paramètres).
- **Itération 1 — basée sur le temps**, abandonnée : `scale = 0.85 + 0.15·sin(2π·1.5·t)`, arrêt après une durée (10 s → jugé trop long → 3 s).
- **Itération 2 — basée sur les frames** (retenue) : compteur `pulseFrame`, créneau `(pulseFrame / PULSE_PHASE_FRAMES) % 2` → **10 frames « petit » (×0,6) puis 10 frames « grand » (×1,0)**, puis arrêt (`pulseFrame >= PULSE_TOTAL_FRAMES`). Constantes en tête de `wpuissance4.cpp`. *Pourquoi le passage frames* : rythme déterministe indépendant de la machine/charge, et plus simple à régler que des Hz.
- **Bug — boule humaine figée petite** : on animait *aussi* le coup humain, mais `bestMove()` (synchrone) **bloque la boucle d'événements** juste après → le timer ne tourne plus, la boule reste figée à la 1ʳᵉ frame (petite) pendant toute la réflexion IA. *Fix* : **ne pas animer le coup humain**, seulement celui de l'IA (et l'ouverture forcée). Leçon : une animation pilotée par timer est gelée par tout travail synchrone long sur le thread UI.
- **Reset si jeu rapide** : `startPulse` remet `pulseFrame = 0` et relance le timer même s'il tournait déjà → un nouveau coup avant la fin de l'animation la fait repartir proprement de zéro.

**Polish UI — croix sur l'alignement gagnant (fait ✅).** À la victoire, un **X** est dessiné au centre de chacune des 4 boules gagnantes.
- **Exposer les 4 cases** : `FENETRES` est privé au `.cpp` → nouvelle méthode publique `Puissance4::winningLine(int cols[4], int rows[4])` (même balayage des 69 fenêtres que `win()`, mais remplit les cases gagnantes ; hors point chaud, appelée en fin de partie). Dans l'UI, remplace `win()` dans `mouseReleaseEvent`/`aiTurn` → **une seule passe** qui détecte *et* récupère la ligne.
- **Dessin** : `paintEvent` trace deux diagonales par case via un `QPen` (gris foncé, `setCapStyle(RoundCap)`). Réglages itérés à la demande : trait `cell/12 → cell/7` (**plus gras**), demi-diagonale `0.28 → 0.16` de la boule (**plus petites**).
- **Ordre d'affichage** : à la détection, `hasWin = true` + `repaint()` **avant** le `QMessageBox` (modal) → les croix sont visibles derrière la boîte. `reset()` remet `hasWin = false`.

**Table de transposition — FAIT ✅** (classe + câblage `minimax` + validation « avec vs sans TT »). La grosse optim qui restait, et le morceau d'algo le plus instructif avant Awalé/échecs.
- **But direct** : ne **pas recalculer** un sous-arbre déjà évalué. Une *transposition* = une même position atteinte par des ordres de coups différents (ex. colonnes `3` puis `4` ⇒ même plateau que `4` puis `3` ⇒ sous-arbre identique). Sans table, le minimax le calcule deux fois — et bien pire en profondeur, le nombre de chemins menant à une position explose.
- **Conséquence** (≠ but) : à temps égal on descend **plus profond** (IA plus forte), ou à profondeur égale on répond **plus vite**. Même gain vu des deux côtés. C'est l'autre moitié de `force = profondeur × éval`, côté profondeur — comme le move ordering et le threading `alpha`, ça **ne change pas le résultat à profondeur égale**.
- **Mécanique** : table `position → (score, profondeur de calcul)`. Avant d'explorer un nœud, on regarde la table ; si présent **et calculé à profondeur ≥** celle voulue → on réutilise, on coupe le sous-arbre.
- **Deux pièges anticipés** :
  1. **La profondeur stockée est obligatoire.** Un score calculé à profondeur 3 ne vaut pas pour une recherche qui veut profondeur 6 → ne réutiliser que si `profondeurStockée ≥ profondeurVoulue`. L'oublier rend l'IA *plus faible* en croyant l'accélérer (erreur classique du débutant).
  2. **Clé de position** : il faut un hash compact et rapide. Standard = *Zobrist hashing*. Le bitboard (`6 × unsigned short`) rend la clé facile. Démarrer par une version **naïve** (clé = copie/recalcul direct du plateau) pour valider la logique, puis passer à Zobrist.
- **Gain conditionnel** : rentable seulement si les transpositions sont fréquentes (vrai au Puissance 4) et si consulter la table coûte moins que recalculer. Décision : à attaquer après avoir profité du niveau actuel ; commencer simple (clé naïve, profondeur stockée) avant d'optimiser le hash.

### Décisions de conception (tranchées avec Claude)
Fichiers `cpp/puissance4/transpositiontable.{h,cpp}`. Deux choix structurants débattus avant d'écrire une ligne :

**1. Structure = table de hachage à adressage direct** (et **pas** tableaux triés + dichotomie). Le critère qui tranche = le **motif d'accès** : une TT est *write-heavy*, le minimax **écrit autant qu'il lit**, en continu pendant la recherche (un `setScore` à chaque nœud fini, un `getScore` à chaque nœud entré). Un tableau trié lit vite (dichotomie O(log n)) mais **insère en O(n)** (décalage pour rester trié) → rédhibitoire sur le point chaud. La table à adressage direct fait **lecture ET écriture en O(1)** : `index = hash % TABLE_SIZE`, collisions gérées par **remplacement** (une TT est un *cache*, perdre une vieille entrée est gratuit). Ça a **supprimé la moitié des membres** du proto initial (`nbIndexes`, `nbScores`, `indexes`/`scores` 2D, `sortDepths`, `sortIndexes`, `getIndex`, `getScoreFromIndex`). *Leçon (rappel) : choisir la structure pour le point chaud — ici l'insertion continue, pas la lecture.*

**2. Clé = exacte et compacte (49 bits dans un `uint64_t`)**, pas un hash lossy (Zobrist gardé pour plus tard), pas la clé brute 96 bits. **Deux rôles à distinguer** que le 1er proto confondait : l'**index** (`hash % N`, petit, collisions normales gérées par remplacement) ET la **clé de vérification** (stockée dans l'entrée, comparée à la lecture pour rejeter un squatteur). Clé exacte ⇒ **jamais de faux positif** (cohérent avec « imbattable = exact, pas approximation »). Encodage Puissance 4 classique **`position + mask`** : `mask` = bitboard d'occupation (1 bit/case), `position` = pions du joueur au trait. Tient en 49 bits via une disposition **par colonne, 7 bits/colonne (6 cases + 1 sentinelle)** — la retenue de l'addition s'arrête dans le bit sentinelle sans déborder sur la colonne voisine. Unique car, par colonne, les pions remplissent depuis le bas → `mask` = suite contiguë de 1 depuis la base → chaque `(hauteur, répartition)` donne une somme distincte. **Bonus gratuit** : le trait est encodé tout seul (`position` = « pions de celui qui joue ») → pas de bit « à qui de jouer » en plus. *Zobrist viendra remplacer `getHash` (même point d'appel) en passe de perf — mais il réintroduira le risque de collision (lossy).*

### Implémentation (faite, relue)
- **Singleton sur le tas** (`getInstance()` + `new`, constructeur privé). **Critique** : l'entrée fait 16 Mo (`SEntry table[1000000]`, 16 o/entrée). Surtout **pas un membre de `Puissance4`** (le minimax fait `Puissance4 other(*this)` à chaque nœud → 16 Mo clonés par nœud = mort) ni une **locale de pile** (stack overflow). Une seule instance, vue par pointeur. Persiste entre coups **et entre parties** — valide car la valeur d'une position à profondeur donnée est déterministe.
- **Entrée** `SEntry { uint64_t hash; int depth; int score; }`. **`memset(table, 0xFF, …)`** au constructeur → case vierge = `hash = 0xFFFF…FFFF` (jamais une vraie clé ≤ 49 bits) et `depth = -1`. *Pourquoi pas `0`* : la clé du **plateau vide est `0`** (`position+mask = 0`) → on ne peut pas s'en servir comme marqueur « libre ». `0xFF` donne un marqueur hors d'atteinte sans cas particulier.
- **`getScore`** : `entry = table[hash % SIZE]` ; hit ssi `entry->hash == hash` **ET** `entry->depth <= depth`, sinon renvoie **`P4INFINITY`** (sentinelle de *miss* — hors de la plage des vrais scores).
- **`getHash`** : double boucle colonne×ligne, `getCell(col,row)` lit la case (0/1/2), `maskIdx` placé en `col*7 + row` (décalage sentinelle `<<= 1` entre colonnes), renvoie `mask + position`. `friend class TranspositionTable;` dans `puissance4.h` pour l'accès (mais `getCell` public suffit).

### Le piège central : sens de la condition de profondeur
`depth` dans le `minimax` part de **0 à la racine** et **monte** (cutoff `depth >= MAX_DEPTH`). Donc un nœud à profondeur `d` est cherché avec une **force = `MAX_DEPTH − d`** (plis explorés dessous). On ne réutilise une entrée que si sa force est **≥** au besoin :
```
force_stockée ≥ force_voulue  ⟺  (MAX_DEPTH − entry->depth) ≥ (MAX_DEPTH − depth)  ⟺  entry->depth ≤ depth
```
→ condition **`entry->depth <= depth`** (et **pas** `>=`). C'est *le* piège noté de longue date (« profondeur stockée obligatoire ») : 1ʳᵉ version écrite avec `>=` → réutilisait un résultat peu profond là où il fallait chercher loin → **IA plus faible en silence**. Le `>=` perd même doublement : il rate aussi les réutilisations valides (résultat profond pour besoin peu profond).

### Bugs rencontrés (et leçons) — TT
- **Sentinelle de *miss* impossible** : `getScore` renvoyait `-1` pour « absent » → mais `-1` est un **vrai score**. Tout `int` est un score valide → il faut une valeur **hors plage** → `P4INFINITY`.
- **Condition de profondeur inversée** (`>=` au lieu de `<=`) — cf. ci-dessus, le bug de fond.
- **Marqueur de case vide = `0`** se confond avec la clé du plateau vide → `memset(0xFF)`.
- **`maskIdx` en `int`** : doublé 42 fois → dépasse le bit 31 → débordement signé (UB) + extension de signe en l'OR-ant dans un `uint64_t`. → `uint64_t`.
- **Disposition par ligne** (1er jet du `mask`) incompatible avec `position + mask` : la retenue traverserait les colonnes horizontalement. → **par colonne avec sentinelle**.
- **`decal` avançant d'1 bit/colonne** alors qu'une case = **2 bits** → masques de colonnes qui se chevauchent, lecture faussée (PLAYER1 lu vide, PLAYER2 lu PLAYER1). → corrigé en lisant via **`getCell(col,row)`** (plus simple et increvable que le masque maison).
- **Ternaire no-op** `position |= (… ? maskIdx : position)` : la branche else `|= position` ne fait rien (sans dégât, mais confus) → simple `if (currentPlayer == player) position |= maskIdx;`.

### Câblage dans `minimax` + validation (fait ✅)
- **Câblé** : probe en tête (`getScore`, si `!= P4INFINITY` → couper), `setScore(hash, depth, best)` avant le `return best`, `getHash` avec le trait du nœud (`this->player`). **Ordre respecté** : `win()`/`isFull()` (incrémental, ≤13 fenêtres) **avant** le probe — inutile de hasher (42 cases) une position terminale.
- **Validé par le test « avec vs sans TT »** : à `MAX_DEPTH` égal, sur la position litigieuse, **même coup joué**, `nodes` en forte baisse. Signature d'une optim correcte (comme move ordering / threading : *ne change pas le résultat, seulement le nombre de nœuds*). Outil = flag global `g_useTT` court-circuitant probe+store pour comparer les deux runs. *Nuance : 1 position = forte présomption, pas preuve formelle (cf. bornes α-β ci-dessous).*

### Reste (optionnel / plus tard)
- **Point différé — bornes α-β** : le `best` stocké peut être une **borne** (coupure `alpha >= beta`), pas la valeur exacte → réutilisation approximative. N'a **pas** changé le coup sur la position testée, mais reste le 1er suspect d'un coup bizarre ailleurs. Vraie correction = drapeau `EXACT / borne_inf / borne_sup` par entrée.
- **Zobrist** : hash incrémental O(1) en remplacement de `getHash` (passe de perf ; réintroduit le risque de collision lossy).

### Lien avec la confrontation au solveur (constat)
Une fois la TT validée, l'IA (ouverture centre forcée) joue un **coup sous-optimal dès son 3ème coup** (classé perdant par gamesolver.org). **Ce n'est pas un bug** (même coup avec/sans TT) : c'est l'**honnête limite profondeur+heuristique** — à profondeur 10, le gain d'une position d'ouverture (résolu à ~15-20 plis) n'est pas prouvable → l'éval tranche, et se trompe. Illustration directe de `force = profondeur × éval`. → matière pour la **partie 5** du [`plan-video.md`](plan-video.md) (analyse coup-par-coup à monter : script Node + API solveur).
