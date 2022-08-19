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

#ifndef __vtkMRMLIhepMlcControlNode_h
#define __vtkMRMLIhepMlcControlNode_h

// Beams includes
#include "vtkSlicerIhepMlcControlModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLModelNode.h>

class vtkMRMLRTBeamNode;
class vtkMRMLTableNode;

class VTK_SLICER_IHEPMLCCONTROL_MODULE_MRML_EXPORT vtkMRMLIhepMlcControlNode : public vtkMRMLNode
{
public:
  static vtkMRMLIhepMlcControlNode *New();
  vtkTypeMacro(vtkMRMLIhepMlcControlNode,vtkMRMLNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLIhepMlcControlNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "IhepMlcControl"; };

  /// Handles events registered in the observer manager
  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

public:
  /// Get beam node
  vtkMRMLRTBeamNode* GetBeamNode();
  /// Set and observe beam node. This updates Normal and View-Up vectors.
  void SetAndObserveBeamNode(vtkMRMLRTBeamNode* node);

  /// Get table node
  vtkMRMLTableNode* GetTableNode();
  /// Set and observe table node.   
  void SetAndObserveTableNode(vtkMRMLTableNode* node);

protected:
  vtkMRMLIhepMlcControlNode();
  ~vtkMRMLIhepMlcControlNode();
  vtkMRMLIhepMlcControlNode(const vtkMRMLIhepMlcControlNode&);
  void operator=(const vtkMRMLIhepMlcControlNode&);
};

#endif
