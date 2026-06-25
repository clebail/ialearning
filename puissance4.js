document.addEventListener("DOMContentLoaded", () => {
    const container = document.querySelector('#game')
    const puissance4 = new Puissance4(container);

    // Radios « qui commence » : à chaque changement, on relance une partie neuve.
    // On réutilise la MÊME instance (reset) plutôt que d'en créer une nouvelle, sinon
    // les listeners de clic posés sur #game s'accumuleraient (un par partie).
    document.querySelectorAll('input[name="starter"]').forEach(radio => {
        radio.addEventListener('change', () => {
            const iaCommence = document.querySelector('input[name="starter"]:checked').value === 'ia';
            puissance4.reset(iaCommence);
        });
    });
});

// Compteur statique : nombre de grilles explorées (partagé par tous les clones)
Puissance4.nodes = 0;
const MAX_DEPTH = 6;

function Puissance4(container) {
    this.container = container;
    this.statsABContainer = document.querySelector('#stats-ab');

    this.attachEvents();   // une seule fois : le listener survit aux reset()
    this.reset(false);     // partie neuve, l'humain ('O') commence
}

// Relance une partie neuve. iaCommence = true → l'IA ('X') joue d'office le centre.
Puissance4.prototype.reset = function(iaCommence) {
    this.cells = [['', '', '', '', '', '', ''],['', '', '', '', '', '', ''],['', '', '', '', '', '', ''],['', '', '', '', '', '', ''],['', '', '', '', '', '', ''],['', '', '', '', '', '', '']];
    this.joueur = 'O';    // l'humain est 'O'
    this.totalAB = 0;     // cumul des grilles testées — alpha-bêta
    this.statsAB = null;  // pas encore de coup IA mesuré

    // Puissance 4 est un jeu résolu : le 1er joueur gagne en démarrant au centre.
    // Si l'IA commence, on lui fait jouer le centre d'office — pas besoin de minimax.
    if (iaCommence) {
        this.joueur = 'X';
        this.play(COLONNE_CENTRE);
    }

    this.draw();
    this.drawStats();
}

Puissance4.prototype.clone = function() {
    const copie = Object.create(Puissance4.prototype);  // un Puissance4 sans constructeur
    copie.cells = structuredClone(this.cells);       // copie profonde du plateau
    copie.joueur = this.joueur;
    return copie;                                    // pas de container : simulation pure
};

Puissance4.prototype.play = function(x) {
    if (!this.canPlay(x)) return '';

    const oldJoueur = this.joueur;
    let y = 5;
    while (this.cells[y][x] !== '') {
        y--;
    }

    this.cells[y][x] = this.joueur;
    this.joueur = this.joueur === 'O' ? 'X' : 'O';
    return oldJoueur;
}

Puissance4.prototype.canPlay = function(x) {
    let y;

    if (x >= 0 && x < 7) {
        for (y=5;y>=0;y--) {
             if (this.cells[y][x] === '') {
                return true;
            }
        }
    }

    return false;
}

// Ordre d'exploration des colonnes : du centre vers les bords (move ordering).
// Le centre participe au plus grand nombre d'alignements → souvent le meilleur coup.
// L'explorer en premier resserre alpha/beta plus tôt → l'α-β coupe davantage.
Puissance4.ORDRE_COLONNES = [3, 2, 4, 1, 5, 0, 6];

// Liste des colonnes jouables, déjà triées centre → bords
Puissance4.prototype.availableColumns = function() {
    return Puissance4.ORDRE_COLONNES.filter(x => this.canPlay(x));
}

Puissance4.prototype.draw = function() {
    let body = "<table>";

    this.cells.forEach((ligne, y) => {
        body += "<tr>";
        ligne.forEach((valeur, x) => {
            body += `<td data-x="${x}" data-y="${y}">${valeur}</td>`;
        });
        body += "</tr>";
    });

    body += "</table>";

    this.container.innerHTML = body;
}

Puissance4.prototype.attachEvents = function() {
    this.container.addEventListener('click', (event) => {
        const cell = event.target.closest('td');
        if (!cell) return;   // clic à côté d'une case

        this.play(Number(cell.dataset.x));
        this.draw();
        if (this.announceIfOver()) {
            return;
        }

        // Tour de l'IA : on mesure minimax ET alpha-bêta (si dispo) pour comparer
        this.statsAB = this.measure('minmaxAB');   // null tant que minmaxAB n'existe pas
        if (this.statsAB) {
            this.totalAB += this.statsAB.nodes;
        }

        const result = this.statsAB;
        if (result && result.move !== null) {
            this.play(result.move);
        }

        this.draw();
        this.drawStats();
        this.announceIfOver();
    });
};

// Lance bestMove avec la méthode de score donnée, en comptant les grilles + le temps.
// Retourne null si la méthode n'existe pas encore (ex : minmaxAB pas codé).
Puissance4.prototype.measure = function(scoreMethod) {
    if (typeof this[scoreMethod] !== 'function') return null;

    Puissance4.nodes = 0;
    const t0 = performance.now();
    const move = this.bestMove(scoreMethod);

    return {
        move,
        nodes: Puissance4.nodes,
        timeMs: (performance.now() - t0).toFixed(1),
    };
};

// Affiche les stats d'exploration du dernier coup de l'IA (les deux algos)
Puissance4.prototype.drawStats = function() {
    this.renderStatsBox(this.statsABContainer, 'Stats IA minimax α-β',
        this.statsAB, this.totalAB, 'α-β : à implémenter (<code>minmaxAB</code>)');
};

Puissance4.prototype.renderStatsBox = function(box, titre, stats, total, placeholder) {
    if (!box) {
        return;
    }

    if (!stats) {
        box.innerHTML = `<h2>${titre}</h2><p>${placeholder}</p>`;
        return;
    }

    const fmt = n => n.toLocaleString('fr-FR');
    box.innerHTML = `
        <h2>${titre}</h2>
        <p>Grilles testées (ce coup) :<br><strong>${fmt(stats.nodes)}</strong></p>
        <p>Temps de calcul :<br><strong>${stats.timeMs} ms</strong></p>
        <p>Total cumulé (partie) :<br><strong>${fmt(total)}</strong></p>
    `;
};

// Annonce le résultat si la partie est finie, et indique si c'est le cas
Puissance4.prototype.announceIfOver = function() {
    const winner = this.win();

    if (winner) {
        alert(winner + " gagne !");
    } else if (this.isFull()) {
        alert("Match nul !");
    }

    return this.isOver();
}

Puissance4.prototype.win = function() {
    for (const cases of Puissance4.FENETRES) {
        const [x0, y0] = cases[0];
        const j = this.cells[y0][x0];
        if (j !== '' && cases.every(([x, y]) => this.cells[y][x] === j)) return j;
    }
    return null;
}

// Les 4 orientations, comme vecteurs (dx, dy). Un seul sens par orientation
// suffit à couvrir tout le plateau (la fenêtre balaie chaque case de départ).
Puissance4.DIRECTIONS = [[1, 0], [0, -1], [1, -1], [-1, -1]]; // →  ↑  ↗  ↖

// La case (x,y) est-elle dans le plateau (7 colonnes × 6 lignes) ?
function dansPlateau(x, y) {
    return x >= 0 && x < 7 && y >= 0 && y < 6;
}

// Géométrie FIXE du plateau, calculée UNE fois au chargement : les 69 alignements
// possibles de 4 cases. On ne stocke plus que les 4 cases — le « complétable » se déduit
// désormais du CONTENU de la fenêtre (voir evaluate), plus besoin d'extrémités avant/après.
function construitFenetres() {
    const fenetres = [];
    for (let y = 0; y < 6; y++) {
        for (let x = 0; x < 7; x++) {
            for (const [dx, dy] of Puissance4.DIRECTIONS) {
                const fx = x + dx * 3, fy = y + dy * 3; // dernière des 4 cases
                if (!dansPlateau(fx, fy)) continue;

                const cases = [];
                for (let i = 0; i < 4; i++) cases.push([x + dx * i, y + dy * i]);
                fenetres.push(cases);
            }
        }
    }
    return fenetres;
}

// Les 69 fenêtres de 4 cases alignées, figées.
Puissance4.FENETRES = construitFenetres();

Puissance4.prototype.isFull = function() {
    return this.cells.every(ligne => ligne.every(c => c !== ''));
}

Puissance4.prototype.isOver = function() {
    return this.win() !== null || this.isFull();
}

// Retourne le SCORE de la position (un nombre, dans tous les cas)
Puissance4.prototype.minmaxAB = function(depth = 0, alpha = -Infinity, beta = Infinity) {
    Puissance4.nodes++;   // une grille de plus explorée

    const winner = this.win();

    if (winner === 'X')  {
        return 10000 - depth;   // 'X' gagne (favorise victoire rapide)
    }

    if (winner === 'O')  {
        return -10000 + depth;  // 'O' gagne
    }

    if (this.isFull()) {
        return 0;             // match nul
    }

    if(depth >= MAX_DEPTH) return this.evaluate();

    const maximise = this.joueur === 'X';      // 'X' maximise, 'O' minimise
    let best = maximise ? -Infinity : Infinity;

    for (const x of this.availableColumns()) {
        const other = this.clone();
        other.play(x);
        const score = other.minmaxAB(depth + 1, alpha, beta);
        if (maximise) {
            best = Math.max(best, score);
            alpha = Math.max(alpha, best);
        } else {
            best = Math.min(best, score);
            beta = Math.min(beta, best);
        }

        if (alpha >= beta) {
            break;
        }
    }

    return best;
}

// Retourne la MEILLEURE COLONNE pour le joueur courant.
Puissance4.prototype.bestMove = function( ) {
    const maximise = this.joueur === 'X';
    let best = maximise ? -Infinity : Infinity;
    let move = null;
    let alpha = -Infinity, beta = Infinity;   // bornes threadées vers les sous-arbres

    for (const x of this.availableColumns()) {
        const other = this.clone();
        other.play(x);
        const score = other.minmaxAB(0, alpha, beta);   // le sous-arbre coupe contre alpha
        if (maximise ? score > best : score < best) {
            best = score;
            move = x;
        }
        // On resserre la borne avec le meilleur score racine trouvé jusqu'ici
        if (maximise) alpha = Math.max(alpha, best);
        else beta = Math.min(beta, best);
    }

    return move;
}

const COLONNE_CENTRE = 3;   // la colonne du milieu (la plus traversée par les fenêtres)
const BONUS_CENTRE = 30;    // petit : un départage, pas une priorité (<< menace de 2 = 100)

// Heuristique : on classe chaque fenêtre de 4 par son CONTENU.
// Une fenêtre mixte (X et O présents) est morte ; sinon 3 = grosse menace, 2 = amorce.
// Avantage sur l'ancien test d'extrémité : gère les trous (X.XX) et les blocages exactement.
Puissance4.prototype.evaluate = function( ) {
    let score = 0;

    for (const cases of Puissance4.FENETRES) {
        let nbX = 0, nbO = 0;
        for (const [x, y] of cases) {
            const c = this.cells[y][x];
            if (c === 'X') nbX++;
            else if (c === 'O') nbO++;
        }
        if (nbX && nbO) continue;            // fenêtre morte : les deux camps y sont présents

        if (nbX === 3) score += 1000;
        else if (nbX === 2) score += 100;
        else if (nbO === 3) score -= 1000;
        else if (nbO === 2) score -= 100;
    }

    // Bonus centre : un pion au milieu est plus flexible (dans le plus de fenêtres).
    // Petit poids → ne départage qu'à menaces égales (typiquement en début de partie).
    for (let y = 0; y < 6; y++) {
        const c = this.cells[y][COLONNE_CENTRE];
        if (c === 'X') score += BONUS_CENTRE;
        else if (c === 'O') score -= BONUS_CENTRE;
    }

    return score;
}
