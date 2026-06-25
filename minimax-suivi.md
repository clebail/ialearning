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
