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

// .NAME vtkSlicerFixedReferenceBeamsLogic - slicer logic class for beams in FixedReference frame
// .SECTION Description
// This class manages the logic associated with manipulating position
// and changing properties of beams

#ifndef __vtkSlicerFixedReferenceBeamsLogic_h
#define __vtkSlicerFixedReferenceBeamsLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// Beams includes
#include "vtkSlicerPatientPositioningModuleLogicExport.h"
#include "vtkMRMLRTBeamNode.h"

/// \ingroup SlicerRt_QtModules_Beams
class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerFixedReferenceBeamsLogic :
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerFixedReferenceBeamsLogic *New();
  vtkTypeMacro(vtkSlicerFixedReferenceBeamsLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Update parent transform of a given beam using its parameters and the IEC logic
  void UpdateTransformForBeam(vtkMRMLRTBeamNode* beamNode);

  /// Update parent transform of a given beam using its parameters and the IEC logic
  /// without using plan node (only isocenter position)
  /// @param beamSequenceScene - inner scene of the beam sequence node
  /// @param beamNode - a current beam node (must be added to the beam sequence node beforehand)
  /// @param beamTransformNode - parent transform of the beam according to the beam parameters and isocenter
  /// @param isocenter - isocenter position
  /// \warning This method is used only in vtkSlicerDicomRtImportExportModuleLogic::vtkInternal::LoadDynamicBeamSequence
  void UpdateTransformForBeam( vtkMRMLScene* beamSequenceScene, vtkMRMLRTBeamNode* beamNode, 
    vtkMRMLLinearTransformNode* beamTransformNode, double isocenter[3]);

protected:
  vtkSlicerFixedReferenceBeamsLogic();
  ~vtkSlicerFixedReferenceBeamsLogic() override;

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneEndImport() override;

  /// Handles events registered in the observer manager
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

private:
  vtkSlicerFixedReferenceBeamsLogic(const vtkSlicerFixedReferenceBeamsLogic&) = delete;
  void operator=(const vtkSlicerFixedReferenceBeamsLogic&) = delete;
};

#endif

