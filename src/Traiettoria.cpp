#include "Traiettoria.hpp"

#include <stdexcept>
#include <cmath>
#include <algorithm>

using namespace std;


// Costruttore di default
Traiettoria::Traiettoria() {
    this->posizioniLaterali = vector<double>();
}


// Costruttore parametrico
Traiettoria::Traiettoria(
    const vector<double>& posizioniLaterali
) {

    // Controlliamo che tutte le posizioni
    // siano comprese tra 0 e 1.
    for (double posizione : posizioniLaterali) {

        if (!std::isfinite(posizione) || posizione < 0.0 || posizione > 1.0) {
            throw invalid_argument(
                "Le posizioni laterali devono essere comprese tra 0 e 1"
            );
        }
    }

    this->posizioniLaterali = posizioniLaterali;
}


// Restituisce tutte le posizioni laterali
const vector<double>& Traiettoria::getPosizioniLaterali() const {
    return this->posizioniLaterali;
}

// Restituisce una specifica posizione laterale
double Traiettoria::getPosizioneLaterale(int indice) const {

    if (
        indice < 0 ||
        indice >= static_cast<int>(this->posizioniLaterali.size())
    ) {
        throw out_of_range(
            "Indice della traiettoria fuori range"
        );
    }

    return this->posizioniLaterali[indice];
}


// Restituisce il numero di posizioni della traiettoria
int Traiettoria::getNumeroPosizioni() const {
    return static_cast<int>(
        this->posizioniLaterali.size()
    );
}


// Aggiunge una posizione laterale
void Traiettoria::aggiungiPosizioneLaterale(
    double posizione
) {

    if (!std::isfinite(posizione) || posizione < 0.0 || posizione > 1.0) {
        throw invalid_argument(
            "La posizione laterale deve essere compresa tra 0 e 1"
        );
    }

    this->posizioniLaterali.push_back(posizione);
}


// Modifica una posizione laterale esistente
void Traiettoria::setPosizioneLaterale(
    int indice,
    double posizione
) {

    if (
        indice < 0 ||
        indice >= static_cast<int>(this->posizioniLaterali.size())
    ) {
        throw out_of_range(
            "Indice della traiettoria fuori range"
        );
    }

    if (!std::isfinite(posizione) || posizione < 0.0 || posizione > 1.0) {
        throw invalid_argument(
            "La posizione laterale deve essere compresa tra 0 e 1"
        );
    }

    this->posizioniLaterali[indice] = posizione;
}


// Converte la traiettoria nelle coordinate reali
// della pista utilizzando le SezionePista.
vector<Punto2D> Traiettoria::calcolaPunti(
    const vector<SezionePista>& sezioni
) const {

    if (
        sezioni.size() !=
        this->posizioniLaterali.size()
    ) {
        throw invalid_argument(
            "Il numero di sezioni deve coincidere con il numero di posizioni della traiettoria"
        );
    }


    vector<Punto2D> punti;

    punti.reserve(sezioni.size());


    for (size_t i = 0; i < sezioni.size(); i++) {

        double posizione =
            this->posizioniLaterali[i];

        Punto2D punto =
            sezioni[i].calcolaPuntoInterno(
                posizione
            );

        punti.push_back(punto);
    }


    return punti;
}

Traiettoria Traiettoria::ricampionaPeriodica(int numeroCampioni) const {
    const int n = getNumeroPosizioni();
    if (n < 3 || numeroCampioni < 3) {
        throw invalid_argument("Servono almeno 3 punti di controllo e 3 campioni");
    }
    vector<double> valori;
    valori.reserve(static_cast<size_t>(numeroCampioni));
    for (int i = 0; i < numeroCampioni; ++i) {
        // Parametro periodico: l'ultimo campione precede il primo,
        // senza duplicare la sezione iniziale.
        const double u = static_cast<double>(i) * n / numeroCampioni;
        const int j = static_cast<int>(std::floor(u));
        const double t = u - j;
        const double t2 = t*t, t3 = t2*t;
        const double w0 = (1.0 - 3.0*t + 3.0*t2 - t3) / 6.0;
        const double w1 = (4.0 - 6.0*t2 + 3.0*t3) / 6.0;
        const double w2 = (1.0 + 3.0*t + 3.0*t2 - 3.0*t3) / 6.0;
        const double w3 = t3 / 6.0;
        const double v = w0*posizioniLaterali[(j+n-1)%n]
                       + w1*posizioniLaterali[j%n]
                       + w2*posizioniLaterali[(j+1)%n]
                       + w3*posizioniLaterali[(j+2)%n];
        valori.push_back(std::clamp(v, 0.0, 1.0));
    }
    return Traiettoria(valori);
}
