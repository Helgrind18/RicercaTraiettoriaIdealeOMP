#include "Traiettoria.hpp"

#include <stdexcept>

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

        if (posizione < 0.0 || posizione > 1.0) {
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

    if (posizione < 0.0 || posizione > 1.0) {
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

    if (posizione < 0.0 || posizione > 1.0) {
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