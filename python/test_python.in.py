import unittest
import sys
import os
import tempfile


print("Current working directory: %s" % os.getcwd())
sys.path.append("./Products/python/")

import libspawn

source_dir = libspawn.path("${PROJECT_SOURCE_DIR}")

idfpath = source_dir.append(libspawn.path("submodules/EnergyPlus/testfiles/RefBldgSmallOfficeNew2004_Chicago.idf"))
epwpath = source_dir.append(libspawn.path("submodules/EnergyPlus/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw"))


spawn_input = R'''{
    "version": "0.1",
    "EnergyPlus": {
      "idf": "%s",
      "weather": "%s"
    },
    "model": {
      "outputVariables": [
        {
          "name":    "Lights Electricity Rate",
          "key":     "Core_ZN_Lights",
          "fmiName": "Core_Zone_Lights_Output"
        }
      ],
      "emsActuators": [
        {
          "variableName"  : "Core_ZN People",
          "componentType" : "People",
          "controlType"   : "Number of People",
          "unit"          : "1",
          "fmiName"       : "Core_Zone_People"
        }
      ]
    }
  }''' % (idfpath.toString(), epwpath.toString())




class TestSpawn(unittest.TestCase):
    def setUp(self):
        print("Setting up Python SWIG/Spawn test harness")
        
    def test_OneSpawn(self):

        # This will safely clean up the temp directory after the spawn has finished
        with tempfile.TemporaryDirectory() as temp_path:
            working_path = libspawn.path(temp_path)
            print("Temp directory created: %s" % working_path.toString());

            spawn1 = libspawn.Spawn("spawn1", spawn_input, working_path);
            spawn1.start()

            self.assertEqual(spawn1.currentTime(), 0.0)

            for day in range(366):
                time = libspawn.days_to_seconds(day)
                spawn1.setTime(time)
                self.assertEqual(spawn1.currentTime(), time)
                lighting_power = spawn1.getValue("Core_Zone_Lights_Output")
                print("Time %s, Day %s; Core_Zone_Lights_Output: %d" % (time, day, lighting_power), flush=True)
                self.assertGreater(lighting_power, 0.0)

            print("Shutting down EnergyPlus", flush=True)
            spawn1.stop()
            print("Spawn / EnergyPlus successfully shut down", flush=True)

            # Make sure spawn1 is fully destroyed before we try to remove the TemporaryDirectory that 
            # was being used
            del spawn1
        
    def tearDown(self):
        print("Tearing down Python test harness", flush=True)

print("Starting test harness", flush = True)

if __name__ == '__main__':
    unittest.main()



print("Finishing test harness?", flush = True)

