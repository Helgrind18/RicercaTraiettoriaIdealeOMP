#include "Gomma.hpp"
#include <stdexcept>
#include <iostream>
using namespace std;

//Costruttore
Gomma::Gomma() {
    this->id = 0;
    this->tipo = TipoGomma::SOFT;
    this->coefficienteGrip = 1; // 1 = 100 %
}

Gomma::Gomma(int id, TipoGomma tipo, double coefficienteGrip) {
    if (id < 0 || id > 3) {
        throw invalid_argument("Id fuori range");
    }
    if (coefficienteGrip < 0 || coefficienteGrip > 1) {
        throw invalid_argument("Impossibile impostare questo tipo di grip");
    }
    this->id = id;
    this->tipo = tipo;
    this->coefficienteGrip = coefficienteGrip;
}

int Gomma::getId() const {
    return this->id;
}

TipoGomma Gomma::getTipo() const {
    return this->tipo;
}

double Gomma::getCoefficienteGrip() const {
    return this->coefficienteGrip;
}

void Gomma::setCoefficienteGrip(double coefficienteGrip) {
    if (coefficienteGrip < 0 || coefficienteGrip > 1) {
        throw invalid_argument("Impossibile impostare questo tipo di grip");
    }
    this->coefficienteGrip = coefficienteGrip;
    return;
}

void Gomma::setTipo(TipoGomma tipo) {
    this->tipo = tipo;
}

void Gomma::riduciGrip(double quantita) {
    if (quantita < 0 || quantita > 1) {
        throw invalid_argument(
            "La quantita di grip da ridurre deve essere compresa tra 0 e 1"
        );
    }

    this->coefficienteGrip -= quantita;

    if (this->coefficienteGrip < 0) {
        this->coefficienteGrip = 0;
    }
}
string Gomma::toString() const {
    switch (this->tipo) {

        case TipoGomma::SOFT:
            return "SOFT";

        case TipoGomma::MEDIUM:
            return "MEDIUM";

        case TipoGomma::HARD:
            return "HARD";
    }

    return "SCONOSCIUTA";
}
