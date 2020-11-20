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
#include <QStack>
#include <QVector>


class WorkerThread;

#define MAX_THRESH_VAL 255
#define MIN_THRESH_VAL 0

struct Pixel
{
	int x;
	int y;
};

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
   void HandleThresholdFinished(const QImage& val);
   
   bool eventFilter(QObject *target, QEvent *event);
   QImage flood(const QImage& img, const Pixel& p, const QVector<Pixel>& conn);
   
private:
   QMenu* fileMenu;
   QAction* openAct;
   QGraphicsScene* scene;
   QGraphicsView* view;
   QImage img;
   int currentThreshold;
   QGraphicsPixmapItem* p = nullptr;
   QGraphicsPixmapItem* overlay = nullptr;
};

class ThresholdThread : public QThread
{
   Q_OBJECT
public:
   ThresholdThread(const int& threshold, const QImage& img)
      : threshVal(threshold)
      , img(img)
   {
      
   }
   
   void run() override
   {
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
   
private:
   int threshVal = 0;
   QImage img;
   
signals:
   void resultReady(const QImage& s);
};

class FloodThread : public QThread
{
   Q_OBJECT
public:
   FloodThread(const QImage& img, const Pixel& p, const QVector<Pixel>& conn)
      : img(img)
      , p(p)
      , conn(conn)
   {
      
   }
   
   void run() override
   {
      QVector<Pixel> s = QVector<Pixel>();
      s.push_back(p);
      
      int imgWidth = img.width();

      bool* visited = new bool[img.height() * img.width()]{ false };
      visited[p.x * img.width() + p.y] = true;

      QStack<Pixel> q = QStack<Pixel>();

      q.push(p);

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
               if (!visited[pixelY.x * imgWidth + pixelY.y] && img.pixel(pixelY.x, pixelY.y) == QColor(Qt::white).rgb())
               {
                  s.push_back(pixelY);
                  visited[pixelY.x * imgWidth + pixelY.y] = true;
                  q.push(pixelY);
               }
            }
         }
      }

      for (auto p : s)
      {
         img.setPixel(QPoint(p.x, p.y), QColor(Qt::blue).rgb());
      }
      
      delete[] visited;
      
      emit resultReady(img);
   }
   
private:
   Pixel p;
   QVector<Pixel> conn;
   QImage img;
   
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
