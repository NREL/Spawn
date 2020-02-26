from pyfmi import load_fmu
import random
import os
import sys

print(sys.argv[1])

model = load_fmu(sys.argv[1])
start_time = 0
final_time = 60 * 10 #24 * 60 * 60

print("instantiate")
model.instantiate(name='Model', visible=True)

print("setup_experiment")
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

# perform siulations
t = start_time
while t <= final_time:
    print("t = " + str(t))
    print("zone temperature: " , model.get('Zone_Temperature')[0])
    print("outside temperature: " , model.get('Outside_Temperature')[0])

    model.time = t

    model.event_update()
    event_info = model.get_event_info()
    t = event_info.nextEventTime


model.terminate()


