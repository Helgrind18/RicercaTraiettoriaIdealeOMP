#include "TestPunto2D.hpp"
#include "Punto2D.hpp"

#include <cassert>
#include <iostream>

void eseguiTestPunto2D() {

    std::cout << "========================================" << std::endl;
    std::cout << "       TEST DELLA CLASSE Punto2D        " << std::endl;
    std::cout << "========================================" << std::endl;


    // -------------------------------------------------
    // TEST 1: COSTRUTTORE VUOTO
    // -------------------------------------------------

    std::cout << "\n[Test 1] Costruttore vuoto" << std::endl;

    Punto2D punto1;

    std::cout << "Punto creato." << std::endl;
    std::cout << "Valore X ottenuto: " << punto1.getX() << std::endl;
    std::cout << "Valore Y ottenuto: " << punto1.getY() << std::endl;

    std::cout << "Valori attesi: X = 0.0, Y = 0.0" << std::endl;

    assert(punto1.getX() == 0.0);
    assert(punto1.getY() == 0.0);

    std::cout << "[OK] Costruttore vuoto funzionante." << std::endl;


    // -------------------------------------------------
    // TEST 2: COSTRUTTORE CON PARAMETRI
    // -------------------------------------------------

    std::cout << "\n[Test 2] Costruttore con parametri" << std::endl;

    Punto2D punto2(10.5, 20.5);

    std::cout << "Creato Punto2D(10.5, 20.5)" << std::endl;
    std::cout << "Valore X ottenuto: " << punto2.getX() << std::endl;
    std::cout << "Valore Y ottenuto: " << punto2.getY() << std::endl;

    std::cout << "Valori attesi: X = 10.5, Y = 20.5" << std::endl;

    assert(punto2.getX() == 10.5);
    assert(punto2.getY() == 20.5);

    std::cout << "[OK] Costruttore con parametri funzionante." << std::endl;


    // -------------------------------------------------
    // TEST 3: SETTER X
    // -------------------------------------------------

    std::cout << "\n[Test 3] Setter setX()" << std::endl;

    std::cout << "Valore X prima di setX(): "
              << punto2.getX() << std::endl;

    std::cout << "Eseguo setX(50.0)..." << std::endl;

    punto2.setX(50.0);

    std::cout << "Valore X dopo setX(): "
              << punto2.getX() << std::endl;

    std::cout << "Valore atteso: X = 50.0" << std::endl;

    assert(punto2.getX() == 50.0);

    std::cout << "[OK] Setter X funzionante." << std::endl;


    // -------------------------------------------------
    // TEST 4: SETTER Y
    // -------------------------------------------------

    std::cout << "\n[Test 4] Setter setY()" << std::endl;

    std::cout << "Valore Y prima di setY(): "
              << punto2.getY() << std::endl;

    std::cout << "Eseguo setY(100.0)..." << std::endl;

    punto2.setY(100.0);

    std::cout << "Valore Y dopo setY(): "
              << punto2.getY() << std::endl;

    std::cout << "Valore atteso: Y = 100.0" << std::endl;

    assert(punto2.getY() == 100.0);

    std::cout << "[OK] Setter Y funzionante." << std::endl;


    // -------------------------------------------------
    // TEST 5: COORDINATE NEGATIVE
    // -------------------------------------------------

    std::cout << "\n[Test 5] Coordinate negative" << std::endl;

    Punto2D punto3(-15.0, -25.0);

    std::cout << "Creato Punto2D(-15.0, -25.0)" << std::endl;
    std::cout << "Valore X ottenuto: " << punto3.getX() << std::endl;
    std::cout << "Valore Y ottenuto: " << punto3.getY() << std::endl;

    std::cout << "Valori attesi: X = -15.0, Y = -25.0" << std::endl;

    assert(punto3.getX() == -15.0);
    assert(punto3.getY() == -25.0);

    std::cout << "[OK] Coordinate negative gestite correttamente."
              << std::endl;


    // -------------------------------------------------
    // TEST 6: OPERATOR == CON PUNTI UGUALI
    // -------------------------------------------------

    std::cout << "\n[Test 6] Confronto tra punti uguali" << std::endl;

    Punto2D puntoUguale1(10.0, 20.0);
    Punto2D puntoUguale2(10.0, 20.0);

    assert(puntoUguale1 == puntoUguale2);

    std::cout << "[OK] Due punti con le stesse coordinate "
                 "vengono riconosciuti come uguali."
              << std::endl;


    // -------------------------------------------------
    // TEST 7: OPERATOR == CON PUNTI DIVERSI
    // -------------------------------------------------

    std::cout << "\n[Test 7] Confronto tra punti diversi" << std::endl;

    Punto2D puntoDiverso1(10.0, 20.0);
    Punto2D puntoDiverso2(11.0, 20.0);

    assert(!(puntoDiverso1 == puntoDiverso2));

    std::cout << "[OK] Due punti con coordinate differenti "
                 "vengono riconosciuti come diversi."
              << std::endl;


    // -------------------------------------------------
    // RISULTATO FINALE
    // -------------------------------------------------

    std::cout << "\n========================================" << std::endl;
    std::cout << "   TUTTI I TEST DI Punto2D SUPERATI!    " << std::endl;
    std::cout << "========================================" << std::endl;
}
