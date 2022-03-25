/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLIsodoseDisplayNode_h
#define __vtkMRMLIsodoseDisplayNode_h

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include "vtkSlicerIsodoseModuleMRMLExport.h"

class VTK_SLICER_ISODOSE_MODULE_MRML_EXPORT vtkMRMLIsodoseDisplayNode : public vtkMRMLDisplayNode
{
public:

  static vtkMRMLIsodoseDisplayNode *New();
  vtkTypeMacro(vtkMRMLIsodoseDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name
  const char* GetNodeTagName() override { return "IsodoseDisplay"; }

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

protected:
  vtkMRMLIsodoseDisplayNode();
  ~vtkMRMLIsodoseDisplayNode() override;
  vtkMRMLIsodoseDisplayNode(const vtkMRMLIsodoseDisplayNode&);
  void operator=(const vtkMRMLIsodoseDisplayNode&);
};

#endif
