/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkImageThreshold.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkImageThreshold -  Flexible threshold
// .SECTION Description
// vtkImageThreshold Can do binary or continous thresholding for lower, upper
// or a range of data.  The output data type may be different than the
// output, but defaults to the same type.


#ifndef __vtkImageThreshold_h
#define __vtkImageThreshold_h


#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImageThreshold : public vtkImageToImageFilter
{
public:
  static vtkImageThreshold *New();
  vtkTypeMacro(vtkImageThreshold,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // The values greater than or equal to the value match.
  void ThresholdByUpper(float thresh);
  
  // Description:
  // The values less than or equal to the value match.
  void ThresholdByLower(float thresh);
  
  // Description:
  // The values in a range (inclusive) match
  void ThresholdBetween(float lower, float upper);
  
  // Description:
  // Determines whether to replace the pixel in range with InValue
  vtkSetMacro(ReplaceIn, int);
  vtkGetMacro(ReplaceIn, int);
  vtkBooleanMacro(ReplaceIn, int);
  
  // Description:
  // Replace the in range pixels with this value.
  void SetInValue(float val);
  vtkGetMacro(InValue, float);
  
  // Description:
  // Determines whether to replace the pixel out of range with OutValue
  vtkSetMacro(ReplaceOut, int);
  vtkGetMacro(ReplaceOut, int);
  vtkBooleanMacro(ReplaceOut, int);

  // Description:
  // Replace the in range pixels with this value.
  void SetOutValue(float val);
  vtkGetMacro(OutValue, float);
  
  // Description:
  // Get the Upper and Lower thresholds.
  vtkGetMacro(UpperThreshold, float);
  vtkGetMacro(LowerThreshold, float);
  
  // Description:
  // Set the desired output scalar type to cast to
  vtkSetMacro(OutputScalarType, int);
  vtkGetMacro(OutputScalarType, int);
  void SetOutputScalarTypeToDouble()
    {this->SetOutputScalarType(VTK_DOUBLE);}
  void SetOutputScalarTypeToFloat()
    {this->SetOutputScalarType(VTK_FLOAT);}
  void SetOutputScalarTypeToLong()
    {this->SetOutputScalarType(VTK_LONG);}
  void SetOutputScalarTypeToUnsignedLong()
    {this->SetOutputScalarType(VTK_UNSIGNED_LONG);};
  void SetOutputScalarTypeToInt()
    {this->SetOutputScalarType(VTK_INT);}
  void SetOutputScalarTypeToUnsignedInt()
    {this->SetOutputScalarType(VTK_UNSIGNED_INT);}
  void SetOutputScalarTypeToShort()
    {this->SetOutputScalarType(VTK_SHORT);}
  void SetOutputScalarTypeToUnsignedShort()
    {this->SetOutputScalarType(VTK_UNSIGNED_SHORT);}
  void SetOutputScalarTypeToChar()
    {this->SetOutputScalarType(VTK_CHAR);}
  void SetOutputScalarTypeToUnsignedChar()
    {this->SetOutputScalarType(VTK_UNSIGNED_CHAR);}
  
protected:
  vtkImageThreshold();
  ~vtkImageThreshold() {};
  vtkImageThreshold(const vtkImageThreshold&) {};
  void operator=(const vtkImageThreshold&) {};

  float UpperThreshold;
  float LowerThreshold;
  int ReplaceIn;
  float InValue;
  int ReplaceOut;
  float OutValue;
  
  int OutputScalarType;

  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
		       int extent[6], int id);
};

#endif



