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

	currentConn = fourConnn;

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
   
   QLabel* threshSliderLabel = new QLabel("Threshold:");
   
   QSlider* threshSlider = new QSlider(Qt::Horizontal, this);
   threshSlider->setRange(MIN_THRESH_VAL, MAX_THRESH_VAL);

   QSpinBox* threshSpinBox = new QSpinBox();
   threshSpinBox->setRange(MIN_THRESH_VAL, MAX_THRESH_VAL);

	threshSliderLayout->addWidget(threshSliderLabel);
	threshSliderLayout->addWidget(threshSlider);
   threshSliderLayout->addWidget(threshSpinBox);

   QObject::connect(threshSlider, &QSlider::valueChanged, this, &MainWindow::HandleThresholdSliderChanged);

   QObject::connect(threshSlider, QOverload<int>::of(&QSlider::valueChanged),
                    [=](int threshValue){ threshSpinBox->setValue(threshValue); });
   
   QObject::connect(threshSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                    [=](int threshValue){ threshSlider->setValue(threshValue); });
   
	toolbar->addWidget(threshSliderWidget);
	toolbar->addWidget(CreateConnectivityButtons());
}

void MainWindow::HandleThresholdSliderChanged(int value)
{
	ThresholdThread* workerThread = new ThresholdThread(value, img);

   QObject::connect(workerThread, &ThresholdThread::resultReady, this, &MainWindow::HandleThresholdFinished);
   QObject::connect(workerThread, &ThresholdThread::finished, workerThread, &QObject::deleteLater);
	workerThread->start();
}

void MainWindow::HandleThresholdFinished(const QImage& val)
{
	p->setPixmap(QPixmap::fromImage(val));
}

void MainWindow::HandleFloodFinished(const QImage& val)
{
	if (overlay == nullptr)
	{
		overlay = scene->addPixmap(QPixmap::fromImage(val));
	}
	else
	{
		overlay->setPixmap(QPixmap::fromImage(val));
	}
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

		this->lastClickedPixel = Pixel(n);

		FloodThread* workerThread = new FloodThread(p->pixmap().toImage(), Pixel(n), (*currentConn));

      QObject::connect(workerThread, &FloodThread::resultReady, this, &MainWindow::HandleFloodFinished);
      QObject::connect(workerThread, &FloodThread::finished, workerThread, &QObject::deleteLater);
		workerThread->start();
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

QGroupBox* MainWindow::CreateConnectivityButtons()
{
	QGroupBox* groupBox = new QGroupBox(tr("Connectivity"));

	QRadioButton* fourWay = new QRadioButton(tr("4 Way"));
	QRadioButton* eightWay = new QRadioButton(tr("8 Way"));
	fourWay->setChecked(true); 
	QVBoxLayout* vbox = new QVBoxLayout;
	vbox->addWidget(fourWay);
	vbox->addWidget(eightWay);

	QObject::connect(fourWay, &QRadioButton::clicked, this, [=]() {
		currentConn = fourConnn;

		FloodThread* workerThread = new FloodThread(p->pixmap().toImage(), Pixel(lastClickedPixel), (*currentConn));

		connect(workerThread, &FloodThread::resultReady, this, &MainWindow::HandleFloodFinished);
		connect(workerThread, &FloodThread::finished, workerThread, &QObject::deleteLater);
		workerThread->start();
		});

	QObject::connect(eightWay, &QRadioButton::clicked, this, [=]() {
		currentConn = eightConn;

		FloodThread* workerThread = new FloodThread(p->pixmap().toImage(), Pixel(lastClickedPixel), (*currentConn));

		connect(workerThread, &FloodThread::resultReady, this, &MainWindow::HandleFloodFinished);
		connect(workerThread, &FloodThread::finished, workerThread, &QObject::deleteLater);
		workerThread->start();
		});

	vbox->addStretch(1);
	groupBox->setLayout(vbox);

	return groupBox;
}


