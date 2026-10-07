// Qt includes
//#include <QDebug>

//#include <vtkTransform.h>
// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
//#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include <cstdlib>
#include <string>

#include "vtkMRMLImagePositioningNode.h"

//------------------------------------------------------------------------------
namespace
{
    static const char* HORIZONTAL_TRANSFORM_NODE_REFERENCE_ROLE = "horizontalTransformNodeRef";
    static const char* VERTICAL_TRANSFORM_NODE_REFERENCE_ROLE = "verticalTransformNodeRef";
    static const char* HORIZONTAL_SCALE_NODE_REFERENCE_ROLE = "horizontalScaleNodeRef";
    static const char* VERTICAL_SCALE_NODE_REFERENCE_ROLE = "verticalScaleNodeRef";
    static const char* DRR_NODE_REFERENCE_ROLE = "drrNodeRef";
    static const char* XRAY_NODE_REFERENCE_ROLE = "xrayNodeRef";
    static const char* ACTIVE_ORIENTATION_ATTRIBUTE_NAME = "ActiveOrientation";

}

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLImagePositioningNode);

vtkMRMLImagePositioningNode::vtkMRMLImagePositioningNode()
{
 //   vtkNew< vtkTransform > horizTransform, verticalTransform, horizScale, verticalScale;
 //   horizTransform->Identity();
 //   verticalTransform->Identity();
 //   horizScale->Identity();
 //   verticalScale->Identity();
}

//----------------------------------------------------------------------------
vtkMRMLImagePositioningNode::~vtkMRMLImagePositioningNode()
{
    this->SetAndObserveHorizontalTransformNode(nullptr);
    this->SetAndObserveVerticalTransformNode(nullptr);
    this->SetAndObserveHorizontalScaleNode(nullptr);
    this->SetAndObserveVerticalScaleNode(nullptr);
    this->SetAndObserveDrrNode(nullptr);
    this->SetAndObserveXrayNode(nullptr);
}

void vtkMRMLImagePositioningNode::WriteXML(ostream& of, int nIndent)
{
    Superclass::WriteXML(of, nIndent);

    // Write all MRML node attributes into output stream
    vtkMRMLWriteXMLBeginMacro(of);


     // add new parameters here
    vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLImagePositioningNode::ReadXMLAttributes(const char** atts)
{
    int disabledModify = this->StartModify();
    vtkMRMLNode::ReadXMLAttributes(atts);

    vtkMRMLReadXMLBeginMacro(atts);



      // add new parameters here
    vtkMRMLReadXMLEndMacro();

    this->EndModify(disabledModify);

    // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLImagePositioningNode::Copy(vtkMRMLNode* anode)
{
    int disabledModify = this->StartModify();

    Superclass::Copy(anode);

    vtkMRMLImagePositioningNode* node = vtkMRMLImagePositioningNode::SafeDownCast(anode);
    if (!node)
    {
        return;
    }

    this->DisableModifiedEventOn();

    vtkMRMLCopyBeginMacro(node);

    this->SetActiveOrientation(node->GetActiveOrientation());
    vtkMRMLCopyEndMacro();

    this->EndModify(disabledModify);

    this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLImagePositioningNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
    MRMLNodeModifyBlocker blocker(this);
    Superclass::CopyContent(anode, deepCopy);

    vtkMRMLImagePositioningNode* node = vtkMRMLImagePositioningNode::SafeDownCast(anode);
    if (!node)
    {
        return;
    }

    vtkMRMLCopyBeginMacro(node);

    this->SetActiveOrientation(node->GetActiveOrientation());
    vtkMRMLCopyEndMacro();
}

void vtkMRMLImagePositioningNode::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);

    vtkMRMLPrintBeginMacro(os, indent);

    os << indent << "HorizontalTransformNode: " << (this->GetHorizontalTransformNode() ? this->GetHorizontalTransformNode()->GetName() : "null") << "\n";
    os << indent << "VerticalTransformNode: " << (this->GetVerticalTransformNode() ? this->GetVerticalTransformNode()->GetName() : "null") << "\n";
    os << indent << "HorizontalScaleNode: " << (this->GetHorizontalScaleNode() ? this->GetHorizontalScaleNode()->GetName() : "null") << "\n";
    os << indent << "VerticalScaleNode: " << (this->GetVerticalScaleNode() ? this->GetVerticalScaleNode()->GetName() : "null") << "\n";
    os << indent << "DrrNode: " << (this->GetDrrNode() ? this->GetDrrNode()->GetName() : "null") << "\n";
    os << indent << "XrayNode: " << (this->GetXrayNode() ? this->GetXrayNode()->GetName() : "null") << "\n";
    os << indent << "ActiveOrientation: " << this->GetActiveOrientation() << "\n";

    // add new parameters here
    vtkMRMLPrintEndMacro();

}
/*
void vtkMRMLImagePositioningNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
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
*/

int vtkMRMLImagePositioningNode::GetActiveOrientation()
{
    const char* orientationAttributeValue = this->GetAttribute(ACTIVE_ORIENTATION_ATTRIBUTE_NAME);
    if (!orientationAttributeValue)
    {
      return vtkMRMLImagePositioningNode::OrientationHorizontal;
    }
    return atoi(orientationAttributeValue);
}

vtkMRMLLinearTransformNode* vtkMRMLImagePositioningNode::GetHorizontalTransformNode()
{
    return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(HORIZONTAL_TRANSFORM_NODE_REFERENCE_ROLE));
}

vtkMRMLLinearTransformNode* vtkMRMLImagePositioningNode::GetVerticalTransformNode()
{
    return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(VERTICAL_TRANSFORM_NODE_REFERENCE_ROLE));
}

vtkMRMLLinearTransformNode* vtkMRMLImagePositioningNode::GetHorizontalScaleNode()
{
    return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(HORIZONTAL_SCALE_NODE_REFERENCE_ROLE));
}

vtkMRMLLinearTransformNode* vtkMRMLImagePositioningNode::GetVerticalScaleNode()
{
    return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(VERTICAL_SCALE_NODE_REFERENCE_ROLE));
}
vtkMRMLScalarVolumeNode* vtkMRMLImagePositioningNode::GetDrrNode()
{
    return vtkMRMLScalarVolumeNode::SafeDownCast(this->GetNodeReference(DRR_NODE_REFERENCE_ROLE));
}

vtkMRMLScalarVolumeNode* vtkMRMLImagePositioningNode::GetXrayNode()
{
    return vtkMRMLScalarVolumeNode::SafeDownCast(this->GetNodeReference(XRAY_NODE_REFERENCE_ROLE));
}


void vtkMRMLImagePositioningNode::SetActiveOrientation(int orientation)
{
    this->SetAttribute(ACTIVE_ORIENTATION_ATTRIBUTE_NAME, std::to_string(orientation).c_str());
}

void vtkMRMLImagePositioningNode::SetAndObserveHorizontalTransformNode(vtkMRMLLinearTransformNode* node)
{
    if (node && this->Scene != node->GetScene())
    {
        vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
        return;
    }

    this->SetNodeReferenceID(HORIZONTAL_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

void vtkMRMLImagePositioningNode::SetAndObserveVerticalTransformNode(vtkMRMLLinearTransformNode* node)
{
    if (node && this->Scene != node->GetScene())
    {
        vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
        return;
    }

    this->SetNodeReferenceID(VERTICAL_TRANSFORM_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

void vtkMRMLImagePositioningNode::SetAndObserveHorizontalScaleNode(vtkMRMLLinearTransformNode* node)
{
    if (node && this->Scene != node->GetScene())
    {
        vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
        return;
    }
    this->SetNodeReferenceID(HORIZONTAL_SCALE_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

void vtkMRMLImagePositioningNode::SetAndObserveVerticalScaleNode(vtkMRMLLinearTransformNode* node)
{
    if (node && this->Scene != node->GetScene())
    {
        vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
        return;
    }
    this->SetNodeReferenceID(VERTICAL_SCALE_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}
void vtkMRMLImagePositioningNode::SetAndObserveDrrNode(vtkMRMLScalarVolumeNode* node)
{
    if (node && this->Scene != node->GetScene())
    {
        vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
        return;
    }

    this->SetNodeReferenceID(DRR_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

void vtkMRMLImagePositioningNode::SetAndObserveXrayNode(vtkMRMLScalarVolumeNode* node)
{
    if (node && this->Scene != node->GetScene())
    {
        vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
        return;
    }

    this->SetNodeReferenceID(XRAY_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}