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

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerPlmDrrModuleWidget.h"
#include "ui_qSlicerPlmDrrModuleWidget.h"

#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPlmDrrModuleWidgetPrivate: public Ui_qSlicerPlmDrrModuleWidget
{
public:
  qSlicerPlmDrrModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPlmDrrModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPlmDrrModuleWidgetPrivate::qSlicerPlmDrrModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPlmDrrModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPlmDrrModuleWidget::qSlicerPlmDrrModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPlmDrrModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerPlmDrrModuleWidget::~qSlicerPlmDrrModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModuleWidget::setup()
{
  Q_D(qSlicerPlmDrrModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  QStringList rtBeamNodes;
  rtBeamNodes.push_back("vtkMRMLRTBeamNode");
  d->MRMLNodeComboBox_RtBeam->setNodeTypes(rtBeamNodes);

  connect( d->MRMLNodeComboBox_RtBeam, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onRTBeamNodeChanged(vtkMRMLNode*)));
}

/// RTBeam Node (RTBeam or RTIonBeam) changed
void
qSlicerPlmDrrModuleWidget::onRTBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerPlmDrrModuleWidget);
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  vtkMRMLRTIonBeamNode* ionBeamNode = vtkMRMLRTIonBeamNode::SafeDownCast(node);
  Q_UNUSED(beamNode);
  Q_UNUSED(ionBeamNode);
}
