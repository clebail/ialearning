document.addEventListener("DOMContentLoaded", () => {
    const container = document.querySelector('#game')
    const morpion = new Morpion(container);
});

function Morpion(container) {
    this.container = container;
    this.cells = [['', '', ''],['', '', ''],['', '', '']];
    this.joueur = 'O';

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

        // Tour de l'IA
        const move = this.bestMove();
        if (move) this.play(move.x, move.y);
        this.draw();
        this.announceIfOver();
    });
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

// Retourne le SCORE de la position (un nombre, dans tous les cas)
Morpion.prototype.minmax = function(depth = 0) {
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

// Retourne le MEILLEUR COUP { x, y } pour le joueur courant
Morpion.prototype.bestMove = function() {
    const maximise = this.joueur === 'X';
    let best = maximise ? -Infinity : Infinity;
    let move = null;

    for (const { x, y } of this.emptyCells()) {
        const other = this.clone();
        other.play(x, y);
        const score = other.minmax();
        if (maximise ? score > best : score < best) {
            best = score;
            move = { x, y };
        }
    }

    return move;
}
