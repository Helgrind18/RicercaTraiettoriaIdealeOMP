//
// Created by helgrind on 02/11/26.
//
#include "ValutatoreTraiettoria.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>
#include <algorithm>

using namespace std;

double ValutatoreTraiettoria::valutaTraiettoria(const Traiettoria &traiettoria, const vector<SezionePista> &sezioni,
                                                const Veicolo &veicolo) const {
    // Questo è il metodo che viene visto dall'esterno
    // Deve occuparsi di coordinare tutti i metodi creati sotto
    //Controllo dell'input, devono essere presenti almeno 3 sezioni
    // Inoltre il numero di posizioni della traiettoria deve coincidere col numero di sezioni
    if (sezioni.size() < 3) {
        throw invalid_argument("Parametri per valutare la traiettoria errati");
    }
    if (static_cast<size_t>(traiettoria.getNumeroPosizioni()) != sezioni.size()) {
        throw invalid_argument("Parametri per valutare la traiettoria errati");
    }

    //Conversione della traiettoria
    vector<Punto2D> punti = traiettoria.calcolaPunti(sezioni); // trasformo le posizioni laterali in coordinate reali

    //Calcolo il profilo di velocità
    vector<double> profiloVelocita = calcolaProfiloVelocita(punti, veicolo);

    //Calcolo il tempo su giro
    double tempoGiro = calcolaTempoGiro(punti, profiloVelocita);

    return tempoGiro;
}

double ValutatoreTraiettoria::calcolaLunghezzaTraiettoria(const vector<Punto2D> &punti) const {
    double lunghezzaTraiettoria = 0.0;
    if (punti.empty()) {
        return lunghezzaTraiettoria;
    }
    if (punti.size() == 1) {
        return lunghezzaTraiettoria;
    }
    for (size_t i = 1; i < punti.size(); i++) {
        const Punto2D &precedente = punti[i - 1];
        const Punto2D &corrente = punti[i];
        lunghezzaTraiettoria += precedente.distanza(corrente);
    }
    const Punto2D &ultimo = punti[punti.size() - 1];
    const Punto2D &primo = punti[0];
    lunghezzaTraiettoria += ultimo.distanza(primo);
    return lunghezzaTraiettoria;
}

double ValutatoreTraiettoria::calcolaCurvatura(const Punto2D &puntoPrecedente, const Punto2D &puntoCorrente,
                                               const Punto2D &puntoSuccessivo) const {
    //Calcolo la distanza fra i tre punti
    double a = puntoPrecedente.distanza(puntoCorrente);
    double b = puntoPrecedente.distanza(puntoSuccessivo);
    double c = puntoCorrente.distanza(puntoSuccessivo);
    const double epsilon = 1e-9;
    if (a < epsilon || b < epsilon || c < epsilon) {
        return 0.0;
    }
    //calcolo l'area del triangolo che si forma
    double prod1 = (puntoCorrente.getX() - puntoPrecedente.getX()) * (puntoSuccessivo.getY() - puntoPrecedente.getY());
    double prod2 = (puntoCorrente.getY() - puntoPrecedente.getY()) * (puntoSuccessivo.getX() - puntoPrecedente.getX());
    double area = abs(prod1 - prod2) / 2.0;
    //calcolo la curvatura
    double curvatura = (4 * area) / (a * b * c);
    return curvatura;
}

vector<double>
ValutatoreTraiettoria::calcolaProfiloVelocita(const vector<Punto2D> &punti, const Veicolo &veicolo) const {
    if (punti.size() < 3) {
        throw invalid_argument("Servono almeno 3 punti per calcolare il profilo di velocita");
    }

    vector<double> profiloVelocita;
    profiloVelocita.reserve(punti.size());
    size_t indicePrecedente = 0;
    size_t indiceSuccessivo = 0;

    // Calcolo la velocità massima in base alla curvatura
    for (size_t i = 0; i < punti.size(); i++) {
        indicePrecedente = (i + punti.size() - 1) % punti.size();
        indiceSuccessivo = (i + 1) % punti.size();
        const Punto2D &precedente = punti[indicePrecedente];
        const Punto2D &corrente = punti[i];
        const Punto2D &successivo = punti[indiceSuccessivo];
        double curvatura = calcolaCurvatura(precedente, corrente, successivo);
        double velocitaMassimaCurvatura = calcolaVelocitaMassimaCurva(curvatura, veicolo);
        profiloVelocita.push_back(velocitaMassimaCurvatura);
    }

    // Accelerazione e frenata fino a convergenza
    const double epsilon = 0.001;
    const int massimoIterazioni = 100;
    bool modificato = true;
    int iterazione = 0;

    while (modificato && iterazione < massimoIterazioni) {
        modificato = false;
        //Gestione accelerazione
        for (size_t i = 0; i < punti.size(); i++) {
            indiceSuccessivo = (i + 1) % punti.size();
            const Punto2D &corrente = punti[i];
            const Punto2D &successivo = punti[indiceSuccessivo];
            double distanza = corrente.distanza(successivo);
            double velocitaCorrente = profiloVelocita[i];
            double velocitaMassimaRaggiungibile = calcolaVelocitaMassimaRaggiungibile(
                velocitaCorrente, veicolo.getAccelerazioneMassima(), distanza);
            double velocitaPuntoSuccessivo = profiloVelocita[indiceSuccessivo];
            if (velocitaPuntoSuccessivo - velocitaMassimaRaggiungibile > epsilon) {
                profiloVelocita[indiceSuccessivo] = velocitaMassimaRaggiungibile;
                modificato = true;
            }
        }

        //Gestione frenata
        for (size_t i = punti.size(); i-- > 0;) {
            indiceSuccessivo = (i + 1) % punti.size();
            const Punto2D &corrente = punti[i];
            const Punto2D &successivo = punti[indiceSuccessivo];
            double distanza = corrente.distanza(successivo);
            double velocitaSuccessiva = profiloVelocita[indiceSuccessivo];
            double velocitaMassimaCorrente = calcolaVelocitaMassimaRaggiungibile(
                velocitaSuccessiva, veicolo.getDecelerazioneMassima(), distanza);
            if (profiloVelocita[i] - velocitaMassimaCorrente > epsilon) {
                profiloVelocita[i] = velocitaMassimaCorrente;
                modificato = true;
            }
        }
        iterazione++;
    }
    return profiloVelocita;
}

double ValutatoreTraiettoria::calcolaVelocitaMassimaRaggiungibile(
    double velocitaIesima,
    const double accelerazione,
    double distanza
) const {
    return sqrt(
        (velocitaIesima * velocitaIesima)
        +
        (
            2.0 *
            distanza *
            accelerazione
        )
    );
}

double ValutatoreTraiettoria::calcolaVelocitaMassimaCurva(double curvatura, const Veicolo &veicolo) const {
    //Controllo che tipo di curva sia, se la curvatura è prossima allo 0, allora, è praticamente da considerare rettilineo
    const double sogliaCurvaturaRettilineo = 1e-4;
    if (curvatura <= sogliaCurvaturaRettilineo) {
        //Allora è un rettilineo, posso andare a velocità massima
        return veicolo.getVelocitaMassima();
    }
    double gripMedio = calcolaGripMedio(veicolo);
    double accelerazioneLateraleMassima = calcolaAccelerazioneLateraleMassima(gripMedio, veicolo);
    double velocitaCurva = calcolaVelocitaCurva(curvatura, accelerazioneLateraleMassima);
    return min(velocitaCurva, veicolo.getVelocitaMassima());
}

double ValutatoreTraiettoria::calcolaAccelerazioneLateraleMassima(double gripMedio, const Veicolo &veicolo) const {
    return gripMedio * veicolo.getAccelerazioneLateraleMassima();
}

double ValutatoreTraiettoria::calcolaVelocitaCurva(double curvatura, double accelerazioneLateraleMassima) const {
    return sqrt(accelerazioneLateraleMassima / curvatura);
}

double ValutatoreTraiettoria::calcolaGripMedio(const Veicolo &veicolo) const {
    // Bisogna calcolare il grip medio delle 4 gomme del veicolo
    double gripMedio = 0.0;
    for (size_t i = 0; i < veicolo.getGomme().size(); i++) {
        gripMedio += veicolo.getGomme()[i].getCoefficienteGrip();
    }
    return gripMedio / veicolo.getGomme().size();
}

double ValutatoreTraiettoria::calcolaTempoSegmento(double lunghezzaSegmento, double velocitaIniziale,
                                                   double velocitaFinale) const {
    //Utilizziamo l'ipotesi di accelerazione costante lungo il segmento
    //Controllo i parametri
    if (velocitaIniziale < 0.0 || velocitaFinale < 0.0) {
        throw invalid_argument("Le velocita non possono essere negative");
    }
    if (lunghezzaSegmento < 0.0) {
        throw invalid_argument("La lunghezza del segmento non puo essere negativa");
    }
    double tempoSegmento = 0.0;
    if (lunghezzaSegmento == 0.0) {
        return tempoSegmento;
    }
    //Adesso calcolo la velocità media
    double vMedia = (velocitaFinale + velocitaIniziale) / 2.0;
    // Evito la divisione per zero
    if (vMedia == 0.0) {
        throw invalid_argument("Velocita media nulla");
    }
    tempoSegmento = lunghezzaSegmento / vMedia;
    return tempoSegmento;
}

double ValutatoreTraiettoria::calcolaTempoGiro(const vector<Punto2D> &punti,
                                               const vector<double> &profiloVelocita) const {
    if (profiloVelocita.size() != punti.size()) {
        throw invalid_argument("Velocità e punti non hanno la stessa dimensione");
    }
    if (profiloVelocita.size() < 2) {
        throw invalid_argument("Devono essere presenti almeno due punti");
    }
    double tempoTotale = 0.0;
    size_t indiceSuccessivo = 0;
    double distanza = 0.0;
    double velocitaCorrente = 0.0;
    double velocitaSuccessiva = 0.0;
    for (size_t i = 0; i < punti.size(); i++) {
        const Punto2D &corrente = punti[i];
        indiceSuccessivo = (i + 1) % punti.size();
        const Punto2D &successivo = punti[indiceSuccessivo];
        distanza = corrente.distanza(successivo);
        //Recupero delle velocità
        velocitaCorrente = profiloVelocita[i];
        velocitaSuccessiva = profiloVelocita[indiceSuccessivo];
        //Calcolo il tempo sul segmento
        tempoTotale += calcolaTempoSegmento(distanza, velocitaCorrente, velocitaSuccessiva);
    }
    return tempoTotale;
}
