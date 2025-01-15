import cpp

// from FunctionCall fc, MemberVariable mv, VariableAccess va
// where mv.getName().toString() = "paramTypes" and 
//         fc.getTarget().getName() = "TEEC_InvokeCommand" and 
//         va = fc.getArgument(2).getAPredecessor() and 
//         va.getTarget()
// select fc.getArgument(1), mv.getAnAssignedValue().getValue()

from FunctionCall fc, AssignExpr ae
where fc.getTarget().getName() = "TEEC_InvokeCommand" and 
        ae.getLValue().toString() = "paramTypes" and
        ae.getLocation().isBefore(fc.getLocation()) and
        not exists (AssignExpr otherae |
                otherae.getLValue().toString() = "paramTypes" and
                otherae.getLocation().isBefore(fc.getLocation()) and
                ae.getLocation().isBefore(otherae.getLocation())
        )
select fc.getArgument(1), ae.getRValue().getValue()

// from FunctionCall fc, MemberVariable mv
// where exists( |
//         fc.getTarget().getName() = "TEEC_InvokeCommand" and
//         mv.getName().toString() = "paramTypes" and 
//         mv.getAnAssignment().getLocation().isBefore(fc.getLocation()) and 
//         not exists(MemberVariable omv |
//                 omv.getName().toString() = "paramTypes" and 
//                 mv.getAnAssignment().getLocation().isBefore(omv.getAnAssignment().getLocation()) and
//                 omv.getAnAssignment().getLocation().isBefore(fc.getLocation()))
//         )
// select fc.getArgument(1), mv.getAnAssignedValue().getValue()
