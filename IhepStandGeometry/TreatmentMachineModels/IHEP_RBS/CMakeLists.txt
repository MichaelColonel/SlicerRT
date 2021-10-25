#-----------------------------------------------------------------------------
# Add models
#-----------------------------------------------------------------------------

set(MACHINE_NAME IHEPStand)

set(MACHINE_MODELS
  FixedReference.stl
  PatientSupportFull.stl
  PatientSupportPart.stl
  TablePlatform.stl
  TableTopSupport.stl
  TableTopMirror.stl
  TableTopOrigin.stl
  TableTopMiddle.stl
  TableTop.stl
)


SlicerMacroConfigureTreatmentMachineComponentModels(
  NAME ${MACHINE_NAME}
  MODELS ${MACHINE_MODELS}
)
