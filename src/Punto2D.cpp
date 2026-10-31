#include "Punto2D.hpp"
//Qui dentro devo inserire l'implementazione dei metodi dell'header
//Punto2D:: vuol dire che sto implementando un metodo appartenente a quella classe
Punto2D::Punto2D() {
    //Costruttore base
    x = 0.0;
    y = 0.0;
}
Punto2D::Punto2D(double x, double y) {
    this->x = x;
    this->y = y;
}
double Punto2D::getX() const {
    return x;
}
double Punto2D::getY() const {
    return y;
}
void Punto2D::setX(double x) {
    this->x = x;
}
void Punto2D::setY(double y) {
    this->y = y;
}

bool Punto2D::operator==(const Punto2D &punto) const{
    return this->x == punto.x && this->y == punto.y;
}
