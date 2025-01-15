/**
 * @name TA_InvokeCommandEntryPoint Fourth Parameter Flow
 * @kind query
 * @id cpp/ta-invoke-command-fourth-param-flow
 * @problem.severity warning
 */

 import cpp

from IfStmt is
select is.getControllingExpr().getLocation()
