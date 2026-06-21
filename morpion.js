document.addEventListener("DOMContentLoaded", () => {
    const container = document.querySelector('#game')
    const morpion = new Morpion(container);
});

function Morpion(container) {
    this.container = container;
    this.statsMMContainer = document.querySelector('#stats-mm');
    this.statsABContainer = document.querySelector('#stats-ab');
    this.cells = [['', '', ''],['', '', ''],['', '', '']];
    this.joueur = 'O';
    this.totalMM = 0;   // cumul des grilles testées — minimax
    this.totalAB = 0;   // cumul des grilles testées — alpha-bêta

    this.attachEvents();
    this.draw();
}

Morpion.prototype.clone = function() {
    const copie = Object.create(Morpion.prototype);  // un Morpion sans constructeur
    copie.cells = structuredClone(this.cells);       // copie profonde du plateau
    copie.joueur = this.joueur;
    return copie;                                    // pas de container : simulation pure
};

Morpion.prototype.play = function(x, y) {
    if (!this.canPlay(x, y)) return '';

    const oldJoueur = this.joueur;
    this.cells[x][y] = this.joueur;
    this.joueur = this.joueur === 'O' ? 'X' : 'O';
    return oldJoueur;
}

Morpion.prototype.canPlay = function(x, y) {
    return x >= 0 && x < 3 && y >= 0 && y < 3 && this.cells[x][y] === '';
}

// Liste des cases vides sous forme { x, y }
Morpion.prototype.emptyCells = function() {
    const cells = [];
    this.cells.forEach((ligne, x) =>
        ligne.forEach((v, y) => { if (v === '') cells.push({ x, y }); })
    );
    return cells;
}

Morpion.prototype.draw = function() {
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

Morpion.prototype.attachEvents = function() {
    this.container.addEventListener('click', (event) => {
        const cell = event.target.closest('td');
        if (!cell) return;   // clic à côté d'une case

        this.play(Number(cell.dataset.x), Number(cell.dataset.y));
        this.draw();
        if (this.announceIfOver()) return;

        // Tour de l'IA : on mesure minimax ET alpha-bêta (si dispo) pour comparer
        this.statsMM = this.measure('minmax');
        this.statsAB = this.measure('minmaxAB');   // null tant que minmaxAB n'existe pas
        if (this.statsMM) this.totalMM += this.statsMM.nodes;
        if (this.statsAB) this.totalAB += this.statsAB.nodes;

        // On joue le coup trouvé (AB en priorité s'il est dispo, sinon minimax)
        const result = this.statsAB || this.statsMM;
        if (result && result.move) this.play(result.move.x, result.move.y);
        this.draw();
        this.drawStats();
        this.announceIfOver();
    });
};

// Lance bestMove avec la méthode de score donnée, en comptant les grilles + le temps.
// Retourne null si la méthode n'existe pas encore (ex : minmaxAB pas codé).
Morpion.prototype.measure = function(scoreMethod) {
    if (typeof this[scoreMethod] !== 'function') return null;

    Morpion.nodes = 0;
    const t0 = performance.now();
    const move = this.bestMove(scoreMethod);

    return {
        move,
        nodes: Morpion.nodes,
        timeMs: (performance.now() - t0).toFixed(1),
    };
};

// Affiche les stats d'exploration du dernier coup de l'IA (les deux algos)
Morpion.prototype.drawStats = function() {
    this.renderStatsBox(this.statsMMContainer, 'Stats IA minimax',
        this.statsMM, this.totalMM, '—');
    this.renderStatsBox(this.statsABContainer, 'Stats IA minimax α-β',
        this.statsAB, this.totalAB, 'α-β : à implémenter (<code>minmaxAB</code>)');
};

Morpion.prototype.renderStatsBox = function(box, titre, stats, total, placeholder) {
    if (!box) return;

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
Morpion.prototype.announceIfOver = function() {
    const winner = this.win();
    if (winner) alert(winner + " gagne !");
    else if (this.isFull()) alert("Match nul !");
    return this.isOver();
}

Morpion.prototype.win = function() {
    const c = this.cells.flat();   // index 0..8 (lecture ligne par ligne)
    const lignes = [
        [0, 1, 2], [3, 4, 5], [6, 7, 8],   // lignes
        [0, 3, 6], [1, 4, 7], [2, 5, 8],   // colonnes
        [0, 4, 8], [2, 4, 6],              // diagonales
    ];

    for (const [a, b, d] of lignes) {
        if (c[a] !== '' && c[a] === c[b] && c[b] === c[d]) {
            return c[a]; // 'O' ou 'X'
        }
    }

    return null; // pas encore de gagnant
}

Morpion.prototype.isFull = function() {
    return this.cells.flat().every(c => c !== '');
}

Morpion.prototype.isOver = function() {
    return this.win() !== null || this.isFull();
}

// Compteur statique : nombre de grilles explorées (partagé par tous les clones)
Morpion.nodes = 0;

// Retourne le SCORE de la position (un nombre, dans tous les cas)
Morpion.prototype.minmax = function(depth = 0) {
    Morpion.nodes++;   // une grille de plus explorée
    const winner = this.win();
    if (winner === 'X')  return 100 - depth;   // X gagne (favorise victoire rapide)
    if (winner === 'O')  return -100 + depth;  // O gagne
    if (this.isFull())   return 0;             // match nul

    const maximise = this.joueur === 'X';      // X maximise, O minimise
    let best = maximise ? -Infinity : Infinity;

    for (const { x, y } of this.emptyCells()) {
        const other = this.clone();
        other.play(x, y);
        const score = other.minmax(depth + 1);
        best = maximise ? Math.max(best, score) : Math.min(best, score);
    }

    return best;
}

// Retourne le MEILLEUR COUP { x, y } pour le joueur courant.
// scoreMethod = nom de la fonction d'évaluation à utiliser ('minmax' ou 'minmaxAB').
Morpion.prototype.bestMove = function(scoreMethod = 'minmax') {
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
