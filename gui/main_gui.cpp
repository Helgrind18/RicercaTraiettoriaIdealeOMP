#include "ApexMainWindow.hpp"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("APEX LAB");
    app.setOrganizationName("HPC Racing Line Optimization");
    QFont font("DejaVu Sans", 10);
    app.setFont(font);
    ApexMainWindow window;
    window.show();
    return app.exec();
}
