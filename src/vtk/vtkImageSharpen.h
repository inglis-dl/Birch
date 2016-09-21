/*=========================================================================

  Program:
  Module:    vtkImageSharpen.h
  Language:  C++

  Copyright (c) Dean Inglis
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.

=========================================================================*/

/**
 * @class vtkImageSharpen
 *
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Sharpen a 2D image.
 *
 * vtkImageSharpen is a simple image-image filter. It allocates output to
 * match the type and size of the input.  It sharpens the image by adding
 * a weighted difference of gaussians image to the original input: essentially
 * a variant implementation of unsharp masking wherein a blurred image is subtracted
 * from the input, but in this case, high frequency content is added back into
 * the input rather than having low frequency content subtracted out.
 * It then attempts to match the contrast characteristics of the input to the
 * output by histogram matching.
 *
 * The amount of sharpening is controlled by the Weight parameter which must
 * be a number >= 0.  The high frequency content is generated by a difference
 * of gaussians (DOG) which is controlled by the StandardDeviation and Radius
 * parameters (both must be > 0).  These last two parameters control the
 * weights and size of the discrete guassian convolution kernel.
 *
 * @see vtkSimpleImageToImageFilter vtkImageGaussianSmooth
 */

#ifndef __vtkImageSharpen_h
#define __vtkImageSharpen_h

#include <vtkSimpleImageToImageFilter.h>

class vtkImageSharpen : public vtkSimpleImageToImageFilter
{
  public:
    static vtkImageSharpen *New();
    vtkTypeMacro(vtkImageSharpen, vtkSimpleImageToImageFilter);
    void PrintSelf(ostream& os, vtkIndent indent);

    //@{
    /**
    * Set/Get the gaussian kernel radius. Default 1.
    * @param Radius
    */
    vtkSetMacro(Radius, double);
    vtkGetMacro(Radius, double);
    //@}

    //@{
    /**
    * Set/Get the standard deviation of the gaussian kernel. Default 2.
    * @param StandardDeviation
    */
    vtkSetMacro(StandardDeviation, double);
    vtkGetMacro(StandardDeviation, double);
    //@}

    //@{
    /**
    * Set/Get the weight for adding DOG to the input.  Default 20.
    * @param Weight
    */
    vtkSetMacro(Weight, double);
    vtkGetMacro(Weight, double);
    //@}

  protected:
    vtkImageSharpen();
    ~vtkImageSharpen() {}

    double Radius;
    double StandardDeviation;
    double Weight;

    virtual void SimpleExecute(vtkImageData* input, vtkImageData* output);

    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *);

  private:
    vtkImageSharpen(const vtkImageSharpen&);  /** Not implemented. */
    void operator=(const vtkImageSharpen&);  /** Not implemented. */
};

#endif
