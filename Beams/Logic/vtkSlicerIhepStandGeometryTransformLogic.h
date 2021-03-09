/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Vinith Suriyakumar and Csaba Pinter,
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkSlicerIhepStandGeometryTransformLogic_h
#define __vtkSlicerIhepStandGeometryTransformLogic_h

#include "vtkSlicerBeamsModuleLogicExport.h"

// Slicer includes
#include "vtkMRMLAbstractLogic.h"

// STD includes
#include <map>
#include <vector>
#include <list>

class vtkGeneralTransform;
class vtkMRMLRTBeamNode;
class vtkMRMLLinearTransformNode;

/// \ingroup SlicerRt_QtModules_Beams
/// \brief Logic representing the IEC standard coordinate systems and transforms.
///
/// The IEC standard describes coordinate systems and a transform hierarchy to
/// represent objects taking part in an external beam radiation therapy delivery in 3D space.
/// With this logic class it is possible to get a transform from any defined coordinate
/// system to another by simply inputting the coordinate systems. The logic can observe an
/// RT beam node to get the geometrical parameters defining the state of the objects involved.
/// Image describing these coordinate frames:
/// http://perk.cs.queensu.ca/sites/perkd7.cs.queensu.ca/files/Project/IEC_Transformations.PNG
///

/*
                          "IEC 61217:2011 Hierarchy"

                   -------------------("f")---------------------
                   |                    |                      |
        ---------("g")                ("i")                  ("s")
        |          |                    |                      |
      ("r")      ("b")                ("o")                  ("e")
                   |                                           |
                 ("w")                                       ("t")
                                                               |
                                                             ("p")

Legend:
  ("f") - Fixed reference system
  ("g") - GANTRY coordinate system
  ("b") - BEAM LIMITING DEVICE or DELINEATOR coordinate system
  ("w") - WEDGE FILTER coordinate system
  ("r") - X-RAY IMAGE RECEPTOR coordinate system
  ("s") - PATIENT SUPPORT coordinate system
  ("e") - Table top eccentric rotation coordinate system
  ("t") - Table top coordinate system
  ("p") - PATIENT coordinate system
  ("i") - Imager coordinate system
  ("o") - Focus coordinate system
*/
/*
 IEC Patient to DICOM Patient transformation:
     Counter clockwise rotation around X-axis, angle = -90 

                       1 0  0 
     Rotation Matrix = 0 0 -1
                       0 1  0

 IEC Patient to RAS Patient transformation:
     Counter clockwise rotation around X-axis, angle = -90 
     Clockwise rotation around Z-axis, angle = 180 

                       -1 0 0 
     Rotation Matrix =  0 0 1
                        0 1 0
*/

class VTK_SLICER_BEAMS_LOGIC_EXPORT vtkSlicerIhepStandGeometryTransformLogic : public vtkMRMLAbstractLogic
{
public:
  enum CoordinateSystemIdentifier
  {
    RAS = 0,
    FixedReference,
    Collimator,
    PatientSupport, // Rotation of patient support
    TableTopInferiorSuperiorMovement, // Inferior-Superior movement of the table top
    TableTop, // Rotation and Vectical movement of table top
    Patient,
    LastIECCoordinateFrame // Last index used for adding more coordinate systems externally
  };
  typedef std::list< CoordinateSystemIdentifier > CoordinateSystemsList;

public:
  static vtkSlicerIhepStandGeometryTransformLogic *New();
  vtkTypeMacro(vtkSlicerIhepStandGeometryTransformLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkSlicerIhepStandGeometryTransformLogic();
  ~vtkSlicerIhepStandGeometryTransformLogic() override;

private:
  vtkSlicerIhepStandGeometryTransformLogic(const vtkSlicerIhepStandGeometryTransformLogic&) = delete;
  void operator=(const vtkSlicerIhepStandGeometryTransformLogic&) = delete;
};

#endif
