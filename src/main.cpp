#include <QApplication>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // 设置应用样式
    app.setStyle("fusion");

    MainWindow window;
    window.show();

    return app.exec();
}
