#include <vtkTransform.h>
// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLMarkupsNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>


#include "vtkMRMLTestMe2Node.h"

vtkMRMLNodeNewMacro(vtkMRMLTestMe2Node);

vtkMRMLTestMe2Node::vtkMRMLTestMe2Node()
  : FiducialNode(nullptr)
  , TransformNode(nullptr)
  , Height(0.0)
{
}

//----------------------------------------------------------------------------
vtkMRMLTestMe2Node::~vtkMRMLTestMe2Node()
{
  this->SetAndObserveFiducialNode(nullptr);
  this->SetAndObserveTransformNode(nullptr);
}
/*
void vtkMRMLTestMe2Node::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
//  vtkMRMLWriteXMLBeginMacro(of);


  // add new parameters here
//  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTestMe2Node::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

 // vtkMRMLReadXMLBeginMacro(atts);



  // add new parameters here
 // vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLTestMe2Node::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLTestMe2Node* node = vtkMRMLTestMe2Node::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();

//  vtkMRMLCopyBeginMacro(node);

  // add new parameters here
//  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLTestMe2Node::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/

/*)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLTestMe2Node* node = vtkMRMLTestMe2Node::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

//  vtkMRMLCopyBeginMacro(node);

  // add new parameters here
//  vtkMRMLCopyEndMacro();
}
*/

void vtkMRMLTestMe2Node::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Height: " << this->Height << "\n";
  os << indent << "FiducialNode: " << (this->FiducialNode ? this->FiducialNode->GetName() : "null") << "\n";
  os << indent << "TransformNode: " << (this->TransformNode ? this->TransformNode->GetName() : "null") << "\n";
/*
  vtkMRMLPrintBeginMacro(os, indent);

  // add new parameters here
  vtkMRMLPrintEndMacro();
  */
}

void vtkMRMLTestMe2Node::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, eventID, callData);

  if (!this->Scene)
  {
    vtkErrorMacro("ProcessMRMLEvents: Invalid MRML scene");
    return;
  }
  if (this->Scene->IsBatchProcessing())
  {
    return;
  }

  // Update the DRR View-Up and normal vectors, if beam geometry or transform was changed
  switch (eventID)
  {
//  case vtkMRMLRTBeamNode::BeamGeometryModified:
//  case vtkMRMLRTBeamNode::BeamTransformModified:
//    this->Modified();
//    break;
  default:
    break;
  }
}
/*
vtkMRMLMarkupsFiducialNode* vtkMRMLTestMe2Node::GetFiducialNode()
{
  return this->FiducialNode;
}
vtkMRMLLinearTransformNode* vtkMRMLTestMe2Node::GetTransformNode()
{
  return this->TransformNode;
}
*/

void vtkMRMLTestMe2Node::SetAndObserveFiducialNode(vtkMRMLMarkupsFiducialNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->FiducialNode = node;

  if (this->FiducialNode)
  {
    int cp = this->FiducialNode->AddControlPoint(0,0,0);
    this->FiducialNode->SetNthControlPointLabel( cp, "Point_F");
  }

  //
  if (this->FiducialNode && this->TransformNode)
  {
    this->FiducialNode->SetAndObserveTransformNodeID(this->TransformNode->GetID());

  }

  this->Modified();
//  this->SetNodeReferenceID(PATIENT_BODY_SEGMENTATION_REFERENCE_ROLE, (node ? node->GetID() : nullptr)); TODO
}

void vtkMRMLTestMe2Node::SetAndObserveTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->TransformNode = node;
  if (this->FiducialNode && this->TransformNode)
  {
    this->FiducialNode->SetAndObserveTransformNodeID(this->TransformNode->GetID());

  }
  this->Modified();
}

void vtkMRMLTestMe2Node::createControlPoint()
{
    vtkVector3d point(0.0, 0.0, 0.0);
    this->FiducialNode->AddControlPoint(point, "Point_F");
}
