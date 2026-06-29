#!/usr/bin/env bash
#
# Analyse coup par coup d'une partie de Puissance 4 face au solveur PARFAIT de
# Pascal Pons (https://connect4.gamesolver.org). Pour chaque coup on demande à
# l'oracle le score de chaque colonne, et on repère où NOTRE IA quitte le coup
# optimal — en particulier le « basculement » gagnant → perdant : le coup exact
# où elle lâche une position théoriquement gagnée.
#
# Outil 100 % externe : l'app C++ reste autonome, sans aucun lien réseau.
#
# Usage :
#   ./solver-analyse.sh <sequence> [joueur_ia] [--base0]
#
#     <sequence>  Colonnes jouées depuis le début, sans séparateur, 1-indexées.
#                 Ex : 4453  →  P1 c4, P2 c4, P1 c5, P2 c3
#     [joueur_ia] 1 = l'IA ouvre (coups impairs)  |  2 = l'IA répond (coups pairs).
#                 Défaut : 1.
#     --base0     Si ta séquence vient des logs C++ (colonnes 0-6) : conversion auto.
#
#   Variable d'env DELAY = pause entre requêtes (défaut 0.4 s, on reste poli).
#
# Lecture du score (TOUJOURS du point de vue du joueur au trait) :
#   > 0 il gagne,  < 0 il perd,  0 nul ;  |score| grand = dénouement proche.
#   Meilleure colonne = le max, en IGNORANT 100 (= colonne pleine, coup illégal).

set -uo pipefail

API_URL="https://connect4.gamesolver.org/solve"

usage() { sed -n '2,30p' "$0" | sed 's/^# \{0,1\}//'; }

# --- Dépendances -----------------------------------------------------------
for bin in curl jq; do
    command -v "$bin" >/dev/null 2>&1 || { echo "Erreur : '$bin' est requis." >&2; exit 1; }
done

# --- Arguments -------------------------------------------------------------
base0=0
positional=()
for a in "$@"; do
    case "$a" in
        --base0)   base0=1 ;;
        -h|--help) usage; exit 0 ;;
        *)         positional+=("$a") ;;
    esac
done

seq="${positional[0]:-}"
ai="${positional[1]:-1}"

[[ -n $seq ]]         || { usage; exit 1; }
[[ $ai == 1 || $ai == 2 ]] || { echo "Erreur : joueur_ia doit valoir 1 ou 2." >&2; exit 1; }

# --- Validation / normalisation de la séquence -----------------------------
if (( base0 )); then
    [[ $seq =~ ^[0-6]+$ ]] || { echo "Erreur : séquence 0-indexée invalide (chiffres 0-6 attendus)." >&2; exit 1; }
    conv=""
    for (( k=0; k<${#seq}; k++ )); do conv+=$(( ${seq:k:1} + 1 )); done
    seq=$conv
else
    [[ $seq =~ ^[1-7]+$ ]] || { echo "Erreur : séquence invalide (chiffres 1-7 attendus, ou --base0 pour des logs 0-6)." >&2; exit 1; }
fi

# --- Appel API -------------------------------------------------------------
api() {  # $1 = pos (peut être vide pour la position de départ) ; renvoie le JSON
    curl -s -m 20 \
        -H 'User-Agent: Mozilla/5.0' \
        -H 'Referer: https://connect4.gamesolver.org/' \
        "${API_URL}?pos=$1"
}

# --- Boucle d'analyse ------------------------------------------------------
len=${#seq}
delay="${DELAY:-0.4}"

ai_optimal_streak=0   # nb de coups optimaux consécutifs de l'IA depuis le début
first_error_ply=0     # 1er coup où l'IA quitte le max (entorse, pas forcément fatale)
counting=1            # on compte la série tant que l'IA reste optimale
bascule_ply=0         # coup où l'IA passe d'une position gagnante (>0) à ≤0
bascule_from=0
bascule_to=0

echo "Séquence analysée : $seq   (l'IA = joueur $ai, marqué d'une *)"
echo
printf "%-4s %-7s %-4s %-7s %-6s %-13s %s\n" "Pli" "Joueur" "Col" "Score" "Best" "Optimale(s)" "Verdict"
printf "%-4s %-7s %-4s %-7s %-6s %-13s %s\n" "---" "------" "---" "-----" "----" "-----------" "-------"

for (( i=1; i<=len; i++ )); do
    prefix=${seq:0:i-1}
    played=${seq:i-1:1}
    (( i % 2 == 1 )) && mover=1 || mover=2
    is_ai=0; [[ $mover -eq $ai ]] && is_ai=1

    json=$(api "$prefix")
    [[ -n $json ]] || { echo "Pli $i : aucune réponse de l'API (réseau ?)." >&2; exit 1; }

    # best (hors 100), score du coup joué, colonnes optimales, partie-finie (tableau uniforme)
    IFS=$'\t' read -r best pscore optcols over <<<"$(
        echo "$json" | jq -r --argjson p $(( played - 1 )) '
            (.score | map(select(. != 100))) as $legal
            | ($legal | max) as $best
            | [ $best,
                .score[$p],
                ([ .score | to_entries[] | select(.value == $best) | .key + 1 ] | join(",")),
                ((.score | max) == (.score | min)) ]
            | @tsv')"

    if [[ $over == true ]]; then
        echo "Pli $i : position déjà terminée (un alignement existe avant ce coup) — arrêt."
        break
    fi

    who="P${mover}"; (( is_ai )) && who="P${mover}*"

    if [[ $pscore == "$best" ]]; then
        verdict="optimal"
    else
        verdict="sous-optimal (-$(( best - pscore )) pt)"
    fi

    printf "%-4s %-7s %-4s %-7s %-6s %-13s %s\n" "$i" "$who" "$played" "$pscore" "$best" "$optcols" "$verdict"

    if (( is_ai )); then
        if (( counting )); then
            if [[ $pscore == "$best" ]]; then
                ai_optimal_streak=$(( ai_optimal_streak + 1 ))
            else
                counting=0
                first_error_ply=$i
            fi
        fi
        if (( bascule_ply == 0 )) && (( best > 0 )) && (( pscore <= 0 )); then
            bascule_ply=$i
            bascule_from=$best
            bascule_to=$pscore
        fi
    fi

    sleep "$delay"
done

# --- Bilan -----------------------------------------------------------------
echo
echo "──────── Bilan ────────"
(( ai == 1 )) && echo "L'IA ouvrait (coups impairs)." || echo "L'IA répondait (coups pairs)."
echo "Coups optimaux de l'IA avant la 1re entorse : $ai_optimal_streak"

if (( first_error_ply > 0 )); then
    echo "1re entorse de l'IA : pli $first_error_ply (elle quitte le coup optimal)."
else
    echo "L'IA n'a jamais quitté le coup optimal sur la séquence donnée."
fi

if (( bascule_ply > 0 )); then
    issue="nul"; (( bascule_to < 0 )) && issue="perdant"
    echo "★ BASCULEMENT gagnant → $issue : pli $bascule_ply"
    echo "   L'IA tenait un gain (score +$bascule_from disponible) et a joué un coup à $bascule_to."
    echo "   → C'est LE coup qui a lâché la partie."
else
    echo "Aucun basculement gagnant→perdant : l'IA n'a jamais été théoriquement gagnante,"
    echo "ou n'a jamais lâché un gain (à vérifier selon le scénario)."
fi
