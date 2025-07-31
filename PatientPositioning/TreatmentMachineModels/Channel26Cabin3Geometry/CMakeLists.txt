#-----------------------------------------------------------------------------
# Add models
#-----------------------------------------------------------------------------

# Geometry based on KUKA KR 210 r2700 extra document
set(MACHINE_NAME Channel26Cabin3Geometry)

set(MACHINE_MODELS
  Channel26Cabin3Geometry.json
  TableTop.stl
  TableFlange.stl
  KR300_R2700-2_BaseFixed.stl
  KR300_R2700-2_BaseRotation.stl
  KR300_R2700-2_Shoulder.stl
  KR300_R2700-2_Elbow_A3.stl
  KR300_R2700-2_Elbow_A4.stl
  KR300_R2700-2_Wrist.stl
  KR300_R2700-2_Flange.stl
  FixedReference.stl
  KR210_R3100-2_BaseFixed.stl
  KR210_R3100-2_BaseRotation.stl
  KR210_R3100-2_Shoulder.stl
  KR210_R3100-2_Elbow_A3.stl
  KR210_R3100-2_Elbow_A4.stl
  KR210_R3100-2_Wrist.stl
  KR210_R3100-2_Flange.stl
)

SlicerMacroConfigureTreatmentMachineComponentModels(
  NAME ${MACHINE_NAME}
  MODELS ${MACHINE_MODELS}
)
