# The full spawn cli name is embedded in the MBL, here we update the name with the latest sha
set(building_mo_path "${mbl_home}/ThermalZones/${MBL_ENERGYPLUS_VERSION_STRING}/Building.mo")
file(READ ${building_mo_path} building_mo_text)
string(REGEX REPLACE "spawn-[0-9]\\.[0-9]\\.[0-9]-.........." ${spawn_cli_name} new_building_mo_text "${building_mo_text}")
file(WRITE ${building_mo_path} "${new_building_mo_text}")
