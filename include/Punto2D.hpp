#ifndef RICERCATRAIETTORIAIDEALEOMP_PUNTO2D_HPP
#define RICERCATRAIETTORIAIDEALEOMP_PUNTO2D_HPP

// Rappresenta un punto nello spazio.
class Punto2D {
private:
    double x;
    double y;

public:
    // Costruttori
    Punto2D();

    Punto2D(double x, double y);

    // Getter
    double getX() const;

    double getY() const;

    // Setter
    void setX(double x);

    void setY(double y);

    // Due punti sono uguali se hanno le stesse coordinate X e Y
    bool operator==(const Punto2D &punto) const;
};

#endif // RICERCATRAIETTORIAIDEALEOMP_PUNTO2D_HPP
