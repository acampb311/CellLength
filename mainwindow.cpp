#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
   : QMainWindow(parent)
   , scene(new QGraphicsScene(this))
   , view(new QGraphicsView())
{
   InitMainWindow();
   CreateActions();
   CreateMenus();
   
   setCentralWidget(view);
   
   view->setScene(scene);
   scene->installEventFilter(this);
   CreateToolbars();
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
   QToolBar* toolbar = addToolBar(tr("Threshold"));
   
   threshSlider->setMaximum(MAX_THRESH_VAL);
   threshSlider->setMinimum(MIN_THRESH_VAL);
   
   connect(threshSlider, &QSlider::valueChanged, this, &MainWindow::HandleThresholdSliderChanged);
   
   toolbar->addWidget(threshSlider);
}

void MainWindow::HandleThresholdSliderChanged(int value)
{
   WorkerThread *workerThread = new WorkerThread();
   workerThread->img = img;
   workerThread->threshVal = value;
   
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
   resize(QSize(screenGeometry.width() / 3, screenGeometry.height() / 2));
}

void MainWindow::HandleClickEvent(QEvent *event)
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
