## Evaluation Possibilities

Any class derived from `BaseVolumeRenderer` can make use of the built-in support for evaluating the parameters of an algorithm. To do so, one has to override the virtual function `FillParameterSpace` like this in the header file:

```c++
virtual void FillParameterSpace(ParameterSpace& pspace) override;
```

The implementation then needs to add the parameters that shall be evaluated. In the following example, two parameters are added:

* the number of ambient occlusion shells `ambient_occlusion_shells`, to be evaluated in the range \[1, 20\] with a stepping of 1. This is an integer parameter.

* the radius for ambient occlusion `ambient_occlusion_radius`, to be evaluated in the range \[0.1, 1.5\] with a stepping of 0.1. This is a float parameter.

The code will interpret this as a 2-dimensional parameter space and evaluate all parameter combinations. In this example, that is $20 \times 15 = 300$ parameter combinations.

```c++
void RC1PExtinctionBasedShading::FillParameterSpace(ParameterSpace& pspace)
{
  pspace.ClearParameterDimensions();
  pspace.AddParameterDimension(new ParameterRangeInt("AmbientOccShells", &ambient_occlusion_shells, 1, 20, 1));
  pspace.AddParameterDimension(new ParameterRangeFloat("AmbientOccRadius", &ambient_occlusion_radius, 0.1f, 1.5f, 0.1f));
}
```

The user is able to start the evaluation from the GUI by pressing the "Start Evaluation" button in the "Rendering Manager". The code in `BaseVolumeRenderer` will then measure the frames per second and take a snapshot for every parameter combination. The results are stored in a newly created subfolder of the current folder (probably under 'data'). It is named according to the scheme 'eval_DATE_TIME'. This folder contains a file 'eval.csv' with the measurements and a subfolder 'img' with the images.

#### Plotting Performance

Assume that 'eval.csv' looks like this:

|StepSize|TimePerFrame (ms)|FramesPerSecond|ImageFile|
|--------|-----------------|---------------|---------|
|0.200000|18.950000        |52.770449      |0000.png |
|0.300000|13.610000        |73.475386      |0001.png |
|0.400000|10.870000        |91.996320      |0002.png |
|0.500000|9.220000         |108.459870     |0003.png |
|0.600000|8.030000         |124.533001     |0004.png |
|0.700000|7.310000         |136.798906     |0005.png |
|0.800000|6.640000         |150.602410     |0006.png |
|0.900000|6.170000         |162.074554     |0007.png |
|1.000000|5.760000         |173.611111     |0008.png |
|1.100000|5.470000         |182.815356     |0009.png |
|1.200000|5.200000         |192.307692     |0010.png |
|1.300000|5.010000         |199.600798     |0011.png |
|1.400000|4.720000         |211.864407     |0012.png |
|1.500000|4.600000         |217.391304     |0013.png |
|1.600000|4.460000         |224.215247     |0014.png |
|1.700000|4.270000         |234.192037     |0015.png |
|1.800000|4.250000         |235.294118     |0016.png |
|1.900000|4.120000         |242.718447     |0017.png |

It can be plotted like this:

```py
from matplotlib import pyplot as plt
import pandas as pd
import numpy as np

d = pd.read_csv('eval.csv', quotechar='"')

plt.plot(d['StepSize'], d['FramesPerSecond'], linestyle='dotted', marker='o')
plt.title('Performance')
plt.ylabel('Frames per Second')
plt.xlabel('Step Size')

plt.show()
```

#### Computing Image Differences

The difference between the images can be assessed using different metrics. One example is the Structural Similarity Index Measure (SSIM). A simple call to [ImageMagick Compare](https://imagemagick.org/script/compare.php) suffices to measure this:

```console
magick compare -metric SSIM 0000.png 0001.png cmp_0000_0001.png
```

This also creates a comparison image where the differences are highlighted. Note that a number of [different metrics](https://imagemagick.org/script/command-line-options.php#metric) are supported and choosing the right one depends on the application. Some of the metrics also have parameters that may need attention.
