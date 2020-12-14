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

   QPushButton* thinButton = new QPushButton(tr("Thin"));
   QObject::connect(thinButton, &QPushButton::clicked, this, [=]() {
      ThinThread* thinThread = new ThinThread(overlay->pixmap().toImage());

      connect(thinThread, &ThinThread::resultReady, this, &MainWindow::HandleFloodFinished);
      connect(thinThread, &ThinThread::finished, thinThread, &QObject::deleteLater);
      thinThread->start();
      });

   
   QPushButton* labelButton = new QPushButton(tr("Label"));
   QObject::connect(labelButton, &QPushButton::clicked, this, [=]() {
      LabelThread* thinThread = new LabelThread(p->pixmap().toImage());
      connect(thinThread, &LabelThread::resultReady, this, &MainWindow::HandleFloodFinished);
      connect(thinThread, &LabelThread::finished, thinThread, &QObject::deleteLater);
      thinThread->start();
      });

   this->operationProgress = new QProgressBar();
   this->operationProgress->setRange(0, 100);

   
	toolbar->addWidget(CreateThresholdControls());
   toolbar->addWidget(CreateConnectivityButtons());
   toolbar->addWidget(thinButton);
   toolbar->addWidget(labelButton);
}

QGroupBox* MainWindow::CreateThresholdControls()
{
   QGroupBox* threshBox = new QGroupBox(tr("Threshold"));
   QVBoxLayout* thresholdControls = new QVBoxLayout(threshBox);
   thresholdControls->setContentsMargins(0, 0, 0, 0);

   QWidget* manualSlider = new QWidget();
   QHBoxLayout* threshSliderLayout = new QHBoxLayout(manualSlider);
   threshSliderLayout->setContentsMargins(0, 0, 0, 0);

   QSlider* threshSlider = new QSlider(Qt::Horizontal, this);
   threshSlider->setRange(MIN_THRESH_VAL, MAX_THRESH_VAL);

   QSpinBox* threshSpinBox = new QSpinBox();
   threshSpinBox->setRange(MIN_THRESH_VAL, MAX_THRESH_VAL);

   threshSliderLayout->addWidget(threshSlider);
   threshSliderLayout->addWidget(threshSpinBox);

   QObject::connect(threshSlider, &QSlider::valueChanged, this, &MainWindow::HandleThresholdSliderChanged);

   QObject::connect(threshSlider, QOverload<int>::of(&QSlider::valueChanged),
                    [=](int threshValue){ threshSpinBox->setValue(threshValue); });
   
   QObject::connect(threshSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                    [=](int threshValue){ threshSlider->setValue(threshValue); });
   
   
   QWidget* otsuCalcWidget = new QWidget();
   QHBoxLayout* otsuLayout = new QHBoxLayout(otsuCalcWidget);
   otsuLayout->setContentsMargins(0, 0, 0, 0);
   
   QPushButton* otsuButton = new QPushButton(tr("Run"));
   otsuLayout->addWidget(new QLabel("Otsu Threshold:"));
   otsuLayout->addWidget(this->otsuThresholdLabel);
   otsuLayout->addWidget(otsuButton);
   QObject::connect(otsuButton, &QPushButton::clicked, this, [=]() {
      OtsuThresholdThread* otsuThread = new OtsuThresholdThread(img);

      connect(otsuThread, &OtsuThresholdThread::ThresholdReady, this, &MainWindow::HandleOtsuThresholdReady);
      connect(otsuThread, &OtsuThresholdThread::finished, otsuThread, &QObject::deleteLater);
      otsuThread->start();
      });
   
   QWidget* adaptThreshWidget = new QWidget();
   QHBoxLayout* adaptThreshLayout = new QHBoxLayout(adaptThreshWidget);
   adaptThreshLayout->setContentsMargins(0, 0, 0, 0);
   
   QLineEdit* areaLineEdit = new QLineEdit();
   areaLineEdit->setMaximumWidth(40);
   QLineEdit* cLineEdit = new QLineEdit();
   cLineEdit->setMaximumWidth(40);
   QPushButton* adaptButton = new QPushButton(tr("Run"));
   adaptThreshLayout->addWidget(new QLabel("Area:"));
   adaptThreshLayout->addWidget(areaLineEdit);
   adaptThreshLayout->addWidget(new QLabel("C:"));
   adaptThreshLayout->addWidget(cLineEdit);
   adaptThreshLayout->addWidget(adaptButton);

   QObject::connect(adaptButton, &QPushButton::clicked, this, [=]() {
      AdaptThresholdThread* workerThread = new AdaptThresholdThread(areaLineEdit->text().toInt(), cLineEdit->text().toInt(), img);

      QObject::connect(workerThread, &AdaptThresholdThread::resultReady, this, &MainWindow::HandleThresholdFinished);
      QObject::connect(workerThread, &AdaptThresholdThread::ProgressUpdate, this, &MainWindow::HandleProgressUpdate);
      QObject::connect(workerThread, &AdaptThresholdThread::finished, workerThread, &QObject::deleteLater);
      workerThread->start();
      
      });
   
   thresholdControls->addWidget(manualSlider);
   thresholdControls->addWidget(otsuCalcWidget);
   thresholdControls->addWidget(adaptThreshWidget);
   thresholdControls->addWidget(this->operationProgress);
   this->operationProgress->setVisible(false);

   
   return threshBox;
}

void MainWindow::HandleOtsuThresholdReady(const int& t)
{
   this->otsuThresholdLabel->setText(QString::number(t));
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

void MainWindow::HandleProgressUpdate(const int& percentDone)
{
   this->operationProgress->setVisible(true);
   this->operationProgress->setValue(percentDone);

   if (percentDone == 99)
   {
      this->operationProgress->setVisible(false);
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
   QVBoxLayout* vbox = new QVBoxLayout(groupBox);

	QRadioButton* fourWay = new QRadioButton(tr("4 Way"));
	QRadioButton* eightWay = new QRadioButton(tr("8 Way"));
	fourWay->setChecked(true); 
	vbox->addWidget(fourWay);
	vbox->addWidget(eightWay);

	QObject::connect(fourWay, &QRadioButton::clicked, this, [=]() {
		currentConn = fourConnn;

		FloodThread* workerThread = new FloodThread(p->pixmap().toImage(), Pixel(this->lastClickedPixel), (*currentConn));

		connect(workerThread, &FloodThread::resultReady, this, &MainWindow::HandleFloodFinished);
		connect(workerThread, &FloodThread::finished, workerThread, &QObject::deleteLater);
		workerThread->start();
		});

	QObject::connect(eightWay, &QRadioButton::clicked, this, [=]() {
		currentConn = eightConn;

		FloodThread* workerThread = new FloodThread(p->pixmap().toImage(), Pixel(this->lastClickedPixel), (*currentConn));

		connect(workerThread, &FloodThread::resultReady, this, &MainWindow::HandleFloodFinished);
		connect(workerThread, &FloodThread::finished, workerThread, &QObject::deleteLater);
		workerThread->start();
		});

	return groupBox;
}


