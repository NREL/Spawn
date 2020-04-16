from pyfmi import load_fmu
import random
import os
import sys

model = load_fmu(sys.argv[1])
start_time = 0.0
final_time = 60 * 10
#final_time = 24 * 60 * 60

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
last_time = start_time
temp = 296.15
densityAir = 1.276; # kg/m^3
heatCapacity = 1000.6; # J/kgK
volume = model.get('Core_ZN_V')[0]

while t <= final_time:
    model.time = t
    model.set('Core_ZN_T', temp);
    people = 0
    lights = 0
    if t % 120 == 0:
        people = 5
        lights = 1
    else:
        people = 1
        lights = 0

    model.set('Core_Zone_People', people)
    model.set('Lights_Schedule', lights)

    print("t = " + str(t))
    print("temp = " + str(temp))
    print("core zone temperature: " , model.get('Zone_Temperature')[0])
    print("attic zone temperature: " , model.get('Attic_Zone_Temperature')[0])
    print("outside temperature: " , model.get('Outside_Temperature')[0])
    print("Core_ZN_QConSen_flow: ", model.get('Core_ZN_QConSen_flow')[0])
    print("people = " + str(people))
    print("Core_Zone_People_Output: ", model.get('Core_Zone_People_Output')[0])
    print("lights = " + str(lights))
    print("Core_Zone_Lights_Output: ", model.get('Core_Zone_Lights_Output')[0])

    model.set('Lights_Schedule', lights * 2.0)
    print("Core_Zone_Lights_Output: ", model.get('Core_Zone_Lights_Output')[0])

    dt = t - last_time
    tempDot = model.get('Core_ZN_QConSen_flow')[0] / ( volume * densityAir * heatCapacity );
    temp = temp + (dt * tempDot);

    model.event_update()
    event_info = model.get_event_info()
    last_time = t
    t = t + 7
    nextEventTime = event_info.nextEventTime
    if t > nextEventTime:
        t = nextEventTime


model.terminate()


