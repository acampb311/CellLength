//
//  ImageOps.cpp
//  CellLength
//
//  Created by Adam Campbell on 11/28/20.
//

#include "ImageOps.h"

QVector<Pixel> ImageOps::Flood(const QImage& img, const Pixel& startPixel, const QVector<Pixel>& conn)
{
   QVector<Pixel> s = QVector<Pixel>();
   QStack<Pixel> q = QStack<Pixel>();
   s.push_back(startPixel);
   q.push(startPixel);

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
            if (!visited[pixelY.x * img.width() + pixelY.y] && img.pixel(pixelY.x, pixelY.y) == img.pixel(startPixel.x, startPixel.y))
            {
               s.push_back(pixelY);
               visited[pixelY.x * img.width() + pixelY.y] = true;
               q.push(pixelY);
            }
         }
      }
   }
   
   delete[] visited;
   return s;
}
