
project( JMI C )

include_directories( ${PROJECT_SOURCE_DIR} 
  ${CMAKE_SOURCE_DIR}/submodules/jmodelica/ThirdParty/Sundials/sundials-2.7.0/include 
  ${CMAKE_BINARY_DIR}/sundials/include/ )

set( JMI_SRC 
  jmi.c
  jmi.h
  jmi_array_none.c
  jmi_array_none.h
  #jmi_block_log.c
  #jmi_block_log.h
  #jmi_block_residual.c
  #jmi_block_residual.h
  #jmi_block_solver.c
  #jmi_block_solver.h
  #jmi_block_solver_impl.h
  #jmi_block_solver_test.c
  #jmi_brent_search.h
  #jmi_brent_solver.c
  #jmi_brent_solver.h
  jmi_callbacks.h
  jmi_chattering.c
  jmi_chattering.h
  jmi_cs.c
  jmi_cs.h
  jmi_delay.c
  jmi_delay.h
  jmi_delay_impl.h
  jmi_dyn_mem.c
  jmi_dyn_mem.h
  jmi_dyn_mem_.c
  jmi_dynamic_state.c
  jmi_dynamic_state.h
  jmi_global.c
  jmi_global.h
  #jmi_kinsol_solver.c
  #jmi_kinsol_solver.h
  jmi_linear_algebra.c
  jmi_linear_algebra.h
  jmi_linear_solver.c
  jmi_linear_solver.h
  jmi_log.c
  jmi_log.h
  jmi_log_impl.h
  jmi_math.c
  jmi_math.h
  jmi_math_ad.c
  jmi_math_ad.h
  jmi_me.c
  jmi_me.h
  #jmi_minpack_solver.c
  #jmi_minpack_solver.h
  jmi_ode_cvode.c
  jmi_ode_cvode.h
  jmi_ode_euler.c
  jmi_ode_euler.h
  jmi_ode_problem.c
  jmi_ode_problem.h
  jmi_ode_solver.c
  jmi_ode_solver.h
  jmi_ode_solver_impl.h
  jmi_ode_solver_test.c
  jmi_realtime_solver.c
  jmi_realtime_solver.h
  jmi_simple_newton.c
  jmi_simple_newton.h
  jmi_types.c
  jmi_types.h
  jmi_util.c
  jmi_util.h
  jmi_work_array.c
  jmi_work_array.h
  ModelicaUtilities.c
  ModelicaUtilities.h
  module_include/jmi_get_set.h
)

add_library( jmi ${JMI_SRC} )

