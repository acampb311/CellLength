#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QPixmap>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QSlider>
#include <QStandardPaths>
#include <QStatusBar>
#include <QString>
#include <QToolBar>
#include <QThread>

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
      
      for (int y = 0; y < img.height(); y++)
      {
         QRgb *line = (QRgb *) returnImg.scanLine(y);
         for (int x = 0; x < img.width(); x++)
         {
            // line[x] has an individual pixel
            line[x] = qGray(img.pixel(x, y)) > threshVal ? QColor(Qt::white).rgb() : 0;
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
