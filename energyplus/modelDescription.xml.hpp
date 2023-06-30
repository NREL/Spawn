#ifndef modelDescription_hh_INCLUDED
#define modelDescription_hh_INCLUDED

#include <string>

std::string modelDescriptionXMLText =
    R"END(
<?xml version="1.0" encoding="UTF-8"?>
<fmiModelDescription fmiVersion="2.0" modelName="epfmi" guid="7904aee5cf80e4c390084a983b3f5e1d">
	<ModelExchange modelIdentifier="epfmi" needsExecutionTool="false" completedIntegratorStepNotNeeded="false" canBeInstantiatedOnlyOncePerProcess="true" canNotUseMemoryManagementFunctions="false" canGetAndSetFMUstate="false" canSerializeFMUstate="false" providesDirectionalDerivative="false" />
	<UnitDefinitions>
		<Unit name="1"/>
		<Unit name="rad">
			<BaseUnit/>
			<DisplayUnit name="rad"/>
			<DisplayUnit name="deg" factor="57.29577951308232"/>
		</Unit>
    <Unit name="J">
      <BaseUnit kg="1" m="2" s="-2"/>
    </Unit>
    <Unit name="lm/m2">
      <BaseUnit cd="1" m="-2"/>
			<DisplayUnit name="lm/m2"/>
			<DisplayUnit name="lux"/>
    </Unit>
    <Unit name="cd.sr">
      <BaseUnit cd="1"/>
			<DisplayUnit name="cd.sr"/>
			<DisplayUnit name="lum"/>
    </Unit>
    <Unit name="kg/s">
      <BaseUnit kg="1" s="-1"/>
    </Unit>
    <Unit name="W">
      <BaseUnit kg="1" m="2" s="-3"/>
    </Unit>
    <Unit name="Pa">
      <BaseUnit kg="1" m="-1" s="-2"/>
    </Unit>
    <Unit name="K">
      <BaseUnit K="1"/>
			<DisplayUnit name="K"/>
			<DisplayUnit name="degC" offset="-273.15"/>
    </Unit>
    <Unit name="s">
      <BaseUnit s="1"/>
    </Unit>
    <Unit name="m3/s">
      <BaseUnit m="3" s="-1"/>
    </Unit>
    <Unit name="m2">
      <BaseUnit m="2"/>
    </Unit>
    <Unit name="m3">
      <BaseUnit m="3"/>
    </Unit>
	</UnitDefinitions>
	<LogCategories>
		<Category name="logLevel1" description="logLevel1 - EnergyPlus Info" />
		<Category name="logLevel2" description="logLevel2 - EnergyPlus Warning" />
		<Category name="logLevel3" description="logLevel3 - EnergyPlus Error" />
		<Category name="logLevel4" description="logLevel4 - EnergyPlus Fatal" />
	</LogCategories>
	<ModelVariables>
	</ModelVariables>
	<ModelStructure>
	</ModelStructure>
</fmiModelDescription>
)END";

#endif // modelDescription_hh_INCLUDED
