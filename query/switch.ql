import cpp

from SwitchCase sc, ReturnStmt rs
where (sc.getSwitchStmt().getExpr().toString() = "cmd_id" or sc.getSwitchStmt().getExpr().toString() = "cmd") and
        sc.getAStmt() instanceof ReturnStmt and
        rs = sc.getAStmt()
select sc.getExpr(), rs.getExpr()
