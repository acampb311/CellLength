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

#include <QVector>


#include <iostream>


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
	void HandleFloodFinished(const QImage& val);
   void HandleProgressUpdate(const int& percentDone);
   void HandleOtsuThresholdReady(const int& t);
	bool eventFilter(QObject* target, QEvent* event);

private:
	QMenu* fileMenu;
	QAction* openAct;
	QGraphicsScene* scene;
	QGraphicsView* view;
	QImage img;
   QProgressBar* operationProgress = nullptr;
	Pixel lastClickedPixel = {};
	QGraphicsPixmapItem* p = nullptr;
	QGraphicsPixmapItem* overlay = nullptr;
   QLabel* otsuThresholdLabel = new QLabel("NA");
	QGroupBox* CreateConnectivityButtons();
   QGroupBox* CreateThresholdControls();
	QVector<Pixel>* currentConn = nullptr;
	QVector<Pixel>* fourConnn = new QVector<Pixel>({ {0, -1}, { -1,0 }, { 0,1 }, { 1,0 } });
	QVector<Pixel>* eightConn = new QVector<Pixel>({ {-1,-1},{0,-1}, {1,-1}, {-1,0},{1,0},{-1,1},{0,1},{1,1}});
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
            line[x] = qGray(img.pixel(x, y)) > (ImageOps::GetAreaMean(img, Pixel(x,y), area) - c)? QColor(Qt::white).rgb() : 0;
         }
         
         emit ProgressUpdate(((double)y/img.height())*100);
      }
      
      emit resultReady(returnImg);
   }

private:
   int area = 0;
   int c = 0;
   QImage img;

signals:
   void ProgressUpdate(const int&);
   void resultReady(const QImage& s);
};

class OtsuThresholdThread : public QThread
{
   Q_OBJECT
public:
   OtsuThresholdThread(const QImage& img)
      : img(img) {};

   void run() override
   {
      ThresholdReady(ImageOps::CalculateOtsu(img));
   }

private:
   QImage img;

signals:
   void ThresholdReady(const int&);
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

		emit resultReady(ImageOps::ImageFromPixelSet(img, s));
	}

private:
   QImage img;
	Pixel startPixel;
	QVector<Pixel> conn;

signals:
	void resultReady(const QImage& s);
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
     
      emit resultReady(img);
   }

private:
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

#endif
