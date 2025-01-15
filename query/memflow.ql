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
        exists(ArrayExpr arrayExpr|
            arrayExpr.getType().toString() = "TEE_Param" and
            arrayExpr.getASuccessor() instanceof Expr and
            source.asExpr() = arrayExpr.getASuccessor()
        )
    }
  
    predicate isSink(DataFlow::Node sink) {
        sink.asExpr() instanceof Expr
    }
}
  
module Flow = DataFlow::Global<FlowConfiguration>;
  
from DataFlow::Node source, DataFlow::Node sink
where Flow::flow(source, sink) and 
    source.asExpr().getAPredecessor().getAPredecessor() instanceof Expr
select source.asExpr().getAPredecessor().getAPredecessor(), 
        source.asExpr(),
        sink.asExpr().getASuccessor().getEnclosingElement(), 
        sink.asExpr().getEnclosingFunction(), 
        sink.asExpr().getASuccessor().getEnclosingElement().getLocation()
 // select source.asExpr().getAPredecessor().getAPredecessor().getAPredecessor(), source.asExpr(), source.asExpr().getType().toString()
 