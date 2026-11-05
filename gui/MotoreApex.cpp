#include "MotoreApex.hpp"

#include "AlgoritmoGenetico.hpp"
#include "GeneraTraiettorie.hpp"  // Nel tuo progetto questa intestazione contiene GeneratoreTraiettorie.
#include "ValutatoreTraiettoria.hpp"
#include "SezionePista.hpp"
#include "Traiettoria.hpp"
#include "Veicolo.hpp"
#include "Gomma.hpp"
#include "Punto2D.hpp"

#include <QFile>
#include <QTextStream>
#include <QStringList>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace {
constexpr double eps = 1e-8;

double distanza(const QPointF &a, const QPointF &b) {
    return std::hypot(a.x() - b.x(), a.y() - b.y());
}

QVector<QPointF> ricampiona(const QVector<QPointF> &input, int n) {
    if (input.size() < 3 || n < 3)
        throw std::invalid_argument("La pista deve avere almeno tre punti e tre sezioni");
    const int m = static_cast<int>(input.size());
    QVector<double> cumulati(m + 1, 0.0);
    for (int i = 0; i < m; ++i)
        cumulati[i + 1] = cumulati[i] + distanza(input[i], input[(i + 1) % m]);
    const double totale = cumulati.back();
    if (!(totale > eps)) throw std::invalid_argument("Pista degenerata (lunghezza nulla)");

    QVector<QPointF> result;
    result.reserve(n);
    int j = 0;
    for (int i = 0; i < n; ++i) {
        const double s = totale * static_cast<double>(i) / n;
        while (j + 1 < m && cumulati[j + 1] < s) ++j;
        const double lung = cumulati[j + 1] - cumulati[j];
        const double t = lung > eps ? (s - cumulati[j]) / lung : 0.0;
        result.push_back(input[j] * (1.0 - t) + input[(j + 1) % m] * t);
    }
    return result;
}

std::vector<SezionePista> creaSezioni(const GeometriaApex &g) {
    std::vector<SezionePista> out;
    out.reserve(static_cast<size_t>(g.centro.size()));
    for (qsizetype i = 0; i < g.centro.size(); ++i) {
        const auto &s = g.sinistro[i];
        const auto &d = g.destro[i];
        out.emplace_back(Punto2D(s.x(), s.y()), Punto2D(d.x(), d.y()));
    }
    return out;
}

QVector<QPointF> proietta(const Traiettoria &traiettoria, const GeometriaApex &g) {
    if (traiettoria.getNumeroPosizioni() != static_cast<int>(g.centro.size()))
        throw std::runtime_error("Numero geni incompatibile con il circuito");
    QVector<QPointF> punti;
    punti.reserve(g.centro.size());
    for (int i = 0; i < traiettoria.getNumeroPosizioni(); ++i) {
        const double t = traiettoria.getPosizioneLaterale(i);
        punti.push_back(g.sinistro[i] * (1.0 - t) + g.destro[i] * t);
    }
    return punti;
}
QVector<double> estraiGeni(const Traiettoria &t) {
    QVector<double> g;
    g.reserve(t.getNumeroPosizioni());
    for (int i = 0; i < t.getNumeroPosizioni(); ++i)
        g.push_back(t.getPosizioneLaterale(i));
    return g;
}

// Si parte dal PROFILO VELOCITA originale del ValutatoreTraiettoria.
// Le grandezze derivate sono cinematiche stimate tra due sezioni, non
// comandi di acceleratore/freno ne' misure di telemetria reale.
void profiliPerGrafici(const QVector<QPointF>& punti, const std::vector<double>& velocita,
                      const std::vector<double>& curvatura,
                      QVector<double>& distanzaKm, QVector<double>& velocitaKmh,
                      QVector<double>& accelerazione, QVector<double>& frenata,
                      QVector<double>& laterale, QVector<double>& curvaturaPerKm) {
    const int n = static_cast<int>(punti.size());
    if (n < 3 || static_cast<int>(velocita.size()) != n ||
        static_cast<int>(curvatura.size()) != n)
        throw std::runtime_error("Velocita, curvatura e sezioni non corrispondenti");
    distanzaKm.resize(n); velocitaKmh.resize(n);
    accelerazione.resize(n); frenata.resize(n); laterale.resize(n);
    curvaturaPerKm.resize(n);
    double cumulata = 0.0;
    for (int i = 0; i < n; ++i) {
        const int succ = (i + 1) % n;
        const double ds = distanza(punti[i], punti[succ]);
        distanzaKm[i] = cumulata / 1000.0;
        velocitaKmh[i] = velocita[i] * 3.6;
        const double a = ds > 1e-8 ?
            (velocita[succ]*velocita[succ] - velocita[i]*velocita[i]) / (2.0 * ds) : 0.0;
        accelerazione[i] = std::max(0.0, a);
        frenata[i] = std::max(0.0, -a);
        // Curvatura prodotta dal ValutatoreTraiettoria: stessa formula,
        // nessun duplicato nel frontend. 1/m -> 1/km solo per il grafico.
        curvaturaPerKm[i] = 1000.0 * curvatura[i];
        laterale[i] = velocita[i]*velocita[i]*curvatura[i];
        cumulata += ds;
    }
}
}  // namespace

QVector<QPointF> caricaCentroPista(const QString &percorso) {
    QFile file(percorso);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error(("Non riesco ad aprire la pista: " + percorso).toStdString());
    QTextStream stream(&file);
    QVector<QPointF> points;
    bool intestazione = true;
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (intestazione) { intestazione = false; continue; }
        line.replace(';', ',');
        const QStringList columns = line.split(',');
        if (columns.size() < 2) throw std::runtime_error("CSV della pista non valido");
        bool okX = false, okY = false;
        const double x = columns[0].toDouble(&okX), y = columns[1].toDouble(&okY);
        if (!okX || !okY || !std::isfinite(x) || !std::isfinite(y))
            throw std::runtime_error("Coordinate del circuito non valide");
        const QPointF p(x, y);
        if (points.isEmpty() || distanza(points.back(), p) > 1e-4)
            points.push_back(p);
    }
    if (points.size() > 3 && distanza(points.front(), points.back()) < 0.5)
        points.pop_back();
    if (points.size() < 3) throw std::runtime_error("Il file pista non contiene abbastanza punti");
    return points;
}

GeometriaApex creaGeometriaPista(const QVector<QPointF> &input, int n, double larghezza) {
    GeometriaApex geo;
    geo.centro = ricampiona(input, n);
    geo.sinistro.reserve(n);
    geo.destro.reserve(n);
    for (int i = 0; i < n; ++i) {
        const QPointF a = geo.centro[(i + n - 1) % n];
        const QPointF b = geo.centro[(i + 1) % n];
        const QPointF tangent = b - a;
        const double norm = std::hypot(tangent.x(), tangent.y());
        if (norm < eps) throw std::runtime_error("Tangente nulla nella geometria della pista");
        const QPointF normale(-tangent.y() / norm, tangent.x() / norm);
        geo.sinistro.push_back(geo.centro[i] + normale * (larghezza * 0.5));
        geo.destro.push_back(geo.centro[i] - normale * (larghezza * 0.5));
    }
    return geo;
}

RisultatoApex eseguiOttimizzazioneApex(const ParametriApex &p, const QString &fileCsv,
        const std::function<void(int, int, double)> &progresso) {
    if (p.sezioni < 3 || p.popolazione < 2 || p.generazioni < 1 ||
        p.torneo < 2 || p.torneo > p.popolazione || p.elite < 0 || p.elite >= p.popolazione)
        throw std::invalid_argument("Configurazione dell'algoritmo non valida");
    if (!(p.grip > 0 && p.grip <= 1))
        throw std::invalid_argument("Il grip deve essere nell'intervallo (0,1]");

    RisultatoApex risultato;
    risultato.parametri = p;
    risultato.geometria = creaGeometriaPista(caricaCentroPista(fileCsv), p.sezioni);
    const auto sezioni = creaSezioni(risultato.geometria);

    const std::array<Gomma, 4> gomme = {
        Gomma(0, TipoGomma::SOFT, p.grip), Gomma(1, TipoGomma::SOFT, p.grip),
        Gomma(2, TipoGomma::SOFT, p.grip), Gomma(3, TipoGomma::SOFT, p.grip)
    };
    const Veicolo veicolo(p.massaKg, p.potenzaKW * 1000.0, p.accelerazione,
                          p.frenata, p.cd, p.areaM2, p.velocitaKmh / 3.6,
                          p.accelerazioneLaterale, gomme);
    ValutatoreTraiettoria valutatore;

    // Il primo estratto di mt19937 e' proprio il seed usato da ottimizza()
    // per generare la popolazione iniziale. Si ricrea qui la stessa sequenza.
    std::mt19937 sorgente(p.seed);
    GeneratoreTraiettorie inizializzatore(sorgente());
    const auto popolazioneIniziale = inizializzatore.generaPopolazione(p.popolazione, p.sezioni);
    double miglioreIniziale = std::numeric_limits<double>::infinity();
    Traiettoria traiettoriaIniziale;
    for (const auto &t : popolazioneIniziale) {
        const double tempo = valutatore.valutaTraiettoria(t, sezioni, veicolo);
        if (tempo < miglioreIniziale) {
            miglioreIniziale = tempo;
            traiettoriaIniziale = t;
        }
    }

    AlgoritmoGenetico algoritmo(p.popolazione, p.generazioni, p.crossover,
                               p.mutazione, p.intensita, p.torneo, p.elite, p.seed);
    const auto start = std::chrono::steady_clock::now();
    const Traiettoria ottima = algoritmo.ottimizza(sezioni, veicolo,
        [&progresso, &start](int completate, int totale) {
            if (!progresso) return;
            const double secondi = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - start).count();
            progresso(completate, totale, secondi);
        });
    const auto stop = std::chrono::steady_clock::now();

    risultato.tempoIniziale = miglioreIniziale;
    risultato.tempoFinale = valutatore.valutaTraiettoria(ottima, sezioni, veicolo);
    risultato.durataMs = std::chrono::duration<double, std::milli>(stop - start).count();
    risultato.iniziale = proietta(traiettoriaIniziale, risultato.geometria);
    risultato.ottimizzata = proietta(ottima, risultato.geometria);
    risultato.geniIniziali = estraiGeni(traiettoriaIniziale);
    risultato.geniOttimizzati = estraiGeni(ottima);
    // Richiediamo gli stessi profili di velocita che usa il valutatore del giro.
    const auto vI = valutatore.profiloVelocitaPerGrafici(traiettoriaIniziale, sezioni, veicolo);
    const auto vO = valutatore.profiloVelocitaPerGrafici(ottima, sezioni, veicolo);
    const auto kI = valutatore.profiloCurvaturaPerGrafici(traiettoriaIniziale, sezioni);
    const auto kO = valutatore.profiloCurvaturaPerGrafici(ottima, sezioni);
    profiliPerGrafici(risultato.iniziale, vI, kI, risultato.distanzaInizialeKm,
                     risultato.velocitaInizialeKmh, risultato.accelerazioneIniziale,
                     risultato.frenataIniziale, risultato.lateraleIniziale,
                     risultato.curvaturaInizialePerKm);
    profiliPerGrafici(risultato.ottimizzata, vO, kO, risultato.distanzaOttimizzataKm,
                     risultato.velocitaOttimizzataKmh, risultato.accelerazioneOttimizzata,
                     risultato.frenataOttimizzata, risultato.lateraleOttimizzata,
                     risultato.curvaturaOttimizzataPerKm);
    if (!std::isfinite(risultato.tempoFinale) || !std::isfinite(risultato.tempoIniziale))
        throw std::runtime_error("Il valutatore ha restituito un tempo non finito");
    return risultato;
}
