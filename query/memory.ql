/**
 * @name TA_InvokeCommandEntryPoint Fourth Parameter Flow
 * @kind query
 * @id cpp/ta-invoke-command-fourth-param-flow
 * @problem.severity warning
 */

import cpp
import semmle.code.cpp.dataflow.new.DataFlow
import semmle.code.cpp.dataflow.new.TaintTracking

module FlowConfiguration implements DataFlow::ConfigSig {
    predicate isSource(DataFlow::Node source) {
        exists(FunctionCall fc, ArrayExpr arrayExpr, AssignExpr assignExpr|
            (fc.getTarget().getName() = "TEE_MemMove" and
            arrayExpr.getType().toString() = "TEE_Param" and
            arrayExpr = fc.getArgument(0).getAPredecessor().getAPredecessor() and 
            source.asExpr() = fc.getArgument(1)) or 
            (fc.getTarget().getName() = "snprintf" and
            arrayExpr.getType().toString() = "TEE_Param" and
            arrayExpr = fc.getArgument(0).getAPredecessor().getAPredecessor() and exists(int i |
            i >= 3 and i <= fc.getNumberOfArguments() - 1 and
            source.asExpr() = fc.getArgument(i))) or
            (arrayExpr.getType().toString() = "TEE_Param" and
            arrayExpr = assignExpr.getLValue().getAPredecessor().getAPredecessor() and 
            source.asExpr() = assignExpr.getLValue()
            )
        )
    }
  
    predicate isSink(DataFlow::Node sink) {
        sink.asExpr() instanceof Expr
    }
}
  
module Flow = DataFlow::Global<FlowConfiguration>;

from DataFlow::Node source, DataFlow::Node sink
where
    Flow::flow(source, sink)
select sink.asExpr(), sink.asExpr().getParent(), sink.asExpr().getLocation()
