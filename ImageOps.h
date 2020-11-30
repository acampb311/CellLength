#ifndef ImageOps_h
#define ImageOps_h

#include <QImage>
#include <QPixmap>
#include <QVector>
#include <QStack>
#include <QGraphicsSceneMouseEvent>
#include <bitset>

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

namespace ImageOps
{

QImage Threshold(const QImage& img, const int& threshVal);

QVector<Pixel> Flood(const QImage& img, const Pixel& startPixel, const QVector<Pixel>& conn);

QImage ImageFromPixelSet(const QImage& img, const QVector<Pixel>& s);

int ImageValue(const QImage& img, const Pixel& p);

bool IsBorder(const QImage& img, const Pixel& p);

bool IsCurveEnd(const QImage& img, const Pixel& p);

bool IsSimple(const QImage& img, const Pixel& p);

}

static QVector<Pixel> neigh = QVector<Pixel>({ {-1,-1},{0,-1}, {1,-1}, {-1,0},{0,0},{1,0},{-1,1},{0,1},{1,1}});
static QVector<Pixel> border = QVector<Pixel>({ {-1,-1},{0,-1}, {1,-1}, {-1,0},{1,0},{-1,1},{0,1},{1,1}});

#endif /* ImageOps_h */
