#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    // Enable High-DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName("CG Lab: Chrome Dino Raster Engine");

    // Dark sleek UI palette for lab presentation
    app.setStyle(QStyleFactory::create("Fusion"));

    MainWindow window;
    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--mobile") {
            window.setMobilePreview(true);
            break;
        }
    }
    window.show();

    return app.exec();
}
