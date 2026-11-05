#ifndef RICERCATRAIETTORIAIDEALEOMP_VALUTATORETRAIETTORIA_HPP
#define RICERCATRAIETTORIAIDEALEOMP_VALUTATORETRAIETTORIA_HPP

#include "Traiettoria.hpp"
#include "SezionePista.hpp"
#include "Veicolo.hpp"
#include "Punto2D.hpp"

#include <vector>
using namespace std;
// Si occupa di valutare una Traiettoria percorsa da un determinato
// Veicolo sulla pista discretizzata.
//
// La valutazione ha come obiettivo principale il calcolo del
// tempo sul giro stimato.
//
// Per effettuare la valutazione vengono considerati:
//
// - lunghezza della traiettoria
// - curvatura della traiettoria
// - grip delle gomme
// - accelerazione massima del veicolo
// - decelerazione massima del veicolo
// - potenza massima
// - resistenza aerodinamica
// - area frontale del veicolo
//
// Il ValutatoreTraiettoria non modifica né la traiettoria,
// né il veicolo, né le sezioni della pista.
class ValutatoreTraiettoria {
public:

    // Calcola il tempo sul giro stimato della traiettoria.
    //
    // La traiettoria deve contenere una posizione laterale
    // per ogni SezionePista.
    //
    // Restituisce il tempo sul giro in secondi.
    double valutaTraiettoria(const Traiettoria& traiettoria,const vector<SezionePista>& sezioni,const Veicolo& veicolo) const;

private:
    // Calcola la lunghezza totale della traiettoria
    // sommando la distanza tra tutti i punti consecutivi.
    double calcolaLunghezzaTraiettoria(const vector<Punto2D>& punti) const;

    // Calcola la curvatura locale della traiettoria
    // utilizzando tre punti consecutivi:
    //
    // puntoPrecedente -> puntoCorrente -> puntoSuccessivo
    //
    // Una curvatura maggiore indica una curva più stretta.
    double calcolaCurvatura(const Punto2D& puntoPrecedente,const Punto2D& puntoCorrente,const Punto2D& puntoSuccessivo) const;


    // Calcola il profilo di velocità della macchina lungo
    // tutti i punti della traiettoria.
    //
    // Il vector restituito contiene una velocità per ogni
    // punto della traiettoria.
    vector<double> calcolaProfiloVelocita(const vector<Punto2D>& punti, const Veicolo& veicolo) const;

    // Calcola la velocità massima consentita in un punto
    // della traiettoria in base alla curvatura e al grip
    // disponibile del veicolo.
    double calcolaVelocitaMassimaCurva(double curvatura, const Veicolo& veicolo) const;

    double calcolaVelocitaMassimaRaggiungibile(double velocitaIesima, const double accelerazione, double distanza) const;

    // Calcola la forza motrice disponibile alla velocita corrente.
    // La forza e limitata sia dall'accelerazione massima sia
    // dalla potenza massima del veicolo.
    double calcolaForzaTrazione(double velocita, const Veicolo& veicolo) const;

    // Calcola la resistenza aerodinamica:
    // F_drag = 1/2 * rho * Cd * A * v^2.
    double calcolaForzaResistenzaAerodinamica(double velocita, const Veicolo& veicolo) const;

    // Calcola l'accelerazione longitudinale realmente disponibile
    // tenendo conto della forza motrice e della resistenza dell'aria.
    double calcolaAccelerazioneEffettiva(double velocita, const Veicolo& veicolo) const;
    // Calcola il tempo necessario per percorrere un singolo
    // segmento della traiettoria conoscendo:
    //
    // - la lunghezza del segmento
    // - la velocità all'inizio
    // - la velocità alla fine
    double calcolaTempoSegmento(double lunghezzaSegmento, double velocitaIniziale,double velocitaFinale) const;

    // Calcola il tempo totale necessario per percorrere
    // la traiettoria utilizzando il profilo di velocità
    // precedentemente calcolato.
    double calcolaTempoGiro(const vector<Punto2D>& punti, const vector<double>& profiloVelocita) const;


    // Calcola un coefficiente di grip rappresentativo
    // del veicolo utilizzando le quattro gomme.
    double calcolaGripMedio(const Veicolo& veicolo) const;

    double calcolaAccelerazioneLateraleMassima(double gripMedio, const Veicolo& veicolo) const;

    double calcolaVelocitaCurva(double curvatura, double accelerazioneLateraleMassima) const;
};

#endif