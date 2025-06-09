#ifndef __vtkMRMLTestMe2Node_h
#define __vtkMRMLTestMe2Node_h

#include "vtkSlicerTestMe2ModuleMRMLExport.h" // What is this?


// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkSmartPointer.h>

class vtkMRMLNode;
class vtkMRMLMarkupsNode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLLinearTransformNode;

class VTK_SLICER_TESTME2_MODULE_MRML_EXPORT vtkMRMLTestMe2Node : public vtkMRMLNode
{
public:
    static vtkMRMLTestMe2Node *New();
    vtkTypeMacro(vtkMRMLTestMe2Node,vtkMRMLNode);
    void PrintSelf(ostream& os, vtkIndent indent) override;
      /// Create instance of a GAD node.
    vtkMRMLNode* CreateNodeInstance() override;
      /// Get unique node XML tag name
/*
        /// Set node attributes from name/value pairs
    void ReadXMLAttributes(const char** atts) override;

    /// Write this node's information to a MRML file in XML format.
    void WriteXML(ostream& of, int indent) override;

    /// Copy the node's attributes to this object
    void Copy(vtkMRMLNode *node) override;

    /// Copy node content (excludes basic data, such a name and node reference)
    vtkMRMLCopyContentMacro(vtkMRMLTestMe2Node);
*/

      /// Get unique node XML tag name
    const char* GetNodeTagName() override { return "TestMe2"; };

    /// Handles events registered in the observer manager
    void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

protected:
    vtkMRMLTestMe2Node();
    virtual ~vtkMRMLTestMe2Node();
    vtkMRMLTestMe2Node(const vtkMRMLTestMe2Node&);
    void operator=(const vtkMRMLTestMe2Node&);
};

#endif
