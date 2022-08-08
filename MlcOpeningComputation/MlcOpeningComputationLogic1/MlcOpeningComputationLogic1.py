import os
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
from math import *
import numpy
from vtk.util import numpy_support

#
# MlcOpeningComputationLogic
#
class MlcOpeningComputationLogic1(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self):
    # Define arrays
    self.bounds_data = [0, 0, 0, 0, 0, 0]
    self.center_data = [0, 0, 0]
    self.segmentModelNode = None
    # Set logic instance to the global variable that supplies it to the calibration curve alignment minimizer function
#    global gelDosimetryLogicInstanceGlobal
#    gelDosimetryLogicInstanceGlobal = self


  #
  # Function to calculate bounds_data[6] and center_data[3] from segment polydata
  #
  def CalculateBoundCenterDataFromPolyData(self, modelPolyData):
#    seg = segmentationNode.GetSegmentation()
#    segmentID = seg.GetSegmentIdBySegmentName(segmentName)
#    data3d = segmentationNode.GetClosedSurfaceInternalRepresentation(segmentID)
    modelPolyData.GetBounds(self.bounds_data)
    modelPolyData.GetCenter(self.center_data)
    return [self.bounds_data, self.center_data]

  # Get polydata of the segment closed surface model
  def CreateSegmentModel(self, segmentationNode, segmentName):
    shNode = slicer.mrmlScene.GetSubjectHierarchyNode()
    segmentationItemID = shNode.GetItemByDataNode(segmentationNode)
    parentItemID = shNode.GetItemParent(segmentationItemID)
    uniqueFolderName = slicer.mrmlScene.GenerateUniqueName('ExternalContourMLC')
    folderItemID = shNode.CreateFolderItem(parentItemID, uniqueFolderName)
    segmentID = segmentationNode.GetSegmentation().GetSegmentIdBySegmentName(segmentName)
    res = slicer.modules.segmentations.logic().ExportSegmentsToModels(segmentationNode, [ segmentID ], folderItemID)
    modelItemID = shNode.GetItemChildWithName(folderItemID, segmentName, True)
    self.segmentModelNode = shNode.GetItemDataNode(modelItemID)
    self.segmentModelNode.GetDisplayNode().SetColor(0., 0., 0.)
    self.segmentModelNode.GetDisplayNode().SetOpacity(1.)
    return self.segmentModelNode.GetPolyData()

  def GetSegmentModelNode(self):
    return self.segmentModelNode
