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
#include <QMainWindow>
#include <QMenuBar>
#include <QPixmap>
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

#include <QVector>

#define MAX_THRESH_VAL 255
#define MIN_THRESH_VAL 0

class Pixel
{
public:
	Pixel() : x(0), y(0) {};

	Pixel(int x, int y) : x(x), y(y) {};

	Pixel(QGraphicsSceneMouseEvent* e) : x(e->scenePos().toPoint().x()), y(e->scenePos().toPoint().y()) {};

	Pixel(const Pixel& p1, const Pixel& p2) : x(p1.x + p2.x), y(p1.y + p2.y) {};
	   
	int x;
	int y;
};

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

	bool eventFilter(QObject* target, QEvent* event);

private:
	QMenu* fileMenu;
	QAction* openAct;
	QGraphicsScene* scene;
	QGraphicsView* view;
	QImage img;
	Pixel lastClickedPixel = {};
	QGraphicsPixmapItem* p = nullptr;
	QGraphicsPixmapItem* overlay = nullptr;
	QGroupBox* CreateConnectivityButtons();
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
		QImage returnImg = img;
      
      if (threshVal != 0)
      {
         for (int y = 0; y < img.height(); y++)
         {
            QRgb* line = (QRgb*)returnImg.scanLine(y);
            for (int x = 0; x < img.width(); x++)
            {
               // line[x] has an individual pixel
               line[x] = qGray(img.pixel(x, y)) > threshVal ? QColor(Qt::white).rgb() : 0;
            }
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
	FloodThread(const QImage& img, const Pixel& startPixel, const QVector<Pixel>& conn)
		: img(img)
		, startPixel(startPixel)
		, conn(conn) {};

	void run() override
	{
		QVector<Pixel> s = QVector<Pixel>();
		QStack<Pixel> q = QStack<Pixel>();
		s.push_back(startPixel);
		q.push(startPixel);

		QPixmap temp(img.size());
		temp.fill(Qt::transparent);

		QImage image = QImage(temp.toImage());

		bool* visited = new bool[img.height() * img.width()]{ false };
		visited[startPixel.x * img.width() + startPixel.y] = true;

		while (q.size() > 0)
		{
			Pixel pixelX = q.pop();

			for (const auto& neighbor : conn)
			{
				Pixel pixelY = Pixel(pixelX, neighbor);

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


		//TODO need to figure out how to get rid of setPixel
		for (const auto& pix : s)
		{
			image.setPixel(QPoint(pix.x, pix.y), QColor(Qt::red).rgb());
		}

		delete[] visited;

		emit resultReady(image);
	}

private:
	Pixel startPixel;
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

#endif
