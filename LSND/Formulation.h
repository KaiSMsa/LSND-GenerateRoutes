#pragma once
#define _PRINT_MODEL
class Ports;
class LabelSetting;
class LabelMother;

class Formulation
{
	typedef IloArray<IloRangeArray> MatrixRange;
	Ports *pbm;
	LabelSetting *labelSet;
	
	double motherCost = 0;
	vector<int> visitedMainPorts;

	IloEnv env;
	IloModel model;
	IloCplex cplex;
	//IloArray<IloExpr> variablesWithPort;
	IloNumVarArray	Z;
	IloRangeArray daughterPortsConstraints, mainPortsConstraints;
	IloRangeArray constraints;
	//IloNumVar		Cmax;
	//IloNumVarArray lambda;
	//IloNumVarArray W;
	//IloRangeArray computeW;
	//IloRangeArray lambdaConstraints;
	//MatrixRange cmaxConstraints;
	//IloObjective maxLambda, maxWork;
	//IloExtractable converion_X, conversion_W, conversion_lambda;
public:
	Formulation(Ports *pbm, LabelSetting *labelSet);
	~Formulation(void);

	void constructCombinedDaughterRoutes();	//to Construct the set R_{dv}^D
	void fixMotherRoute(LabelMother &motherRoute);
	void initVariables();
	void addConstraints();
	void clearConstraints();
	void addObjectiveFuntion();
	bool solve();
	bool isFeasible();
	IloNum getObjValue();
	//void printSolution();
};


