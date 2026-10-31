#include "TestPista.hpp"
#include "Pista.hpp"

#include <cassert>
#include <iostream>

void eseguiTestPista() {

    std::cout << "\n========================================" << std::endl;
    std::cout << "         TEST DELLA CLASSE Pista        " << std::endl;
    std::cout << "========================================" << std::endl;


    // -------------------------------------------------
    // TEST 1: PISTA VUOTA
    // -------------------------------------------------

    std::cout << "\n[Test 1] Costruttore vuoto" << std::endl;

    Pista pista;

    std::cout << "Punti bordo sinistro: "
              << pista.getNumeroPuntiBordoSinistro() << std::endl;

    std::cout << "Punti bordo destro: "
              << pista.getNumeroPuntiBordoDestro() << std::endl;

    std::cout << "Punti totali: "
              << pista.getNumeroPuntiTotali() << std::endl;

    assert(pista.getBordoSinistro().empty());
    assert(pista.getBordoDestro().empty());
    assert(pista.getNumeroPuntiBordoSinistro() == 0);
    assert(pista.getNumeroPuntiBordoDestro() == 0);
    assert(pista.getNumeroPuntiTotali() == 0);

    std::cout << "[OK] Pista inizializzata correttamente vuota."
              << std::endl;


    // -------------------------------------------------
    // TEST 2: BORDO SINISTRO
    // -------------------------------------------------

    std::cout << "\n[Test 2] Aggiunta punti al bordo sinistro"
              << std::endl;

    Punto2D s1(0.0, 10.0);
    Punto2D s2(10.0, 10.0);
    Punto2D s3(20.0, 10.0);

    pista.aggiungiPuntoBordoSinistro(s1);
    pista.aggiungiPuntoBordoSinistro(s2);
    pista.aggiungiPuntoBordoSinistro(s3);

    std::cout << "Punti bordo sinistro: "
              << pista.getNumeroPuntiBordoSinistro()
              << std::endl;

    std::cout << "Valore atteso: 3" << std::endl;

    assert(pista.getNumeroPuntiBordoSinistro() == 3);

    std::cout << "[OK] Punti aggiunti correttamente al bordo sinistro."
              << std::endl;


    // -------------------------------------------------
    // TEST 3: BORDO DESTRO
    // -------------------------------------------------

    std::cout << "\n[Test 3] Aggiunta punti al bordo destro"
              << std::endl;

    Punto2D d1(0.0, 0.0);
    Punto2D d2(20.0, 0.0);

    pista.aggiungiPuntoBordoDestro(d1);
    pista.aggiungiPuntoBordoDestro(d2);

    std::cout << "Punti bordo destro: "
              << pista.getNumeroPuntiBordoDestro()
              << std::endl;

    std::cout << "Valore atteso: 2" << std::endl;

    assert(pista.getNumeroPuntiBordoDestro() == 2);

    std::cout << "[OK] Punti aggiunti correttamente al bordo destro."
              << std::endl;


    // -------------------------------------------------
    // TEST 4: BORDI INDIPENDENTI
    // -------------------------------------------------

    std::cout << "\n[Test 4] Bordi indipendenti" << std::endl;

    std::cout << "Bordo sinistro: "
              << pista.getNumeroPuntiBordoSinistro()
              << std::endl;

    std::cout << "Bordo destro: "
              << pista.getNumeroPuntiBordoDestro()
              << std::endl;

    assert(pista.getNumeroPuntiBordoSinistro() == 3);
    assert(pista.getNumeroPuntiBordoDestro() == 2);

    std::cout << "[OK] I bordi possono avere dimensioni differenti."
              << std::endl;


    // -------------------------------------------------
    // TEST 5: NUMERO TOTALE
    // -------------------------------------------------

    std::cout << "\n[Test 5] Numero totale dei punti" << std::endl;

    std::cout << "Numero totale ottenuto: "
              << pista.getNumeroPuntiTotali()
              << std::endl;

    std::cout << "Numero totale atteso: 5" << std::endl;

    assert(pista.getNumeroPuntiTotali() == 5);

    std::cout << "[OK] Conteggio totale corretto."
              << std::endl;


    // -------------------------------------------------
    // TEST 6: ORDINE DI INSERIMENTO
    // -------------------------------------------------

    std::cout << "\n[Test 6] Ordine dei punti" << std::endl;

    assert(pista.getBordoSinistro()[0] == s1);
    assert(pista.getBordoSinistro()[1] == s2);
    assert(pista.getBordoSinistro()[2] == s3);

    assert(pista.getBordoDestro()[0] == d1);
    assert(pista.getBordoDestro()[1] == d2);

    std::cout << "[OK] L'ordine dei punti viene preservato."
              << std::endl;


    // -------------------------------------------------
    // TEST 7: PUNTO RIPETUTO
    // -------------------------------------------------

    std::cout << "\n[Test 7] Punto ripetuto" << std::endl;

    int prima = pista.getNumeroPuntiBordoSinistro();

    std::cout << "Dimensione prima dell'inserimento: "
              << prima << std::endl;

    std::cout << "Aggiungo nuovamente il punto (0,10)..."
              << std::endl;

    pista.aggiungiPuntoBordoSinistro(s1);

    int dopo = pista.getNumeroPuntiBordoSinistro();

    std::cout << "Dimensione dopo l'inserimento: "
              << dopo << std::endl;

    std::cout << "Valore atteso: " << prima + 1 << std::endl;

    assert(dopo == prima + 1);

    std::cout << "[OK] La Pista non vieta automaticamente "
                 "la ripetizione di un punto."
              << std::endl;


    // -------------------------------------------------
    // RISULTATO FINALE
    // -------------------------------------------------

    std::cout << "\n========================================" << std::endl;
    std::cout << "     TUTTI I TEST DI Pista SUPERATI!    " << std::endl;
    std::cout << "========================================" << std::endl;
}
