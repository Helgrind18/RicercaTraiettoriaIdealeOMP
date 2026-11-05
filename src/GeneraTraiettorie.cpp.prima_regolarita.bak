#include "GeneraTraiettorie.hpp"
#include <stdexcept>
using namespace std;


// Costruttore
GeneratoreTraiettorie::GeneratoreTraiettorie(unsigned int seed) : generatore(seed) {}

// Genera una singola traiettoria casuale.
//
// Ogni posizione laterale viene generata
// uniformemente nell'intervallo [0.0, 1.0].
Traiettoria GeneratoreTraiettorie::generaTraiettoriaCasuale(int numeroSezioni) {
    if (numeroSezioni <= 0) {
        throw invalid_argument("Il numero di sezioni deve essere maggiore di zero");
    }
    // Genera valori double compresi tra 0.0 e 1.0.
    uniform_real_distribution<double> distribuzione(0.0,1.0);
    vector<double> posizioniLaterali;
    posizioniLaterali.reserve(numeroSezioni);
    for (int i = 0; i < numeroSezioni; i++) {
        double posizioneCasuale = distribuzione(this->generatore);
        posizioniLaterali.push_back(posizioneCasuale);
    }
    return Traiettoria(posizioniLaterali);
}


// Genera una popolazione di traiettorie casuali.
//
// Ogni traiettoria contiene numeroSezioni
// posizioni laterali comprese tra 0 e 1.
vector<Traiettoria> GeneratoreTraiettorie::generaPopolazione(int numeroTraiettorie,int numeroSezioni) {
    if (numeroTraiettorie <= 0) {
        throw invalid_argument("Il numero di traiettorie deve essere maggiore di zero");
    }
    if (numeroSezioni <= 0) {
        throw invalid_argument("Il numero di sezioni deve essere maggiore di zero");
    }
    vector<Traiettoria> popolazione;
    popolazione.reserve(numeroTraiettorie);
    for (int i = 0; i < numeroTraiettorie; i++) {
        Traiettoria traiettoria = generaTraiettoriaCasuale(numeroSezioni);
        popolazione.push_back(traiettoria);
    }
    return popolazione;
}


// Genera una traiettoria che passa sempre
// al centro di ogni sezione della pista.
//
// Tutte le posizioni laterali sono quindi 0.5.
Traiettoria GeneratoreTraiettorie::generaTraiettoriaCentrale(int numeroSezioni) const {
    if (numeroSezioni <= 0) {
        throw invalid_argument("Il numero di sezioni deve essere maggiore di zero");
    }
    vector<double> posizioniLaterali;
    posizioniLaterali.reserve(numeroSezioni);
    for (int i = 0; i < numeroSezioni; i++) {
        posizioniLaterali.push_back(0.5);
    }
    return Traiettoria(posizioniLaterali);
}