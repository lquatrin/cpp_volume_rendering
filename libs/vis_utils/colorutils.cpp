#include "colorutils.h"

#include <cmath>
#include <cstdlib>
#include <string>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#define DegToRad(x) ((x)*(glm::pi<double>())/180)
#define RadToDeg(x) ((x)/(glm::pi<double>())*180)

// XYZ to...
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
void ColorConverter::XYZtoRGB (double* xyz, double* rgb)
{
  float x = xyz[0];
  float y = xyz[1];
  float z = xyz[2];

  double r = fmax(fmin(x * 2.0413690 - y * 0.5649464 - z * 0.3446944, 100.0), 0.0);
  double g = fmax(fmin(x * -0.9692660 + y * 1.8760108 + z * 0.0415560, 100.0), 0.0);
  double b = fmax(fmin(x * 0.0134474 + y * -0.1183897 + z * 1.0154096, 100.0), 0.0);

  rgb[0] = r * (255.0 / 100.0);
  rgb[1] = g * (255.0 / 100.0);
  rgb[2] = b * (255.0 / 100.0);
}

void ColorConverter::XYZtoLAB (double* xyz, double* lab)
{
  double X = xyz[0];
  double Y = xyz[1];
  double Z = xyz[2];

  // D65
  double Xn = 95.0470;
  double Yn = 100.000;
  double Zn = 108.883;

  // D50
  //double Xn = 96.6797;
  //double Yn = 100.000;
  //double Zn = 82.5188;

  double deltaS = 6.0 / 29.0;
  double dS3 = deltaS * deltaS * deltaS;

  double xdxn = X / Xn;
  double ydyn = Y / Yn;
  double zdzn = Z / Zn;

  double L = 116.0 * (ydyn > dS3 ? pow(ydyn, 1.0 / 3.0) : ((ydyn / (3.0 * deltaS * deltaS)) + (4.0 / 29.0))) - 16.0;
  double a = 500.0 * (
    (xdxn > dS3 ? pow(xdxn, 1.0 / 3.0) : ((xdxn / (3.0 * deltaS * deltaS)) + (4.0 / 29.0))) -
    (ydyn > dS3 ? pow(ydyn, 1.0 / 3.0) : ((ydyn / (3.0 * deltaS * deltaS)) + (4.0 / 29.0)))
    );

  double b = 200.0 * (
    (ydyn > dS3 ? pow(ydyn, 1.0 / 3.0) : ((ydyn / (3.0 * deltaS * deltaS)) + (4.0 / 29.0))) -
    (zdzn > dS3 ? pow(zdzn, 1.0 / 3.0) : ((zdzn / (3.0 * deltaS * deltaS)) + (4.0 / 29.0)))
    );

  lab[0] = L;
  lab[1] = a;
  lab[2] = b;
}

// RGB to...
void ColorConverter::RGBtoXYZ (double* rgb, double* xyz)
{
  // D65 MAX VALUES
  //double Xn = 95.0470;
  //double Yn = 100.000;
  //double Zn = 108.883;

  float r = rgb[0] / 255.0;
  float g = rgb[1] / 255.0;
  float b = rgb[2] / 255.0;


  float x = r * 0.5767309 + g * 0.1855540 + b * 0.1881852;
  float y = r * 0.2973769 + g * 0.6273491 + b * 0.0752741;
  float z = r * 0.0270343 + g * 0.0706872 + b * 0.9911085;

  xyz[0] = x * 100.0;
  xyz[1] = y * 100.0;
  xyz[2] = z * 100.0;
}

void ColorConverter::RGBtoLAB (double* rgb, double* lab)
{
  double xyz[3];
  ColorConverter::RGBtoXYZ(rgb, xyz);
  ColorConverter::XYZtoLAB(xyz, lab);
}

// LAB to...
void ColorConverter::LABtoXYZ (double* lab, double* xyz)
{
  // D65
  double Xn = 95.0470;
  double Yn = 100.000;
  double Zn = 108.883;

  // D50
  //double Xn = 96.6797;
  //double Yn = 100.000;
  //double Zn = 82.5188;

  // +0 - +100
  double L = lab[0];
  // -128 - +127
  double a = lab[1];
  // -128 - +127
  double b = lab[2];

  double deltaS = 6.0 / 29.0;
  double fx = (((L + 16.0) / 116.0) + (a / 500.0));
  double X = Xn * (fx > deltaS ? fx*fx*fx : 3.0 * deltaS * deltaS * (fx - (4.0 / 29.0)));

  double fy = (L + 16.0) / 116.0;
  double Y = Yn * (fy > deltaS ? fy*fy*fy : 3.0 * deltaS * deltaS * (fy - (4.0 / 29.0)));

  double fz = (((L + 16.0) / 116.0) - (b / 200.0));
  double Z = Zn * (fz > deltaS ? fz*fz*fz : 3.0 * deltaS * deltaS * (fz - (4.0 / 29.0)));

  xyz[0] = X;
  xyz[1] = Y;
  xyz[2] = Z;
}

void ColorConverter::LABtoRGB (double* lab, double* rgb)
{
  double xyz[3];
  ColorConverter::LABtoXYZ(lab, xyz);
  ColorConverter::XYZtoRGB(xyz, rgb);
}

#define USE_OPENCV_RGB_TO_LAB_CONVERSION
// Step 1: RGB to XYZ
//         http://www.easyrgb.com/index.php?X=MATH&H=02#text2
// Step 2: XYZ to Lab
//         http://www.easyrgb.com/index.php?X=MATH&H=07#text7
void ColorSpaces::RGBtoLAB(double* I_rgb, double* o_lab)
{
  double r = I_rgb[0] / 255.0;
  double g = I_rgb[1] / 255.0;
  double b = I_rgb[2] / 255.0;

  // https://en.wikipedia.org/wiki/SRGB
  // if we need to transform srgb back to rgb
  r = ((r > 0.04045) ? pow((r + 0.055) / 1.055, 2.4) : (r / 12.92)) * 100.0;
  g = ((g > 0.04045) ? pow((g + 0.055) / 1.055, 2.4) : (g / 12.92)) * 100.0;
  b = ((b > 0.04045) ? pow((b + 0.055) / 1.055, 2.4) : (b / 12.92)) * 100.0;
  // else
  //r = r * 100.0;
  //g = g * 100.0;
  //b = b * 100.0;

// https://docs.opencv.org/3.4/de/d25/imgproc_color_conversions.html
#ifdef USE_OPENCV_RGB_TO_LAB_CONVERSION
  double x = (r * 0.412453 + g * 0.357580 + b * 0.180423) /  95.0456;
  double y = (r * 0.212671 + g * 0.715160 + b * 0.072169) / 100.0000;
  double z = (r * 0.019334 + g * 0.119193 + b * 0.950227) / 108.8754;

  double L = ((y > 0.008856) ? 116.0 * cbrt(y) - 16.0 : 903.3 * y);

  x = ((x > 0.008856) ? cbrt(x) : (7.787 * x) + (16.0 / 116.0));
  y = ((y > 0.008856) ? cbrt(y) : (7.787 * y) + (16.0 / 116.0));
  z = ((z > 0.008856) ? cbrt(z) : (7.787 * z) + (16.0 / 116.0));

  double _a = 500.0 * (x - y);
  double _b = 200.0 * (y - z);

  o_lab[0] = L;
  o_lab[1] = _a;
  o_lab[2] = _b;

// https://github.com/berendeanicolae/ColorSpace/blob/master/src/Conversion.cpp
#else
  double x = r*0.4124564 + g*0.3575761 + b*0.1804375;
  double y = r*0.2126729 + g*0.7151522 + b*0.0721750;
  double z = r*0.0193339 + g*0.1191920 + b*0.9503041;
  
  x = x / 95.047;
  y = y / 100.00;
  z = z / 108.883;
  
  x = (x > 0.008856) ? cbrt(x) : (7.787 * x + 16.0 / 116.0);
  y = (y > 0.008856) ? cbrt(y) : (7.787 * y + 16.0 / 116.0);
  z = (z > 0.008856) ? cbrt(z) : (7.787 * z + 16.0 / 116.0);


  o_lab[0] = (116.0 * y) - 16.0;
  o_lab[1] = 500.0 * (x - y);
  o_lab[2] = 200.0 * (y - z);
#endif
}

// calculate the perceptual distance between colors in CIELAB
double deltaE(double* labA, double* labB)
{
  double deltaL = labA[0] - labB[0];
  double deltaA = labA[1] - labB[1];
  double deltaB = labA[2] - labB[2];
  double c1 = glm::sqrt(labA[1] * labA[1] + labA[2] * labA[2]);
  double c2 = glm::sqrt(labB[1] * labB[1] + labB[2] * labB[2]);
  double deltaC = c1 - c2;
  double deltaH = deltaA * deltaA + deltaB * deltaB - deltaC * deltaC;
  deltaH = deltaH < 0.0 ? 0.0 : glm::sqrt(deltaH);
  double sc = 1.0 + 0.045 * c1;
  double sh = 1.0 + 0.015 * c1;
  double deltaLKlsl = deltaL / (1.0);
  double deltaCkcsc = deltaC / (sc);
  double deltaHkhsh = deltaH / (sh);
  double i = deltaLKlsl * deltaLKlsl + deltaCkcsc * deltaCkcsc + deltaHkhsh * deltaHkhsh;
  return i < 0.0 ? 0.0 : glm::sqrt(i);
}

// http://colormine.org/delta-e-calculator/cie2000
double Cie2000Comparison(double* rgb_a, double* rgb_b)
{
  const double eps = 1e-5;

  double i_lab_a[3];
  //ColorConverter::RGBtoLAB(rgb_a, i_lab_a);
  ColorSpaces::RGBtoLAB(rgb_a, i_lab_a);
  LAB lab_a;
  lab_a.l = i_lab_a[0];
  lab_a.a = i_lab_a[1];
  lab_a.b = i_lab_a[2];

  double i_lab_b[3];
  //ColorConverter::RGBtoLAB(rgb_b, i_lab_b);
  ColorSpaces::RGBtoLAB(rgb_b, i_lab_b);
  LAB lab_b;
  lab_b.l = i_lab_b[0];
  lab_b.a = i_lab_b[1];
  lab_b.b = i_lab_b[2];

  // calculate ci, hi, i=1,2
  double c1 = sqrt((lab_a.a * lab_a.a) + (lab_a.b * lab_a.b));
  double c2 = sqrt((lab_b.a * lab_b.a) + (lab_b.b * lab_b.b));

  double meanC = (c1 + c2) / 2.0;
  double meanC7 = pow(meanC, 7);

  double g = 0.5 * (1 - sqrt(meanC7 / (meanC7 + 6103515625.))); // 0.5*(1-sqrt(meanC^7/(meanC^7+25^7)))
  double a1p = lab_a.a * (1.0 + g);
  double a2p = lab_b.a * (1.0 + g);

  c1 = sqrt((a1p * a1p) + (lab_a.b * lab_a.b));
  c2 = sqrt((a2p * a2p) + (lab_b.b * lab_b.b));
  double h1 = fmod(atan2(lab_a.b, a1p) + 2 * glm::pi<double>(), 2 * glm::pi<double>());
  double h2 = fmod(atan2(lab_b.b, a2p) + 2 * glm::pi<double>(), 2 * glm::pi<double>());

  // compute deltaL, deltaC, deltaH
  double deltaL = lab_b.l - lab_a.l;
  double deltaC = c2 - c1;
  double deltah;

  if (c1 * c2 < eps) {
    deltah = 0;
  }
  if (fabs(h2 - h1) <= glm::pi<double>()) {
    deltah = h2 - h1;
  }
  else if (h2 > h1) {
    deltah = h2 - h1 - 2 * glm::pi<double>();
  }
  else {
    deltah = h2 - h1 + 2 * glm::pi<double>();
  }

  double deltaH = 2 * sqrt(c1 * c2) * sin(deltah / 2);

  // calculate CIEDE2000
  double meanL = (lab_a.l + lab_b.l) / 2;
  meanC = (c1 + c2) / 2.0;
  meanC7 = pow(meanC, 7);
  double meanH;

  if (c1 * c2 < eps) {
    meanH = h1 + h2;
  }
  if (fabs(h1 - h2) <= glm::pi<double>() + eps) {
    meanH = (h1 + h2) / 2;
  }
  else if (h1 + h2 < 2 * glm::pi<double>()) {
    meanH = (h1 + h2 + 2 * glm::pi<double>()) / 2;
  }
  else {
    meanH = (h1 + h2 - 2 * glm::pi<double>()) / 2;
  }

  double T = 1
    - 0.17 * cos(meanH - DegToRad(30))
    + 0.24 * cos(2 * meanH)
    + 0.32 * cos(3 * meanH + DegToRad(6))
    - 0.2 *  cos(4 * meanH - DegToRad(63));
  double sl = 1 + (0.015 * pow(meanL - 50, 2)) / sqrt(20 + pow(meanL - 50, 2));
  double sc = 1 + 0.045 * meanC;
  double sh = 1 + 0.015 * meanC * T;
  double rc = 2 * sqrt(meanC7 / (meanC7 + 6103515625.));
  double rt = -sin(DegToRad(60 * exp(-pow((RadToDeg(meanH) - 275) / 25, 2)))) * rc;

  double stsrt = fabs(pow(deltaL / sl, 2) + pow(deltaC / sc, 2) + pow(deltaH / sh, 2) + rt * deltaC / sc * deltaH / sh);
  return sqrt(stsrt);


  /*
  * //https://github.com/berendeanicolae/ColorSpace/blob/master/src/Comparison.cpp
  const double eps = 1e-5;
		Lab lab_a;
		Lab lab_b;

		a->To<Lab>(&lab_a);
		b->To<Lab>(&lab_b);

		// calculate ci, hi, i=1,2
		double c1 = sqrt(SQR(lab_a.a) + SQR(lab_a.b));
		double c2 = sqrt(SQR(lab_b.a) + SQR(lab_b.b));
		double meanC = (c1 + c2) / 2.0;
		double meanC7 = POW7(meanC);

		double g = 0.5*(1 - sqrt(meanC7 / (meanC7 + 6103515625.))); // 0.5*(1-sqrt(meanC^7/(meanC^7+25^7)))
		double a1p = lab_a.a * (1 + g);
		double a2p = lab_b.a * (1 + g);

		c1 = sqrt(SQR(a1p) + SQR(lab_a.b));
		c2 = sqrt(SQR(a2p) + SQR(lab_b.b));
		double h1 = fmod(atan2(lab_a.b, a1p) + 2*M_PI, 2*M_PI);
		double h2 = fmod(atan2(lab_b.b, a2p) + 2*M_PI, 2*M_PI);

		// compute deltaL, deltaC, deltaH
		double deltaL = lab_b.l - lab_a.l;
		double deltaC = c2 - c1;
		double deltah;

		if (c1*c2 < eps) {
			deltah = 0;
		}
		if (std::abs(h2 - h1) <= M_PI) {
			deltah = h2 - h1;
		}
		else if (h2 > h1) {
			deltah = h2 - h1 - 2* M_PI;
		}
		else {
			deltah = h2 - h1 + 2 * M_PI;
		}

		double deltaH = 2 * sqrt(c1*c2)*sin(deltah / 2);

		// calculate CIEDE2000
		double meanL = (lab_a.l + lab_b.l) / 2;
		meanC = (c1 + c2) / 2.0;
		meanC7 = POW7(meanC);
		double meanH;

		if (c1*c2 < eps) {
			meanH = h1 + h2;
		}
		if (std::abs(h1 - h2) <= M_PI + eps) {
			meanH = (h1 + h2) / 2;
		}
		else if (h1 + h2 < 2*M_PI) {
			meanH = (h1 + h2 + 2*M_PI) / 2;
		}
		else {
			meanH = (h1 + h2 - 2*M_PI) / 2;
		}

		double T = 1
			- 0.17*cos(meanH - DegToRad(30))
			+ 0.24*cos(2 * meanH)
			+ 0.32*cos(3 * meanH + DegToRad(6))
			- 0.2*cos(4 * meanH - DegToRad(63));
		double sl = 1 + (0.015*SQR(meanL - 50)) / sqrt(20 + SQR(meanL - 50));
		double sc = 1 + 0.045*meanC;
		double sh = 1 + 0.015*meanC*T;
		double rc = 2 * sqrt(meanC7 / (meanC7 + 6103515625.));
		double rt = -sin(DegToRad(60 * exp(-SQR((RadToDeg(meanH) - 275) / 25)))) * rc;

		return sqrt(SQR(deltaL / sl) + SQR(deltaC / sc) + SQR(deltaH / sh) + rt * deltaC / sc * deltaH / sh);
  */
}