/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkBMPWriter.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


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
#include "vtkBMPWriter.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkBMPWriter* vtkBMPWriter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkBMPWriter");
  if(ret)
    {
    return (vtkBMPWriter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkBMPWriter;
}




vtkBMPWriter::vtkBMPWriter()
{
  this->FileLowerLeft = 1;
}

void vtkBMPWriter::WriteFileHeader(ofstream *file, vtkImageData *cache)
{
  int min0, max0, min1, max1, min2, max2;
  long temp;
  int width, height, dataWidth;
  int row;
  
  // Find the length of the rows to write.
  cache->GetWholeExtent(min0, max0, min1, max1, min2, max2);
  width = (max0 - min0 + 1);
  height = (max1 - min1 + 1);
  
  dataWidth = ((width*3+3)/4)*4;

  // spit out the BMP header
  file->put((char)66);
  file->put((char)77);
  temp = (long)(dataWidth*height) + 54L;
  file->put((char)(temp%256));
  file->put((char)((temp%65536L)/256));
  file->put((char)(temp/65536L));
  for (row = 0; row < 5; row++)
    {
    file->put((char)0);
    }
  file->put((char)54);
  file->put((char)0);
  file->put((char)0);
  file->put((char)0);
  
  // info header
  file->put((char)40);
  file->put((char)0);
  file->put((char)0);
  file->put((char)0);
  
  file->put((char)(width%256));
  file->put((char)(width/256));
  file->put((char)0);
  file->put((char)0);
  
  file->put((char)(height%256));
  file->put((char)(height/256));
  file->put((char)0);
  file->put((char)0);
  
  file->put((char)1);
  file->put((char)0);
  file->put((char)24);
  for (row = 0; row < 25; row++)
    {
    file->put((char)0);
    }
}


void vtkBMPWriter::WriteFile(ofstream *file, vtkImageData *data,
			     int extent[6])
{
  int idx1, idx2;
  int rowLength, rowAdder, i; // in bytes
  unsigned char *ptr;
  int bpp;
  unsigned long count = 0;
  unsigned long target;
  float progress = this->Progress;
  float area;
  int *wExtent;
  
  bpp = data->GetNumberOfScalarComponents();
  
  // Make sure we actually have data.
  if ( !data->GetPointData()->GetScalars())
    {
    vtkErrorMacro(<< "Could not get data from input.");
    return;
    }

  // take into consideration the scalar type
  if (data->GetScalarType() != VTK_UNSIGNED_CHAR)
    {
    vtkErrorMacro("PNMWriter only accepts unsigned char scalars!");
    return; 
    }

  // Row length of x axis
  rowLength = extent[1] - extent[0] + 1;
  rowAdder = (4 - ((extent[1]-extent[0] + 1)*3)%4)%4;

  wExtent = this->GetInput()->GetWholeExtent();
  area = ((extent[5] - extent[4] + 1)*(extent[3] - extent[2] + 1)*
	  (extent[1] - extent[0] + 1)) / 
    ((wExtent[5] -wExtent[4] + 1)*(wExtent[3] -wExtent[2] + 1)*
     (wExtent[1] -wExtent[0] + 1));
    
  target = (unsigned long)((extent[5]-extent[4]+1)*
			   (extent[3]-extent[2]+1)/(50.0*area));
  target++;

  for (idx2 = extent[4]; idx2 <= extent[5]; ++idx2)
    {
    for (idx1 = extent[2]; idx1 <= extent[3]; idx1++)
      {
      if (!(count%target))
	{
	this->UpdateProgress(progress + count/(50.0*target));
	}
      count++;
      ptr = (unsigned char *)data->GetScalarPointer(extent[0], idx1, idx2);
      if (bpp == 1)
	{
	for (i = 0; i < rowLength; i++)
	  {
	  file->put(ptr[i]);
	  file->put(ptr[i]);
	  file->put(ptr[i]);
	  }
	}
      if (bpp == 2)
	{
	for (i = 0; i < rowLength; i++)
	  {
	  file->put(ptr[i*2]);
	  file->put(ptr[i*2]);
	  file->put(ptr[i*2]);
	  }
	}
      if (bpp == 3)
	{
	for (i = 0; i < rowLength; i++)
	  {
	  file->put(ptr[i*3 + 2]);
	  file->put(ptr[i*3 + 1]);
	  file->put(ptr[i*3]);
	  }
	}
      if (bpp == 4)
	{
	for (i = 0; i < rowLength; i++)
	  {
	  file->put(ptr[i*4 + 2]);
	  file->put(ptr[i*4 + 1]);
	  file->put(ptr[i*4]);
	  }
	}
      for (i = 0; i < rowAdder; i++)
	{
	file->put((char)0);
	}
      }
    }
}



