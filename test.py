from pyfmi import load_fmu
import random
import os
import sys

print(sys.argv[1])

model = load_fmu(sys.argv[1])
start_time = 0
final_time = 24 * 60 * 60

#instantiate fmu
model.instantiate(name='Model', visible=True)

# initialize the simulation
model.setup_experiment(start_time=start_time, stop_time=final_time)

# get information for the model
print ("model version: ", model.get_version())

# get name for the model
print ("model name: ", model.get_name())

#get model identifier
print ("model identifier: ", model.get_identifier())

#get model generation tool
print ("model generation tool: ", model.get_generation_tool())

#get model guid
print ("model guid: ", model.get_guid())

#get model generation date time
print ("model generation-date-time: ", model.get_generation_date_and_time() )

#get model platform
platform = model.get_model_types_platform()
print ("model platform: ", platform)


# get all the model variables
# This one is not working: outputs = model.get_output_list()
outputs = model.get_model_variables(causality=3)
for x in outputs.keys():
    print("outputs: " + x + ": value ==> ", model.get(x))
    

# get input vars
#inputs = model.get_input_list()
#for x in inputs.keys():
#    print("inputs: " + x + ": value ==>", model.get(x) )
    

## get ode sizes
#print("model ode sizes: ", model.get_ode_sizes())
#
#
#default_start_time = model.get_default_experiment_start_time()
#print ("default-start-time:", default_start_time)
#
#default_stop_time = model.get_default_experiment_stop_time()
#print ("default-stop-time:", default_stop_time)
#
#default_step = model.get_default_experiment_step()
#print ("default-step:", default_step)
#
#fmu_author = model.get_author()
#print ("fmu-author:", fmu_author)


# perform siulations
t = 0
while t < 10: 
    print("t = " + str(t))

    model.event_update()

    event_info = model.get_event_info()
    #get the time step
    lastTime = t
    #t = event_info.nextEventTime
    t = t + 1
    
    print("update time")
    model.time = t

    for x in outputs.keys():
        print("outputs: " + x + ": value ==> ", model.get(x))

model.terminate()
   

