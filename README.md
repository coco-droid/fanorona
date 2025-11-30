
## 1. Architecture Globale et Modules

Le projet est structuré de manière modulaire pour séparer clairement les responsabilités.

### Structure des Modules
*   **Core (`src/core/`)** : Le cœur du moteur. Il contient la boucle principale (`MainLoop`), gère le temps (`DeltaTime`) et orchestre la communication entre les gestionnaires de scènes et d'événements.
*   **Logic (`src/logic/`)** : Contient la "Vérité" du jeu. C'est ici que l'état de la partie (`GameLogic`), le tour par tour, et la validation des règles sont gérés. Ce module est totalement indépendant du graphisme.
*   **Plateau & Pions (`src/plateau/`, `src/pions/`)** : Définit les structures de données fondamentales (le Graphe du plateau, les entités Pièces).
*   **AI (`src/ai/`)** : Le cerveau du jeu. Contient les algorithmes de décision (Minimax, MCTS, Markov) et les fonctions d'évaluation.
*   **Graphics & UI** : Gère le rendu SDL2, les textures, et les composants d'interface utilisateur (Boutons, Labels).

---

## 1. Logique de Jeu et Structures de Données

### Représentation du Plateau : Le Graphe (Détails)
Le Fanorona se joue sur une grille 5x9. Bien que visuellement rectangulaire, la logique interne traite le plateau comme un **Graphe de Connexions**.

#### Schéma ASCII du Graphe
Voici une représentation simplifiée du coin haut-gauche du plateau. Les chiffres sont les IDs des nœuds (0 à 44).
Les lignes (`-`, `|`, `\`, `/`) représentent les arêtes (mouvements possibles).

```text
0 -- 1 -- 2 -- 3 -- 4 ... (Ligne 0)
| \  |  / | \  |  / |
9 --10 --11 --12 --13 ... (Ligne 1)
|  / | \  |  / | \  |
18--19 --20 --21 --22 ... (Ligne 2)
...
```

*   **Nœuds Forts vs Faibles** :
    *   Le nœud **10** (Ligne 1, Col 1) est une intersection "Faible". Il est connecté uniquement à `1`, `9`, `11`, `19` (Haut, Gauche, Droite, Bas).
    *   Le nœud **11** (Ligne 1, Col 2) est une intersection "Forte". Il possède 8 voisins (Haut, Bas, Gauche, Droite + 4 Diagonales).
    *   **Calcul** : Un nœud est fort si `(ligne + colonne) % 2 == 0`.

#### Stockage de l'Information
L'information n'est pas stockée dans une matrice 2D `board[y][x]`.
Elle est stockée dans un tableau linéaire de nœuds `Node nodes[45]`.

Chaque nœud contient une **Liste d'Adjacence pré-calculée** :
```c
// Exemple conceptuel pour le Nœud 0 (Coin haut-gauche)
Node node_0 = {
    .id = 0,
    .piece = <Pointeur vers Piece Blanche>,
    .neighbors = {1, 9, 10}, // Droite, Bas, Diagonale Bas-Droite
    .count = 3
};
```

#### Modification du Graphe en Cours de Jeu
La **structure** du graphe (les connexions) est immuable (statique).
Seul l'**état** (le contenu des nœuds) change via des pointeurs.

1.  **Déplacement** :
    *   `nodes[depart].piece` devient `NULL`.
    *   `nodes[arrivee].piece` reçoit l'adresse de la pièce.
    *   Les coordonnées internes de la pièce `(r, c)` sont mises à jour.
2.  **Capture** :
    *   La pièce capturée est marquée `alive = false`.
    *   Le nœud qui la contenait : `nodes[capture_id].piece = NULL`.

### Comparatif : Graphe vs Matrice

Pourquoi utiliser un Graphe plutôt qu'une Matrice `int board[5][9]` ?

| Caractéristique | Matrice 2D (`board[y][x]`) | Graphe (Liste d'Adjacence) |
| :--- | :--- | :--- |
| **Mémoire** | **Plus économe**. Juste 45 entiers. | Plus lourd. Structures, pointeurs, listes de voisins. |
| **Vitesse (Lecture)** | Rapide (`board[y][x]`). | Rapide (`nodes[id]`). |
| **Vitesse (Algorithme)** | **Lente**. Pour trouver un coup, il faut calculer `x+1`, vérifier `x<9`, vérifier `y>0`... à chaque itération. | **Très Rapide**. On boucle juste sur `node->neighbors`. Pas de calcul de bornes, pas de "if hors limites". |
| **Logique Diagonale** | Complexe. Il faut coder des exceptions pour les cases sans diagonales. | **Simple**. Les diagonales existent ou n'existent pas dans la liste des voisins. Le code est générique. |

#### Pourquoi la Matrice est un piège pour ce jeu ?
Certains pourraient penser qu'une matrice est plus simple en utilisant des codes (ex: `0=Vide`, `1=Blanc`, `2=Noir`, `3=Visité`). **C'est une fausse bonne idée** pour deux raisons :
1.  **Le problème des "Murs Invisibles"** : Dans une matrice, la case (1,1) est géographiquement à côté de (0,0). Mais dans Fanorona, elles ne sont pas forcément connectées (intersections faibles). Avec une matrice, il faut ajouter des `if ((r+c)%2 != 0)` partout pour interdire les diagonales. Avec un Graphe, si la connexion n'existe pas, elle n'est juste pas dans la liste. On ne teste pas, on parcourt.
2.  **Le Nettoyage** : Si on marque les cases visitées dans la matrice (code 3), il faut parcourir toute la matrice (45 cases) à la fin de chaque simulation pour remettre des 0. Avec un Graphe, on utilise une petite liste `visited_nodes` de 5 éléments. Pour nettoyer, on fait juste `count = 0`. C'est beaucoup plus rapide pour l'IA qui simule des millions de coups.

**Conclusion** : Le Graphe consomme un peu plus de RAM (négligeable sur PC moderne, quelques Ko) mais rend l'IA et la validation des coups beaucoup plus rapides et le code plus propre (pas de `if` imbriqués pour les bords du plateau).

### Détails des Règles et Calculs Vectoriels

Le moteur de règles utilise l'algèbre vectorielle simple pour déterminer les captures.

#### Exemple de Calcul : Capture par Percussion (Mika)
Imaginons une pièce Blanche en **10** qui veut aller en **11**.
1.  **Vecteur de Mouvement** :
    *   Départ : `(r=1, c=1)`
    *   Arrivée : `(r=1, c=2)`
    *   Vecteur $\vec{v} = (0, 1)$ (Vers la droite).
2.  **Projection (Raycast)** :
    *   On regarde la case suivante après l'arrivée : `Arrivée + v` = `(1, 3)` -> Nœud **12**.
3.  **Vérification** :
    *   Si le Nœud **12** contient une pièce Noire -> **CAPTURE !**
    *   On continue dans la même direction (`12 + v` = `13`) tant qu'il y a des ennemis.

#### Exemple de Calcul : Capture par Aspiration (Miala)
Même mouvement (10 vers 11).
1.  **Vecteur Inverse** :
    *   On regarde derrière le départ : `Départ - v` = `(1, 0)` -> Nœud **9**.
2.  **Vérification** :
    *   Si le Nœud **9** contient une pièce Noire -> **CAPTURE !**

C'est cette abstraction mathématique qui permet de gérer les 8 directions (horizontales, verticales, diagonales) avec **une seule fonction générique** (`detect_capture`), au lieu d'écrire 8 fonctions différentes.

### Les Pièces (Entités)
Les pièces sont des structures dynamiques (`Piece`) allouées sur le tas (heap).
*   Elles possèdent une propriété `owner` (WHITE/BLACK) et un état `alive` (booléen).
*   **Pointeurs** : Le lien entre le plateau et les pièces se fait par pointeurs. `board->nodes[i].piece` pointe vers la structure de la pièce. Cela permet de déplacer une pièce simplement en changeant l'adresse pointée par le nœud, sans copier de données.

### Moteur de Règles (`src/logic/rules.c`)
C'est le module le plus complexe en termes de logique métier. Il implémente les règles spécifiques du Fanorona via une approche vectorielle.

#### 1. Le Concept de Vecteur de Mouvement
Tout déplacement est défini mathématiquement par un vecteur $(dr, dc)$.
*   **Exemple** : Aller de $(2,2)$ vers $(2,3)$ donne le vecteur $(0, 1)$.
*   **Exemple** : Aller de $(2,2)$ vers $(1,3)$ donne le vecteur $(-1, 1)$.
Ce vecteur est crucial pour calculer les captures.

#### 2. Algorithme de Capture (Raycasting)
La fonction `detect_capture` utilise ce vecteur pour "lancer un rayon" sur le plateau.

*   **Percussion (Mika)** : On regarde "devant" le mouvement.
    *   `Position_Test = Arrivée + Vecteur`
    *   Tant que `Position_Test` contient un ennemi, on capture et on avance encore (`+ Vecteur`).
*   **Aspiration (Miala)** : On regarde "derrière" le mouvement.
    *   `Position_Test = Départ - Vecteur`
    *   Tant que `Position_Test` contient un ennemi, on capture et on recule encore (`- Vecteur`).

#### 3. Gestion des Priorités et États
*   **Capture Obligatoire** : Avant d'accepter un coup simple ("Paika"), le code lance `has_any_capture_available`. Cette fonction scanne tout le plateau. Si *une seule* capture est possible ailleurs, le coup simple est rejeté.
*   **Combo (Enchaînement)** :
    *   Le système garde une liste `visited_positions` pour le tour actuel.
    *   Règle stricte : Interdiction de repasser par une position déjà occupée ce tour-ci (évite les boucles infinies).
    *   Règle stricte : Interdiction de capturer deux fois de suite dans la même direction vectorielle (ex: deux fois vers le Nord).

---

## 3. Intelligence Artificielle (AI)

L'IA est conçue pour être modulaire et performante. Elle ne "voit" pas le jeu graphiquement, elle manipule des données abstraites.

### Comment l'IA perçoit le jeu ? (`BoardSnapshot`)
Pour optimiser les calculs, l'IA ne travaille pas sur la structure lourde `Board` utilisée par le jeu principal. Elle utilise des **Snapshots** (`BoardSnapshot`).
*   C'est une version compacte et légère du plateau (tableaux d'octets au lieu de structures lourdes).
*   Cela permet de copier et simuler des milliers d'états par seconde sans surcharger la mémoire.

### Algorithmes de Décision
L'IA utilise une approche hybride selon la phase de jeu :

1.  **Minimax avec Élagage Alpha-Beta** :
    *   C'est un algorithme récursif qui construit un **Arbre de Recherche**.
    *   **Racine** : L'état actuel du plateau.
    *   **Branches** : Chaque coup légal possible.
    *   **Feuilles** : L'état du plateau après N coups (profondeur définie par la difficulté).
    *   L'IA maximise son score tout en supposant que l'adversaire jouera parfaitement pour minimiser ce score.
    *   **Alpha-Beta** : Si l'IA trouve un coup "suffisamment bon", elle arrête d'explorer les branches qui sont manifestement pires (coupure), ce qui accélère le calcul.

2.  **Fonction d'Évaluation (`evaluate`)** :
    *   Comment l'IA sait qu'elle gagne ? Elle calcule un score numérique pour un état donné.
    *   **Matériel** : +100 points par pièce alliée, -100 par pièce ennemie.
    *   **Mobilité** : Bonus si elle a beaucoup de mouvements possibles (liberté d'action).
    *   **Centralité** : Bonus pour le contrôle du centre du plateau (nœuds stratégiques).
    *   **Victoire** : Si l'adversaire a 0 pièce, le score est `INFINITY`.

---

## 4. Système Graphique et Moteur (`src/core/`)

Le rendu est basé sur SDL2 mais abstrait via un système de Scènes.

### Gestionnaire de Scènes (Scene Manager)
Le jeu fonctionne comme une machine à états finis.
*   Le `SceneManager` détient un pointeur vers la `Scene` active (Menu, Jeu, Fin).
*   **Boucle de Jeu** :
    1.  `Input` : Le `EventManager` capture les clics et touches.
    2.  `Update` : La scène active met à jour sa logique (animations, timers).
    3.  `Render` : La scène active dessine ses composants.

### UI et Événements
*   **Arbre de composants** : L'interface est construite hiérarchiquement (Boutons, Textes).
*   **Raycasting (Simplifié)** : Lors d'un clic souris, le système vérifie si les coordonnées $(x, y)$ de la souris sont à l'intérieur des cercles définissant les intersections du plateau. Si oui, l'événement est traduit en "Clic sur Nœud ID 12" et envoyé à la logique.

---

## 5. Concepts Techniques Clés

Pour expliquer le code demain, voici les concepts informatiques utilisés :

*   **Pointeurs** : Omniprésents. Utilisés pour le graphe (voisins), pour lier les pièces aux nœuds, et pour passer les structures lourdes (`GameLogic*`, `Board*`) aux fonctions sans copie inutile.
*   **Vecteurs Mathématiques** : Utilisés dans `rules.c` pour calculer les directions de capture et l'alignement des pièces.
*   **Arbres (Trees)** : Structure de données implicite créée par la récursion de l'algorithme Minimax de l'IA.
*   **Allocation Dynamique (`malloc`/`free`)** : Toutes les entités du jeu (Pièces, Joueurs, Scènes) sont allouées dynamiquement pour gérer la mémoire précisément (voir les fonctions `_create` et `_destroy`).

---

## 6. Mini-Quiz de Soutenance (Questions & Réponses)

Voici une liste de questions techniques probables et les réponses idéales basées sur votre code.

### Q1 : Comment votre code valide-t-il un mouvement ?
> **Réponse** : La validation se fait en 3 étapes :
> 1. **Topologie** : Je vérifie via le Graphe si les nœuds sont voisins (`neighbors[]`).
> 2. **Disponibilité** : Je vérifie si la case d'arrivée est vide (`piece == NULL`).
> 3. **Légalité (Règles)** : Je vérifie la "Capture Obligatoire". Si une capture est possible ailleurs sur le plateau, un mouvement simple est interdit.

### Q2 : Comment distinguez-vous la Percussion (Mika) de l'Aspiration (Miala) ?
> **Réponse** : C'est purement mathématique. J'utilise le vecteur de déplacement $(dx, dy)$.
> *   Pour la **Percussion**, j'additionne ce vecteur à la position d'arrivée (`pos + v`).
> *   Pour l'**Aspiration**, je soustrais ce vecteur à la position de départ (`pos - v`).
> Si les deux renvoient des ennemis, je demande au joueur de choisir (gestion d'ambiguïté).

### Q3 : Pourquoi utiliser un Graphe et pas un tableau 2D classique ?
> **Réponse** : Pour simplifier la gestion des diagonales. Dans une matrice, gérer les intersections "fortes" (avec diagonales) et "faibles" (sans) demande beaucoup de `if`. Avec un graphe, une diagonale est juste un voisin comme un autre dans la liste d'adjacence. L'algorithme de recherche de chemin est donc le même pour toutes les directions.

### Q4 : Comment gérez-vous l'enchaînement des captures (Combo) ?
> **Réponse** : Je stocke l'état du tour dans des variables temporaires : `visited_positions` (tableau d'IDs) et `last_direction`.
> À chaque étape du combo, je vérifie que la nouvelle position n'est pas dans `visited_positions` (interdiction de repasser au même endroit) et que la direction n'est pas identique à la précédente si c'est une capture successive (selon la variante des règles).

### Q5 : Comment l'IA "voit-elle" le plateau ?
> **Réponse** : Elle ne regarde pas les objets graphiques. Elle utilise une structure optimisée appelée `BoardSnapshot`. C'est une copie compacte du plateau (juste des octets) qui permet de simuler des milliers de coups par seconde sans modifier le vrai plateau de jeu affiché à l'écran.

### Q6 : Pourquoi ne pas simplement utiliser une matrice et marquer les cases visitées avec un numéro spécial ?
> **Réponse** : C'est une question de performance et de simplicité logique.
> 1. **Performance** : Si je modifie la matrice pour marquer "Visité", je dois parcourir tout le tableau pour le nettoyer après chaque coup simulé. Avec ma méthode (liste `visited[]`), le nettoyage est instantané ($O(1)$).
> 2. **Logique** : Le problème principal du Fanorona n'est pas de stocker l'état, mais de gérer les connexions (diagonales autorisées ou non). Une matrice m'obligerait à recalculer la parité `(x+y)%2` à chaque mouvement. Le Graphe me donne directement les chemins valides sans calcul mathématique à chaque frame. C'est comme avoir un GPS qui connaît déjà les routes au lieu de regarder une carte et vérifier les sens interdits à chaque carrefour.
