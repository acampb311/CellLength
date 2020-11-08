#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QFileDialog>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include <QImage>
#include <QGraphicsView>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	resize(QSize(screenGeometry.width() / 3, screenGeometry.height() / 2));
	QGraphicsView* view = new QGraphicsView();
	setCentralWidget(view);


	QImage img = QImage("C:\\Users\\acamp\\Desktop\\sperm\\easy\\24708.1_1 at 20X.jpg");
	int i = 0;

	QImage img2 = img;
	int asdf = 429496729500;
	for (int x = 0; x < img.height(); ++x) 
	{ 
		for (int y = 0; y < img.width(); ++y) 
		{ 
			auto a = img.height();
			if (img.pixel(x, y) < asdf)
				asdf = img.pixel(x, y);
			img2.setPixel(x, y, qGray(img.pixel(x, y)) > 5 ? 1 : 0);
		} 
	}

	QGraphicsScene* scene = new QGraphicsScene(this);
	
	QGraphicsPixmapItem *p = scene->addPixmap(QPixmap::fromImage(img));
	view->setScene(scene);
	view->fitInView(p, Qt::KeepAspectRatio);
	statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
}

