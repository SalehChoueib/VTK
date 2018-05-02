//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2012 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================
#include "vtkmGradient.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

#include "vtkmlib/ArrayConverters.h"
#include "vtkmlib/DataSetConverters.h"
#include "vtkmlib/PolyDataConverter.h"
#include "vtkmlib/Storage.h"

#include "vtkmCellSetExplicit.h"
#include "vtkmCellSetSingleType.h"
#include "vtkmFilterPolicy.h"

#include <vtkm/filter/Gradient.h>
#include <vtkm/filter/PointAverage.h>

vtkStandardNewMacro(vtkmGradient)

namespace
{
struct GradientTypes
    : vtkm::ListTagBase<
                        vtkm::Float32,
                        vtkm::Float64,
                        vtkm::Vec<vtkm::Float32,3>,
                        vtkm::Vec<vtkm::Float64,3>,
                        vtkm::Vec< vtkm::Vec<vtkm::Float32,3>, 3>,
                        vtkm::Vec< vtkm::Vec<vtkm::Float64,3>, 3>
                        >
{
};

//------------------------------------------------------------------------------
class vtkmGradientFilterPolicy
      : public vtkm::filter::PolicyBase<vtkmGradientFilterPolicy>
  {
  public:
    typedef GradientTypes FieldTypeList;
    typedef tovtkm::TypeListTagVTMOut FieldStorageList;

    typedef tovtkm::CellListStructuredInVTK StructuredCellSetList;
    typedef tovtkm::CellListUnstructuredInVTK UnstructuredCellSetList;
    typedef tovtkm::CellListAllInVTK AllCellSetList;

    typedef vtkm::TypeListTagFieldVec3 CoordinateTypeList;
    typedef tovtkm::PointListInVTK CoordinateStorageList;

    typedef vtkm::filter::PolicyDefault::DeviceAdapterList DeviceAdapterList;
  };

vtkm::cont::DataSet CopyDataSetStructure(const vtkm::cont::DataSet& ds)
{
  vtkm::cont::DataSet cp;
  for (vtkm::IdComponent i = 0; i < ds.GetNumberOfCoordinateSystems(); ++i)
  {
    cp.AddCoordinateSystem(ds.GetCoordinateSystem(i));
  }
  for (vtkm::IdComponent i = 0; i < ds.GetNumberOfCellSets(); ++i)
  {
    cp.AddCellSet(ds.GetCellSet(i));
  }
  return cp;
}

} // anonymous namespace

//------------------------------------------------------------------------------
vtkmGradient::vtkmGradient()
{
}

//------------------------------------------------------------------------------
vtkmGradient::~vtkmGradient()
{
}

//------------------------------------------------------------------------------
void vtkmGradient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
int vtkmGradient::RequestData(vtkInformation* request,
                              vtkInformationVector** inputVector,
                              vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkDataSet* input =
      vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDataSet* output =
      vtkDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->ShallowCopy(input);

  // grab the input array to process to determine the field want to compute
  // the gradient for
  int association = this->GetInputArrayAssociation(0, inputVector);
  vtkDataArray* inputArray = this->GetInputArrayToProcess(0, inputVector);
  if (inputArray == nullptr || inputArray->GetName() == nullptr ||
      inputArray->GetName()[0] == '\0')
  {
    vtkErrorMacro("Invalid input array.");
    return 0;
  }

  try
  {
    // convert the input dataset to a vtkm::cont::DataSet. We explicitly drop
    // all arrays from the conversion as this algorithm doesn't change topology
    // and therefore doesn't need input fields converted through the VTK-m filter
    auto in = tovtkm::Convert(input, tovtkm::FieldsFlag::None);
    vtkm::cont::Field field = tovtkm::Convert(inputArray, association);
    in.AddField(field);

    const bool fieldIsPoint = field.GetAssociation() == vtkm::cont::Field::ASSOC_POINTS;
    const bool fieldIsCell = field.GetAssociation() == vtkm::cont::Field::ASSOC_CELL_SET;
    const bool fieldIsVec = (inputArray->GetNumberOfComponents() == 3);
    const bool fieldIsScalar = inputArray->GetDataType() == VTK_FLOAT ||
                              inputArray->GetDataType() == VTK_DOUBLE;
    const bool fieldValid = (fieldIsPoint || fieldIsCell) &&
                            fieldIsScalar &&
                            (field.GetName() != std::string());

    if (!fieldValid)
    {
      vtkWarningMacro(<< "Unsupported field type\n"
                      << "Falling back to vtkGradientFilter.");
      return this->Superclass::RequestData(request, inputVector, outputVector);
    }

    vtkmGradientFilterPolicy policy;
    auto passNoFields =
      vtkm::filter::FieldSelection(vtkm::filter::FieldSelection::MODE_NONE);
    vtkm::filter::Gradient filter;
    filter.SetFieldsToPass(passNoFields);
    filter.SetColumnMajorOrdering();

    if (fieldIsVec)
    { //this properties are only valid when processing a vec<3> field
      filter.SetComputeDivergence(this->ComputeDivergence != 0);
      filter.SetComputeVorticity(this->ComputeVorticity != 0);
      filter.SetComputeQCriterion(this->ComputeQCriterion != 0);
    }

    if (this->ResultArrayName)
    {
      filter.SetOutputFieldName(this->ResultArrayName);
    }

    if (this->DivergenceArrayName)
    {
      filter.SetDivergenceName(this->DivergenceArrayName);
    }

    if (this->VorticityArrayName)
    {
      filter.SetVorticityName(this->VorticityArrayName);
    }

    if (this->QCriterionArrayName)
    {
      filter.SetQCriterionName(this->QCriterionArrayName);
    }
    else
    {
      filter.SetQCriterionName("Q-criterion");
    }

    // Run the VTK-m Gradient Filter
    // -----------------------------
    vtkm::cont::DataSet result;
    if (fieldIsPoint)
    {
      filter.SetComputePointGradient(!this->FasterApproximation);
      filter.SetActiveField(field.GetName(), vtkm::cont::Field::ASSOC_POINTS);
      result = filter.Execute(in, policy);

      //When we have faster approximation enabled the VTK-m gradient will output
      //a cell field not a point field. So at that point we will need to convert
      //back to a point field
      if (this->FasterApproximation)
      {
        vtkm::filter::PointAverage cellToPoint;
        cellToPoint.SetFieldsToPass(passNoFields);

        auto c2pIn = result;
        result = CopyDataSetStructure(result);

        if (this->ComputeGradient)
        {
          cellToPoint.SetActiveField(
            field.GetName(), vtkm::cont::Field::ASSOC_CELL_SET);
          auto ds = cellToPoint.Execute(c2pIn, policy);
          result.AddField(ds.GetField(0));
        }
        if (this->ComputeDivergence && fieldIsVec)
        {
          cellToPoint.SetActiveField(
            filter.GetDivergenceName(), vtkm::cont::Field::ASSOC_CELL_SET);
          auto ds = cellToPoint.Execute(c2pIn, policy);
          result.AddField(ds.GetField(0));
        }
        if (this->ComputeVorticity && fieldIsVec)
        {
          cellToPoint.SetActiveField(filter.GetVorticityName(),
            vtkm::cont::Field::ASSOC_CELL_SET);
          auto ds = cellToPoint.Execute(c2pIn, policy);
          result.AddField(ds.GetField(0));
        }
        if (this->ComputeQCriterion && fieldIsVec)
        {
          cellToPoint.SetActiveField(filter.GetQCriterionName(),
            vtkm::cont::Field::ASSOC_CELL_SET);
          auto ds = cellToPoint.Execute(c2pIn, policy);
          result.AddField(ds.GetField(0));
        }
      }
    }
    else
    {
      //we need to convert the field to be a point field
      vtkm::filter::PointAverage cellToPoint;
      cellToPoint.SetFieldsToPass(passNoFields);
      cellToPoint.SetActiveField(field.GetName(), field.GetAssociation());
      cellToPoint.SetOutputFieldName(field.GetName());
      in = cellToPoint.Execute(in, policy);

      filter.SetComputePointGradient(false);
      filter.SetActiveField(field.GetName(), vtkm::cont::Field::ASSOC_POINTS);
      result = filter.Execute(in, policy);
    }

    // convert arrays back to VTK
    if (!fromvtkm::ConvertArrays(result, output))
    {
      vtkErrorMacro(<< "Unable to convert VTKm DataSet back to VTK");
      return 0;
    }
  }
  catch (const vtkm::cont::Error& e)
  {
    vtkWarningMacro(<< "VTK-m error: " << e.GetMessage()
                    << "Falling back to serial implementation.");
    return this->Superclass::RequestData(request, inputVector, outputVector);
  }

  return 1;
}
