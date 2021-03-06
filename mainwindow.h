#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QRect>
#include <QScreen>
#include <QSlider>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStatusBar>
#include <QString>
#include <QToolBar>
#include <QThread>
#include <QStack>
#include <QFormLayout>
#include <QWheelEvent>
#include <QVector>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

#include <iostream>
#include <algorithm>

#include "ImageOps.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

	void CreateActions();
	void CreateMenus();
	void InitMainWindow();
	void CreateToolbars();

private slots:
	void OpenFile();
	void HandleClickEvent(QEvent* event);
	void HandleThresholdSliderChanged(int value);
	void HandleThresholdFinished(const QImage& val);
	void HandleFloodFinished(const QImage& val, const int& numberPixels);
   void HandleProgressUpdate(const int& percentDone, const QString& operation);
   void HandleOtsuThresholdReady(const int& t);
	bool eventFilter(QObject* target, QEvent* event);

private:
	QMenu* fileMenu;
	QAction* openAct;
	QGraphicsScene* scene;
	QGraphicsView* view;
	QImage img;
   QProgressBar* operationProgress = nullptr;
   QLabel* statusBarLabel = nullptr;
	Pixel lastClickedPixel = {};
	QGraphicsPixmapItem* p = nullptr;
	QGraphicsPixmapItem* overlay = nullptr;
   QLabel* otsuThresholdLabel = new QLabel("NA");
	QGroupBox* CreateConnectivityButtons();
   QGroupBox* CreateThresholdControls();
	QVector<Pixel>* currentConn = nullptr;
};

class ThresholdThread : public QThread
{
	Q_OBJECT
public:
	ThresholdThread(const int& threshold, const QImage& img)
		: threshVal(threshold)
		, img(img) {};

	void run() override
	{
		emit resultReady(ImageOps::Threshold(img, threshVal));
	}

private:
	int threshVal = 0;
	QImage img;

signals:
	void resultReady(const QImage& s);
};

class AdaptThresholdThread : public QThread
{
   Q_OBJECT
public:
   AdaptThresholdThread(const int& area, const int& c, const QImage& img)
      : area(area)
      , c(c)
      , img(img) {};

   void run() override
   {
      QImage returnImg = img;

      for (int y = 0; y < img.height(); y++)
      {
         QRgb* line = (QRgb*)returnImg.scanLine(y);
         
         for (int x = 0; x < img.width(); x++)
         {
            // line[x] has an individual pixel
            line[x] = (qGray(img.pixel(x, y)) > (ImageOps::GetAreaMean(img, Pixel(x,y), area) - c))? QColor(Qt::white).rgb() : 0;
         }
         
         emit ProgressUpdate(((double)y/img.height())*100, "Adapt Threshold: ");
      }
      
      emit resultReady(returnImg);
   }

private:
   int area = 0;
   int c = 0;
   QImage img;

signals:
   void ProgressUpdate(const int&, const QString);
   void resultReady(const QImage& s);
};

class OtsuThresholdThread : public QThread
{
   Q_OBJECT
public:
   OtsuThresholdThread(const int& area, const int& c, const QImage& img)
      : area(area)
      , c(c)
      , img(img) {};

   void run() override
   {
      QImage returnImg = img;

      for (int y = 0; y < img.height(); y++)
      {
         QRgb* line = (QRgb*)returnImg.scanLine(y);
         
         for (int x = 0; x < img.width(); x++)
         {
            auto hist = ImageOps::GetAreaHistogram(img, Pixel(x,y), area);

            line[x] = qGray(img.pixel(x, y)) > (ImageOps::CalculateOtsu(img, hist, 4*area*area)-c) ? QColor(Qt::white).rgb() : 0;
         }
         
         emit ProgressUpdate(((double)y/img.height())*100, "Otsu Threshold: ");
      }
      
      emit resultReady(returnImg);
   }

private:
   QImage img;
   int area = 0;
   int c = 0;
signals:
   void ProgressUpdate(const int&, const QString);
   void resultReady(const QImage& s);
};

class FloodThread : public QThread
{
	Q_OBJECT
public:
	FloodThread(const QImage& img, const Pixel& startPixel, const QVector<Pixel>& conn)
		: img(img)
		, startPixel(startPixel)
		, conn(conn) {};

	void run() override
	{
      QVector<Pixel> s = ImageOps::Flood(img, startPixel, conn);

		emit resultReady(ImageOps::ImageFromPixelSet(img, s, QColor(Qt::red)), s.count());
	}

private:
   QImage img;
	Pixel startPixel;
	QVector<Pixel> conn;

signals:
	void resultReady(const QImage& s, const int& numPixels);
};

class ThinThread : public QThread
{
   Q_OBJECT
public:
   ThinThread(const QImage& img)
      : img(img) {};

   void run() override
   {
      while (true)
      {
         auto borderPixels = ImageOps::GetBorderPixels(img);
         int numRemoved = 0;

         for (const auto& currentPix : borderPixels)
         {
            if (ImageOps::IsSimple(img, currentPix) && !ImageOps::IsCurveEnd(img, currentPix))
            {
               numRemoved++;

               img.setPixel(QPoint(currentPix.x, currentPix.y), QColor(Qt::black).rgb());
            }
         }

         if (numRemoved == 0)
         {
            break;
         }
      }

      QVector<Pixel> s;
      for (int y = 0; y < img.height(); y++)
      {
         for (int x = 0; x < img.width(); x++)
         {
            if (img.pixel(x, y) == QColor(Qt::red).rgb())
            {
               s.push_back(Pixel(x,y));
            }
         }
      }

      emit resultReady(ImageOps::ImageFromPixelSet(img, s, QColor(Qt::red)), s.count());
   }

private:
   QImage img;
   
   
signals:
   void resultReady(const QImage& s, const int& numPix);
};

static bool sorty(const QVector<Pixel> &s1, const QVector<Pixel> &s2)
{
   return s1.count() < s2.count();
}

class LabelThread : public QThread
{
   Q_OBJECT
public:
   LabelThread(const QImage& img)
      : img(img) {};

   void run() override
   {
      ProgressIndicator *p = new ProgressIndicator();
      
      auto b = ImageOps::LabelComponents(img, p);
      
      std::sort(b.begin(), b.end(), sorty);
      emit resultReady(ImageOps::ImageFromPixelSet(img, b.back(), QColor(Qt::red)), b.back().count());
      delete p;
   }

private:
   QImage img;

   
signals:
   void resultReady(const QImage& s, const int&);
};

class CleanThread : public QThread
{
   Q_OBJECT
public:
   CleanThread(const QImage& img)
      : img(img) {};

   void run() override
   {
      ProgressIndicator *p = new ProgressIndicator();
      connect(p, &ProgressIndicator::ProgressUpdate, this, &CleanThread::Handle);

      auto b = ImageOps::LabelComponents(img, p);
      
      std::sort(b.begin(), b.end(), sorty);
      
      QVector<Pixel>daddyObject;
      
      for (const auto& comp : b)
      {
         if (comp.count() > 200)
         {
            daddyObject.append(comp);
         }
      }

      emit resultReady(ImageOps::ImageFromPixelSet(img, daddyObject, QColor(Qt::white)));
      emit ProgressUpdate(100, "");

      delete p;
   }

private:
   QImage img;
   
public slots:
   void Handle(const int& value, const QString& operationName)
   {
      emit ProgressUpdate(value, operationName);
   }
   
signals:
   void ProgressUpdate(const int& value, const QString& operationName);
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

#endif
