#include "VistaPista.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPolygonF>
#include <QFont>
#include <QRectF>
#include <QPen>
#include <QColor>
#include <algorithm>
#include <cmath>
#include <functional>

namespace {
const QColor ColoreIniziale("#FFB062");
const QColor ColoreOttimizzata("#25E7DE");

// Il motore genera gia' una traiettoria campionata ad alta risoluzione.
// Qt la disegna direttamente, senza modificare la geometria valutata:
// antialiasing e giunzioni tonde migliorano l'aspetto visivo.
QPainterPath formaChiusa(const QVector<QPointF>& punti,
                         const std::function<QPointF(const QPointF&)>& converti) {
    QPainterPath path;
    if (punti.isEmpty()) return path;
    path.moveTo(converti(punti.front()));
    for (qsizetype i=1; i<punti.size(); ++i) path.lineTo(converti(punti[i]));
    if (punti.size()>2) path.closeSubpath();
    return path;
}

void traccia(QPainter &p, const QPainterPath &path, const QColor &colore, bool tratteggiata) {
    if (path.isEmpty()) return;
    p.setBrush(Qt::NoBrush);
    // Contorno scuro per separare la traiettoria dal bordo della pista e
    // dalle altre linee, soprattutto quando le traiettorie coincidono.
    p.setPen(QPen(QColor(5, 12, 22, 200), 2.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPath(path);
    QPen pen(colore, 1.7, tratteggiata ? Qt::DashLine : Qt::SolidLine,
             Qt::RoundCap, Qt::RoundJoin);
    if (tratteggiata) pen.setDashPattern({5.0, 3.0});
    p.setPen(pen);
    p.drawPath(path);
}

} // namespace

VistaPista::VistaPista(QWidget *parent) : QWidget(parent) {
    setMinimumSize(460, 330);
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
    timer_.setInterval(25);
    connect(&timer_, &QTimer::timeout, this, [this] {
        const auto &percorso = ottimizzata_.isEmpty() ? geometria_.centro : ottimizzata_;
        if (!percorso.isEmpty()) {
            avanzamento_ = std::fmod(avanzamento_ + 0.19 * velocitaAnimazione_, double(percorso.size()));
            update();
        }
    });
}

void VistaPista::mostraGeometria(const GeometriaApex &geo) {
    geometria_ = geo;
    iniziale_.clear();
    ottimizzata_.clear();
    tempoIniziale_ = tempoFinale_ = 0;
    avanzamento_ = 0;
    resetVista();
}
void VistaPista::mostraRisultato(const RisultatoApex &r) {
    geometria_ = r.geometria;
    iniziale_ = r.iniziale;
    ottimizzata_ = r.ottimizzata;
    tempoIniziale_ = r.tempoIniziale;
    tempoFinale_ = r.tempoFinale;
    avanzamento_ = 0;
    resetVista();
}
void VistaPista::impostaVisibilita(bool iniziale, bool ottimizzata) {
    mostraIniziale_ = iniziale;
    mostraOttimizzata_ = ottimizzata;
    update();
}
void VistaPista::impostaModalitaConfronto(ModalitaConfronto modalita) {
    modalita_ = modalita;
    resetVista();
}
void VistaPista::impostaAmplificazione(double fattore) {
    // Soglia di leggibilita' del solo asfalto: 0 reale, 1 sottile, 2 comfort.
    // La traiettoria usa sempre le sue coordinate metriche originali.
    amplificazione_ = std::clamp(fattore, 0.0, 2.0);
    update();
}
void VistaPista::avviaAnimazione(bool avvia) {
    if (avvia) timer_.start(); else timer_.stop();
    update();
}
void VistaPista::resetVista() {
    zoom_ = 1.0;
    spostamento_ = QPointF(0, 0);
    update();
}
void VistaPista::impostaVelocitaAnimazione(double velocita) {
    velocitaAnimazione_ = velocita;
}

// Restituisce l'area effettiva in cui la pista viene disegnata.
// Serve ad ancorare lo zoom esattamente al punto indicato dal mouse.
QRectF VistaPista::areaDisegno(const QPointF &posizione) const {
    const QRectF intera(5, 5, width()-10, height()-10);
    QRectF pannello = intera;
    if (modalita_ == ModalitaConfronto::Affiancato) {
        const double gap = 6.0;
        const double larghezza = (intera.width()-gap)/2.0;
        const QRectF sx(intera.left(), intera.top(), larghezza, intera.height());
        const QRectF dx(intera.left()+larghezza+gap, intera.top(), larghezza, intera.height());
        pannello = (posizione.x() >= dx.left()) ? dx : sx;
    }
    return pannello.adjusted(8, 60, -8, -36);
}

void VistaPista::applicaZoom(double fattore, const QPointF &punto) {
    if (fattore <= 0.0) return;
    const double nuovo = std::clamp(zoom_ * fattore, 0.5, 100.0);
    const double rapporto = nuovo / zoom_;
    const QPointF delta = punto - areaDisegno(punto).center();
    // Non muovere il punto del circuito che si trova sotto al cursore.
    // Le due viste condividono zoom e pan, quindi restano sincronizzate.
    spostamento_ = spostamento_ * rapporto + delta * (1.0 - rapporto);
    zoom_ = nuovo;
    update();
}

void VistaPista::modificaZoom(double fattore) {
    applicaZoom(fattore, areaDisegno(rect().center()).center());
}

void VistaPista::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor("#0A111E"));
    p.setPen(QPen(QColor(32, 46, 68, 70), 1));
    for (int x = 0; x < width(); x += 30) p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 30) p.drawLine(0, y, width(), y);

    if (geometria_.centro.size() < 3) {
        p.setPen(QColor("#8FA6C3"));
        p.drawText(rect(), Qt::AlignCenter, "SELEZIONA UN CIRCUITO");
        return;
    }

    double minX = geometria_.centro[0].x(), maxX = minX;
    double minY = geometria_.centro[0].y(), maxY = minY;
    for (const auto &pt : geometria_.centro) {
        minX = std::min(minX, pt.x()); maxX = std::max(maxX, pt.x());
        minY = std::min(minY, pt.y()); maxY = std::max(maxY, pt.y());
    }
    const QPointF centro((minX + maxX) * .5, (minY + maxY) * .5);
    const double spazioX = std::max(1., maxX-minX);
    const double spazioY = std::max(1., maxY-minY);

    // Distanze REALI tra le due traiettorie (metri nel CSV), indipendenti
    // dall'amplificazione impostata per la visualizzazione.
    double mediaMetri = 0.0, massimoMetri = 0.0;
    const auto nConfrontabili = std::min(iniziale_.size(), ottimizzata_.size());
    for (qsizetype i = 0; i < nConfrontabili; ++i) {
        const double dist = std::hypot(iniziale_[i].x() - ottimizzata_[i].x(),
                                       iniziale_[i].y() - ottimizzata_[i].y());
        mediaMetri += dist;
        if (dist > massimoMetri) massimoMetri = dist;
    }
    if (nConfrontabili > 0) mediaMetri /= double(nConfrontabili);

    // Disegna un pannello per ogni traiettoria, oppure entrambe nello stesso.
    // Tutti i pannelli usano la stessa scala per rendere il confronto onesto.
    const auto disegnaVista = [&](const QRectF &r, bool disegnaIniziale, bool disegnaOttimizzata,
                                 const QString &titolo, const QColor &colore, double tempo, bool doppia) {
        p.save();
        p.setPen(QPen(QColor("#284056"), 1.2));
        p.setBrush(QColor(13, 25, 40, 135));
        p.drawRoundedRect(r, 11, 11);

        // Etichetta ancorata al pannello, non soggetta a zoom.
        QFont f = p.font();
        f.setPixelSize(12);
        f.setBold(true);
        p.setFont(f);
        p.setPen(colore);
        p.drawText(QRectF(r.left()+17, r.top()+13, r.width()-34, 18),
                   Qt::AlignLeft | Qt::AlignVCenter, titolo);
        if (tempo > 0) {
            f.setPixelSize(17);
            p.setFont(f);
            p.setPen(QColor("#EEF6FF"));
            p.drawText(QRectF(r.left()+17, r.top()+33, r.width()-34, 23),
                       Qt::AlignLeft | Qt::AlignVCenter, QString::number(tempo, 'f', 3) + " s");
        }

        const QRectF area = r.adjusted(8, 60, -8, -36);
        if (area.width() < 40 || area.height() < 40) { p.restore(); return; }
        p.setClipRect(area);
        // Un piccolo margine evita che l'asfalto, piu' largo del centro pista,
        // venga tagliato ai bordi della vista quando non e' attivo lo zoom.
        const double scala = std::max(.001, std::min((area.width()-58.) / spazioX,
                                                    (area.height()-58.) / spazioY)) * zoom_;
        const auto conv = [&](const QPointF &v) -> QPointF {
            return QPointF(area.center().x() + (v.x()-centro.x())*scala + spostamento_.x(),
                           area.center().y() - (v.y()-centro.y())*scala + spostamento_.y());
        };
        // Il problema del vecchio renderer era il poligono ottenuto unendo
        // due bordi spostati indipendentemente: alle curve strette i contorni
        // si intersecavano, creando triangoli e strisce che tagliavano la pista.
        //
        // SOLUZIONE: una sola fascia di asfalto continua, costruita tramite
        // lo stroke arrotondato della linea centrale. Non ci sono poligoni
        // tra bordi a sezioni consecutive, quindi niente triangolazioni spurie.
        const QPainterPath centroPath = formaChiusa(geometria_.centro, conv);
        double larghezzaMediaMetri = 0.0;
        const qsizetype nBordi = std::min({geometria_.centro.size(),
                                          geometria_.sinistro.size(),
                                          geometria_.destro.size()});
        for (qsizetype i = 0; i < nBordi; ++i) {
            const QPointF d = geometria_.destro[i] - geometria_.sinistro[i];
            larghezzaMediaMetri += std::hypot(d.x(), d.y());
        }
        if (nBordi > 0) larghezzaMediaMetri /= double(nBordi);

        // Nella panoramica i 12 m fisici possono occupare meno di 2 pixel.
        // Un fondino sottile rende leggibile l'asfalto, senza dilatare la
        // traiettoria. Nessuna soglia di 28 px e nessuna rimappatura laterale.
        // Con lo zoom torna automaticamente prevalente la larghezza metrica.
        const double minimoPixel = amplificazione_ < 0.5 ? 0.0 :
                                   (amplificazione_ < 1.5 ? 6.0 : 10.0);
        const double larghezzaAsfalto = std::max(minimoPixel, larghezzaMediaMetri * scala);

        QPainterPathStroker bordoStroker;
        bordoStroker.setWidth(larghezzaAsfalto + 1.4);
        bordoStroker.setCapStyle(Qt::RoundCap);
        bordoStroker.setJoinStyle(Qt::RoundJoin);
        const QPainterPath contornoAsfalto = bordoStroker.createStroke(centroPath);

        QPainterPathStroker asfaltoStroker;
        asfaltoStroker.setWidth(larghezzaAsfalto);
        asfaltoStroker.setCapStyle(Qt::RoundCap);
        asfaltoStroker.setJoinStyle(Qt::RoundJoin);
        const QPainterPath asfalto = asfaltoStroker.createStroke(centroPath);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#7893A3"));
        p.drawPath(contornoAsfalto);
        p.setBrush(QColor("#2B3F4D"));
        p.drawPath(asfalto);

        // Stessa trasformazione affine per pista e traiettorie, a ogni zoom.
        // Niente normalizzazione, clamp o amplificazione dei geni: anche le
        // differenze piccole rimangono proporzionate alla geometria reale.
        const auto &iP = iniziale_;
        const auto &oP = ottimizzata_;
        const QPainterPath inizialePath = formaChiusa(iP, conv);
        const QPainterPath ottimizzataPath = formaChiusa(oP, conv);

        const bool okI = disegnaIniziale && iP.size() > 1;
        const bool okO = disegnaOttimizzata && oP.size() > 1;

        // Non tagliare i raccordi sulla sagoma: un eventuale scostamento
        // deve restare visibile, non diventare una linea interrotta.
        // In overlay, segmenti grigi collegano le posizioni iniziale/finale
        // per una selezione di sezioni, rendendo visibile lo spostamento.
        if (doppia && okI && okO) {
            QPen sep(QColor(202, 221, 236, 95), 1.0, Qt::DashLine);
            p.setPen(sep);
            const qsizetype passo = std::max(qsizetype(1), nConfrontabili / 16);
            for (qsizetype j = 0; j < nConfrontabili; j += passo)
                p.drawLine(conv(iP[j]), conv(oP[j]));
        }
        if (okI) {
            traccia(p, inizialePath, ColoreIniziale, doppia);
        }
        if (okO) {
            traccia(p, ottimizzataPath, ColoreOttimizzata, false);
        }

        if (!okI && !okO) {
            p.setPen(QColor("#829DB6"));
            p.drawText(area, Qt::AlignCenter,
                       iniziale_.isEmpty() && ottimizzata_.isEmpty()
                       ? "PREMI AVVIA OTTIMIZZAZIONE"
                       : "TRAIETTORIA NASCOSTA: RIATTIVA IL FILTRO");
        }

        // Linea di partenza trasversale alla fascia visibile, non ai bordi
        // originali che nella vista completa sarebbero larghi < 2 pixel.
        if (!geometria_.centro.isEmpty() && !geometria_.sinistro.isEmpty()
            && !geometria_.destro.isEmpty()) {
            const QPointF direzione = conv(geometria_.destro.front())
                                    - conv(geometria_.sinistro.front());
            const double lunghezza = std::hypot(direzione.x(), direzione.y());
            if (lunghezza > 1e-6) {
                const QPointF normale = direzione / lunghezza;
                const QPointF punto = conv(geometria_.centro.front());
                p.setPen(QPen(QColor("#EAF6FF"), 2.0));
                p.drawLine(punto - normale * (larghezzaAsfalto * .48),
                           punto + normale * (larghezzaAsfalto * .48));
            }
        }

        // Il marcatore segue la stessa polilinea campionata mostrata a schermo.
        // Animazione illustrativa: non rappresenta la velocita' del veicolo.
        const QPainterPath *percorsoAuto = nullptr;
        qsizetype numeroPunti = 0;
        if (okO) { percorsoAuto = &ottimizzataPath; numeroPunti = oP.size(); }
        else if (okI && !doppia) { percorsoAuto = &inizialePath; numeroPunti = iP.size(); }
        if (percorsoAuto && numeroPunti >= 2) {
            const double t = std::fmod(avanzamento_ / double(numeroPunti), 1.0);
            const QPointF pos = percorsoAuto->pointAtPercent(t);
            const double angolo = -percorsoAuto->angleAtPercent(t);
            p.save();
            p.translate(pos);
            p.rotate(angolo);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 236, 219, 44));
            p.drawEllipse(QPointF(0, 0), 9, 9);
            p.setBrush(QColor("#E0FEFF"));
            QPolygonF automobile{{6,0}, {-4,-3}, {-2,0}, {-4,3}};
            p.drawPolygon(automobile);
            p.restore();
        }
        p.setClipping(false);
        f.setPixelSize(10);
        f.setBold(false);
        p.setFont(f);
        p.setPen(QColor("#8DA8C1"));
        p.drawText(QRectF(r.left()+15, r.bottom()-29, r.width()-30, 20),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   amplificazione_ < 0.5 ?
                   "SCALA REALE  ·  ROTELLINA: ZOOM  ·  TRASCINA: SPOSTA" :
                   "TRAIETTORIA IN SCALA  ·  ASFALTO LEGGIBILE  ·  ROTELLINA: ZOOM");
        p.restore();
    };

    const QRectF intera(5, 5, width()-10, height()-10);
    if (modalita_ == ModalitaConfronto::Affiancato) {
        const double gap = 6.;
        const double larghezza = (intera.width()-gap)/2.;
        const QRectF sx(intera.left(), intera.top(), larghezza, intera.height());
        const QRectF dx(intera.left()+larghezza+gap, intera.top(), larghezza, intera.height());
        disegnaVista(sx, mostraIniziale_, false, "01  /  TRAIETTORIA INIZIALE", ColoreIniziale, tempoIniziale_, false);
        disegnaVista(dx, false, mostraOttimizzata_, "02  /  TRAIETTORIA OTTIMIZZATA", ColoreOttimizzata, tempoFinale_, false);
    } else if (modalita_ == ModalitaConfronto::Sovrapposto) {
        disegnaVista(intera, mostraIniziale_, mostraOttimizzata_,
                    "CONFRONTO SOVRAPPOSTO  /  ARANCIONE VS TURCHESE",
                    QColor("#E6F3FF"), 0.0, true);
        if (nConfrontabili > 0) {
            p.setFont(QFont("Sans Serif", 9, QFont::DemiBold));
            p.setPen(QColor("#C8DCEE"));
            p.drawText(QRectF(intera.right()-350, intera.top()+24, 328, 25),
                       Qt::AlignRight,
                       QString("SCOSTAMENTO MEDIO: %1 m   •   MAX: %2 m")
                           .arg(mediaMetri, 0, 'f', 2).arg(massimoMetri, 0, 'f', 2));
        }
    }
}

void VistaPista::wheelEvent(QWheelEvent *e) {
    if (e->angleDelta().y() != 0) {
        const double tacche = e->angleDelta().y()/120.0;
        applicaZoom(std::pow(1.22, tacche), e->position());
    }
    e->accept();
}
void VistaPista::mouseDoubleClickEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        applicaZoom(2.5, e->position());
        e->accept();
    }
}
void VistaPista::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        trascinando_ = true;
        mouseUltimo_ = e->position();
        setCursor(Qt::ClosedHandCursor);
    }
}
void VistaPista::mouseMoveEvent(QMouseEvent *e) {
    if (trascinando_) {
        spostamento_ += e->position() - mouseUltimo_;
        mouseUltimo_ = e->position();
        update();
    }
}
void VistaPista::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        trascinando_ = false;
        setCursor(Qt::OpenHandCursor);
    }
}
