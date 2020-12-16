#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, scene(new QGraphicsScene(this))
	, view(new QGraphicsView())
{
	InitMainWindow();
   
   QWidget* statusBarWidget = new QWidget();
   QHBoxLayout* statusBarLayout = new QHBoxLayout(statusBarWidget);
   statusBarLabel = new QLabel();
   statusBarLayout->setContentsMargins(0, 0, 0, 0);

   this->operationProgress = new QProgressBar();
   this->operationProgress->setRange(0, 100);
   statusBarLayout->addWidget(this->statusBarLabel);
   statusBarLayout->addWidget(this->operationProgress);
   statusBar()->addWidget(statusBarWidget);
   
	CreateActions();
	CreateMenus();
	CreateToolbars();

	currentConn = fourConnn;

	setCentralWidget(view);

	view->setScene(scene);
	scene->installEventFilter(this);
   
	this->statusBarLabel->setText("Ready");
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
   std::cout<< "ratio: " <<view->devicePixelRatio() <<std::endl;
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
   
   QPushButton* cleanButton = new QPushButton(tr("Clean"));
   QObject::connect(cleanButton, &QPushButton::clicked, this, [=]() {
      CleanThread* thinThread = new CleanThread(p->pixmap().toImage());

      connect(thinThread, &CleanThread::ProgressUpdate, this, &MainWindow::HandleProgressUpdate);
      connect(thinThread, &CleanThread::resultReady, this, &MainWindow::HandleThresholdFinished);
      connect(thinThread, &CleanThread::finished, thinThread, &QObject::deleteLater);
      thinThread->start();
      });
   
   QPushButton* dilateButton = new QPushButton(tr("Dilate"));
   QObject::connect(dilateButton, &QPushButton::clicked, this, [=]() {
      HandleThresholdFinished(ImageOps::Dilate(p->pixmap().toImage()));
      });
   
   QPushButton* erodeButton = new QPushButton(tr("Erode"));
   QObject::connect(erodeButton, &QPushButton::clicked, this, [=]() {
      HandleThresholdFinished(ImageOps::Erode(p->pixmap().toImage()));
      });

   QPushButton* thinButton = new QPushButton(tr("Thin"));
   QObject::connect(thinButton, &QPushButton::clicked, this, [=]() {
      ThinThread* thinThread = new ThinThread(overlay->pixmap().toImage());

      connect(thinThread, &ThinThread::resultReady, this, &MainWindow::HandleFloodFinished);
      connect(thinThread, &ThinThread::finished, thinThread, &QObject::deleteLater);
      thinThread->start();
      p->setPixmap(QPixmap::fromImage(img));
      });

   
   QPushButton* labelButton = new QPushButton(tr("Label"));
   QObject::connect(labelButton, &QPushButton::clicked, this, [=]() {
      LabelThread* thinThread = new LabelThread(p->pixmap().toImage());
      connect(thinThread, &LabelThread::resultReady, this, &MainWindow::HandleFloodFinished);
      connect(thinThread, &LabelThread::finished, thinThread, &QObject::deleteLater);
      thinThread->start();
      });

	toolbar->addWidget(CreateThresholdControls());
   toolbar->addWidget(CreateConnectivityButtons());
   toolbar->addWidget(cleanButton);
   toolbar->addWidget(dilateButton);
   toolbar->addWidget(erodeButton);

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
   
   QLineEdit* otsuAreaLineEdit = new QLineEdit();
   otsuAreaLineEdit->setMaximumWidth(40);
   QLineEdit* otsuCLineEdit = new QLineEdit();
   otsuCLineEdit->setMaximumWidth(40);
   QPushButton* otsuButton = new QPushButton(tr("Run Otsu"));
   otsuLayout->addWidget(new QLabel("Area:"));
   otsuLayout->addWidget(otsuAreaLineEdit);
   otsuLayout->addWidget(new QLabel("C:"));
   otsuLayout->addWidget(otsuCLineEdit);
   otsuLayout->addWidget(otsuButton);
   
//   otsuLayout->addWidget(new QLabel("Otsu Threshold:"));
//   otsuLayout->addWidget(this->otsuThresholdLabel);
//   otsuLayout->addWidget(otsuButton);
   QObject::connect(otsuButton, &QPushButton::clicked, this, [=]() {
      OtsuThresholdThread* otsuThread = new OtsuThresholdThread(otsuAreaLineEdit->text().toInt(), otsuCLineEdit->text().toInt(), img);

      QObject::connect(otsuThread, &OtsuThresholdThread::ProgressUpdate, this, &MainWindow::HandleProgressUpdate);
      connect(otsuThread, &OtsuThresholdThread::resultReady, this, &MainWindow::HandleThresholdFinished);
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
   QPushButton* adaptButton = new QPushButton(tr("Run Mean"));
   adaptThreshLayout->addWidget(new QLabel("Area:"));
   adaptThreshLayout->addWidget(areaLineEdit);
   adaptThreshLayout->addWidget(new QLabel("C:"));
   adaptThreshLayout->addWidget(cLineEdit);
   adaptThreshLayout->addWidget(adaptButton);

   QObject::connect(adaptButton, &QPushButton::clicked, this, [=]() {
      AdaptThresholdThread* workerThread = new AdaptThresholdThread(areaLineEdit->text().toInt(), cLineEdit->text().toInt(), img);
      this->statusBarLabel->setText("Calculating Adaptive Threshold:");
      QObject::connect(workerThread, &AdaptThresholdThread::resultReady, this, &MainWindow::HandleThresholdFinished);
      QObject::connect(workerThread, &AdaptThresholdThread::ProgressUpdate, this, &MainWindow::HandleProgressUpdate);
      QObject::connect(workerThread, &AdaptThresholdThread::finished, workerThread, &QObject::deleteLater);
      workerThread->start();
      
      });
   
   thresholdControls->addWidget(manualSlider);
   thresholdControls->addWidget(otsuCalcWidget);
   thresholdControls->addWidget(adaptThreshWidget);
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

void MainWindow::HandleFloodFinished(const QImage& val, const int& numPixels)
{
   this->statusBarLabel->setText(QString::number(numPixels/3.06) + " mm");
	if (overlay == nullptr)
	{
		overlay = scene->addPixmap(QPixmap::fromImage(val));
	}
	else
	{
		overlay->setPixmap(QPixmap::fromImage(val));
	}
}

void MainWindow::HandleProgressUpdate(const int& percentDone, const QString& operation)
{
   this->operationProgress->setVisible(true);
   this->operationProgress->setValue(percentDone);
   this->statusBarLabel->setText(operation);
   
   if (percentDone > 98)
   {
      this->statusBarLabel->setText("Ready");
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

   this->statusBarLabel->setText(stat);
}

bool MainWindow::eventFilter(QObject* target, QEvent* event)
{
	if (target == scene && event->type() == QEvent::GraphicsSceneMousePress)
	{
		HandleClickEvent(event);
	}
   else if (target == scene && event->type() == QEvent::GraphicsSceneWheel)
   {
      if (QApplication::keyboardModifiers() & Qt::ControlModifier)
      {
         QGraphicsSceneWheelEvent* n = (QGraphicsSceneWheelEvent*)event;

         view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
         // Scale the view / do the zoom
         qreal factor;

         if (n->delta() > 0)
         {
           factor = 1.1;
         }
         else
         {
           factor = 0.9;
         }
         
         view->scale(factor, factor);
      }

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


