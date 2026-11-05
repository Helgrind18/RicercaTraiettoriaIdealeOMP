#include "GeneraTraiettorie.hpp"
#include <stdexcept>
#include <algorithm>
#include <cmath>
using namespace std;


// Costruttore
GeneratoreTraiettorie::GeneratoreTraiettorie(unsigned int seed) : generatore(seed) {}

// Genera una traiettoria casuale REGOLARE per un circuito chiuso.
//
// Invece di estrarre un gene indipendente per ogni sezione, vengono
// estratti meno punti di controllo e interpolati con una B-spline
// cubica periodica. Le funzioni base sono positive e sommano a 1:
// quindi i geni rimangono nell'intervallo [0,1] senza tagli o clamp.
//
// La periodicita' collega in modo continuo anche l'ultima sezione
// alla prima: non compare uno spigolo sulla linea di partenza.
Traiettoria GeneratoreTraiettorie::generaTraiettoriaCasuale(int numeroSezioni) {
    if (numeroSezioni <= 0) {
        throw invalid_argument("Il numero di sezioni deve essere maggiore di zero");
    }

    // Un punto di controllo ogni ~8 sezioni, almeno 4 quando possibile.
    // Per piste con meno di 4 sezioni non creiamo piu' controlli dei geni.
    const int numeroControlli = min(numeroSezioni, max(4, 1 + (numeroSezioni - 1) / 8));
    uniform_real_distribution<double> distribuzione(0.0, 1.0);

    vector<double> controlli;
    controlli.reserve(numeroControlli);
    for (int j = 0; j < numeroControlli; ++j) {
        controlli.push_back(distribuzione(this->generatore));
    }

    vector<double> posizioniLaterali;
    posizioniLaterali.reserve(numeroSezioni);
    for (int i = 0; i < numeroSezioni; ++i) {
        // Parametro continuo lungo il circuito chiuso, [0,numeroControlli).
        const double u = static_cast<double>(i) * numeroControlli / numeroSezioni;
        const int j = static_cast<int>(u);
        const double t = u - j;
        const double t2 = t * t;
        const double t3 = t2 * t;

        // Basi della B-spline cubica uniforme: raccordo C2.
        const double b0 = (1.0 - 3.0*t + 3.0*t2 - t3) / 6.0;
        const double b1 = (4.0 - 6.0*t2 + 3.0*t3) / 6.0;
        const double b2 = (1.0 + 3.0*t + 3.0*t2 - 3.0*t3) / 6.0;
        const double b3 = t3 / 6.0;

        // Indici ciclici: anche prima/dopo la linea di partenza.
        const double posizione =
            b0 * controlli[(j + numeroControlli - 1) % numeroControlli] +
            b1 * controlli[j % numeroControlli] +
            b2 * controlli[(j + 1) % numeroControlli] +
            b3 * controlli[(j + 2) % numeroControlli];

        posizioniLaterali.push_back(clamp(posizione, 0.0, 1.0));
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