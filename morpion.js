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

Morpion.prototype.play = function(x, y) {
    if (this.canPlay(x, y)) {
        this.cells[x][y] = this.joueur;
        this.joueur = String.fromCharCode('O'.charCodeAt(0) + 'X'.charCodeAt(0) - this.joueur.charCodeAt(0));

        this.draw();

        return true
    }

    return false;
}

Morpion.prototype.canPlay = function(x, y) {
    if (x >= 0 && x < 3 && y >= 0 && y < 3) {
        return this.cells[x][y] === '';
    }

    return false;
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

        const x = Number(cell.dataset.x);
        const y = Number(cell.dataset.y);

        this.play(x, y);
        this.isWin();
    });
};

Morpion.prototype.isWin = function() {
    const winner = this.win();

    if (winner) {
        alert(winner + " gagne !");
    }
}

Morpion.prototype.win = function() {
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

Morpion.prototype.isFull = function() {
    return this.cells.every(ligne => ligne.every(c => c !== ''));
};

Morpion.prototype.isOver = function() {
    return this.win() !== null || this.isFull();
};
