vtk_module(vtkFiltersReebGraph
  DEPENDS
    vtkFiltersCore
  TEST_DEPENDS
    vtkRendering${VTK_RENDERING_BACKEND}
    vtkTestingRendering
    vtkInteractionStyle
  )
