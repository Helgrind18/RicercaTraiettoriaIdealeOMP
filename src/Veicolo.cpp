#include "Veicolo.hpp"

#include <stdexcept>

using namespace std;


// Costruttore di default
Veicolo::Veicolo()
    : gomme{
        Gomma(0, TipoGomma::SOFT, 1.0),
        Gomma(1, TipoGomma::SOFT, 1.0),
        Gomma(2, TipoGomma::SOFT, 1.0),
        Gomma(3, TipoGomma::SOFT, 1.0)
    } {
    this->massa = 0.0;
    this->potenzaMassima = 0.0;
    this->accelerazioneMassima = 0.0;
    this->decelerazioneMassima = 0.0;
    this->coefficienteResistenzaAerodinamica = 0.0;
}


// Costruttore parametrico
Veicolo::Veicolo(
    double massa,
    double potenzaMassima,
    double accelerazioneMassima,
    double decelerazioneMassima,
    double coefficienteResistenzaAerodinamica,
    const array<Gomma, 4> &gomme
) {
    if (
        massa <= 0.0 ||
        potenzaMassima <= 0.0 ||
        accelerazioneMassima <= 0.0 ||
        decelerazioneMassima <= 0.0 ||
        coefficienteResistenzaAerodinamica <= 0.0
    ) {
        throw invalid_argument(
            "Parametri di costruzione del veicolo non corretti"
        );
    }

    // Controllo che l'id di ogni gomma corrisponda
    // alla sua posizione nell'array.
    for (int i = 0; i < 4; i++) {
        if (gomme[i].getId() != i) {
            throw invalid_argument(
                "L'id della gomma non corrisponde alla sua posizione"
            );
        }
    }

    // Tutte le gomme devono avere la stessa mescola.
    TipoGomma tipoCorretto = gomme[0].getTipo();
    for (const Gomma &gomma: gomme) {
        if (gomma.getTipo() != tipoCorretto) {
            throw invalid_argument(
                "La macchina deve avere la stessa mescola su tutte le gomme"
            );
        }
    }

    this->massa = massa;
    this->potenzaMassima = potenzaMassima;
    this->accelerazioneMassima = accelerazioneMassima;
    this->decelerazioneMassima = decelerazioneMassima;
    this->coefficienteResistenzaAerodinamica =
            coefficienteResistenzaAerodinamica;

    this->gomme = gomme;
}


double Veicolo::getMassa() const {
    return this->massa;
}


double Veicolo::getPotenzaMassima() const {
    return this->potenzaMassima;
}


double Veicolo::getAccelerazioneMassima() const {
    return this->accelerazioneMassima;
}


double Veicolo::getDecelerazioneMassima() const {
    return this->decelerazioneMassima;
}


double Veicolo::getCoefficienteResistenzaAerodinamica() const {
    return this->coefficienteResistenzaAerodinamica;
}


const array<Gomma, NUMERO_GOMME> &Veicolo::getGomme() const {
    return this->gomme;
}


void Veicolo::setMassa(double massa) {
    if (massa <= 0.0) {
        throw invalid_argument(
            "La massa non puo essere negativa o nulla"
        );
    }

    this->massa = massa;
}


void Veicolo::setPotenzaMassima(double potenzaMassima) {
    if (potenzaMassima <= 0.0) {
        throw invalid_argument(
            "La potenza non puo essere negativa o nulla"
        );
    }

    this->potenzaMassima = potenzaMassima;
}


void Veicolo::setAccelerazioneMassima(double accelerazioneMassima) {
    if (accelerazioneMassima <= 0.0) {
        throw invalid_argument(
            "L'accelerazione non puo essere negativa o nulla"
        );
    }

    this->accelerazioneMassima = accelerazioneMassima;
}


void Veicolo::setDecelerazioneMassima(double decelerazioneMassima) {
    if (decelerazioneMassima <= 0.0) {
        throw invalid_argument(
            "La decelerazione non puo essere negativa o nulla"
        );
    }

    // Convenzione: la decelerazione viene mantenuta positiva.
    this->decelerazioneMassima = decelerazioneMassima;
}


void Veicolo::setCoefficienteResistenzaAerodinamica(
    double coefficienteResistenzaAerodinamica
) {
    if (coefficienteResistenzaAerodinamica <= 0.0) {
        throw invalid_argument(
            "Il coefficiente non puo essere negativo o nullo"
        );
    }

    this->coefficienteResistenzaAerodinamica =
            coefficienteResistenzaAerodinamica;
}


const Gomma &Veicolo::getGomma(int id) const {
    if (id < 0 || id > 3) {
        throw invalid_argument(
            "Id della gomma fuori range"
        );
    }

    return this->gomme[id];
}


void Veicolo::cambiaGomma(int id, const Gomma &nuova) {
    if (id < 0 || id > 3) {
        throw invalid_argument(
            "Id della gomma fuori range"
        );
    }

    if (nuova.getId() != id) {
        throw invalid_argument(
            "L'id della nuova gomma non corrisponde alla posizione"
        );
    }

    // Manteniamo la regola secondo cui il veicolo
    // deve utilizzare la stessa mescola su tutte le gomme.
    TipoGomma tipoAttuale = this->gomme[0].getTipo();

    if (nuova.getTipo() != tipoAttuale) {
        throw invalid_argument(
            "La nuova gomma deve avere la stessa mescola delle altre"
        );
    }

    this->gomme[id] = nuova;
}
