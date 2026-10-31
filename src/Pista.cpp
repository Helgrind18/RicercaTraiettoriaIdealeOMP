#include "Pista.hpp"

using namespace std;

// Costruttore
Pista::Pista() {
    this->bordoSinistro = vector<Punto2D>();
    this->bordoDestro = vector<Punto2D>();
}

void Pista::aggiungiPuntoBordoSinistro(const Punto2D& punto) {
    this->bordoSinistro.push_back(punto);
}

void Pista::aggiungiPuntoBordoDestro(const Punto2D& punto) {
    this->bordoDestro.push_back(punto);
}

const vector<Punto2D>& Pista::getBordoSinistro() const {
    return this->bordoSinistro;
}

const vector<Punto2D>& Pista::getBordoDestro() const {
    return this->bordoDestro;
}
int Pista::getNumeroPuntiBordoSinistro() const {
    return static_cast<int>(this->bordoSinistro.size());
}

// Restituisce il numero di punti del bordo destro
int Pista::getNumeroPuntiBordoDestro() const {
    return static_cast<int>(this->bordoDestro.size());
}

// Restituisce il numero totale di punti memorizzati
int Pista::getNumeroPuntiTotali() const {
    return static_cast<int>(
        this->bordoSinistro.size() + this->bordoDestro.size()
    );
}
