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
- [ ] Puissance 4 — profondeur + heuristique
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

## Puissance 4 (Connect Four) — EN COURS

Démarré par **copie de `morpion.js`** (renommé `Puissance4`, un seul algo `minmaxAB`). Objectif de l'étape : arbre trop gros pour être exploré entièrement → passage obligé à une **profondeur limitée + fonction d'évaluation heuristique** (alignements de 2/3, contrôle du centre…). Vrai changement d'échelle par rapport au morpion.

### Fait
- [x] Plateau **6 lignes × 7 colonnes** : `cells[ligne][colonne]` (6 sous-tableaux de longueur 7). `draw` cohérent : boucle externe = `<tr>` = ligne (`data-x`), interne = `<td>` = colonne (`data-y`).
- [x] `play` prend un seul argument (une colonne) au lieu de `{x, y}`. IA commentée le temps de valider le jeu à la main.
- [x] **Axe tranché** : convention `cells[y][x]` avec `x` = colonne (0..6), `y` = ligne (0..5). Gravité = fixer la colonne, descendre la ligne.
- [x] **`canPlay` corrigé** : boucle `for` au lieu du `forEach` (le `return` sort vraiment de la fonction — piège du morpion), bornes colonne `x < 7`, balayage `for (y = 5; y >= 0; y--)`.
- [x] **Jouable à la main ✅** : `play` part de `let y = 5` (bas du plateau ; `y = 6` plantait car `cells[6]` n'existe pas), et le clic lit `cell.dataset.y` (la colonne). On clique une colonne → le pion tombe sur la 1ʳᵉ case libre en partant du bas, colonnes pleines bloquées.

### À FAIRE — détection de victoire (et le reste de la migration)
**État : jouable, mais pas de gagnant** — `announceIfOver()` est encore commenté (bloc IA), et `win()` est toujours la logique morpion 3×3. On pose des pions en alternance, personne ne gagne.

Ordre conseillé :
1. **`win()` — 4 alignés** dans les 4 directions (→, ↓, ↘, ↙), n'importe où sur le 7×6. Les 8 alignements en dur du morpion sautent → balayer le plateau et tester 4 cases depuis chaque case. *(le vrai morceau de plomberie)*
2. Réactiver `announceIfOver()` → valider les victoires à la main (toi contre toi).
3. **`availableColumns()`** remplace `emptyCells` : liste des **colonnes** jouables (≤ 7), pas des 42 cases.
4. Puis seulement : rebrancher l'IA avec **profondeur + heuristique** (voir section dédiée ci-dessous).
5. Nettoyage : supprimer la `Puissance4.nodesAB` déclarée mais morte (le code lit/écrit `Puissance4.nodes`).

### À FAIRE — migrer la logique morpion → Puissance 4 (encore en 3×3)
- `win()` : 4 alignés **n'importe où**, dans les 4 directions (→, ↓, ↘, ↙). Les 8 alignements en dur du morpion sautent → balayer le plateau et tester 4 cases depuis chaque case.
- `emptyCells()` → `availableColumns()` : liste des **colonnes** jouables (≤ 7), pas des 42 cases. C'est ce qui borne le facteur de branchement.
- Compteur : un seul algo ici → garder `Puissance4.nodes`, **supprimer** la `Puissance4.nodesAB` déclarée (morte : le code lit/écrit `nodes`). Plus besoin de table `COMPTEURS`.

### À FAIRE — LE concept neuf : profondeur limitée + heuristique
L'arbre est trop gros pour descendre aux feuilles. `minmaxAB` doit **s'arrêter à `MAX_DEPTH`** et, sur une position *non terminale*, renvoyer `this.evaluate()` au lieu d'un score de fin de partie :
```js
if (depth >= MAX_DEPTH) return this.evaluate();
```
`evaluate()` note la position sans la finir : alignements de 2/3 (les siens +, l'adversaire −), bonus colonne centrale, etc. **C'est le vrai morceau de l'étape** — le morpion descendait toujours jusqu'au bout, donc n'a jamais imposé d'heuristique.
