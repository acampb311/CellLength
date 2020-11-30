#include "ImageOps.h"

QImage ImageOps::Threshold(const QImage& img, const int& threshVal)
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
   
   return returnImg;
}

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

QImage ImageOps::ImageFromPixelSet(const QImage& img, const QVector<Pixel>& s)
{
   QPixmap temp(img.size());
   temp.fill(Qt::transparent);

   QImage image = QImage(temp.toImage());

   //TODO need to figure out how to get rid of setPixel
   for (const auto& pix : s)
   {
      image.setPixel(QPoint(pix.x, pix.y), QColor(Qt::red).rgb());
   }
   
   return image;
}


int ImageOps::ImageValue(const QImage& img, const Pixel& p)
{
   if (img.valid(p.x, p.y))
   {
      return img.pixel(p.x, p.y) == QColor(Qt::white).rgb();
   }
   
   return 0;
}

bool ImageOps::IsBorder(const QImage& img, const Pixel& p)
{
   for (const auto& pix : border)
   {
      Pixel tempPix = Pixel(p,pix);
      if (img.valid(tempPix.x, tempPix.y))
      {
         if (img.pixel(tempPix.x, tempPix.y) == QColor(Qt::black).rgb())
         {
            return true;
         }
      }
   }
   
   return false;
}

bool ImageOps::IsCurveEnd(const QImage& img, const Pixel& p)
{
   int numberNeighbors = 0;
   for (const auto& pix : border)
   {
      Pixel tempPix = Pixel(p,pix);
      if (img.valid(tempPix.x, tempPix.y))
      {
         if (img.pixel(tempPix.x, tempPix.y) == QColor(Qt::white).rgb())
         {
            numberNeighbors++;
            if (numberNeighbors > 1)
            {
               return false;
            }
         }
      }
   }
   
   return true;
}

bool ImageOps::IsSimple(const QImage& img, const Pixel& p)
{
   std::bitset<9> simpleKey;
   
   int idx = 0;
   for (const auto& pix : neigh)
   {
      simpleKey[idx] = ImageValue(img, Pixel(Pixel(p.x, p.y),pix));
      idx++;
   }

   return isSimpleTable[simpleKey.to_ulong()];
}
