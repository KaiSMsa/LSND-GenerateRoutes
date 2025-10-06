// LSND.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Ports.h"
#include "LabelSetting.h"
#include "Solution.h"
#include "TriTas.h"

route_type route_t = MOTHER_ROUTES_ONLY;			//	MOTHER_ROUTES_ONLY	MOTHER_DAUGHTER_ROUTES
cycle_type cycle_mother = SIMPLE_CYCLE;				//  SIMPLE_CYCLE		BUTTERFLY_CYCLE
cycle_type cycle_t = BUTTERFLY_CYCLE;				//  SIMPLE_CYCLE		BUTTERFLY_CYCLE
cargo_type cargo_t = UNDEFINED_CAPACITY;			//	ROUTE_CARGO			SYSTEM_CARGO			UNDEFINED_CAPACITY
visit_frequency freq = WEEKLY;						//	WEEKLY				TWICE_WEEK
generation_mode g_mode = HEURISTIC_ROUTES;			//	ALL_ROUTES			HEURISTIC_ROUTES

int _tmain_for_mother_routes();
int _tmain_for_mother_daughter_routes_heuristics();
int _tmain_for_mother_daughter_routes_all();

int _tmain(int argc, _TCHAR* argv[])
{
	/* Comment/Uncomment the following code to run the appropriate function*/
	_tmain_for_mother_routes();		//to generate all mother routes
	// _tmain_for_mother_daughter_routes_heuristics();		// to generate a limited number of mother and daughter routes suing parameter e, check the code for additional parameters
	// _tmain_for_mother_daughter_routes_all();				// to generate all mother and daughter routes, check the code for additional parameters
	return 0;
}

int _tmain_for_mother_routes()		//generate mother routes only
{
	clock_t start, end;
	double generation_time;
	int nbMotherRoutes = 0;

	/* Set the directory and the demand file*/
	std::string path = "InstancesV5/";
	std::string fileName ="C4-ND";
	std::string fullPath = path + fileName + ".txt";

	Ports instance;

	/* The different parameters for route geenration */
	route_t = MOTHER_ROUTES_ONLY;
	cycle_mother = SIMPLE_CYCLE;
	cycle_t = SIMPLE_CYCLE;
	cargo_t = ROUTE_CARGO;
	freq = WEEKLY;
	g_mode = ALL_ROUTES;

	instance.readData(fullPath, route_t, cycle_mother);
	instance.setAllPortsToMain();

	start = clock();
	LabelSetting label(&instance);
	label.portAllocation();
	label.setCargoTypes(cargo_t);
	label.setRouteTypes(route_t);
	label.setVisitFrequency(freq);

	label.generateMotherRoutes();

	nbMotherRoutes = label.getNbMotherRoutes();
	end = clock();
	generation_time = (double)(end - start) / CLOCKS_PER_SEC;
	cout << nbMotherRoutes << "	mother routes" << endl;
	cout << generation_time << " Elapsed time for route generation" << endl;

	/* Write the output into a file*/
	string outFile = fileName + "-MRoutes.dat";
	label.writeOPLFormat("Temp", outFile.c_str());		//The output is placed under Temp folder
	return 0;
}

int _tmain_for_mother_daughter_routes_heuristics()	//generate mother and daughter routes heuristically, using parameter (e)
{
	clock_t start, end;
	int nbDaughterRoutes = 0, nbMotherRoutes = 0;
	double generation_time;
	// 1-	Setting the parameters
	// K_D and K_M are the neighbor parameters for heurstic route generation

	//// Use the following for parameter (e=1), as in the TRE paper
	//int K_D = 1;										
	//int K_M = 1;
	// 
	//// Use the following for parameter (e=2), as in the TRE paper
	int K_D = 2;
	int K_M = 2;

	/* Set the directory and the demand file*/
	std::string path = "InstancesV5/";
	std::string fileName = "A3-ND";
	std::string fullPath = path + fileName + ".txt";

	Ports instance;

	/* The different parameters for route geenration */
	route_t = MOTHER_DAUGHTER_ROUTES;
	cycle_mother = SIMPLE_CYCLE;
	cycle_t = BUTTERFLY_CYCLE;		//Simple: SIMPLE_CYCLE		Butterfly: BUTTERFLY_CYCLE
	cargo_t = UNDEFINED_CAPACITY;
	freq = WEEKLY;
	g_mode = HEURISTIC_ROUTES;

	instance.readData(fullPath, route_t, cycle_mother);
	instance.setAllPortsToMain();

	start = clock();
	LabelSetting label(&instance);
	label.portAllocationHeuristic(K_D);
	label.setMotherNeighbors(K_M);

	label.setCargoTypes(cargo_t);
	label.setRouteTypes(route_t);
	label.setVisitFrequency(freq);

	label.generateDaughterLabelsHeuristic(cycle_t);
	nbDaughterRoutes = label.getNbDaughterRoutes();

	//label.constructCombinedDaughterRoutes();
	label.generateMotherRoutes();

	nbMotherRoutes = label.getNbMotherRoutes();
	end = clock();
	generation_time = (double)(end - start) / CLOCKS_PER_SEC;
	cout << nbMotherRoutes << "	mother routes" << endl;
	cout << nbDaughterRoutes << "	daughter routes" << endl;
	cout << generation_time << " Elapsed time for route generation" << endl;

	string outFile = fileName;
	if (cycle_t == SIMPLE_CYCLE)
		outFile += "-S-";
	else if(cycle_t == BUTTERFLY_CYCLE)
		outFile += "-B-";
	outFile += K_M + ".dat";
	label.writeOPLFormat("Temp", outFile.c_str());		//The output is placed under Temp folder

	return 0;
}

int _tmain_for_mother_daughter_routes_all()	//generate all mother and daughter routes
{
	clock_t start, end;
	int nbDaughterRoutes = 0, nbMotherRoutes = 0;
	double generation_time;

	/* Set the directory and the demand file*/
	std::string path = "InstancesV5/";
	std::string fileName = "A4-ND";
	std::string fullPath = path + fileName + ".txt";

	Ports instance;

	/* The different parameters for route geenration */
	route_t = MOTHER_DAUGHTER_ROUTES;
	cycle_mother = SIMPLE_CYCLE;
	cycle_t = SIMPLE_CYCLE;		//Simple: SIMPLE_CYCLE		Butterfly: BUTTERFLY_CYCLE
	cargo_t = UNDEFINED_CAPACITY;
	freq = WEEKLY;
	g_mode = ALL_ROUTES;

	instance.readData(fullPath, route_t, cycle_mother);
	instance.setAllPortsToMain();

	start = clock();
	LabelSetting label(&instance);
	label.portAllocation();
	label.setCargoTypes(cargo_t);
	label.setRouteTypes(route_t);
	label.setVisitFrequency(freq);

	label.generateDaughterLabels(cycle_t);
	nbDaughterRoutes = label.getNbDaughterRoutes();

	//label.constructCombinedDaughterRoutes();
	label.generateMotherRoutes();

	nbMotherRoutes = label.getNbMotherRoutes();
	end = clock();
	generation_time = (double)(end - start) / CLOCKS_PER_SEC;
	cout << nbMotherRoutes << "	mother routes" << endl;
	cout << nbDaughterRoutes << "	daughter routes" << endl;
	cout << generation_time << " Elapsed time for route generation" << endl;

	string outFile = fileName + "-B-All.dat";
	label.writeOPLFormat("Temp", outFile.c_str());	//The output is placed under Temp folder
	return 0;
}
