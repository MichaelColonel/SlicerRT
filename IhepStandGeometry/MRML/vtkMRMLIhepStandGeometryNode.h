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

#ifndef __vtkMRMLIhepStandGeometryNode_h
#define __vtkMRMLIhepStandGeometryNode_h

// Beams includes
#include "vtkSlicerIhepStandGeometryModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>

class vtkMRMLLinearTransformNode;
class vtkMRMLRTBeamNode;
class vtkMRMLMarkupsClosedCurveNode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLMarkupsLineNode;

/// \ingroup SlicerRt_QtModules_IhepStandGeometry

class VTK_SLICER_IHEPSTANDGEOMETRY_MODULE_MRML_EXPORT vtkMRMLIhepStandGeometryNode : public vtkMRMLNode
{
public:

  static vtkMRMLIhepStandGeometryNode *New();
  vtkTypeMacro(vtkMRMLIhepStandGeometryNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create instance of a GAD node. 
  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs 
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format. 
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object 
  void Copy(vtkMRMLNode *node) override;

  /// Copy node content (excludes basic data, such a name and node reference)
  vtkMRMLCopyContentMacro(vtkMRMLIhepStandGeometryNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "IhepStandGeometry"; };

public:
  /// Get beam node
  vtkMRMLRTBeamNode* GetBeamNode();
  /// Set and observe beam node. This updates Normal and View-Up vectors.
  void SetAndObserveBeamNode(vtkMRMLRTBeamNode* node);

  /// Get/Set patient support rotation angle
  vtkSetMacro(PatientSupportRotationAngle, double);
  vtkGetMacro(PatientSupportRotationAngle, double);

protected:
  vtkMRMLIhepStandGeometryNode();
  ~vtkMRMLIhepStandGeometryNode();
  vtkMRMLIhepStandGeometryNode(const vtkMRMLIhepStandGeometryNode&);
  void operator=(const vtkMRMLIhepStandGeometryNode&);

protected:
  double PatientSupportRotationAngle;
};

#endif
