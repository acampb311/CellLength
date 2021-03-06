#ifndef ImageOps_h
#define ImageOps_h

#include <QImage>
#include <QPixmap>
#include <QVector>
#include <QStack>
#include <QGraphicsSceneMouseEvent>
#include <bitset>
#include <QMutex>
#include <QObject>

#define MAX_THRESH_VAL 255
#define MIN_THRESH_VAL 0

class ProgressIndicator : public QObject
{
   Q_OBJECT
public:
   ProgressIndicator() {};
signals:
   void ProgressUpdate(const int& value, const QString& operationName);
};

static constexpr bool isSimpleTable[] =
{
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,1,1,1,1,0,0,1,1,0,0
};

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

static QVector<Pixel>* fourConnn = new QVector<Pixel>({ {0, -1}, { -1,0 }, { 0,1 }, { 1,0 } });
static QVector<Pixel>* eightConn = new QVector<Pixel>({ {-1,-1},{0,-1}, {1,-1}, {-1,0},{1,0},{-1,1},{0,1},{1,1}});

static bool* multiVisited;
static QImage multiImg;

namespace ImageOps
{



QImage Threshold(const QImage& img, const int& threshVal);

QImage AdaptiveThreshold(const QImage& img, const int& area);

QImage Dilate(const QImage& img);

QImage Erode(const QImage& img);

int RealImageValue(const QImage& img, const Pixel& p);

QVector<QVector<Pixel>> LabelComponents(const QImage& img, ProgressIndicator* progress);

QVector<Pixel> LabelOp(const Pixel& pix);

int CalculateOtsu(const QImage& img, const QVector<int>& histogram, const int& N);

int GetAreaMean(const QImage& img, const Pixel& p, const int& area);

QVector<int> GetAreaHistogram(const QImage& img, const Pixel& p, const int& area);

QVector<Pixel> Flood(const QImage& img, const Pixel& startPixel, const QVector<Pixel>& conn);

QImage ImageFromPixelSet(const QImage& img, const QVector<Pixel>& s, const QColor& color);

int ImageValue(const QImage& img, const Pixel& p);

bool IsBorder(const QImage& img, const Pixel& p);

bool IsCurveEnd(const QImage& img, const Pixel& p);

bool IsSimple(const QImage& img, const Pixel& p);

QVector<Pixel> GetBorderPixels(const QImage& img);

}

static QVector<Pixel> neigh = QVector<Pixel>({ {-1,-1},{0,-1}, {1,-1}, {-1,0},{0,0},{1,0},{-1,1},{0,1},{1,1}});
static QVector<Pixel> border = QVector<Pixel>({ {-1,-1},{0,-1}, {1,-1}, {-1,0},{1,0},{-1,1},{0,1},{1,1}});

#endif /* ImageOps_h */
