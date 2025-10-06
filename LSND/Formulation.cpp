#include "stdafx.h"
#include "Formulation.h"
#include "Ports.h"
#include "LabelSetting.h"

Formulation::Formulation(Ports *pbm, LabelSetting *labelSet)
{
	this->pbm = pbm;
	this->labelSet = labelSet;

	model = IloModel(env, "Feeder daughter Network Design");
	cplex = IloCplex(env);

	Z = IloNumVarArray(env);
	constraints = IloRangeArray(env);
}

Formulation::~Formulation()
{
}

void Formulation::constructCombinedDaughterRoutes()
{

}

void Formulation::fixMotherRoute(LabelMother &motherRoute)
{
	motherCost = motherRoute.costCM;
	vector<int> indexVisitedPorts(pbm->nbPorts + 1, 0);
	for (IntIterator p = motherRoute.route.begin(); p != motherRoute.route.end(); p++)
		indexVisitedPorts[*p] = 1;
	for (int p = 1; p <= pbm->nbPorts; p++)
	if (indexVisitedPorts[p])
		visitedMainPorts.push_back(p);
}

void Formulation::initVariables()
{
	char name[51];
	//variablesWithPort = IloArray<IloExpr>(env, pbm->nbPorts + 1);

	//initialize the variables
	for (int v = 1; v <= labelSet->nbVesselType; v++)
	for (int port = 1; port <= pbm->nbPorts; port++)
	for (vector<Label>::iterator dLabel = labelSet->setDaughterLabels[port].begin(); dLabel != labelSet->setDaughterLabels[port].end(); dLabel++)
	{
		IloNumVar z(env, 0, 1, ILOBOOL);
		sprintf(name, "Z_%d_%d", v, dLabel->index);
		z.setName(name);
		Z.add(z);
	}

	////group the variables including a given port p in variablesWithPort
	//for (int p = 1; p <= pbm->nbPorts; p++)
	////if (pbm->getPortType(p) == Ports::S_port)	//p is a daughter port
	//{
	//	IloExpr exp(env);
	//	for (int pm = 1; pm <= pbm->nbPorts; pm++)
	//	for (vector<Label>::iterator dLabel = labelSet->setDaughterLabels[pm].begin(); dLabel != labelSet->setDaughterLabels[pm].end(); dLabel++)
	//	for (vector<int>::iterator visitedPort = dLabel->route.begin(); visitedPort != dLabel->route.end(); visitedPort++)
	//	if (*visitedPort == p)
	//	{
	//		exp += Z[dLabel->index - 1];
	//		continue;
	//	}
	//	variablesWithPort[p] = exp;
	//}
}

void Formulation::addConstraints()
{
	char name[51];
	daughterPortsConstraints = IloRangeArray(env);
	mainPortsConstraints = IloRangeArray(env);

	for (int p = 1; p <= pbm->nbPorts; p++)
	if (pbm->getPortType(p) == Ports::S_port)	//p is a daughter port
	{
		//Each daughter port must be visited by only one daughter route
		IloExpr exp(env);
		for (int v = 1; v <= labelSet->nbVesselType; v++)
		for (vector<int>::iterator pm = visitedMainPorts.begin(); pm != visitedMainPorts.end(); pm++)
		for (vector<Label>::iterator dLabel = labelSet->setDaughterLabels[*pm].begin(); dLabel != labelSet->setDaughterLabels[*pm].end(); dLabel++)
		for (vector<int>::iterator visitedPort = dLabel->route.begin(); visitedPort != dLabel->route.end(); visitedPort++)
		if (*visitedPort == p)
		{ 
			//include the d variable to the constraint
			exp += Z[dLabel->index - 1];
			continue;
		}
		daughterPortsConstraints.add(exp == 1);
		sprintf(name, "D_port(%d)", p);
		daughterPortsConstraints[daughterPortsConstraints.getSize() - 1].setName(name);
	}
	else if (pbm->getPortType(p) == Ports::M_port)	//p is a main port
	{
		//For each main port, it has to be visited at least one time
		//is it visited by the mother route
		int visited = 0;
		IloExpr exp(env);
		for (int v = 1; v <= labelSet->nbVesselType; v++)
		for (vector<int>::iterator pm = visitedMainPorts.begin(); pm != visitedMainPorts.end(); pm++)
		{
			if (*pm == p)
				visited = 1;
			for (vector<Label>::iterator dLabel = labelSet->setDaughterLabels[*pm].begin(); dLabel != labelSet->setDaughterLabels[*pm].end(); dLabel++)
			for (vector<int>::iterator visitedPort = dLabel->route.begin(); visitedPort != dLabel->route.end(); visitedPort++)
			if (*visitedPort == p)
			{
				//include the d variable to the constraint
				exp += Z[dLabel->index - 1];
				continue;
			}
		}
		mainPortsConstraints.add(exp + visited - 1 >= 0);
		sprintf(name, "Main_port(%d)", p);
		mainPortsConstraints[mainPortsConstraints.getSize() - 1].setName(name);
	}
	constraints.add(daughterPortsConstraints);
	constraints.add(mainPortsConstraints);
	model.add(constraints);
}

void Formulation::clearConstraints()
{
	constraints.clear();
}

void Formulation::addObjectiveFuntion()
{
	IloExpr exp(env);
	for (int v = 1; v <= labelSet->nbVesselType; v++)
	for (int m_port = 1; m_port <= pbm->nbPorts; m_port++)
	for (vector<Label>::iterator dLabel = labelSet->setDaughterLabels[m_port].begin(); dLabel != labelSet->setDaughterLabels[m_port].end(); dLabel++)
		exp += Z[dLabel->index - 1] * dLabel->cost;
	IloObjective obj(env, exp, IloObjective::Minimize);
	model.add(obj);
}

bool Formulation::solve(void)
{
	try {
		cplex.extract(model);
		//cplex.setParam(IloCplex::HeurFreq, 100);
		//cplex.setParam(IloCplex::MIPEmphasis, 1);
		//cplex.setParam(IloCplex::TiLim, 6 * 3600);
#ifdef _PRINT_MODEL
		cplex.exportModel("model.lp");
#endif
		bool res = cplex.solve();
		return res;
	}
	catch (IloException& e) {
		cerr << "ERROR: " << e.getMessage() << endl;
		return false;
	}
	catch (...) {
		cerr << "Error" << endl;
		return false;
	}
}

bool Formulation::isFeasible() {
	return cplex.getStatus() == IloAlgorithm::Optimal;
}

IloNum Formulation::getObjValue()
{
	return cplex.getObjValue() + motherCost;
}
