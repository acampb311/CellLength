#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, scene(new QGraphicsScene(this))
	, view(new QGraphicsView())
{
	InitMainWindow();
	CreateActions();
	CreateMenus();
	CreateToolbars();

	setCentralWidget(view);

	view->setScene(scene);
	scene->installEventFilter(this);

	statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
}

void MainWindow::OpenFile()
{
	QFileDialog dialog;
	initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);
	auto filePath = dialog.getOpenFileName();

	img = QImage(filePath);
	setWindowTitle(filePath);

	if (p == nullptr)
	{
		p = scene->addPixmap(QPixmap::fromImage(img));
	}
	else
	{
		p->setPixmap(QPixmap::fromImage(img));
	}

	view->fitInView(p, Qt::KeepAspectRatio);
}

void MainWindow::CreateActions()
{
	openAct = new QAction(tr("&Open"), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open a sperm cell picture"));
	connect(openAct, &QAction::triggered, this, &MainWindow::OpenFile);
}

void MainWindow::CreateMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAct);
}

void MainWindow::CreateToolbars()
{
	QToolBar* toolbar = new QToolBar();
	addToolBar(Qt::LeftToolBarArea, toolbar);

	QWidget* threshSliderWidget = new QWidget(this);
	QHBoxLayout* threshSliderLayout = new QHBoxLayout(threshSliderWidget);
	QLabel* threshSliderValueLabel = new QLabel("0");
	QLabel* threshSliderLabel = new QLabel("Threshold Value:");

	QSlider* threshSlider = new QSlider(Qt::Horizontal, this);
	threshSlider->setMaximum(MAX_THRESH_VAL);
	threshSlider->setMinimum(MIN_THRESH_VAL);

	threshSliderLayout->addWidget(threshSliderLabel);
	threshSliderLayout->addWidget(threshSlider);
	threshSliderLayout->addWidget(threshSliderValueLabel);

	connect(threshSlider, &QSlider::valueChanged, this, &MainWindow::HandleThresholdSliderChanged);

	QObject::connect(threshSlider, &QSlider::valueChanged, this, [=]() {
		threshSliderValueLabel->setText(QString::number(threshSlider->value()));
		});

	toolbar->addWidget(threshSliderWidget);
}

void MainWindow::HandleThresholdSliderChanged(int value)
{
	WorkerThread* workerThread = new WorkerThread();
	workerThread->img = img;
	workerThread->threshVal = value;
	currentThreshold = value;

	connect(workerThread, &WorkerThread::resultReady, this, &MainWindow::HandleThresholdFinished);
	connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
	workerThread->start();
}

void MainWindow::HandleThresholdFinished(QImage val)
{
	p->setPixmap(QPixmap::fromImage(val));
}

void MainWindow::InitMainWindow()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	resize(QSize(screenGeometry.width() / 1.5, screenGeometry.height() / 1.5));
}

void MainWindow::HandleClickEvent(QEvent* event)
{
	QGraphicsSceneMouseEvent* n = (QGraphicsSceneMouseEvent*)event;

	QString stat = "";

	if (img.valid(n->scenePos().toPoint()))
	{
		stat = "Clicked: {"
			+ QString::number(n->scenePos().toPoint().x())
			+ " , "
			+ QString::number(n->scenePos().toPoint().y())
			+ "} Value: "
			+ QString::number(qGray(img.pixel(n->scenePos().toPoint())));
		Pixel temp;
		temp.x = n->scenePos().toPoint().x();
		temp.y = n->scenePos().toPoint().y();

		flood(img, temp);
	}
	else
	{
		stat = "Outside boundary";
	}

	statusBar()->showMessage(stat);
}

bool MainWindow::eventFilter(QObject* target, QEvent* event)
{
	if (target == scene && event->type() == QEvent::GraphicsSceneMousePress)
	{
		HandleClickEvent(event);
	}

	return QMainWindow::eventFilter(target, event);
}

QVector<Pixel> MainWindow::flood(QImage img, Pixel p)
{
	QVector<Pixel> s = QVector<Pixel>();
	s.push_back(p);

	QVector<Pixel> fourConn = { {0,-1},{-1,0},{0,1},{1,0} };
	int imgWidth = img.width();

	bool* visited = new bool[img.height() * img.width()]{ false };
	visited[p.x * img.width() + p.y] = true;

	QStack<Pixel> q = QStack<Pixel>();

	q.push(p);

	QImage returnImg = img;
	for (int y = 0; y < img.height(); y++)
	{
		QRgb* line = (QRgb*)returnImg.scanLine(y);
		for (int x = 0; x < img.width(); x++)
		{
			// line[x] has an individual pixel
			line[x] = qGray(img.pixel(x, y)) > currentThreshold ? QColor(Qt::white).rgb() : 0;
		}
	}


	while (q.size() > 0)
	{
		Pixel pixelX = q.pop();

		for (const auto& neighbor : fourConn)
		{
			Pixel pixelY = pixelX;
			pixelY.x += neighbor.x;
			pixelY.y += neighbor.y;

			if (img.valid(pixelY.x, pixelY.y))
			{
				auto g = qGray(img.pixel(pixelY.x, pixelY.y));
				if (!visited[pixelY.x * imgWidth + pixelY.y] && returnImg.pixel(pixelY.x, pixelY.y) == QColor(Qt::white).rgb())
				{
					s.push_back(pixelY);
					visited[pixelY.x * imgWidth + pixelY.y] = true;
					q.push(pixelY);
				}
			}
		}
	}


	QImage img2 = img;

	for (auto p : s)
	{
		img2.setPixel(QPoint(p.x, p.y), QColor(Qt::blue).rgb());
	}

	scene->addPixmap(QPixmap::fromImage(img2));

	return s;
}


