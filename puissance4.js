document.addEventListener("DOMContentLoaded", () => {
    const container = document.querySelector('#game')
    const puissance4 = new Puissance4(container);
});

// Compteur statique : nombre de grilles explorées (partagé par tous les clones)
Puissance4.nodesAB = 0;

function Puissance4(container) {
    this.container = container;
     this.statsABContainer = document.querySelector('#stats-ab');
    this.cells = [['', '', ''],['', '', ''],['', '', '']];
    this.joueur = 'O';
    this.totalAB = 0;   // cumul des grilles testées — alpha-bêta

    this.attachEvents();
    this.draw();
}

Puissance4.prototype.clone = function() {
    const copie = Object.create(Puissance4.prototype);  // un Puissance4 sans constructeur
    copie.cells = structuredClone(this.cells);       // copie profonde du plateau
    copie.joueur = this.joueur;
    return copie;                                    // pas de container : simulation pure
};

Puissance4.prototype.play = function(x, y) {
    if (!this.canPlay(x, y)) return '';

    const oldJoueur = this.joueur;
    this.cells[x][y] = this.joueur;
    this.joueur = this.joueur === 'O' ? 'X' : 'O';
    return oldJoueur;
}

Puissance4.prototype.canPlay = function(x, y) {
    return x >= 0 && x < 3 && y >= 0 && y < 3 && this.cells[x][y] === '';
}

// Liste des cases vides sous forme { x, y }
Puissance4.prototype.emptyCells = function() {
    const cells = [];
    this.cells.forEach((ligne, x) =>
        ligne.forEach((v, y) => { if (v === '') cells.push({ x, y }); })
    );
    return cells;
}

Puissance4.prototype.draw = function() {
    let body = "<table>";

    this.cells.forEach((ligne, x) => {
        body += "<tr>";
        ligne.forEach((valeur, y) => {
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

        this.play(Number(cell.dataset.x), Number(cell.dataset.y));
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
        if (result && result.move) {
            this.play(result.move.x, result.move.y);
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
    const c = this.cells;

    const lignes = [
        // lignes horizontales
        [c[0][0], c[0][1], c[0][2]],
        [c[1][0], c[1][1], c[1][2]],
        [c[2][0], c[2][1], c[2][2]],
        // colonnes
        [c[0][0], c[1][0], c[2][0]],
        [c[0][1], c[1][1], c[2][1]],
        [c[0][2], c[1][2], c[2][2]],
        // diagonales
        [c[0][0], c[1][1], c[2][2]],
        [c[0][2], c[1][1], c[2][0]],
    ];

    for (const [a, b, d] of lignes) {
        if (a !== '' && a === b && b === d) {
            return a; // 'O' ou 'X'
        }
    }

    return null; // pas encore de gagnant
}

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
        return 100 - depth;   // 'X' gagne (favorise victoire rapide)
    }

    if (winner === 'O')  {
        return -100 + depth;  // 'O' gagne
    }

    if (this.isFull()) {
        return 0;             // match nul
    }

    const maximise = this.joueur === 'X';      // 'X' maximise, 'O' minimise
    let best = maximise ? -Infinity : Infinity;

    for (const { x, y } of this.emptyCells()) {
        const other = this.clone();
        other.play(x, y);
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

// Retourne le MEILLEUR COUP { x, y } pour le joueur courant.
// scoreMethod = nom de la fonction d'évaluation à utiliser ('minmax' ou 'minmaxAB').
Puissance4.prototype.bestMove = function(scoreMethod = 'minmax') {
    const maximise = this.joueur === 'X';
    let best = maximise ? -Infinity : Infinity;
    let move = null;

    for (const { x, y } of this.emptyCells()) {
        const other = this.clone();
        other.play(x, y);
        const score = other[scoreMethod]();
        if (maximise ? score > best : score < best) {
            best = score;
            move = { x, y };
        }
    }

    return move;
}
