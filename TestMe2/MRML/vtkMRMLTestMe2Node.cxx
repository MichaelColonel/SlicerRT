#include <vtkTransform.h>
// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>


#include "vtkMRMLTestMe2Node.h"

vtkMRMLNodeNewMacro(vtkMRMLTestMe2Node);

vtkMRMLTestMe2Node::vtkMRMLTestMe2Node()
{
}

//----------------------------------------------------------------------------
vtkMRMLTestMe2Node::~vtkMRMLTestMe2Node()
{
}

void vtkMRMLTestMe2Node::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  // add new parameters here
//  vtkMRMLPrintEndMacro();
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
