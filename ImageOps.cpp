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
   //If the pixel we are looking at currently is background, no need to look any further.
   if (ImageValue(img, p) == 0)
   {
      return false;
   }
   
   //look through the surrounding 8 pixels to see if any are background.
   for (const auto& pix : border)
   {
      //If any of the surrounding pixels are 0 (background) we are on a border pixel since
      //we have already determined that the central pixel is a an object.
      if (ImageValue(img, Pixel(p,pix)) == 0)
      {
         return true;
      }
   }
   
   return false;
}

bool ImageOps::IsCurveEnd(const QImage& img, const Pixel& p)
{
   //If the pixel we are looking at currently is background, no need to look any further.
   if (ImageValue(img, p) == 0)
   {
      return false;
   }
   
   //Keep track of the number of neighboring pixels, if this value goes above 1, we know
   //that we are not a curve end.
   int numberNeighbors = 0;
   
   //Go through the 8 bordering pixels to see how many are object pixels
   for (const auto& pix : border)
   {
      if (ImageValue(img, Pixel(p,pix)) == 1)
      {
         numberNeighbors++;
         
         if (numberNeighbors > 1)
         {
            return false;
         }
      }
   }
   
   return true;
}

bool ImageOps::IsSimple(const QImage& img, const Pixel& p)
{
   std::bitset<9> simpleKey;
   
   //construct a binary key according to the following pixel
   //positions in a 3x3 matrix
   //0 1 2
   //3 4 5
   //6 7 8
   
   //ex.
   //1 0 0
   //1 0 1
   //0 1 0
   //would be represented by {1,0,0,1,0,1,0,1,0} and the final value of 298.
   //This relys on our previously constructed simple cell lookup table.
   int idx = 0;
   for (const auto& pix : neigh)
   {
      simpleKey[idx] = ImageValue(img, Pixel(Pixel(p.x, p.y), pix));
      idx++;
   }

   return isSimpleTable[simpleKey.to_ulong()];
}
