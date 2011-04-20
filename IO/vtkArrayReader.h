/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkArrayReader.h
  
-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkArrayReader -  Reads sparse and dense vtkArray data written by vtkArrayWriter.
//
// .SECTION Description
// Reads sparse and dense vtkArray data written with vtkArrayWriter.
//
// Outputs:
//   Output port 0: vtkArrayData containing a dense or sparse array.
//
// .SECTION See Also
// vtkArrayWriter
//
// .SECTION Thanks
// Developed by Timothy M. Shead (tshead@sandia.gov) at Sandia National Laboratories.

#ifndef __vtkArrayReader_h
#define __vtkArrayReader_h

#include "vtkArrayDataAlgorithm.h"

class VTK_IO_EXPORT vtkArrayReader :
  public vtkArrayDataAlgorithm
{
public:
  static vtkArrayReader* New();
  vtkTypeMacro(vtkArrayReader, vtkArrayDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the filesystem location from which data will be read.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  // Description:
  // Read an arbitrary array from a stream.  Note: you MUST always
  // open streams in binary mode to prevent problems reading files
  // on Windows.
  static vtkArray* Read(istream& stream);

  // Description:
  // Read an arbitrary array from a character buffer.
  static vtkArray* Read(const char* buffer);

  // Description:
  // Read an arbitrary array from a character buffer with specified length.
  static vtkArray* Read(const char* buffer, int length);

protected:
  vtkArrayReader();
  ~vtkArrayReader();

  int RequestData(
    vtkInformation*, 
    vtkInformationVector**, 
    vtkInformationVector*);

  char* FileName;

private:
  vtkArrayReader(const vtkArrayReader&); // Not implemented
  void operator=(const vtkArrayReader&);   // Not implemented
};

#endif

