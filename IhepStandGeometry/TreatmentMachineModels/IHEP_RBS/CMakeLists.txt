#-----------------------------------------------------------------------------
# Add models
#-----------------------------------------------------------------------------

set(MACHINE_NAME IHEP_RBS)

set(MACHINE_MODELS
  FixedReference.stl
  PatientSupportRotation.stl
  PatientSupportRotationNew.stl
  PatientSupportStandY.stl
  TableTopStand.stl
  TableTopStandMirror.stl
  TableTopStandOrigin.stl
  TableTopStandMiddle.stl
  TableTop.stl
)


SlicerMacroConfigureTreatmentMachineComponentModels(
  NAME ${MACHINE_NAME}
  MODELS ${MACHINE_MODELS}
)
