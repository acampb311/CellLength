#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, scene(new QGraphicsScene(this))
    , view(new QGraphicsView())
{
	initMainWindow();
	createActions();
	createMenus();

	setCentralWidget(view);

	view->setScene(scene);
    scene->installEventFilter(this);
    
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
    auto filePath = dialog.getOpenFileName();
//    img = Threshold(150,QImage(filePath));
    img = QImage(filePath);
    setWindowTitle(filePath);

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

bool MainWindow::eventFilter(QObject* target, QEvent* event)
{
    if (target == scene)
    {
        if (event->type() == QEvent::GraphicsSceneMousePress)
        {
            QGraphicsSceneMouseEvent* n = new QGraphicsSceneMouseEvent();
            n = (QGraphicsSceneMouseEvent*)event;
            QString stat = "";
            
            if (img.valid(n->scenePos().toPoint()))
            {
                stat = "Clicked: {"
                + QString::number(n->scenePos().toPoint().x())
                + " , "
                + QString::number(n->scenePos().toPoint().y())
                + "} Value: "
                + QString::number(qGray(img.pixel(n->scenePos().toPoint())));
            }
            else
            {
                stat = "Outside boundary";
            }
            
            statusBar()->showMessage(stat);
        }
    }
    return QMainWindow::eventFilter(target, event);
}
