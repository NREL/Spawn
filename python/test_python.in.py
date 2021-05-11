import unittest
import sys
import os


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
    def test_OneSpawn(self):
        working_path = libspawn.Temp_Directory()

        print("Temp directory created: %s" % working_path.dir().toString());

        spawn1 = libspawn.Spawn("spawn1", spawn_input, working_path.dir());
        spawn1.start()

        self.assertEqual(spawn1.currentTime(), 0.0)

        for day in range(366):
            time = libspawn.days_to_seconds(day)
            print("Setting time to %s for day %s" % (time, day))
            spawn1.setTime(time)
            self.assertEqual(spawn1.currentTime(), time)
            print("Getting Core_Zone_Lights_Output")
            lighting_power = spawn1.getValue("Core_Zone_Lights_Output")
            print("Core_Zone_Lights_Output: %d", lighting_power)
            self.assertGreater(lighting_power, 0.0)

        spawn1.stop()


if __name__ == '__main__':
    unittest.main()


