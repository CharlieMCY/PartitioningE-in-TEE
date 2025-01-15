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
            arrayExpr.getASuccessor().getASuccessor() instanceof Expr and
            source.asExpr() = arrayExpr.getASuccessor().getASuccessor()
        )
        // exists(ArrayExpr ae, Operation o|
        //     ae.getArrayOffset() instanceof Operation and
        //     o = ae.getArrayOffset() and
        //     (exists(Expr e |
        //         e.getType().toString() = "TEE_Param" and
        //         o.getAPredecessor().getAPredecessor().getAPredecessor().getAPredecessor() instanceof Expr and
        //         e = o.getAPredecessor().getAPredecessor().getAPredecessor().getAPredecessor() and 
        //         source.asExpr() = o.getAPredecessor().getAPredecessor()
        //     ) 
        //     or 
        //     exists(Expr e |
        //         e.getType().toString() = "TEE_Param" and
        //         o.getAPredecessor().getAPredecessor().getAPredecessor() instanceof Expr and
        //         e = o.getAPredecessor().getAPredecessor().getAPredecessor() and 
        //         source.asExpr() = o.getAPredecessor()
        //     )
        //     )
        // )
    }
 
    predicate isSink(DataFlow::Node sink) {
        sink.asExpr() instanceof Expr
    }
}
 
module Flow = DataFlow::Global<FlowConfiguration>;
 
from DataFlow::Node source, DataFlow::Node sink
where Flow::flow(source, sink) and
    exists(ArrayExpr ae |
        ae.getArrayOffset() instanceof Operation and
        sink.asExpr().getParent() = ae.getArrayOffset()
    )
select source.asExpr().getAPredecessor().getAPredecessor().getAPredecessor(), source.asExpr(), source.asExpr().getType().toString(), sink.asExpr().getParent(), 
        sink.asExpr().getEnclosingFunction(), sink.asExpr().getParent().getLocation()
// select source.asExpr().getAPredecessor().getAPredecessor().getAPredecessor(), source.asExpr(), source.asExpr().getType().toString()

// from ArrayExpr ae, Operation o, Expr e
// where ae.getArrayOffset() instanceof Operation and
//     o = ae.getArrayOffset() and 
//     e.getType().toString() = "TEE_Param" and
//     o.getAPredecessor().getAPredecessor().getAPredecessor().getAPredecessor() instanceof Expr and
//     e = o.getAPredecessor().getAPredecessor().getAPredecessor().getAPredecessor()
// select o.getAPredecessor(), o.getAPredecessor().getAPredecessor()