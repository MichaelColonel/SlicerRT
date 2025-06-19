
// Qt includes
#include <QDebug>

#include <vtkTransform.h>
// MRML includes
#include <vtkMRMLScene.h>
//#include <vtkMRMLScalarVolumeNode.h>
//#include <vtkMRMLMarkupsNode.h>

// VTK includes
//#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>


#include "vtkMRMLTestMe2Node.h"

//------------------------------------------------------------------------------
namespace
{

static const char* FIDUCIAL_NODE_REFERENCE_ROLE = "fiducialNodeRef";
static const char* TRANSFORM_NODE_REFERENCE_ROLE = "transformNodeRef";

}

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTestMe2Node);

vtkMRMLTestMe2Node::vtkMRMLTestMe2Node()
//  : FiducialNode(nullptr)
//  , TransformNode(nullptr)
  : Height(0.0)
  , RotateXAngle(0.0)
{
}

//----------------------------------------------------------------------------
vtkMRMLTestMe2Node::~vtkMRMLTestMe2Node()
{
  this->SetAndObserveFiducialNode(nullptr);
  this->SetAndObserveTransformNode(nullptr);
}

void vtkMRMLTestMe2Node::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);

  vtkMRMLWriteXMLFloatMacro(height, Height);
  vtkMRMLWriteXMLFloatMacro(rotateXAngle, RotateXAngle);
 // if (this->GetFiducialNode())
//    of << " fiducialNodeRef=\"" << this->GetFiducialNode()->GetID() << "\"";

//  if (this->GetTransformNode())
 //   of << " transformNodeRef=\"" << this->GetTransformNode()->GetID() << "\"";

  // add new parameters here
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTestMe2Node::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);


  vtkMRMLReadXMLFloatMacro(height, Height);
  vtkMRMLReadXMLFloatMacro(rotateXAngle, RotateXAngle);

//  if (!strcmp(xmlReadAttName, "fiducialNodeRef")) {
 //     this->SetNodeReferenceID("fiducialNodeRef", xmlReadAttValue);
//    }
 // else if (!strcmp(xmlReadAttName, "transformNodeRef")) {
 //     this->SetNodeReferenceID("transformNodeRef", xmlReadAttValue);
//    }

  // add new parameters here
  vtkMRMLReadXMLEndMacro();

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

  this->DisableModifiedEventOn();

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyFloatMacro(Height);
  vtkMRMLCopyFloatMacro(RotateXAngle);
 // this->SetAndObserveFiducialNode(node->GetFiducialNode());
 // this->SetAndObserveTransformNode(node->GetTransformNode());
  // add new parameters here
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLTestMe2Node::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLTestMe2Node* node = vtkMRMLTestMe2Node::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyFloatMacro(Height);
  vtkMRMLCopyFloatMacro(RotateXAngle);
 // this->SetAndObserveFiducialNode(node->GetFiducialNode());
 // this->SetAndObserveTransformNode(node->GetTransformNode());

  // add new parameters here
  vtkMRMLCopyEndMacro();
}

void vtkMRMLTestMe2Node::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);

  vtkMRMLPrintFloatMacro(Height);
  vtkMRMLPrintFloatMacro(RotateXAngle);
  os << indent << "FiducialNode: " << (this->GetFiducialNode() ? this->GetFiducialNode()->GetName() : "null") << "\n";
  os << indent << "TransformNode: " << (this->GetTransformNode() ? this->GetTransformNode()->GetName() : "null") << "\n";

  // add new parameters here
  vtkMRMLPrintEndMacro();

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

}

vtkMRMLMarkupsFiducialNode* vtkMRMLTestMe2Node::GetFiducialNode()
{
  return vtkMRMLMarkupsFiducialNode::SafeDownCast(this->GetNodeReference(FIDUCIAL_NODE_REFERENCE_ROLE));
}
vtkMRMLLinearTransformNode* vtkMRMLTestMe2Node::GetTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(TRANSFORM_NODE_REFERENCE_ROLE));
}


void vtkMRMLTestMe2Node::SetAndObserveFiducialNode(vtkMRMLMarkupsFiducialNode* node)
{
  if (node && this->Scene != node->GetScene())
  {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
  }

  this->SetNodeReferenceID(FIDUCIAL_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));

 // this->FiducialNode = node;

  this->Modified();
}

void vtkMRMLTestMe2Node::SetAndObserveTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetNodeReferenceID(TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));

  this->Modified();
}
