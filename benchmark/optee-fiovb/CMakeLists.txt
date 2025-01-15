project (fiovb C)

set (SRC host/main.c)
set (PRINTENV fiovb_printenv)
set (SETENV fiovb_setenv)

add_executable (${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME}
			   PRIVATE ta/include
			   PRIVATE include)

target_link_libraries (${PROJECT_NAME} PRIVATE teec)

install (TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

install (CODE "execute_process( \
	       COMMAND ${CMAKE_COMMAND} -E create_symlink \
	       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME} \
	       ${PRINTENV} \
	       )"
)

install (CODE "execute_process( \
	       COMMAND ${CMAKE_COMMAND} -E create_symlink \
	       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME} \
	       ${SETENV} \
	       )"
)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/../${PRINTENV}
	       ${CMAKE_CURRENT_BINARY_DIR}/../${SETENV}
	 DESTINATION ${CMAKE_INSTALL_BINDIR}
)
