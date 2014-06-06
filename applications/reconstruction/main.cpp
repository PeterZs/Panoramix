#include <QApplication>
#include <QtCore>
#include <QtGui>

#include "mainwin.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
	//Q_INIT_RESOURCE(reconstruction);
    
    MainWind mwind;
    mwind.setWindowIcon(QIcon(":/resources/dragon-icon.png"));
    mwind.show();

	return app.exec();
}