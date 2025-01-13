#-----------------------------------------------------------------------------
# Add models
#-----------------------------------------------------------------------------

# Geometry based on KUKA KR 210 r2700 extra document
set(MACHINE_NAME Cabin26AGeometry)

set(MACHINE_MODELS
  Cabin26AGeometry.json
  FixedReference.stl
  TableTop.stl
  TableFlange.stl
  TableRobotBaseFixed.stl
  TableRobotBaseRotation.stl
  TableRobotShoulder.stl
  TableRobotElbow.stl
  TableRobotWrist.stl
)


SlicerMacroConfigureTreatmentMachineComponentModels(
  NAME ${MACHINE_NAME}
  MODELS ${MACHINE_MODELS}
)
