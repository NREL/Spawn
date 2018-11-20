/*
    Copyright (C) 2009-2018 Modelon AB

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


package CCodeGenGlobalsTests

package Reinit

model ReinitCTest1
    Real x;
equation
    der(x) = 1;
    when time > 2 then
        reinit(x, 1);
    end when;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="Reinit_ReinitCTest1",
        description="",
        variability_propagation=false,
        relational_time_events=false,
        template="
$C_global_temps$
-----
$C_ode_derivatives$
-----
$C_ode_initialization$
-----
$C_dae_blocks_residual_functions$
-----
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;

-----

int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_1) = _x_0;
    _der_x_3 = 1;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _time - (2), _sw(0), JMI_REL_GT);
    }
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    if (JMI_GLOBAL(tmp_1) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_1);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----

int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    _der_x_3 = 1;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _time - (2), _sw(0), JMI_REL_GT);
    }
    _temp_1_1 = _sw(0);
    _x_0 = 0.0;
    pre_temp_1_1 = JMI_FALSE;
    JMI_DYNAMIC_FREE()
    return ef;
}

-----
static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870916;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870916;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch(jmi, _time - (2), _sw(0), JMI_REL_GT);
            }
            _temp_1_1 = _sw(0);
        }
        if (LOG_EXP_AND(_temp_1_1, LOG_EXP_NOT(pre_temp_1_1))) {
            JMI_GLOBAL(tmp_1) = AD_WRAP_LITERAL(1);
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}


-----
")})));
end ReinitCTest1;


model ReinitCTest2
    Real x,y;
equation
    der(x) = 1;
	der(y) = 2;
    when y > 2 then
        reinit(x, 1);
    end when;
    when x > 2 then
        reinit(y, 1);
    end when;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="Reinit_ReinitCTest2",
        description="",
        variability_propagation=false,
        template="
$C_global_temps$
-----
$C_ode_derivatives$
-----
$C_ode_initialization$
-----
$C_dae_blocks_residual_functions$
-----
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;
    jmi_real_t tmp_2;

-----

int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_1) = _x_0;
    JMI_GLOBAL(tmp_2) = _y_1;
    _der_x_6 = 1;
    _der_y_7 = 2;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _y_1 - (2), _sw(0), JMI_REL_GT);
    }
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    if (jmi->atInitial || jmi->atEvent) {
        _sw(1) = jmi_turn_switch(jmi, _x_0 - (2), _sw(1), JMI_REL_GT);
    }
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[1]);
    if (JMI_GLOBAL(tmp_1) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_1);
        jmi->reinit_triggered = 1;
    }
    if (JMI_GLOBAL(tmp_2) != _y_1) {
        _y_1 = JMI_GLOBAL(tmp_2);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----

int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    _der_x_6 = 1;
    _der_y_7 = 2;
    _y_1 = 0.0;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _y_1 - (2), _sw(0), JMI_REL_GT);
    }
    _temp_1_2 = _sw(0);
    _x_0 = 0.0;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(1) = jmi_turn_switch(jmi, _x_0 - (2), _sw(1), JMI_REL_GT);
    }
    _temp_2_3 = _sw(1);
    pre_temp_1_2 = JMI_FALSE;
    pre_temp_2_3 = JMI_FALSE;
    JMI_DYNAMIC_FREE()
    return ef;
}

-----
static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870918;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870918;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch(jmi, _y_1 - (2), _sw(0), JMI_REL_GT);
            }
            _temp_1_2 = _sw(0);
        }
        if (LOG_EXP_AND(_temp_1_2, LOG_EXP_NOT(pre_temp_1_2))) {
            JMI_GLOBAL(tmp_1) = AD_WRAP_LITERAL(1);
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

static int dae_block_1(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 2 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870919;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870919;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(1) = jmi_turn_switch(jmi, _x_0 - (2), _sw(1), JMI_REL_GT);
            }
            _temp_2_3 = _sw(1);
        }
        if (LOG_EXP_AND(_temp_2_3, LOG_EXP_NOT(pre_temp_2_3))) {
            JMI_GLOBAL(tmp_2) = AD_WRAP_LITERAL(1);
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}


-----
")})));
end ReinitCTest2;

model ReinitCTest3
    Real x,y;
equation
    der(x) = 1;
    der(y) = 2;
    when time > 2 then
        reinit(x, 1);
    elsewhen time > 1 then
        reinit(y, 1);
    end when;
    
annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="Reinit_ReinitCTest3",
        description="",
        variability_propagation=false,
        relational_time_events=false,
        template="
$C_global_temps$
-----
$C_ode_derivatives$
-----
$C_ode_initialization$
-----
$C_dae_blocks_residual_functions$
-----
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;
    jmi_real_t tmp_2;

-----

int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_1) = _x_0;
    JMI_GLOBAL(tmp_2) = _y_1;
    _der_x_6 = 1;
    _der_y_7 = 2;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(1) = jmi_turn_switch(jmi, _time - (1), _sw(1), JMI_REL_GT);
    }
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _time - (2), _sw(0), JMI_REL_GT);
    }
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    if (JMI_GLOBAL(tmp_1) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_1);
        jmi->reinit_triggered = 1;
    }
    if (JMI_GLOBAL(tmp_2) != _y_1) {
        _y_1 = JMI_GLOBAL(tmp_2);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----

int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    _der_x_6 = 1;
    _der_y_7 = 2;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _time - (2), _sw(0), JMI_REL_GT);
    }
    _temp_1_2 = _sw(0);
    if (jmi->atInitial || jmi->atEvent) {
        _sw(1) = jmi_turn_switch(jmi, _time - (1), _sw(1), JMI_REL_GT);
    }
    _temp_2_3 = _sw(1);
    _x_0 = 0.0;
    _y_1 = 0.0;
    pre_temp_1_2 = JMI_FALSE;
    pre_temp_2_3 = JMI_FALSE;
    JMI_DYNAMIC_FREE()
    return ef;
}

-----
static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870919;
        x[1] = 536870918;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870918;
        x[1] = 536870919;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(1) = jmi_turn_switch(jmi, _time - (1), _sw(1), JMI_REL_GT);
            }
            _temp_2_3 = _sw(1);
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch(jmi, _time - (2), _sw(0), JMI_REL_GT);
            }
            _temp_1_2 = _sw(0);
        }
        if (LOG_EXP_AND(_temp_1_2, LOG_EXP_NOT(pre_temp_1_2))) {
            JMI_GLOBAL(tmp_1) = AD_WRAP_LITERAL(1);
        } else {
            if (LOG_EXP_AND(_temp_2_3, LOG_EXP_NOT(pre_temp_2_3))) {
                JMI_GLOBAL(tmp_2) = AD_WRAP_LITERAL(1);
            }
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}


-----
")})));
end ReinitCTest3;

model ReinitCTest4
    function f
        input Real[:] x;
        output Real y = sum(x);
        algorithm
        annotation(Inline=false);
    end f;

    Real x;
equation
    der(x) = f({time});
    when time > 2 then
        reinit(x, f({1}));
    end when;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="Reinit_ReinitCTest4",
        description="",
        variability_propagation=false,
        relational_time_events=false,
        template="
$C_global_temps$
-----
$C_ode_derivatives$
-----
$C_ode_initialization$
-----
$C_dae_blocks_residual_functions$
-----
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;
    jmi_real_t tmp_2;
    int tmp_2_computed;

-----

int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_ARR(STAT, jmi_real_t, jmi_array_t, tmp_3, 1, 1)
    JMI_GLOBAL(tmp_1) = _x_0;
    JMI_ARRAY_INIT_1(STAT, jmi_real_t, jmi_array_t, tmp_3, 1, 1, 1)
    jmi_array_ref_1(tmp_3, 1) = _time;
    _der_x_3 = func_CCodeGenGlobalsTests_Reinit_ReinitCTest4_f_exp0(tmp_3);
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _time - (2), _sw(0), JMI_REL_GT);
    }
    dae_block_0_set_up(jmi);
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    if (JMI_GLOBAL(tmp_1) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_1);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----

int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_ARR(STAT, jmi_real_t, jmi_array_t, tmp_3, 1, 1)
    JMI_ARRAY_INIT_1(STAT, jmi_real_t, jmi_array_t, tmp_3, 1, 1, 1)
    jmi_array_ref_1(tmp_3, 1) = _time;
    _der_x_3 = func_CCodeGenGlobalsTests_Reinit_ReinitCTest4_f_exp0(tmp_3);
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _time - (2), _sw(0), JMI_REL_GT);
    }
    _temp_1_1 = _sw(0);
    _x_0 = 0.0;
    pre_temp_1_1 = JMI_FALSE;
    JMI_DYNAMIC_FREE()
    return ef;
}

-----
void dae_block_0_set_up(jmi_t* jmi) {
    JMI_GLOBAL(tmp_2_computed) = 0;
}

static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_ARR(STAT, jmi_real_t, jmi_array_t, tmp_4, 1, 1)
    if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870916;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870916;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch(jmi, _time - (2), _sw(0), JMI_REL_GT);
            }
            _temp_1_1 = _sw(0);
        }
        if (LOG_EXP_AND(_temp_1_1, LOG_EXP_NOT(pre_temp_1_1))) {
            JMI_ARRAY_INIT_1(STAT, jmi_real_t, jmi_array_t, tmp_4, 1, 1, 1)
            jmi_array_ref_1(tmp_4, 1) = AD_WRAP_LITERAL(1);
            JMI_GLOBAL(tmp_1) = JMI_CACHED(tmp_2, func_CCodeGenGlobalsTests_Reinit_ReinitCTest4_f_exp0(tmp_4));
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}


-----
")})));
end ReinitCTest4;

//TODO: The result in this test isn't ideal since the reinit operator is
// handled in the original system as well even though it won't be triggered.
// This is however not the fault of reinit but when initial()...
model ReinitCTest5
    Real x;
equation
    der(x) = time;
    when initial() then
        reinit(x, 1);
    end when;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="Reinit_ReinitCTest5",
        description="Test the reinit operator in the initial system",
        template="
$C_global_temps$
-----
$C_ode_derivatives$
-----
$C_ode_initialization$
-----
$C_dae_blocks_residual_functions$
-----
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;
    jmi_real_t tmp_2;

-----

int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_2) = _x_0;
    _der_x_1 = _time;
    if (_atInitial) {
        JMI_GLOBAL(tmp_2) = AD_WRAP_LITERAL(1);
    }
    if (JMI_GLOBAL(tmp_2) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_2);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----

int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_1) = _x_0;
    _der_x_1 = _time;
    _x_0 = 0.0;
    JMI_GLOBAL(tmp_1) = AD_WRAP_LITERAL(1);
    if (JMI_GLOBAL(tmp_1) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_1);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----

-----
")})));
end ReinitCTest5;

model ReinitCTest6
    Real x;
equation
    der(x) = time;
    when {initial(), time > 2} then
        reinit(x, 1);
    end when;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="Reinit_ReinitCTest6",
        description="Test the reinit operator in the initial system",
        template="
$C_global_temps$
-----
$C_ode_derivatives$
-----
$C_ode_initialization$
-----
$C_dae_blocks_residual_functions$
-----
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;
    jmi_real_t tmp_2;

-----

int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_2) = _x_0;
    _der_x_3 = _time;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (2), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    if (JMI_GLOBAL(tmp_2) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_2);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----

int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_1) = _x_0;
    _der_x_3 = _time;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (2), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    _temp_1_1 = _sw(0);
    _x_0 = 0.0;
    JMI_GLOBAL(tmp_1) = AD_WRAP_LITERAL(1);
    pre_temp_1_1 = JMI_FALSE;
    if (JMI_GLOBAL(tmp_1) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_1);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----
static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870916;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870916;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch_time(jmi, _time - (2), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
            }
            _temp_1_1 = _sw(0);
        }
        if (LOG_EXP_OR(_atInitial, LOG_EXP_AND(_temp_1_1, LOG_EXP_NOT(pre_temp_1_1)))) {
            JMI_GLOBAL(tmp_2) = AD_WRAP_LITERAL(1);
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}


-----
")})));
end ReinitCTest6;


model ReinitCTest7
    Real x(start = 1);
equation
    der(x) = -x;
  when x < 0.9 then
    reinit(x, 0.8);
  elsewhen x < 0.7 then
    reinit(x, 0.4);
  end when;

    annotation(__JModelica(UnitTesting(tests={
        CCodeGenTestCase(
            name="ReinitCTest7",
            description="Reinit of same var in different elsewhen branches",
            template="
$C_global_temps$
-----
$C_ode_derivatives$
",
            generatedCode="
    jmi_real_t tmp_1;
    jmi_real_t tmp_2;

-----

int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_1) = _x_0;
    JMI_GLOBAL(tmp_2) = _x_0;
    _der_x_5 = - _x_0;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(1) = jmi_turn_switch(jmi, _x_0 - (0.7), _sw(1), JMI_REL_LT);
    }
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _x_0 - (0.9), _sw(0), JMI_REL_LT);
    }
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    if (JMI_GLOBAL(tmp_1) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_1);
        jmi->reinit_triggered = 1;
    } else if (JMI_GLOBAL(tmp_2) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_2);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}
")})));
end ReinitCTest7;


model ReinitCTest8
    Real x(start = 1);
    Real y(start = 2);
    Real z(start = 3);
equation
    der(x) = -x;
    der(y) = -y;
    der(z) = -z;
  when x < 0.9 then
    reinit(x, 0.8);
    reinit(y, 1.8);
  elsewhen x < 0.7 then
    reinit(z, 2.4);
    reinit(x, 0.4);
  elsewhen x < 0.5 then
    reinit(z, 2.1);
    reinit(x, 0.1);
    reinit(y, 1.1);
  end when;

    annotation(__JModelica(UnitTesting(tests={
        CCodeGenTestCase(
            name="ReinitCTest8",
            description="Reinit of same var in different elsewhen branches, check grouping of several sets of reinits",
            template="
$C_global_temps$
-----
$C_ode_derivatives$
",
            generatedCode="
    jmi_real_t tmp_1;
    jmi_real_t tmp_2;
    jmi_real_t tmp_3;
    jmi_real_t tmp_4;
    jmi_real_t tmp_5;
    jmi_real_t tmp_6;
    jmi_real_t tmp_7;

-----

int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_1) = _x_0;
    JMI_GLOBAL(tmp_2) = _y_1;
    JMI_GLOBAL(tmp_3) = _z_2;
    JMI_GLOBAL(tmp_4) = _x_0;
    JMI_GLOBAL(tmp_5) = _z_2;
    JMI_GLOBAL(tmp_6) = _x_0;
    JMI_GLOBAL(tmp_7) = _y_1;
    _der_x_9 = - _x_0;
    _der_y_10 = - _y_1;
    _der_z_11 = - _z_2;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(2) = jmi_turn_switch(jmi, _x_0 - (0.5), _sw(2), JMI_REL_LT);
    }
    if (jmi->atInitial || jmi->atEvent) {
        _sw(1) = jmi_turn_switch(jmi, _x_0 - (0.7), _sw(1), JMI_REL_LT);
    }
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch(jmi, _x_0 - (0.9), _sw(0), JMI_REL_LT);
    }
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    if (JMI_GLOBAL(tmp_1) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_1);
        jmi->reinit_triggered = 1;
    } else if (JMI_GLOBAL(tmp_4) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_4);
        jmi->reinit_triggered = 1;
    } else if (JMI_GLOBAL(tmp_6) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_6);
        jmi->reinit_triggered = 1;
    }
    if (JMI_GLOBAL(tmp_2) != _y_1) {
        _y_1 = JMI_GLOBAL(tmp_2);
        jmi->reinit_triggered = 1;
    } else if (JMI_GLOBAL(tmp_7) != _y_1) {
        _y_1 = JMI_GLOBAL(tmp_7);
        jmi->reinit_triggered = 1;
    }
    if (JMI_GLOBAL(tmp_3) != _z_2) {
        _z_2 = JMI_GLOBAL(tmp_3);
        jmi->reinit_triggered = 1;
    } else if (JMI_GLOBAL(tmp_5) != _z_2) {
        _z_2 = JMI_GLOBAL(tmp_5);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}
")})));
end ReinitCTest8;

model ReinitCTest9
    Real x;
    Boolean y,z;
equation
    der(x) = 1;
    y = time > 1;
    z = y <> pre(y);
    if z then
        when z then
            reinit(x, 1);
        end when;
    end if;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="Reinit_ReinitCTest9",
        description="",
        template="
$C_global_temps$
-----
$C_ode_derivatives$
-----
$C_ode_initialization$
-----
$C_dae_blocks_residual_functions$
-----
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;

-----

int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(tmp_1) = _x_0;
    _der_x_5 = 1;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    if (JMI_GLOBAL(tmp_1) != _x_0) {
        _x_0 = JMI_GLOBAL(tmp_1);
        jmi->reinit_triggered = 1;
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----

int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    _der_x_5 = 1;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    _y_1 = _sw(0);
    pre_y_1 = JMI_FALSE;
    _z_2 = COND_EXP_EQ(_y_1, pre_y_1, JMI_FALSE, JMI_TRUE);
    _x_0 = 0.0;
    pre_z_2 = JMI_FALSE;
    if (_z_2) {
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

-----
static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870916;
        x[1] = 536870917;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870917;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
            }
            _y_1 = _sw(0);
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            _z_2 = COND_EXP_EQ(_y_1, pre_y_1, JMI_FALSE, JMI_TRUE);
        }
        if (_z_2) {
            if (LOG_EXP_AND(_z_2, LOG_EXP_NOT(pre_z_2))) {
                JMI_GLOBAL(tmp_1) = AD_WRAP_LITERAL(1);
            }
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}


-----
")})));
end ReinitCTest9;

end Reinit;

package WhenTestCache

model WhenTestCache1
    function f
        input Real x;
        output Real y = x;
        algorithm
        annotation(Inline=false);
    end f;
    
    discrete Real y;
initial equation
    y = 0;
equation
    when time > 1 then
        y = f(time);
    end when;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="WhenTestCache_WhenTestCache1",
        description="",
        template="
$C_global_temps$
$C_ode_derivatives$
$C_ode_initialization$
$C_dae_blocks_residual_functions$
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;
    int tmp_1_computed;


int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    dae_block_0_set_up(jmi);
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    JMI_DYNAMIC_FREE()
    return ef;
}


int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    _temp_1_1 = _sw(0);
    _y_0 = 0;
    pre_y_0 = _y_0;
    pre_temp_1_1 = JMI_FALSE;
    JMI_DYNAMIC_FREE()
    return ef;
}

void dae_block_0_set_up(jmi_t* jmi) {
    JMI_GLOBAL(tmp_1_computed) = 0;
}

static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_SOLVED_REAL_VALUE_REFERENCE) {
        x[0] = 2;
    } else if (evaluation_mode == JMI_BLOCK_DISCRETE_REAL_VALUE_REFERENCE) {
        x[0] = 2;
    } else if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870915;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870915;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
            }
            _temp_1_1 = _sw(0);
        }
        _y_0 = COND_EXP_EQ(LOG_EXP_AND(_temp_1_1, LOG_EXP_NOT(pre_temp_1_1)), JMI_TRUE, JMI_CACHED(tmp_1, func_CCodeGenGlobalsTests_WhenTestCache_WhenTestCache1_f_exp0(_time)), pre_y_0);
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}


")})));
end WhenTestCache1;

model WhenTestCache2
    function f
        input Real x;
        output Real y = x;
    algorithm
        annotation(Inline=false);
    end f;

    Real y, x;
equation
    when time > 1 then
        y = f(time + x);
        x = f(time + y);
    end when;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="WhenTestCache_WhenTestCache2",
        description="",
        template="
$C_global_temps$
$C_ode_derivatives$
$C_dae_blocks_residual_functions$
",
        generatedCode="


int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    JMI_DYNAMIC_FREE()
    return ef;
}

static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_VALUE_REFERENCE) {
        x[0] = 2;
    } else if (evaluation_mode == JMI_BLOCK_SOLVED_REAL_VALUE_REFERENCE) {
        x[0] = 3;
    } else if (evaluation_mode == JMI_BLOCK_DISCRETE_REAL_VALUE_REFERENCE) {
        x[0] = 3;
        x[1] = 2;
    } else if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870916;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870916;
    } else if (evaluation_mode == JMI_BLOCK_EQUATION_NOMINAL_AUTO) {
        (*res)[0] = 1;
    } else if (evaluation_mode == JMI_BLOCK_INITIALIZE) {
        x[0] = _y_0;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
            _y_0 = x[0];
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
            }
            _temp_1_2 = _sw(0);
        }
        _x_1 = COND_EXP_EQ(LOG_EXP_AND(_temp_1_2, LOG_EXP_NOT(pre_temp_1_2)), JMI_TRUE, func_CCodeGenGlobalsTests_WhenTestCache_WhenTestCache2_f_exp0(_time + _y_0), pre_x_1);
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
            (*res)[0] = COND_EXP_EQ(LOG_EXP_AND(_temp_1_2, LOG_EXP_NOT(pre_temp_1_2)), JMI_TRUE, func_CCodeGenGlobalsTests_WhenTestCache_WhenTestCache2_f_exp0(_time + _x_1), pre_y_0) - (_y_0);
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}

")})));
end WhenTestCache2;

model WhenTestCache3
    function f
        input Real x;
        output Real y = x;
        algorithm
        annotation(Inline=false);
    end f;
    
    Real x;
algorithm
    x := time;
    when sample(1,1) then
        x := f(time);
    end when;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="WhenTestCache_WhenTestCache3",
        description="",
        template="
$C_global_temps$
$C_ode_derivatives$
$C_dae_blocks_residual_functions$
",
        generatedCode="


int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    __sampleItr_1_2 = pre__sampleItr_1_2;
    _x_0 = _time;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1 + pre__sampleItr_1_2), _sw(0), JMI_REL_GEQ);
    }
    _temp_1_1 = LOG_EXP_AND(LOG_EXP_NOT(_atInitial), _sw(0));
    if (LOG_EXP_AND(_temp_1_1, LOG_EXP_NOT(pre_temp_1_1))) {
        __sampleItr_1_2 = pre__sampleItr_1_2 + 1;
    }
    if (jmi->atInitial || jmi->atEvent) {
        _sw(1) = jmi_turn_switch_time(jmi, _time - (AD_WRAP_LITERAL(1) + (pre__sampleItr_1_2 + AD_WRAP_LITERAL(1))), _sw(1), JMI_REL_LT);
    }
    if (_sw(1) == JMI_FALSE) {
        jmi_assert_failed(\"Too long time steps relative to sample interval.\", JMI_ASSERT_ERROR);
    }
    if (LOG_EXP_AND(_temp_1_1, LOG_EXP_NOT(pre_temp_1_1))) {
        _x_0 = func_CCodeGenGlobalsTests_WhenTestCache_WhenTestCache3_f_exp0(_time);
    }
    pre_x_0 = _x_0;
    pre_temp_1_1 = _temp_1_1;
    pre__sampleItr_1_2 = __sampleItr_1_2;
    JMI_DYNAMIC_FREE()
    return ef;
}

")})));
end WhenTestCache3;

model WhenTestCache4
    function f
        input Real x;
        output Real y = x;
        algorithm
        annotation(Inline=false);
    end f;
    
    discrete Real y;
equation
    when {time > 1, initial()} then
        y = f(time);
    end when;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="WhenTestCache_WhenTestCache4",
        description="",
        template="
$C_global_temps$
$C_ode_derivatives$
$C_ode_initialization$
$C_dae_blocks_residual_functions$
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;
    int tmp_1_computed;


int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    dae_block_0_set_up(jmi);
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    JMI_DYNAMIC_FREE()
    return ef;
}


int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    _temp_1_1 = _sw(0);
    _y_0 = func_CCodeGenGlobalsTests_WhenTestCache_WhenTestCache4_f_exp0(_time);
    pre_y_0 = 0.0;
    pre_temp_1_1 = JMI_FALSE;
    JMI_DYNAMIC_FREE()
    return ef;
}

void dae_block_0_set_up(jmi_t* jmi) {
    JMI_GLOBAL(tmp_1_computed) = 0;
}

static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_SOLVED_REAL_VALUE_REFERENCE) {
        x[0] = 2;
    } else if (evaluation_mode == JMI_BLOCK_DISCRETE_REAL_VALUE_REFERENCE) {
        x[0] = 2;
    } else if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870915;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870915;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
            }
            _temp_1_1 = _sw(0);
        }
        _y_0 = COND_EXP_EQ(LOG_EXP_OR(LOG_EXP_AND(_temp_1_1, LOG_EXP_NOT(pre_temp_1_1)), _atInitial), JMI_TRUE, JMI_CACHED(tmp_1, func_CCodeGenGlobalsTests_WhenTestCache_WhenTestCache4_f_exp0(_time)), pre_y_0);
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}


")})));
end WhenTestCache4;

model WhenTestCache5
    function f
        input Real x;
        output Real y = x;
        algorithm
        annotation(Inline=false);
    end f;
    
    discrete Real y;
equation
    if time > 1 then
        when {time > 1} then
            y = f(time);
        end when;
    else
        y = 0;
    end if;

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="WhenTestCache_WhenTestCache5",
        description="",
        template="
$C_global_temps$
$C_ode_derivatives$
$C_ode_initialization$
$C_dae_blocks_residual_functions$
$C_dae_init_blocks_residual_functions$
",
        generatedCode="
    jmi_real_t tmp_1;
    int tmp_1_computed;


int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    dae_block_0_set_up(jmi);
    ef |= jmi_solve_block_residual(jmi->dae_block_residuals[0]);
    JMI_DYNAMIC_FREE()
    return ef;
}


int model_ode_initialize_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    _temp_1_1 = _sw(0);
    pre_temp_1_1 = JMI_FALSE;
    pre_y_0 = 0.0;
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (AD_WRAP_LITERAL(1)), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
    }
    _y_0 = COND_EXP_EQ(_sw(0), JMI_TRUE, COND_EXP_EQ(LOG_EXP_AND(_temp_1_1, LOG_EXP_NOT(pre_temp_1_1)), JMI_TRUE, func_CCodeGenGlobalsTests_WhenTestCache_WhenTestCache5_f_exp0(_time), pre_y_0), AD_WRAP_LITERAL(0));
    JMI_DYNAMIC_FREE()
    return ef;
}

void dae_block_0_set_up(jmi_t* jmi) {
    JMI_GLOBAL(tmp_1_computed) = 0;
}

static int dae_block_0(jmi_t* jmi, jmi_real_t* x, jmi_real_t* residual, int evaluation_mode) {
    /***** Block: 1 *****/
    jmi_real_t** res = &residual;
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (evaluation_mode == JMI_BLOCK_SOLVED_REAL_VALUE_REFERENCE) {
        x[0] = 2;
    } else if (evaluation_mode == JMI_BLOCK_DISCRETE_REAL_VALUE_REFERENCE) {
        x[0] = 2;
    } else if (evaluation_mode == JMI_BLOCK_SOLVED_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870915;
    } else if (evaluation_mode == JMI_BLOCK_DIRECTLY_IMPACTING_NON_REAL_VALUE_REFERENCE) {
        x[0] = 536870915;
    } else if (evaluation_mode & JMI_BLOCK_EVALUATE || evaluation_mode & JMI_BLOCK_WRITE_BACK) {
        if ((evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) == 0) {
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
                _sw(0) = jmi_turn_switch_time(jmi, _time - (1), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
            }
            _temp_1_1 = _sw(0);
        }
        if (evaluation_mode & JMI_BLOCK_EVALUATE_NON_REALS) {
            _sw(0) = jmi_turn_switch_time(jmi, _time - (AD_WRAP_LITERAL(1)), _sw(0), jmi->eventPhase ? (JMI_REL_GEQ) : (JMI_REL_GT));
        }
        _y_0 = COND_EXP_EQ(_sw(0), JMI_TRUE, COND_EXP_EQ(LOG_EXP_AND(_temp_1_1, LOG_EXP_NOT(pre_temp_1_1)), JMI_TRUE, JMI_CACHED(tmp_1, func_CCodeGenGlobalsTests_WhenTestCache_WhenTestCache5_f_exp0(_time)), pre_y_0), AD_WRAP_LITERAL(0));
        if (evaluation_mode & JMI_BLOCK_EVALUATE) {
        }
    }
    JMI_DYNAMIC_FREE()
    return ef;
}


")})));
end WhenTestCache5;


end WhenTestCache;

package GlobalVariables

model GlobalConstantScalar1
    function f1
        input Real x;
        output Real y;
        constant Real c = 3;
        constant Real[1] d = {3};
        external "C" y = f(x,c,d);
    end f1;
    function f2
        input Real x;
        output Real y;
        constant Real c = 3;
        constant Real[1] d = {3};
        external "C";
    end f2;
    
    Real y = f1(time) + f2(time);

    annotation(__JModelica(UnitTesting(tests={
        CCodeGenTestCase(
            name="GlobalConstantScalar1",
            description="Constants in external calls",
            variability_propagation=false,
            template="
$C_global_temps$
$C_model_init_eval_independent_globals$
$C_function_headers$
$C_functions$
",
            generatedCode="
    jmi_real_t CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_c;
    jmi_array_t* CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_d;
    jmi_real_t CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_c;
    jmi_array_t* CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_d;

jmi_array_t* jmi_global_tmp_1(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_1, 1, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_1, 1, 1, 1)
    jmi_array_val_1(tmp_1, 1) = 3;
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_1;
}

jmi_array_t* jmi_global_tmp_2(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_2, 1, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_2, 1, 1, 1)
    jmi_array_val_1(tmp_2, 1) = 3;
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_2;
}

int model_init_eval_independent_globals_0(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_c) = 3;
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_d) = jmi_global_tmp_1(jmi);
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_c) = 3;
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_d) = jmi_global_tmp_2(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

int model_init_eval_independent_globals_0(jmi_t* jmi);

int model_init_eval_independent_globals(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    ef |= model_init_eval_independent_globals_0(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_def0(jmi_real_t x_v, jmi_real_t* y_o);
jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_exp0(jmi_real_t x_v);
void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_def1(jmi_real_t x_v, jmi_real_t* y_o);
jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_exp1(jmi_real_t x_v);

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_def0(jmi_real_t x_v, jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    extern double f(double, double, double*);
    y_v = f(x_v, JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_c), JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_d)->var);
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_exp0(jmi_real_t x_v) {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f1_def0(x_v, &y_v);
    return y_v;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_def1(jmi_real_t x_v, jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    extern double f2(double, double, double*, size_t);
    y_v = f2(x_v, JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_c), JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_d)->var, jmi_array_size(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_d), 0));
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_exp1(jmi_real_t x_v) {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantScalar1_f2_def1(x_v, &y_v);
    return y_v;
}
")})));
end GlobalConstantScalar1;

model GlobalConstantArray1
    
    constant Real[:] c = {4,0,5};
    
    function f
        input Real x;
        input Integer i;
        output Real y = x .+ c[i];
    algorithm
        annotation(Inline=false);
    end f;

    Real y = f(time, 2);

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="GlobalConstantArray1",
        description="",
        template="
$C_global_temps$
$C_model_init_eval_independent_globals$
$C_functions$
",
        generatedCode="
    jmi_array_t* CCodeGenGlobalsTests_GlobalVariables_GlobalConstantArray1_c;

jmi_array_t* jmi_global_tmp_1(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_1, 3, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_1, 3, 1, 3)
    jmi_array_val_1(tmp_1, 1) = 4;
    jmi_array_val_1(tmp_1, 3) = 5;
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_1;
}

int model_init_eval_independent_globals_0(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantArray1_c) = jmi_global_tmp_1(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

int model_init_eval_independent_globals_0(jmi_t* jmi);

int model_init_eval_independent_globals(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    ef |= model_init_eval_independent_globals_0(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantArray1_f_def0(jmi_real_t x_v, jmi_real_t i_v, jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    y_v = x_v + jmi_array_val_1(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantArray1_c), i_v);
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantArray1_f_exp0(jmi_real_t x_v, jmi_real_t i_v) {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantArray1_f_def0(x_v, i_v, &y_v);
    return y_v;
}
")})));
end GlobalConstantArray1;

model GlobalConstantRecordScalar1
    record R1
        Real[:] a;
    end R1;
    
    record R2
        R1 r1;
    end R2;
    
    function f
        input Real x;
        input Integer i;
        constant R2 r2 = R2(R1(1:2));
        output Real y = x + r2.r1.a[i];
    algorithm
    annotation(Inline=false);
    end f;
    
    Real y = f(time, 2);
    
annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="GlobalConstantRecordScalar1",
        description="",
        variability_propagation=false,
        template="
$C_records$
$C_global_temps$
$C_model_init_eval_independent_globals$
$C_functions$
",
        generatedCode="
typedef struct R1_0_r_ R1_0_r;
struct R1_0_r_ {
    jmi_array_t* a;
};
JMI_ARRAY_TYPE(R1_0_r, R1_0_ra)


typedef struct R2_1_r_ R2_1_r;
struct R2_1_r_ {
    R1_0_r* r1;
};
JMI_ARRAY_TYPE(R2_1_r, R2_1_ra)



    R2_1_r* CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordScalar1_f_r2;

jmi_array_t* jmi_global_tmp_1(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1, 2)
    jmi_array_val_1(tmp_1, 1) = AD_WRAP_LITERAL(1);
    jmi_array_val_1(tmp_1, 2) = AD_WRAP_LITERAL(2);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_1;
}

R1_0_r* jmi_global_tmp_2(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    R1_0_r* tmp_2;
    JMI_GLOBALS_INIT()
    tmp_2 = jmi_dynamic_function_pool_alloc(&dyn_mem, 1*sizeof(R1_0_r), TRUE);
    tmp_2->a = jmi_global_tmp_1(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_2;
}

R2_1_r* jmi_global_tmp_3(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    R2_1_r* tmp_3;
    JMI_GLOBALS_INIT()
    tmp_3 = jmi_dynamic_function_pool_alloc(&dyn_mem, 1*sizeof(R2_1_r), TRUE);
    tmp_3->r1 = jmi_global_tmp_2(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_3;
}

int model_init_eval_independent_globals_0(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordScalar1_f_r2) = jmi_global_tmp_3(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

int model_init_eval_independent_globals_0(jmi_t* jmi);

int model_init_eval_independent_globals(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    ef |= model_init_eval_independent_globals_0(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordScalar1_f_def0(jmi_real_t x_v, jmi_real_t i_v, jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    y_v = x_v + jmi_array_val_1(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordScalar1_f_r2)->r1->a, i_v);
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordScalar1_f_exp0(jmi_real_t x_v, jmi_real_t i_v) {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordScalar1_f_def0(x_v, i_v, &y_v);
    return y_v;
}
")})));
end GlobalConstantRecordScalar1;

model GlobalConstantRecordArray1
    record R
        Real[:] a;
    end R;
    
    function f
        input Real x;
        input Integer i;
        constant R[:] c = {R(1.0:2.0), R(3.0:4.0)};
        output Real y = c[i].a[i] + x;
        algorithm
    annotation(Inline=false);
    end f;
    
    Real y = f(time, 2);
    
annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="GlobalConstantRecordArray1",
        description="",
        variability_propagation=false,
        template="
$C_global_temps$
$C_model_init_eval_independent_globals$
$C_functions$
",
        generatedCode="
    R_0_ra* CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray1_f_c;

jmi_array_t* jmi_global_tmp_1(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1, 2)
    jmi_array_val_1(tmp_1, 1) = 1.0;
    jmi_array_val_1(tmp_1, 2) = 2.0;
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_1;
}

jmi_array_t* jmi_global_tmp_2(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_2, 2, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_2, 2, 1, 2)
    jmi_array_val_1(tmp_2, 1) = 3.0;
    jmi_array_val_1(tmp_2, 2) = 4.0;
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_2;
}

R_0_ra* jmi_global_tmp_3(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, R_0_r, R_0_ra, tmp_3, 2, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, R_0_r, R_0_ra, tmp_3, 2, 1, 2)
    jmi_array_rec_1(tmp_3, 1)->a = jmi_global_tmp_1(jmi);
    jmi_array_rec_1(tmp_3, 2)->a = jmi_global_tmp_2(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_3;
}

int model_init_eval_independent_globals_0(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray1_f_c) = jmi_global_tmp_3(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

int model_init_eval_independent_globals_0(jmi_t* jmi);

int model_init_eval_independent_globals(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    ef |= model_init_eval_independent_globals_0(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray1_f_def0(jmi_real_t x_v, jmi_real_t i_v, jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    y_v = jmi_array_val_1(jmi_array_rec_1(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray1_f_c), i_v)->a, i_v) + x_v;
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray1_f_exp0(jmi_real_t x_v, jmi_real_t i_v) {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray1_f_def0(x_v, i_v, &y_v);
    return y_v;
}")})));
end GlobalConstantRecordArray1;

model GlobalConstantRecordArray2
    record R1
        Real[:] a;
    end R1;
    
    record R2
        R1 r1;
    end R2;
    
    record R3
        R2 r2;
    end R3;
    
    function f
        input Real x;
        input Integer i;
        constant R3[:] c = {R3(R2(R1(1.0:2.0))),R3(R2(R1(3.0:4.0)))};
        output Real y = c[i].r2.r1.a[i] + x;
        algorithm
    annotation(Inline=false);
    end f;
    
    Real y = f(time, 2);
    
annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="GlobalConstantRecordArray2",
        description="",
        variability_propagation=false,
        template="
$C_global_temps$
$C_model_init_eval_independent_globals$
$C_functions$
",
        generatedCode="
    R3_2_ra* CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray2_f_c;

jmi_array_t* jmi_global_tmp_1(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1, 2)
    jmi_array_val_1(tmp_1, 1) = 1.0;
    jmi_array_val_1(tmp_1, 2) = 2.0;
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_1;
}

R1_0_r* jmi_global_tmp_2(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    R1_0_r* tmp_2;
    JMI_GLOBALS_INIT()
    tmp_2 = jmi_dynamic_function_pool_alloc(&dyn_mem, 1*sizeof(R1_0_r), TRUE);
    tmp_2->a = jmi_global_tmp_1(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_2;
}

R2_1_r* jmi_global_tmp_3(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    R2_1_r* tmp_3;
    JMI_GLOBALS_INIT()
    tmp_3 = jmi_dynamic_function_pool_alloc(&dyn_mem, 1*sizeof(R2_1_r), TRUE);
    tmp_3->r1 = jmi_global_tmp_2(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_3;
}

jmi_array_t* jmi_global_tmp_4(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_4, 2, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_4, 2, 1, 2)
    jmi_array_val_1(tmp_4, 1) = 3.0;
    jmi_array_val_1(tmp_4, 2) = 4.0;
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_4;
}

R1_0_r* jmi_global_tmp_5(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    R1_0_r* tmp_5;
    JMI_GLOBALS_INIT()
    tmp_5 = jmi_dynamic_function_pool_alloc(&dyn_mem, 1*sizeof(R1_0_r), TRUE);
    tmp_5->a = jmi_global_tmp_4(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_5;
}

R2_1_r* jmi_global_tmp_6(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    R2_1_r* tmp_6;
    JMI_GLOBALS_INIT()
    tmp_6 = jmi_dynamic_function_pool_alloc(&dyn_mem, 1*sizeof(R2_1_r), TRUE);
    tmp_6->r1 = jmi_global_tmp_5(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_6;
}

R3_2_ra* jmi_global_tmp_7(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, R3_2_r, R3_2_ra, tmp_7, 2, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, R3_2_r, R3_2_ra, tmp_7, 2, 1, 2)
    jmi_array_rec_1(tmp_7, 1)->r2 = jmi_global_tmp_3(jmi);
    jmi_array_rec_1(tmp_7, 2)->r2 = jmi_global_tmp_6(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_7;
}

int model_init_eval_independent_globals_0(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray2_f_c) = jmi_global_tmp_7(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

int model_init_eval_independent_globals_0(jmi_t* jmi);

int model_init_eval_independent_globals(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    ef |= model_init_eval_independent_globals_0(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray2_f_def0(jmi_real_t x_v, jmi_real_t i_v, jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    y_v = jmi_array_val_1(jmi_array_rec_1(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray2_f_c), i_v)->r2->r1->a, i_v) + x_v;
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray2_f_exp0(jmi_real_t x_v, jmi_real_t i_v) {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRecordArray2_f_def0(x_v, i_v, &y_v);
    return y_v;
}
")})));
end GlobalConstantRecordArray2;


model GlobalConstantForPowInt1
    record R
        Real[:] a;
    end R;
    
    function f
        input Real x;
        constant Integer[:] c = 1:2;
        output Real y = x;
    algorithm
        for i in 1:2 loop
            y := y ^ c[i];
        end for;
    annotation(Inline=false);
    end f;
    
    Real y = f(time);
    
annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="GlobalVariables_GlobalConstantForPowInt1",
        description="",
        variability_propagation=false,
        template="$C_functions$",
        generatedCode="
void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantForPowInt1_f_def0(jmi_real_t x_v, jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    jmi_real_t i_0i;
    jmi_int_t i_0ie;
    jmi_int_t i_0in;
    y_v = x_v;
    i_0in = 0;
    i_0ie = floor((2) - (1));
    for (i_0i = 1; i_0in <= i_0ie; i_0i = 1 + (++i_0in)) {
        y_v = jmi_pow_function(\"CCodeGenGlobalsTests.GlobalVariables.GlobalConstantForPowInt1.f\", y_v, jmi_array_val_1(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantForPowInt1_f_c), i_0i), \"y ^ global(CCodeGenGlobalsTests.GlobalVariables.GlobalConstantForPowInt1.f.c[i])\");
    }
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantForPowInt1_f_exp0(jmi_real_t x_v) {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantForPowInt1_f_def0(x_v, &y_v);
    return y_v;
}

")})));
end GlobalConstantForPowInt1;

model GlobalConstantRef1
    
    record R1
        Real[:] x;
    end R1;
    
    record R2
        extends R1;
    end R2;
    
    record R3
        extends R1;
    end R3;
    
    package P
        constant R1 r1 = R1(1:2);
        constant R2 r2 = r1;
    end P;
    
    function f
        input Real x;
        input Integer i;
        output Real y = x + P.r2.x[i];
    algorithm
        annotation(Inline=false);
    end f;

    Real y = f(time, 2);

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="GlobalConstantRef1",
        description="",
        template="
$C_global_temps$
$C_model_init_eval_independent_globals$
$C_functions$
",
        generatedCode="
    R1_0_r* CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef1_P_r2;

jmi_array_t* jmi_global_tmp_1(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1, 2)
    jmi_array_val_1(tmp_1, 1) = AD_WRAP_LITERAL(1);
    jmi_array_val_1(tmp_1, 2) = AD_WRAP_LITERAL(2);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_1;
}

R1_0_r* jmi_global_tmp_2(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    R1_0_r* tmp_2;
    JMI_GLOBALS_INIT()
    tmp_2 = jmi_dynamic_function_pool_alloc(&dyn_mem, 1*sizeof(R1_0_r), TRUE);
    tmp_2->x = jmi_global_tmp_1(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_2;
}

int model_init_eval_independent_globals_0(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef1_P_r2) = jmi_global_tmp_2(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

int model_init_eval_independent_globals_0(jmi_t* jmi);

int model_init_eval_independent_globals(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    ef |= model_init_eval_independent_globals_0(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef1_f_def0(jmi_real_t x_v, jmi_real_t i_v, jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    y_v = x_v + jmi_array_val_1(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef1_P_r2)->x, i_v);
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef1_f_exp0(jmi_real_t x_v, jmi_real_t i_v) {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef1_f_def0(x_v, i_v, &y_v);
    return y_v;
}
")})));
end GlobalConstantRef1;

model GlobalConstantRef2
    
    record R1
        Real[:] x;
    end R1;
    
    record R2
        extends R1;
    end R2;
    
    record R3
        extends R1;
    end R3;
    
    package P
        constant R1 r1(x=1:2);
        constant R2 r2 = r1;
    end P;
    
    function f
        input Real x;
        input Integer i;
        output Real y = x + P.r2.x[i];
    algorithm
        annotation(Inline=false);
    end f;

    Real y = f(time, 2);

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="GlobalConstantRef2",
        description="",
        template="
$C_global_temps$
$C_model_init_eval_independent_globals$
$C_functions$
",
        generatedCode="
    R1_0_r* CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef2_P_r2;

jmi_array_t* jmi_global_tmp_1(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, jmi_real_t, jmi_array_t, tmp_1, 2, 1, 2)
    jmi_array_val_1(tmp_1, 1) = AD_WRAP_LITERAL(1);
    jmi_array_val_1(tmp_1, 2) = AD_WRAP_LITERAL(2);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_1;
}

R1_0_r* jmi_global_tmp_2(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    R1_0_r* tmp_2;
    JMI_GLOBALS_INIT()
    tmp_2 = jmi_dynamic_function_pool_alloc(&dyn_mem, 1*sizeof(R1_0_r), TRUE);
    tmp_2->x = jmi_global_tmp_1(jmi);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_2;
}

int model_init_eval_independent_globals_0(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef2_P_r2) = jmi_global_tmp_2(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

int model_init_eval_independent_globals_0(jmi_t* jmi);

int model_init_eval_independent_globals(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    ef |= model_init_eval_independent_globals_0(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef2_f_def0(jmi_real_t x_v, jmi_real_t i_v, jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    y_v = x_v + jmi_array_val_1(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef2_P_r2)->x, i_v);
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef2_f_exp0(jmi_real_t x_v, jmi_real_t i_v) {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef2_f_def0(x_v, i_v, &y_v);
    return y_v;
}
")})));
end GlobalConstantRef2;

model GlobalConstantRef3
    
    constant R1[1] r = {R1(2)};
    record R1
        parameter Real x;
    end R1;
    function f
        input Integer i;
        output Real y = 1 + r[i].x;
    algorithm
    end f;
    
    Real y = f(integer(time));

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="GlobalConstantRef3",
        description="",
        template="
$C_global_temps$
$C_model_init_eval_independent_globals$
$C_functions$
$C_ode_derivatives$
",
        generatedCode="
    R1_0_ra* CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef3_r;

R1_0_ra* jmi_global_tmp_1(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    JMI_ARR(DYNA, R1_0_r, R1_0_ra, tmp_1, 1, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(DYNA, R1_0_r, R1_0_ra, tmp_1, 1, 1, 1)
    jmi_array_rec_1(tmp_1, 1)->x = AD_WRAP_LITERAL(2);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_1;
}

int model_init_eval_independent_globals_0(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef3_r) = jmi_global_tmp_1(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

int model_init_eval_independent_globals_0(jmi_t* jmi);

int model_init_eval_independent_globals(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    ef |= model_init_eval_independent_globals_0(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}



int model_ode_derivatives_base(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    if (jmi->atInitial || jmi->atEvent) {
        _sw(0) = jmi_turn_switch_time(jmi, _time - (pre_temp_1_2), _sw(0), JMI_REL_LT);
    }
    if (jmi->atInitial || jmi->atEvent) {
        _sw(1) = jmi_turn_switch_time(jmi, _time - (pre_temp_1_2 + AD_WRAP_LITERAL(1)), _sw(1), JMI_REL_GEQ);
    }
    _temp_1_2 = COND_EXP_EQ(LOG_EXP_OR(LOG_EXP_OR(_sw(0), _sw(1)), _atInitial), JMI_TRUE, floor(_time), pre_temp_1_2);
    pre_temp_1_2 = _temp_1_2;
    _y_1 = 1 + jmi_array_rec_1(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantRef3_r), _temp_1_2)->x;
    JMI_DYNAMIC_FREE()
    return ef;
}
")})));
end GlobalConstantRef3;

model GlobalConstantExternalObject1
    
    package P
        model EO
            extends ExternalObject;
            function constructor
                input Real[:] x;
                output EO eo;
                external;
            end constructor;
            function destructor
                input EO eo;
                external;
            end destructor;
        end EO;
        
        function f
            output Real y;
            external "C" y = f(eo1);
        end f;
        
        constant Real x = 1;
        constant EO eo = EO({x});
        constant EO eo1 = eo;
    end P;
    
    function f
        input Real x;
        output Real y = x + P.f();
    algorithm
    end f;

    Real y = f(time);

annotation(__JModelica(UnitTesting(tests={
    CCodeGenTestCase(
        name="GlobalConstantExternalObject1",
        description="",
        template="
$C_global_temps$
$C_model_init_eval_independent_globals$
$C_functions$
$C_destruct_external_object$
",
        generatedCode="
    jmi_extobj_t CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_eo;
    jmi_extobj_t CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_eo1;

jmi_extobj_t jmi_global_tmp_1(jmi_t* jmi) {
    JMI_DYNAMIC_INIT()
    jmi_extobj_t tmp_1;
    JMI_ARR(STAT, jmi_real_t, jmi_array_t, tmp_2, 1, 1)
    JMI_GLOBALS_INIT()
    JMI_ARRAY_INIT_1(STAT, jmi_real_t, jmi_array_t, tmp_2, 1, 1, 1)
    jmi_array_ref_1(tmp_2, 1) = 1.0;
    tmp_1 = func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_EO_constructor_exp2(tmp_2);
    JMI_GLOBALS_FREE()
    JMI_DYNAMIC_FREE()
    return tmp_1;
}

int model_init_eval_independent_globals_0(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_eo) = jmi_global_tmp_1(jmi);
    JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_eo1) = JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_eo);
    JMI_DYNAMIC_FREE()
    return ef;
}

int model_init_eval_independent_globals_0(jmi_t* jmi);

int model_init_eval_independent_globals(jmi_t* jmi) {
    int ef = 0;
    JMI_DYNAMIC_INIT()
    ef |= model_init_eval_independent_globals_0(jmi);
    JMI_DYNAMIC_FREE()
    return ef;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_f_def0(jmi_real_t* y_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(REA, y_v)
    extern double f(void*);
    y_v = f(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_eo1));
    JMI_RET(GEN, y_o, y_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_real_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_f_exp0() {
    JMI_DEF(REA, y_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_f_def0(&y_v);
    return y_v;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_EO_destructor_def1(jmi_extobj_t eo_v) {
    JMI_DYNAMIC_INIT()
    extern void destructor(void*);
    destructor(eo_v);
    JMI_DYNAMIC_FREE()
    return;
}

void func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_EO_constructor_def2(jmi_array_t* x_a, jmi_extobj_t* eo_o) {
    JMI_DYNAMIC_INIT()
    JMI_DEF(EXO, eo_v)
    extern void* constructor(double*, size_t);
    eo_v = constructor(x_a->var, jmi_array_size(x_a, 0));
    JMI_RET(GEN, eo_o, eo_v)
    JMI_DYNAMIC_FREE()
    return;
}

jmi_extobj_t func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_EO_constructor_exp2(jmi_array_t* x_a) {
    JMI_DEF(EXO, eo_v)
    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_EO_constructor_def2(x_a, &eo_v);
    return eo_v;
}


    func_CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_EO_destructor_def1(JMI_GLOBAL(CCodeGenGlobalsTests_GlobalVariables_GlobalConstantExternalObject1_P_eo));

")})));
end GlobalConstantExternalObject1;

end GlobalVariables;

end CCodeGenGlobalsTests;
