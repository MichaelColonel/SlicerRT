/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// ExternalBeamPlanning includes
#include "qSlicerSubjectHierarchyRTBeamPlugin.h"

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLModelDisplayNode.h>

// MRML Widgets includes
#include <qMRMLNodeComboBox.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QIcon>
#include <QStandardItem>
#include <QAction>

//-----------------------------------------------------------------------------
/// \ingroup SlicerRt_QtModules_RtHierarchy
class qSlicerSubjectHierarchyRTBeamPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyRTBeamPlugin);
protected:
  qSlicerSubjectHierarchyRTBeamPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyRTBeamPluginPrivate(qSlicerSubjectHierarchyRTBeamPlugin& object);
  ~qSlicerSubjectHierarchyRTBeamPluginPrivate();
public:
  QIcon BeamIcon;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyRTBeamPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRTBeamPluginPrivate::qSlicerSubjectHierarchyRTBeamPluginPrivate(qSlicerSubjectHierarchyRTBeamPlugin& object)
 : q_ptr(&object)
{
  this->BeamIcon = QIcon(":Icons/Beam.png");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRTBeamPluginPrivate::~qSlicerSubjectHierarchyRTBeamPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyRTBeamPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRTBeamPlugin::qSlicerSubjectHierarchyRTBeamPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyRTBeamPluginPrivate(*this) )
{
  this->m_Name = QString("RTBeam");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyRTBeamPlugin::~qSlicerSubjectHierarchyRTBeamPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyRTBeamPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is nullptr";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLRTBeamNode"))
    {
    return 1.0; // Only this plugin can handle this node

    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyRTBeamPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (!itemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
  }

  // RT beam
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if ( associatedNode && associatedNode->IsA("vtkMRMLRTBeamNode") )
  {
    return 1.0;
  }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyRTBeamPlugin::roleForPlugin()const
{
  return "RT beam";
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyRTBeamPlugin::icon(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyRTBeamPlugin);

  if (!itemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
  }

  if (this->canOwnSubjectHierarchyItem(itemID))
  {
    return d->BeamIcon;
  }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyRTBeamPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyRTBeamPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  qDebug() << Q_FUNC_INFO << " Test";
 
  // Model
  vtkMRMLModelNode* associatedModelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (associatedModelNode)
    {
    // Return with 1 if volume is shown using volume rendering
    int numberOfDisplayNodes = associatedModelNode->GetNumberOfDisplayNodes();
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
      {
      vtkMRMLDisplayNode* displayNode = associatedModelNode->GetNthDisplayNode(displayNodeIndex);
      if (!displayNode)
        {
        continue;
        }
//      if (displayNode->IsA("vtkMRMLColorBarDisplayNode"))
//        {
//        displayNode->SetVisibility(false);
//        continue;
        // scalar volume display node does not control visibility, visibility in those
        // views will be collected from slice views below
//        displayNode->VisibilityOff();
//        }
      if (displayNode && displayNode->IsA("vtkMRMLModelDisplayNode"))
        {
        displayNode->SetVisibility(visible);
        }
      }
    }
  // Default
  else
    {
    qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->setDisplayVisibility(itemID, visible);
    }
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyRTBeamPlugin::getDisplayVisibility(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return -1;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return -1;
    }

  // Sanity checks for model
  vtkMRMLRTBeamNode* rtBeamNode = vtkMRMLRTBeamNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!rtBeamNode)
    {
    return -1;
    }

  // Return with 1 if volume is shown using volume rendering
  int numberOfDisplayNodes = rtBeamNode->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
    {
    vtkMRMLDisplayNode* displayNode = rtBeamNode->GetNthDisplayNode(displayNodeIndex);
    if (!displayNode)
      {
      continue;
      }
    if (displayNode->IsA("vtkMRMLColorBarDisplayNode"))
      {
      // scalar volume display node does not control visibility, visibility in those
      // views will be collected from slice views below
      return 0;
      }
//    if (displayNode->IsA("vtkMRMLColorBarDisplayNode"))
//      {
      // scalar volume display node does not control visibility, visibility in those
      // views will be collected from slice views below
//      return 0.3;
//      }
    if (displayNode->GetVisibility())
      {
      return 1;
      }
    }
  return 0;
}
