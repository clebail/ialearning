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
- [ ] Morpion — alpha-bêta
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

### Simplifications passées (refacto)
- Swap joueur lisible : `this.joueur === 'O' ? 'X' : 'O'` (au lieu du calcul via `charCodeAt`)
- `win()` / `isFull()` compactés via `this.cells.flat()` (indices `0..8`)
- Helper `emptyCells()` → dédoublonne les boucles de `minmax` et `bestMove` (`for (const {x,y} of ...)`)
- Gestion de fin de partie unifiée dans `announceIfOver()` (victoire + match nul)

> Prochaine étape : **alpha-bêta** sur ce même code → résultat identique, beaucoup moins de nœuds explorés.
