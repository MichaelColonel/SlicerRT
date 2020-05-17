/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLRoomsEyeViewNode_h
#define __vtkMRMLRoomsEyeViewNode_h

// Beams includes
#include "vtkSlicerPlmDrrModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLModelNode.h>


class vtkMRMLLinearTransformNode;
class vtkMRMLRTBeamNode;
class vtkMRMLMarkupsClosedCurveNode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLMarkupsLineNode;

/// \ingroup SlicerRt_QtModules_PlmDrr
class VTK_SLICER_PLMDRR_MODULE_MRML_EXPORT vtkMRMLPlmDrrNode : public vtkMRMLNode
{
public:
  static vtkMRMLPlmDrrNode *New();
  vtkTypeMacro(vtkMRMLPlmDrrNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create instance of a GAD node. 
  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs 
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format. 
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object 
  void Copy(vtkMRMLNode *node) override;

  /// Get unique node XML tag name (like Volume, Model) 
  const char* GetNodeTagName() override { return "PlmDrr"; };

public:
 
  /// Get beam node
  vtkMRMLRTBeamNode* GetBeamNode();
  /// Set and observe beam node
  void SetAndObserveBeamNode(vtkMRMLRTBeamNode* node);

  vtkGetMacro(IsocenterImagerDistance, double);
  vtkSetMacro(IsocenterImagerDistance, double);

  vtkGetVector2Macro( ImagerCenterOffset, double);
  vtkSetVector2Macro( ImagerCenterOffset, double);

  vtkGetVector2Macro( ImageDimention, int);
  vtkSetVector2Macro( ImageDimention, int);

  vtkGetVector2Macro( ImageSpacing, double);
  vtkSetVector2Macro( ImageSpacing, double);

  vtkGetVector2Macro( ImageCenter, int);
  vtkSetVector2Macro( ImageCenter, int);

  vtkGetVector4Macro( ImageWindow, int);
  vtkSetVector4Macro( ImageWindow, int);

  vtkGetMacro( RotateX, double);
  vtkSetMacro( RotateX, double);

  vtkGetMacro( RotateY, double);
  vtkSetMacro( RotateY, double);

  vtkGetMacro( RotateZ, double);
  vtkSetMacro( RotateZ, double);

protected:
  vtkMRMLPlmDrrNode();
  ~vtkMRMLPlmDrrNode();
  vtkMRMLPlmDrrNode(const vtkMRMLPlmDrrNode&);
  void operator=(const vtkMRMLPlmDrrNode&);

protected:
  double IsocenterImagerDistance; // fabs(SID - SAD)
  double ImagerCenterOffset[2]; // x,y
  int ImageDimention[2]; // columns, rows
  double ImageSpacing[2]; // x,y
  int ImageCenter[2]; // column, row
  int ImageWindow[4]; // column1, column2, row1, row2
  double RotateX;
  double RotateY;
  double RotateZ;
};

#endif
