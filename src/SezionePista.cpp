#include "SezionePista.hpp"

#include <cmath>
#include <stdexcept>

using namespace std;


// Costruttore di default
SezionePista::SezionePista() {
    this->puntoSinistro = Punto2D();
    this->puntoDestro = Punto2D();
}


SezionePista::SezionePista(
    const Punto2D &puntoSinistro,
    const Punto2D &puntoDestro
) {
    this->puntoSinistro = puntoSinistro;
    this->puntoDestro = puntoDestro;
}


const Punto2D &SezionePista::getPuntoSinistro() const {
    return this->puntoSinistro;
}


const Punto2D &SezionePista::getPuntoDestro() const {
    return this->puntoDestro;
}


void SezionePista::setPuntoSinistro(
    const Punto2D &puntoSinistro
) {
    this->puntoSinistro = puntoSinistro;
}


void SezionePista::setPuntoDestro(
    const Punto2D &puntoDestro
) {
    this->puntoDestro = puntoDestro;
}


// Calcola la distanza euclidea tra il punto sinistro
// e il punto destro della sezione.
double SezionePista::calcolaLarghezza() const {
    double differenzaX =
            this->puntoDestro.getX() -
            this->puntoSinistro.getX();

    double differenzaY =
            this->puntoDestro.getY() -
            this->puntoSinistro.getY();

    return sqrt(
        differenzaX * differenzaX +
        differenzaY * differenzaY
    );
}


// Calcola un punto interno alla sezione tramite
// interpolazione lineare tra il bordo sinistro
// e il bordo destro.
Punto2D SezionePista::calcolaPuntoInterno(
    double posizione
) const {
    if (posizione < 0.0 || posizione > 1.0) {
        throw invalid_argument(
            "La posizione deve essere compresa tra 0 e 1"
        );
    }

    double x =
            this->puntoSinistro.getX() +
            posizione *
            (
                this->puntoDestro.getX() -
                this->puntoSinistro.getX()
            );

    double y =
            this->puntoSinistro.getY() +
            posizione *
            (
                this->puntoDestro.getY() -
                this->puntoSinistro.getY()
            );

    return Punto2D(x, y);
}
