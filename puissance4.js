document.addEventListener("DOMContentLoaded", () => {
    const container = document.querySelector('#game')
    const puissance4 = new Puissance4(container);
});

// Compteur statique : nombre de grilles explorées (partagé par tous les clones)
Puissance4.nodesAB = 0;
const MAX_DEPTH = 5;

function Puissance4(container) {
    this.container = container;
    this.statsABContainer = document.querySelector('#stats-ab');
    this.cells = [['', '', '', '', '', '', ''],['', '', '', '', '', '', ''],['', '', '', '', '', '', ''],['', '', '', '', '', '', ''],['', '', '', '', '', '', ''],['', '', '', '', '', '', '']];
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

// Liste des colonnes jouables
Puissance4.prototype.availableColumns = function() {
    const columns = [];
    for (let x=0;x<7;x++) {
        if( (this.canPlay(x))) {
            columns.push(x)
        }
    }

    return columns;
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
    if (this.nbAlignes('O', 4)) {
        return 'O';
    }

    return this.nbAlignes('X', 4);
}

Puissance4.prototype.nbAlignes = function(j, nb) {
    let y, x, i;

    // Alignement horizontal (→)
    for (y=5;y>=0;y--) {
        for (x=0;x<=7-nb;x++) {
            if (this.cells[y][x] === j) {
                let ok = true;
                for (i = 1; i < nb; i++) {
                    if (this.cells[y][x] !== this.cells[y][x + i]) {
                        ok = false;
                    }
                }
                if(ok) {
                    if (nb === 4) {
                        return j;
                    }

                    if ((x > 0 && this.cells[y][x-1] === '') || (x + nb < 7 && this.cells[y][x+nb] === '')) {
                        return j;
                    }
                }
            }
        }
    }

    // Alignement vertical (on lit vers le haut ; en pratique seule l'extrémité haute peut être vide — gravité)
    for (x=0;x<7;x++) {
        for (y=5;y>=nb-1;y--) {
            if (this.cells[y][x] === j) {
                let ok = true;
                for (i = 1; i < nb; i++) {
                    if (this.cells[y][x] !== this.cells[y-i][x]) {
                        ok = false;
                    }
                }
                if(ok) {
                    if (nb === 4) {
                        return j;
                    }

                    if ((y < 5 && this.cells[y+1][x] === '') || (y - nb >= 0 && this.cells[y-nb][x] === '')) {
                        return j;
                    }
                }
            }
        }
    }

    // Alignement diagonale ↗
    for (y=5;y>=nb-1;y--) {
        for (x=0;x<=7-nb;x++) {
            if (this.cells[y][x] === j) {
                let ok = true;
                for (i = 1; i < nb; i++) {
                    if (this.cells[y][x] !== this.cells[y-i][x+i]) {
                        ok = false;
                    }
                }
                if(ok) {
                    if (nb === 4) {
                        return j;
                    }

                    if ((y < 5 && x > 0 && this.cells[y+1][x-1] === '') || (y - nb >= 0 && x + nb < 7 && this.cells[y-nb][x+nb] === '')) {
                        return j;
                    }
                }
            }
        }
    }

    // Alignement diagonale ↖
    for (y=5;y>=nb-1;y--) {
        for (x=nb-1;x<7;x++) {
            if (this.cells[y][x] === j) {
                let ok = true;
                for (i = 1; i < nb; i++) {
                    if (this.cells[y][x] !== this.cells[y-i][x-i]) {
                        ok = false;
                    }
                }
                if(ok) {
                    if (nb === 4) {
                        return j;
                    }

                    if ((y < 5 && x < 6 && this.cells[y+1][x+1] === '') || (y - nb >= 0 && x - nb >= 0 && this.cells[y-nb][x-nb] === '')) {
                        return j;
                    }
                }
            }
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

// Retourne le MEILLEUR COUP { x, y } pour le joueur courant.
Puissance4.prototype.bestMove = function( ) {
    const maximise = this.joueur === 'X';
    let best = maximise ? -Infinity : Infinity;
    let move = null;

    for (const x of this.availableColumns()) {
        const other = this.clone();
        other.play(x);
        const score = other.minmaxAB();
        if (maximise ? score > best : score < best) {
            best = score;
            move = x;
        }
    }

    return move;
}

Puissance4.prototype.evaluate = function( ) {
    let score = 0;

    if (this.nbAlignes('X', 3)) score += 1000;
    if (this.nbAlignes('X', 2)) score += 100;
    if (this.nbAlignes('O', 3)) score -= 1000;
    if (this.nbAlignes('O', 2)) score -= 100;

    return score;
}
