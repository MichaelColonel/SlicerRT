#ifndef __vtkMRMLImagePositioningNode_h
#define __vtkMRMLImagePositioningNode_h

#include "vtkSlicerImagePositioningModuleMRMLExport.h"


// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkSmartPointer.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScalarVolumeNode.h>

class VTK_SLICER_IMAGEPOSITIONING_MODULE_MRML_EXPORT vtkMRMLImagePositioningNode : public vtkMRMLNode
{
public:
    enum OrientationType
    {
      OrientationHorizontal = 0,
      OrientationVertical = 1
    };

    static vtkMRMLImagePositioningNode* New();
    vtkTypeMacro(vtkMRMLImagePositioningNode, vtkMRMLNode);
    void PrintSelf(ostream& os, vtkIndent indent) override;
    /// Create instance of a GAD node.
    vtkMRMLNode* CreateNodeInstance() override;

    /// Set node attributes from name/value pairs

    void ReadXMLAttributes(const char** atts) override;

    /// Write this node's information to a MRML file in XML format.
    void WriteXML(ostream& of, int indent) override;

    /// Copy the node's attributes to this object
    void Copy(vtkMRMLNode* node) override;

    /// Copy node content (excludes basic data, such a name and node reference)
    vtkMRMLCopyContentMacro(vtkMRMLImagePositioningNode);


    /// Get unique node XML tag name
    const char* GetNodeTagName() override { return "ImagePositioning"; };

    /// Handles events registered in the observer manager
//    void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

    int GetActiveOrientation();
    vtkMRMLLinearTransformNode* GetHorizontalTransformNode();
    vtkMRMLLinearTransformNode* GetVerticalTransformNode();
    vtkMRMLLinearTransformNode* GetHorizontalScaleNode();
    vtkMRMLLinearTransformNode* GetVerticalScaleNode();
    vtkMRMLScalarVolumeNode* GetDrrNode();
    vtkMRMLScalarVolumeNode* GetXrayNode();

    void SetActiveOrientation(int orientation);
    void SetAndObserveHorizontalTransformNode(vtkMRMLLinearTransformNode* node);
    void SetAndObserveVerticalTransformNode(vtkMRMLLinearTransformNode* node);
    void SetAndObserveHorizontalScaleNode(vtkMRMLLinearTransformNode* node);
    void SetAndObserveVerticalScaleNode(vtkMRMLLinearTransformNode* node);
    void SetAndObserveDrrNode(vtkMRMLScalarVolumeNode* node);
    void SetAndObserveXrayNode(vtkMRMLScalarVolumeNode* node);

 //   vtkGetMacro(Height, double);
 //   vtkSetMacro(Height, double);

  protected:
    vtkMRMLImagePositioningNode();
    virtual ~vtkMRMLImagePositioningNode();
    vtkMRMLImagePositioningNode(const vtkMRMLImagePositioningNode&);
    void operator=(const vtkMRMLImagePositioningNode&);
};

#endif