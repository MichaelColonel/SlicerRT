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

#ifndef __vtkMRMLPlmDrrNode_h
#define __vtkMRMLPlmDrrNode_h

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
  enum AlgorithmReconstuctionType { EXACT, UNIFORM };
  enum HounsfieldUnitsConversionType { PREPROCESS, INLINE, NONE };
  enum ThreadingType { CPU, CUDA, OPENCL };

  static vtkMRMLPlmDrrNode *New();
  vtkTypeMacro(vtkMRMLPlmDrrNode,vtkMRMLNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLRTPlanNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "PlmDrr"; };

  void GetRTImagePosition(double position[2]);

public:
 
  /// Get beam node
  vtkMRMLRTBeamNode* GetBeamNode();
  /// Set and observe beam node
  void SetAndObserveBeamNode(vtkMRMLRTBeamNode* node);

  vtkGetMacro(AlgorithmReconstuction, AlgorithmReconstuctionType);
  vtkSetMacro(AlgorithmReconstuction, AlgorithmReconstuctionType);

  vtkGetMacro(HUConversion, HounsfieldUnitsConversionType);
  vtkSetMacro(HUConversion, HounsfieldUnitsConversionType);

  vtkGetMacro(Threading, ThreadingType);
  vtkSetMacro(Threading, ThreadingType);

  vtkGetMacro(ExponentialMappingFlag, bool);
  vtkSetMacro(ExponentialMappingFlag, bool);

  vtkGetMacro(AutoscaleFlag, bool);
  vtkSetMacro(AutoscaleFlag, bool);

  vtkGetVector2Macro(AutoscaleRange, int);
  vtkSetVector2Macro(AutoscaleRange, int);

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
  int ImageCenter[2]; // column, row (calculated from imager offset and image data)
  int ImageWindow[4]; // column1, column2, row1, row2 (y0, y1, x0, x1)
  double RotateX; // not used
  double RotateY; // not used
  double RotateZ; // deg
  AlgorithmReconstuctionType AlgorithmReconstuction;
  HounsfieldUnitsConversionType HUConversion;
  ThreadingType Threading;
  bool ExponentialMappingFlag;
  bool AutoscaleFlag;
  int AutoscaleRange[2];
};

#endif
