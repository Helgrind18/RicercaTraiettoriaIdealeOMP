#ifndef RICERCATRAIETTORIAIDEALEOMP_GENERATORETRAIETTORIE_HPP
#define RICERCATRAIETTORIAIDEALEOMP_GENERATORETRAIETTORIE_HPP

#include "Traiettoria.hpp"

#include <random>
#include <vector>
using namespace std;
// Si occupa della generazione delle traiettorie iniziali
// utilizzate dall'algoritmo genetico.
//
// Ogni traiettoria contiene una posizione laterale per ogni
// sezione della pista.
//
// Le posizioni vengono generate nell'intervallo:
//
// 0.0 = bordo sinistro
// 0.5 = centro pista
// 1.0 = bordo destro
//
// La classe utilizza un generatore pseudocasuale con seed,
// in modo da poter riprodurre gli stessi esperimenti.
class GeneratoreTraiettorie {
private:

    // Generatore pseudocasuale utilizzato per creare
    // le posizioni laterali delle traiettorie.
    mt19937 generatore;

public:

    // Costruisce il generatore utilizzando il seed specificato.
    explicit GeneratoreTraiettorie(unsigned int seed);

    // Genera una singola traiettoria casuale contenente
    // una posizione laterale per ogni sezione.
    //
    // numeroSezioni deve essere maggiore di zero.
    Traiettoria generaTraiettoriaCasuale(int numeroSezioni);

    // Genera una popolazione di traiettorie casuali.
    //
    // numeroTraiettorie indica quanti individui creare.
    // numeroSezioni indica quante posizioni deve contenere
    // ogni traiettoria.
    vector<Traiettoria> generaPopolazione(int numeroTraiettorie,int numeroSezioni);

    // Genera una traiettoria che passa sempre
    // per il centro della pista.
    //
    // Ogni posizione laterale sarà uguale a 0.5.
    Traiettoria generaTraiettoriaCentrale(int numeroSezioni) const;
};

#endif