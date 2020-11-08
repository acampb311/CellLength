#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, view(new QGraphicsView())
	, scene(new QGraphicsScene(this))
{
	initMainWindow();
	createActions();
	createMenus();

	setCentralWidget(view);

	view->setScene(scene);

	statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
}

QImage MainWindow::Threshold(const int& thresh, const QImage& input)
{
	QImage returnImg = input;

	for (int x = 0; x < input.height(); ++x)
	{
		for (int y = 0; y < input.width(); ++y)
		{
			returnImg.setPixel(x, y, qGray(input.pixel(x, y)) > thresh ? 1 : 0);
		}
	}

	return returnImg;
}

void MainWindow::openFile()
{
	QFileDialog dialog;
	initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

	QImage img = QImage(dialog.getOpenFileName());

	QGraphicsPixmapItem* p = scene->addPixmap(QPixmap::fromImage(img));
	view->fitInView(p, Qt::KeepAspectRatio);
}

void MainWindow::createActions()
{
	openAct = new QAction(tr("&Open"), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open a sperm cell picture"));
	connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAct);
}

void MainWindow::initMainWindow()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	resize(QSize(screenGeometry.width() / 3, screenGeometry.height() / 2));
}



