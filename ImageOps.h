//
//  ImageOps.h
//  CellLength
//
//  Created by Adam Campbell on 11/28/20.
//

#ifndef ImageOps_h
#define ImageOps_h

#include <QImage>
#include <QVector>
#include <QStack>
#include <QGraphicsSceneMouseEvent>

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
   QVector<Pixel> Flood(const QImage& img, const Pixel& startPixel, const QVector<Pixel>& conn);
}

#endif /* ImageOps_h */
