std::string modelDescriptionXMLText = 
R"END(
<?xml version="1.0" encoding="UTF-8"?>
<fmiModelDescription fmiVersion="2.0" modelName="SingleZone" guid="7904aee5cf80e4c390084a983b3f5e1d">
	<ModelExchange modelIdentifier="SingleZone" needsExecutionTool="false" completedIntegratorStepNotNeeded="false" canBeInstantiatedOnlyOncePerProcess="true" canNotUseMemoryManagementFunctions="false" canGetAndSetFMUstate="false" canSerializeFMUstate="false" providesDirectionalDerivative="false" />
	<LogCategories>
		<Category name="logLevel1" description="logLevel1 - fatal error" />
		<Category name="logLevel2" description="logLevel2 - error" />
		<Category name="logLevel3" description="logLevel3 - warning" />
		<Category name="logLevel4" description="logLevel4 - info" />
		<Category name="logLevel5" description="logLevel5 - verbose" />
		<Category name="logLevel6" description="logLevel6 - debug" />
	</LogCategories>
	<ModelVariables>
	</ModelVariables>
</fmiModelDescription>
)END";

