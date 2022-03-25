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

#include "vtkMRMLIsodoseDisplayNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLIsodoseDisplayNode);

//-----------------------------------------------------------------------------
vtkMRMLIsodoseDisplayNode::vtkMRMLIsodoseDisplayNode()
{
}

//-----------------------------------------------------------------------------
vtkMRMLIsodoseDisplayNode::~vtkMRMLIsodoseDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLIsodoseDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMRMLIsodoseDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLIsodoseDisplayNode::ReadXMLAttributes(const char** atts)
{
  // Read all MRML node attributes
  this->Superclass::ReadXMLAttributes(atts);
}
