#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
#include <QMenuBar>
#include <QAction>
#include <QGraphicsSceneMouseEvent>

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QToolBar>
#include <QSlider>
#include <QThread>
#include <QString>

class WorkerThread;

#define MAX_THRESH_VAL 255
#define MIN_THRESH_VAL 0

class MainWindow : public QMainWindow
{
   Q_OBJECT
   
public:
   MainWindow(QWidget *parent = nullptr);
   ~MainWindow();
   
   void CreateActions();
   void CreateMenus();
   void InitMainWindow();
   void CreateToolbars();
   
private slots:
   void OpenFile();
   void HandleClickEvent(QEvent *event);
   void HandleThresholdSliderChanged(int value);
   void HandleThresholdFinished(QImage val);
   
   bool eventFilter(QObject *target, QEvent *event);

   
private:
   QMenu* fileMenu;
   QAction* openAct;
   QGraphicsScene* scene;
   QGraphicsView* view;
   QImage img;
   QSlider* threshSlider = new QSlider(Qt::Horizontal);
   QGraphicsPixmapItem* p = nullptr;
};

class WorkerThread : public QThread
{
   Q_OBJECT
public:
   int threshVal = 0;
   QImage img;
   
   void run() override {
      
      QImage returnImg = img;
      
      for (int x = 0; x < img.height(); ++x)
      {
         for (int y = 0; y < img.width(); ++y)
         {
            returnImg.setPixel(x, y, qGray(img.pixel(x, y)) > threshVal ? QColor(Qt::white).rgb() : 0);
         }
      }
      
      emit resultReady(returnImg);
   }
   
signals:
   void resultReady(const QImage& s);
};


// https://doc.qt.io/qt-5/qtwidgets-widgets-imageviewer-example.html
static void initializeImageFileDialog(QFileDialog& dialog, QFileDialog::AcceptMode acceptMode)
{
   static bool firstDialog = true;
   
   if (firstDialog) {
      firstDialog = false;
      const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
      dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
   }
   
   QStringList mimeTypeFilters;
   const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
   ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
   for (const QByteArray& mimeTypeName : supportedMimeTypes)
      mimeTypeFilters.append(mimeTypeName);
   mimeTypeFilters.sort();
   dialog.setMimeTypeFilters(mimeTypeFilters);
   dialog.selectMimeTypeFilter("image/jpeg");
   if (acceptMode == QFileDialog::AcceptSave)
      dialog.setDefaultSuffix("jpg");
}

#endif // MAINWINDOW_H
