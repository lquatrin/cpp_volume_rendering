#ifndef VIS_UTILS_COLOR_UTILS_H
#define VIS_UTILS_COLOR_UTILS_H

// https://www.nixsensor.com/free-color-converter/
// https://docs.opencv.org/3.3.0/de/d25/imgproc_color_conversions.html
// D65 BASE VALUES XYZ = [95.0470, 100.000, 108.883]
////////////////////////////////////////////////////////////
// -> RGB:
//  . R = [   0  255]
//  . G = [   0  255]
//  . B = [   0  255]
////////////////////////////////////////////////////////////
// -> LAB: https://en.wikipedia.org/wiki/CIELAB_color_space
//  . L = [   0  100]
//  . A = [-128 +127]
//  . B = [-128 +127]
////////////////////////////////////////////////////////////
// D50 VALUES
/// Max XYZ values
// X = 95.0470
// Y = 100.000
// Z = 108.883
class ColorConverter
{
public:
  // XYZ to...
  static void XYZtoRGB (double* xyz, double* rgb);
  static void XYZtoLAB (double* xyz, double* lab);

  // RGB to...
  static void RGBtoXYZ (double* rgb, double* xyz);
  static void RGBtoLAB (double* rgb, double* lab);

  // LAB to...
  static void LABtoXYZ (double* lab, double* xyz);
  static void LABtoRGB (double* lab, double* rgb);

protected:
private:
};

// D65 VALUES
/// Max XYZ values
// X = 95.0470
// Y = 100.000
// Z = 108.883
class ColorSpaces
{
public:
  static void RGBtoLAB (double* rgb, double* lab);

protected:
private:
};

// calculate the perceptual distance between colors in CIELAB
double deltaE(double* labA, double* labB);

class LAB
{
public:
  LAB()
  {
    l = 0.0;
    a = 0.0;
    b = 0.0;
  }

  double l, a, b;
};

//void ConvertRGBtoLAB (double* rgb, LAB* o_lab)
//{
//  double r = rgb[0] / 255.0;
//  double g = rgb[1] / 255.0;
//  double b = rgb[2] / 255.0;
//
//  r = ((r > 0.04045) ? pow((r + 0.055) / 1.055, 2.4) : (r / 12.92)) * 100.0;
//  g = ((g > 0.04045) ? pow((g + 0.055) / 1.055, 2.4) : (g / 12.92)) * 100.0;
//  b = ((b > 0.04045) ? pow((b + 0.055) / 1.055, 2.4) : (b / 12.92)) * 100.0;
//
//  double x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
//  double y = r * 0.2126729 + g * 0.7151522 + b * 0.0721750;
//  double z = r * 0.0193339 + g * 0.1191920 + b * 0.9503041;
//
//  x = x / 95.047;
//  y = y / 100.00;
//  z = z / 108.883;
//
//  x = (x > 0.008856) ? cbrt(x) : (7.787 * x + 16.0 / 116.0);
//  y = (y > 0.008856) ? cbrt(y) : (7.787 * y + 16.0 / 116.0);
//  z = (z > 0.008856) ? cbrt(z) : (7.787 * z + 16.0 / 116.0);
//
//  o_lab->l = (116.0 * y) - 16;
//  o_lab->a = 500 * (x - y);
//  o_lab->b = 200 * (y - z);
//}

double Cie2000Comparison(double* rgb_a, double* rgb_b);


#endif
