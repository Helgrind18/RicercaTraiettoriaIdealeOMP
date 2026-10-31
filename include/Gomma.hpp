#ifndef RICERCATRAIETTORIAIDEALEOMP_GOMMA_HPP
#define RICERCATRAIETTORIAIDEALEOMP_GOMMA_HPP

#include <string>
using namespace std;
enum class TipoGomma {
    SOFT,
    MEDIUM,
    HARD
};

class Gomma {
private:
    int id;  // 0 = AS, 1 = AD, 2 = PS, 3 = PD
    TipoGomma tipo;
    double coefficienteGrip; // 0 = 0% ... 1 = 100%

public:
    Gomma();

    Gomma(
        int id,
        TipoGomma tipo,
        double coefficienteGrip
    );

    int getId() const;

    TipoGomma getTipo() const;

    double getCoefficienteGrip() const;

    void setTipo(TipoGomma tipo);

    void setCoefficienteGrip(double coefficienteGrip);

    void riduciGrip(double quantita);

    string toString() const;
};

#endif