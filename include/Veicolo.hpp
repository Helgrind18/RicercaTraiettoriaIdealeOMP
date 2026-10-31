//
// Created by helgrind on 31/10/26.
//

#ifndef RICERCATRAIETTORIAIDEALEOMP_VEICOLO_HPP
#define RICERCATRAIETTORIAIDEALEOMP_VEICOLO_HPP

#include "Gomma.hpp"
#include <array>
using namespace std;
#define NUMERO_GOMME 4

class Veicolo {
private:
    double massa;
    double potenzaMassima;
    double accelerazioneMassima;
    double decelerazioneMassima;
    double coefficienteResistenzaAerodinamica;

    // Le quattro gomme del veicolo:
    // 0 = anteriore sinistra
    // 1 = anteriore destra
    // 2 = posteriore sinistra
    // 3 = posteriore destra
    array<Gomma, NUMERO_GOMME> gomme;

public:
    Veicolo();

    Veicolo(
        double massa,
        double potenzaMassima,
        double accelerazioneMassima,
        double decelerazioneMassima,
        double coefficienteResistenzaAerodinamica,
        const array<Gomma, NUMERO_GOMME> &gomme
    );

    double getMassa() const;

    double getPotenzaMassima() const;

    double getAccelerazioneMassima() const;

    double getDecelerazioneMassima() const;

    double getCoefficienteResistenzaAerodinamica() const;

    void setMassa(double massa);

    void setPotenzaMassima(double potenzaMassima);

    void setAccelerazioneMassima(double accelerazioneMassima);

    void setDecelerazioneMassima(double decelerazioneMassima);

    void setCoefficienteResistenzaAerodinamica(
        double coefficienteResistenzaAerodinamica
    );

    const array<Gomma, NUMERO_GOMME> &getGomme() const;

    // Restituisce la gomma identificata dall'id specificato.
    // L'id deve essere compreso tra 0 e 3.
    const Gomma &getGomma(int id) const;

    // Sostituisce la gomma nella posizione indicata dall'id.
    // L'id deve essere compreso tra 0 e 3.
    void cambiaGomma(int id, const Gomma &nuova);
};

#endif
