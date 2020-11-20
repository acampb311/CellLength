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
   ThresholdThread* workerThread = new ThresholdThread(value, img);

	currentThreshold = value;

	connect(workerThread, &ThresholdThread::resultReady, this, &MainWindow::HandleThresholdFinished);
	connect(workerThread, &ThresholdThread::finished, workerThread, &QObject::deleteLater);
	workerThread->start();
}

void MainWindow::HandleThresholdFinished(const QImage& val)
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

      QVector<Pixel> fourConn = { {0,-1},{-1,0},{0,1},{1,0} };
      auto ret = flood(p->pixmap().toImage(), temp, fourConn);
      
      if (overlay == nullptr)
      {
         overlay = scene->addPixmap(QPixmap::fromImage(ret));
      }
      else
      {
         overlay->setPixmap(QPixmap::fromImage(ret));
      }
      
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

QImage MainWindow::flood(const QImage& img, const Pixel& startPixel, const QVector<Pixel>& conn)
{
	QVector<Pixel> s = QVector<Pixel>();
   QStack<Pixel> q = QStack<Pixel>();
	s.push_back(startPixel);
   q.push(startPixel);
   
   QImage *image = new QImage(img.width(), img.height(), QImage::Format_Mono);
   
	bool* visited = new bool[img.height() * img.width()]{ false };
	visited[startPixel.x * img.width() + startPixel.y] = true;

   QImage ret = img;

	while (q.size() > 0)
	{
		Pixel pixelX = q.pop();

		for (const auto& neighbor : conn)
		{
			Pixel pixelY = pixelX;
			pixelY.x += neighbor.x;
			pixelY.y += neighbor.y;

			if (img.valid(pixelY.x, pixelY.y))
			{
				if (!visited[pixelY.x * img.width() + pixelY.y] && img.pixel(pixelY.x, pixelY.y) == QColor(Qt::white).rgb())
				{
					s.push_back(pixelY);
					visited[pixelY.x * img.width() + pixelY.y] = true;
					q.push(pixelY);
				}
			}
		}
	}
   
   image->setColorCount(2);
   image->setColor(0, qRgba(255, 0, 0, 255)); // Index #0 = Red
   image->setColor(1, qRgba(0, 0, 0, 0));     // Index #1 = Transparent

   // Testing - Fill the image with pixels:

   for (int y = 0; y < img.height(); y++)
   {
      QRgb *line = (QRgb *) image->scanLine(y);
      for (int x = 0; x < img.width(); x++)
      {
         // line[x] has an individual pixel
         line[x]->setPixel( = qRgba(0, 0, 0, 0);
      }
   }

	for (const auto& pix : s)
	{
      image->setPixel(QPoint(pix.x, pix.y), qRgba(255, 0, 0, 255));
	}
   
   delete[] visited;

	return ret;
}


