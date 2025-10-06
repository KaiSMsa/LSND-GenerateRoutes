#include "stdafx.h"
#include "LabelSetting.h"
#include "Ports.h"

void display_vector(vector<int> v) {
	for (int p = 0; p < v.size(); p++)
		cout << v[p] << " ";
}

template <typename T> vector<size_t> sort_indices(const vector<T> &v) {

	// initialize original index locations
	vector<size_t> idx(v.size());
	iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	// using std::stable_sort instead of std::sort
	// to avoid unnecessary index re-orderings
	// when v contains elements of equal values 
	stable_sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] < v[i2]; });

	return idx;
}

void Label::updateLabel(int port)
{
	this->qPickUp += pbm->getPickUp(port);
	this->qNeed = max(this->qPickUp, this->qNeed + pbm->getDeliver(port));
	this->qDelivery += pbm->getDeliver(port);
	this->cargoTimeMPort = (this->qPickUp + this->qDelivery) / pbm->CARGO_HANDLING_TIME_M_PORT;
	this->mod += pow(2, port);
	double cargoTime = pbm->getPortType(port) == Ports::S_port ? pbm->CARGO_HANDLING_TIME_PORT : pbm->CARGO_HANDLING_TIME_M_PORT;
	double cargo_time_port = (pbm->ports[port].toDeliver + pbm->ports[port].toPickUp) / cargoTime;

	if (this->route.empty())
	{
		this->sailTime += pbm->time_matrix[this->hub][port];
		this->returnSailTime = pbm->time_matrix[port][this->hub];
		this->time += pbm->time_matrix[this->hub][port] + cargo_time_port;
		this->arrivalTimes.push_back(this->sailTime);
	}
	else
	{
		this->sailTime += pbm->time_matrix[this->route.back()][port];
		this->returnSailTime = pbm->time_matrix[port][this->hub];
		this->time += pbm->time_matrix[this->route.back()][port] + cargo_time_port;
		this->arrivalTimes.push_back(this->departureTimes.back() + pbm->time_matrix[this->route.back()][port]);
	}
	this->departureTimes.push_back(arrivalTimes.back() + cargo_time_port);
	//update previous visited ports
	double timeDelivery = pbm->ports[port].toDeliver / pbm->CARGO_HANDLING_TIME_M_PORT;	//time to load the vessel with the current delivery at the transshipment port
	for (int i = 0; i < arrivalTimes.size(); i++)
	{
		arrivalTimes[i] += timeDelivery;
		departureTimes[i] += timeDelivery;
	}
	this->timeLoop1 = this->time + this->returnSailTime + this->cargoTimeMPort;
	this->route.push_back(port);
}

bool Label::hasHub()
{
	return pbm->getPortType(this->hub) == Ports::M_port;
}

bool Label::differentPorts(const Label &compare)
{
	for (auto p : compare.route)
		if (!(this->mod % (int)pow(2, p + 1) < pow(2, p)))
			return false;

	return true;
}

void Label::setHubPort(int hub)
{
	this->hub = hub;
	nextPosition = 0;
}

void Label::addPortHeuristic(int port)
{
	if (nextPosition == -1)
		this->route.push_back(port);
	else
		this->route.insert(this->route.begin() + nextPosition++, port);
}

void Label::updateLabelHeuristic()
{
	this->qPickUp = this->qNeed = this->qDelivery = 0;
	this->sailTime = this->time = this->mod = 0;
	this->arrivalTimes.clear(); this->departureTimes.clear();
	for (IntIterator port = this->route.begin(); port != this->route.end(); port++)
	{
		this->mod += pow(2, *port);
		this->qPickUp += pbm->getPickUp(*port);
		this->qNeed = max(this->qPickUp, this->qNeed + pbm->getDeliver(*port));
		this->qDelivery += pbm->getDeliver(*port);
		double cargoTime = pbm->getPortType(*port) == Ports::S_port ? pbm->CARGO_HANDLING_TIME_PORT : pbm->CARGO_HANDLING_TIME_M_PORT;
		double cargo_time_port = (pbm->ports[*port].toDeliver + pbm->ports[*port].toPickUp) / cargoTime;

		if (port == this->route.begin())
		{
			this->sailTime += pbm->time_matrix[this->hub][*port];
			this->returnSailTime = pbm->time_matrix[*port][this->hub];
			this->time += pbm->time_matrix[this->hub][*port] + cargo_time_port;
			this->arrivalTimes.push_back(this->sailTime);
		}
		else
		{
			this->sailTime += pbm->time_matrix[*(port - 1)][*port];	//distance between pred(port) and port
			this->returnSailTime = pbm->time_matrix[*port][this->hub];
			this->time += pbm->time_matrix[*(port - 1)][*port] + cargo_time_port;
			this->arrivalTimes.push_back(this->departureTimes.back() + pbm->time_matrix[*(port - 1)][*port]);
		}
		this->departureTimes.push_back(arrivalTimes.back() + cargo_time_port);
		//update previous visited ports
		double timeDelivery = pbm->ports[*port].toDeliver / pbm->CARGO_HANDLING_TIME_M_PORT;	//time to load the vessel with the current delivery at the transshipment port
		for (int i = 0; i < arrivalTimes.size(); i++)
		{
			arrivalTimes[i] += timeDelivery;
			departureTimes[i] += timeDelivery;
		}
	}
	this->cargoTimeMPort = (this->qPickUp + this->qDelivery) / pbm->CARGO_HANDLING_TIME_M_PORT;
	this->timeLoop1 = this->time + this->returnSailTime + this->cargoTimeMPort;
}

void Label::copyToNewLabelHeuristic(Label &lbl)
{
	this->route.insert(this->route.begin(), this->hub);
	IntIterator iport = this->route.begin();
	for (IntIterator port = this->route.begin() + 1; port != this->route.end(); port++)
		if (*iport < *port)	//northernmost port
		{
			iport = port;
		}
	if (iport != this->route.begin())
	{
		std::copy(iport, route.end(), back_inserter(lbl.route));
		lbl.route.insert(lbl.route.end(), route.begin(), iport - 1);
	}
	else
	{
		std::copy(iport, route.end() - 1, back_inserter(lbl.route));
	}
	this->route.erase(this->route.begin());
}

void Label::updateLabelLoop2(Label &loop1, int port)
{
	this->hub = loop1.hub;
	double cargoTime = pbm->getPortType(port) == Ports::S_port ? pbm->CARGO_HANDLING_TIME_PORT : pbm->CARGO_HANDLING_TIME_M_PORT;
	double cargo_time_port = (pbm->getDeliver(port) + pbm->getPickUp(port)) / cargoTime;

	if (this->route.empty())
	{
		this->mod = loop1.mod + pow(2, port);
		this->isLoop2 = true;
		this->qPickUp = pbm->getPickUp(port);
		this->qDelivery = pbm->getDeliver(port);
		this->qNeed = 0;

		this->sailTime = loop1.time + loop1.returnSailTime + loop1.cargoTimeMPort + pbm->time_matrix[this->hub][port];
		this->returnSailTime = pbm->time_matrix[port][this->hub];
		this->time = this->sailTime + cargo_time_port;
		this->arrivalTimes.push_back(this->sailTime);
	}
	else
	{
		this->mod += pow(2, port);
		this->qPickUp += pbm->getPickUp(port);
		this->qDelivery += pbm->getDeliver(port);

		this->sailTime += pbm->time_matrix[this->route.back()][port];
		this->returnSailTime = pbm->time_matrix[port][this->hub];
		this->time += pbm->time_matrix[this->route.back()][port] + cargo_time_port;
		this->arrivalTimes.push_back(this->departureTimes.back() + pbm->time_matrix[this->route.back()][port]);
	}
	this->qNeed = max(this->qPickUp, this->qNeed + pbm->getDeliver(port));
	this->cargoTimeMPort = (this->qPickUp + this->qDelivery) / cargoTime;
	this->departureTimes.push_back(arrivalTimes.back() + cargo_time_port);
	//update previous visited ports
	double timeDelivery = pbm->ports[port].toDeliver / pbm->CARGO_HANDLING_TIME_M_PORT;	//time to load the vessel with the current delivery at the transshipment port
	for (int i = 0; i < arrivalTimes.size(); i++)
	{
		arrivalTimes[i] += timeDelivery;
		departureTimes[i] += timeDelivery;
	}
	this->timeLoop2 = this->time + this->returnSailTime + this->cargoTimeMPort;
	this->route.push_back(port);
}

void Label::updateLabelLoop2Heuristic(Label &loop1)
{
	this->hub = loop1.hub;
	this->loop1 = &loop1;
	this->mod = loop1.mod;
	this->isLoop2 = true;
	this->qNeed = 0;
	arrivalTimes.clear();
	departureTimes.clear();

	double cargoTime, cargo_time_port, timeDelivery;
	IntIterator port = this->route.begin();

	cargoTime = pbm->getPortType(*port) == Ports::S_port ? pbm->CARGO_HANDLING_TIME_PORT : pbm->CARGO_HANDLING_TIME_M_PORT;
	cargo_time_port = (pbm->getDeliver(*port) + pbm->getPickUp(*port)) / cargoTime;
	this->sailTime = loop1.time + loop1.returnSailTime + loop1.cargoTimeMPort + pbm->time_matrix[this->hub][*port];
	this->returnSailTime = pbm->time_matrix[*port][this->hub];
	this->time = this->sailTime + cargo_time_port;
	this->qPickUp = pbm->getPickUp(*port);
	this->qDelivery = pbm->getDeliver(*port);
	this->qNeed = max(this->qPickUp, this->qNeed + pbm->getDeliver(*port));
	this->cargoTimeMPort = (this->qPickUp + this->qDelivery) / cargoTime;

	timeDelivery = pbm->ports[*port].toDeliver / pbm->CARGO_HANDLING_TIME_M_PORT;	//time to load the vessel with the current delivery at the transshipment port
	this->arrivalTimes.push_back(this->sailTime + timeDelivery);
	this->departureTimes.push_back(arrivalTimes.back() + cargo_time_port);

	for (port++; port != this->route.end(); port++)
	{
		this->mod += pow(2, *port);
		this->qPickUp += pbm->getPickUp(*port);
		this->qDelivery += pbm->getDeliver(*port);
		this->qNeed = max(this->qPickUp, this->qNeed + pbm->getDeliver(*port));

		this->sailTime += pbm->time_matrix[this->route.back()][*port];
		this->returnSailTime = pbm->time_matrix[*port][this->hub];

		cargoTime = pbm->getPortType(*port) == Ports::S_port ? pbm->CARGO_HANDLING_TIME_PORT : pbm->CARGO_HANDLING_TIME_M_PORT;
		cargo_time_port = (pbm->getDeliver(*port) + pbm->getPickUp(*port)) / cargoTime;
		this->time += pbm->time_matrix[this->route.back()][*port] + cargo_time_port;
		this->cargoTimeMPort = (this->qPickUp + this->qDelivery) / cargoTime;

		timeDelivery = pbm->ports[*port].toDeliver / pbm->CARGO_HANDLING_TIME_M_PORT;	//time to load the vessel with the current delivery at the transshipment port
		this->arrivalTimes.push_back(this->departureTimes.back() + pbm->time_matrix[this->route.back()][*port] + timeDelivery);
		this->departureTimes.push_back(arrivalTimes.back() + cargo_time_port + timeDelivery);
		//update previous times with the time to load the current delivery at the transshipment port
		for (int i = 0; i < arrivalTimes.size() - 1; i++)
		{
			arrivalTimes[i] += timeDelivery;
			departureTimes[i] += timeDelivery;
		}
	}
	this->vessel = max(this->vessel, loop1.vessel);
	this->timeLoop2 = this->time + this->returnSailTime + this->cargoTimeMPort;
}

visit_frequency LabelMother::frequency = WEEKLY;

void LabelMother::setFrequency(visit_frequency freq)
{
	frequency = freq;
}

void LabelMother::addPort(int port)
{
	route.push_back(port);
	updateDirection();
	//compute the cargoes
	vector<int> nbVisits(pbm->nbPorts + 1, 0), numVisit(route.size(), 0);
	int nbVisitedPorts = 0;
	for (int p = 0; p < route.size(); p++)
	{
		nbVisits[route[p]] ++;
		numVisit[p] = nbVisits[route[p]];	//to count the first or the second visit
		if (nbVisits[route[p]] == 1)
			nbVisitedPorts++;
	}

	if (this->c_cargo == SYSTEM_CARGO)	// || this->c_cargo == UNDEFINED_CAPACITY)
		totalCargo = pbm->totalDelivery + pbm->totalPickUp;
	else  //ROUTE_CARGO 
	{
		totalCargo = 0;
		for (int p = 0; p < route.size(); p++)
			if (nbVisits[route[p]] == 1)
				totalCargo += pbm->getDeliver(route[p]) + pbm->getPickUp(route[p]);
			else if (numVisit[p] == 1)
				totalCargo += pbm->getDeliver(route[p]);
			else if (numVisit[p] == 2)
				totalCargo += pbm->getPickUp(route[p]);
	}
	//compute the highest handled cargoes
	maxCargoHandled = 0;
	if (this->c_cargo == SYSTEM_CARGO)
		maxCargoHandled = pbm->totalDelivery;
	else
		for (int p = 0; p < route.size(); p++)
		{
			if (nbVisits[route[p]] == 1)
				maxCargoHandled += pbm->getDeliver(route[p]);
			else if (numVisit[p] == 1)
				maxCargoHandled += pbm->getDeliver(route[p]);
		}

	double currentCargo = maxCargoHandled;
	for (int p = 0; p < route.size(); p++)
	{
		if (nbVisits[route[p]] == 1)
			currentCargo += -pbm->getDeliver(route[p]) + pbm->getPickUp(route[p]);
		else if (numVisit[p] == 1)
			currentCargo -= pbm->getDeliver(route[p]);
		else if (numVisit[p] == 2)
			currentCargo += pbm->getPickUp(route[p]);
		maxCargoHandled = max(maxCargoHandled, currentCargo);
	}
	//Assign the appropriate vessel
	if (c_cargo == ROUTE_CARGO)
	{
		for (int v = pbm->motherVessels.size() - 1; v >= 0; v--)
			if (pbm->motherVessels[v].capacity >= this->maxCargoHandled)
				indexVesselType = v;
			else
				break;
		if (pbm->motherVessels[indexVesselType].capacity < maxCargoHandled)
			indexVesselType = -1;
	}
	else //LabelMother::SYSTEM_CARGO
		indexVesselType = 0;
}

void LabelMother::computeSailingTime()
{
	if (route.size() == 1)
		sailTime = pbm->time_matrix[CM][route.back()];
	else
		sailTime = pbm->time_matrix[route[route.size() - 2]][route.back()];
	time += sailTime;
}

void LabelMother::computeTimesWithCM_Port()
{
	routeCM = route;
	routeCM.push_back(1);

	double totTimeAtPorts = totalCargo / pbm->CARGO_HANDLING_TIME_M_PORT;
	double timeAtCM = totalCargo / pbm->CARGO_HANDLING_TIME_CP;


	double returnTime = pbm->time_matrix[route.back()][this->CM];
	this->timeCM = this->time + timeAtCM + totTimeAtPorts + returnTime;
}

void LabelMother::computeCostsWithCM_Port()
{
	double costCargoMainPorts = totalCargo * pbm->CARGO_HANDLING_COST_PORT;
	double costCargoCM = totalCargo * pbm->CARGO_HANDLING_COST_CP;
	double costCargo = costCargoMainPorts + costCargoCM;
	int nbVessels;

	if (this->timeCM <= 3.5)
	{
		cout << "Mother route less than half a week. ADJUST THE COSTS" << endl;
		exit(1);
	}
	//double bunkerCost = /*pbm->time_matrix[route.back()][this->CM]*/ this->time * pbm->COST_BUNKER * pbm->FUEL_CONSUMPTION;
	if (c_cargo == SYSTEM_CARGO)
	{
		this->bunkerCost = this->time * pbm->COST_BUNKER * pbm->FUEL_CONSUMPTION;
		double sailTimeCM = this->time + pbm->time_matrix[route.back()][this->CM];
		this->bunkerCostCM = sailTimeCM * pbm->COST_BUNKER * pbm->FUEL_CONSUMPTION;
		this->cargoCost = costCargo;
		this->costPorts = pbm->FIXED_PORT_COST_MOTHER * (this->route.size() + 1);
		this->costCM = costCargo + this->bunkerCostCM;
		this->costCM += pbm->FIXED_PORT_COST_MOTHER * (this->route.size() + 1);	//call port costs including the continental port
		if (frequency == WEEKLY)
			nbVessels = ((int)this->timeCM / pbm->MAX_TIME) + 1;
		else
			nbVessels = ((int)this->timeCM / pbm->MAX_TIME / 2) + 1;
		this->costCM += pbm->TIME_CHARTER_MOTHER * nbVessels;

		//if (frequency == TWICE_WEEK && nbVessels == 1)	//	One vessel that can ensure twice visit
		//	this->costCM = (this->costCM - pbm->TIME_CHARTER_MOTHER) * 2 + pbm->TIME_CHARTER_MOTHER;
		//else
		//	this->costCM *= nbVessels;
	}
	else if (c_cargo == UNDEFINED_CAPACITY)
	{
		this->bunkerCost = this->time * pbm->COST_BUNKER;	//without  * pbm->FUEL_CONSUMPTION
		double sailTimeCM = this->time + pbm->time_matrix[route.back()][this->CM];
		this->bunkerCostCM = sailTimeCM * pbm->COST_BUNKER;	//without  * pbm->FUEL_CONSUMPTION
		this->cargoCost = costCargo;
		this->costPorts = pbm->FIXED_PORT_COST_MOTHER * (this->route.size() + 1);
		this->costCM = costCargo;		//without pbm->TIME_CHARTER_MOTHER + this->bunkerCostCM
		this->costCM += pbm->FIXED_PORT_COST_MOTHER * (this->route.size() + 1);	//call port costs including the continental port
		if (frequency == WEEKLY)
			nbVessels = ((int)this->timeCM / pbm->MAX_TIME) + 1;
		else
			nbVessels = ((int)this->timeCM / pbm->MAX_TIME / 2) + 1;
		//this->costCM *= nbWeeks;
	}
	else if (c_cargo == ROUTE_CARGO)
	{
		this->bunkerCost = this->time * pbm->COST_BUNKER * pbm->motherVessels[indexVesselType].fuelConsumption;
		double sailTimeCM = this->time + pbm->time_matrix[route.back()][this->CM];
		this->bunkerCostCM = sailTimeCM * pbm->COST_BUNKER; //without * pbm->motherVessels[indexVesselType].fuelConsumption;
		this->cargoCost = costCargo;
		this->costPorts = pbm->FIXED_PORT_COST_MOTHER * (this->route.size() + 1);
		this->costCM = costCargo;		//without pbm->motherVessels[indexVesselType].charterCost + this->bunkerCostCM;
		this->costCM += pbm->FIXED_PORT_COST_MOTHER * (this->route.size() + 1);	//call port costs including the continental port
		if (frequency == WEEKLY)
			nbVessels = ((int)this->timeCM / pbm->MAX_TIME) + 1;
		else
			nbVessels = ((int)this->timeCM / pbm->MAX_TIME / 2) + 1;
		if (frequency == TWICE_WEEK && nbVessels == 1)	//	One vessel that can ensure twice visit
			this->costCM = (this->costCM - pbm->TIME_CHARTER_MOTHER) * 2 + pbm->TIME_CHARTER_MOTHER;
		else
			this->costCM *= nbVessels;
	}
	this->nbVessel = nbVessels;
}

void LabelMother::updateDirection()
{

	if (route.size() == 1)
	{
		northestPort = route.back();
		indexNorthestPort = 0;
	}
}

bool LabelMother::feasibleRoute(route_type r_type)	//CHECK the south-bound journey
{
	vector<int> theRoute;

	if (c_cargo == ROUTE_CARGO)
		if (indexVesselType == -1) 	//check vessel capacity
			return false;

	if (r_type == LabelMother::PARTIAL_ROUTE)
		theRoute = route;
	else
		theRoute = routeCM;
	bool infeasible = false;

	for (int p = 0; p < theRoute.size() - 1 && !infeasible; p++)
		if (theRoute[p] == theRoute.back())	//previsouly visited
			return false;

	if (theRoute.size() > 1)
		if (this->goingUP)
		{
			if (pbm->indexPorts[theRoute[theRoute.size() - 2]] < pbm->indexPorts[theRoute[theRoute.size() - 1]])
			{
				this->northestPort = theRoute.back();
				this->indexNorthestPort = theRoute.size() - 1;

				if (pbm->getPortType(theRoute[theRoute.size() - 2]) == Ports::M_port_double_visit)
				if (r_type != LabelMother::COMPLETE_ROUTE)
					mainPortsToVisit.push_back(theRoute[theRoute.size() - 2]);
				else
					infeasible = true;
				else
					infeasible = true;

			}
			else
			{
				this->goingUP = false;
				if (pbm->getPortType(theRoute.back()) == Ports::M_port)
					for (int p = 0; p <= indexNorthestPort && !infeasible; p++)
						if (theRoute[p] == theRoute.back())	//previsouly visited
							infeasible = true;

				if (!infeasible)
					if (mainPortsToVisit.empty())
					{
						if (pbm->getPortType(theRoute.back()) == Ports::M_port_double_visit)	//Not visited in the north-going bound
							infeasible = true;
					}
					else
					{
						if (theRoute.back() == mainPortsToVisit.back())
							mainPortsToVisit.pop_back();
						else if (pbm->getPortType(theRoute.back()) == Ports::M_port && pbm->indexPorts[mainPortsToVisit.back()] > pbm->indexPorts[theRoute.back()])
							infeasible = true;
						else if (pbm->getPortType(theRoute.back()) == Ports::M_port_double_visit)
							infeasible = true;

						if (r_type == LabelMother::COMPLETE_ROUTE && !mainPortsToVisit.empty())
							infeasible = true;
						//else	// if (pbm->getPortType(theRoute.back()) == Ports::M_port_double_visit && )
						//	infeasible = true;
					}
			}
		}
		else
		{
			if (pbm->indexPorts[theRoute[theRoute.size() - 2]] < pbm->indexPorts[theRoute[theRoute.size() - 1]])
				infeasible = true;
			else
			if (pbm->getPortType(theRoute.back()) == Ports::M_port)
				for (int p = 0; p <= indexNorthestPort && !infeasible; p++)
					if (theRoute[p] == theRoute.back())	//previsouly visited
						infeasible = true;

			if (!infeasible)
				if (!mainPortsToVisit.empty())
				{
					if (r_type == LabelMother::COMPLETE_ROUTE)
						infeasible = true;
					else if (theRoute.back() == mainPortsToVisit.back())
						mainPortsToVisit.pop_back();
					else if (pbm->getPortType(theRoute.back()) == Ports::M_port && pbm->indexPorts[mainPortsToVisit.back()] > pbm->indexPorts[theRoute.back()])
						infeasible = true;
					else if (pbm->getPortType(theRoute.back()) == Ports::M_port_double_visit)	// && pbm->indexPorts[mainPortsToVisit.back()] < pbm->indexPorts[theRoute.back()])
						infeasible = true;
				}
				else
				{
					if (r_type == LabelMother::COMPLETE_ROUTE && pbm->getPortType(theRoute.back()) != Ports::CP_port)
						infeasible = true;
					else if (/*this->indexNorthestPort == 0 &&*/ pbm->getPortType(theRoute.back()) == Ports::M_port_double_visit)	//if some double-visit ports are visted, then all double-visit ports must be visited
						infeasible = true;
				}
		}

	int nbVisitedPorts = 0;
	for (int p = 0; p < theRoute.size(); p++)
		if (pbm->getPortType(theRoute[p]) == Ports::M_port || pbm->getPortType(theRoute[p]) == Ports::M_port_double_visit)
			nbVisitedPorts++;

	if (nbVisitedPorts == 0 || nbVisitedPorts > pbm->MAX_VISIT_MAIN_PORTS)
		return false;
	return !infeasible;
}

bool Label::feasibleRoute()
{
	double totTime = time + returnSailTime + cargoTimeMPort;
	double qN = this->qNeed;
	if (this->isLoop2)
		qN = max(qN, this->loop1->qNeed);
	if (totTime - EPSILON > pbm->MAX_TIME)
		return false;
	for (int v = 1; v <= pbm->nbVessels; v++)
		if (qN <= pbm->vessels[v].capacity)
		{
			this->vessel = v;
			return true;
		}
	return false;	//pbm->MAX_HANDLING;
}

bool Label::feasibleMotherRoute()
{
	for (int v = 1; v <= pbm->motherVessels.size(); v++)
		if (this->qNeed <= pbm->motherVessels[v].capacity)
		{
			this->vessel = v;
			return true;
		}
	return false;	//pbm->MAX_HANDLING;
}

bool Label::feasibleRoute(int vesselCapacity)
{
	double totTime = time + returnSailTime + cargoTimeMPort;
	return totTime - EPSILON <= pbm->MAX_TIME && qNeed <= vesselCapacity;	//pbm->MAX_HANDLING;
}

int  Label::getDelivery()
{
	if (isLoop2)
		return loop1->qDelivery + this->qDelivery;
	else
		return this->qDelivery;
}

int  Label::getPickUp()
{
	if (isLoop2)
		return loop1->qPickUp + this->qPickUp;
	else
		return this->qPickUp;
}

double Label::getArrivalTime(int port)
{
	for (int p = 0; p < route.size(); p++)
		if (route[p] == port)
			return arrivalTimes[p];
	if (isLoop2)
		for (int p = 0; p < loop1->route.size(); p++)
			if (loop1->route[p] == port)
				return loop1->arrivalTimes[p];
	return -1;
}

double Label::getDepartureTime(int port)
{
	for (int p = 0; p < route.size(); p++)
		if (route[p] == port)
			return departureTimes[p];
	if (isLoop2)
		for (int p = 0; p < loop1->route.size(); p++)
			if (loop1->route[p] == port)
				return loop1->departureTimes[p];
	return -1;
}

double Label::getWholeTime()
{
	return time + returnSailTime + cargoTimeMPort;
}

bool Label::isPortVisited(int port)
{
	for (IntIterator p = route.begin(); p != route.end(); p++)
		if (port == *p)
			return true;
	if (isLoop2)
		for (IntIterator p = loop1->route.begin(); p != loop1->route.end(); p++)
			if (port == *p)
				return true;
	return false;

}

bool Label::isVistedInLoop1(int port)
{
	Label *lbl = this;
	if (isLoop2)
		lbl = this->loop1;
	for (IntIterator p = lbl->route.begin(); p != lbl->route.end(); p++)
		if (port == *p)
			return true;
	return false;
}

LabelSetting::LabelSetting(Ports *PBM)
{
	this->pbm = PBM;
	vesselCapacity.push_back(0);
	nbVesselType = pbm->nbVessels;
	for (int v = 1; v <= nbVesselType; v++)
		vesselCapacity.push_back(pbm->vessels[v].capacity);
	nbReplicates = 1;
	K_M = pbm->nbPorts;
}


LabelSetting::~LabelSetting()
{
}

void LabelSetting::portAllocation()
{
	portsToMain.resize(pbm->nbPorts + 1);
	for (int i = 2; i <= pbm->nbPorts; i++)
	{
		if (pbm->getPortType(i) == Ports::M_port || pbm->getPortType(i) == Ports::M_port_double_visit)
		{
			portsToMain[i].push_back(0);
			for (int j = 1; j <= pbm->nbPorts; j++)
				if (i != j && pbm->time_matrix[i][j] <= pbm->maxTimeFromHub &&
					pbm->getPortType(j) != Ports::CP_port)
					portsToMain[i].push_back(j);
		}
	}
}

void LabelSetting::portAllocationHeuristic(int K)
{
	portsToMain.resize(pbm->nbPorts + 1);
	for (int i = 3; i <= pbm->nbPorts; i++)
	{
		portsToMain[i].push_back(0);
		for (int j = i - 1; j >= max(i - K, 2); j--)
			portsToMain[i].push_back(j);
	}
	//portsToMain[1].push_back(0);
	//for (int j = 0; j < pbm->neighborPorts[1].size(); j++)
	//	portsToMain[1].push_back(pbm->neighborPorts[1][j]);
	//for (int i = 2; i <= pbm->nbPorts; i++)
	//{
	//	portsToMain[i].push_back(0);
	//	for (int j = 0; j < min(K, pbm->neighborPorts[i].size()); j++)
	//		portsToMain[i].push_back(pbm->neighborPorts[i][j]);
	//}
}

void LabelSetting::setRouteTypes(route_type type)
{
	typeRoute = type;
}

void LabelSetting::setCargoTypes(cargo_type c_type)
{
	cargoType = c_type;
}

void LabelSetting::setVisitFrequency(visit_frequency freq)
{
	frequency = freq;
	LabelMother::setFrequency(freq);
}

void LabelSetting::setSpeedMode(control_speed speed)
{
	speed_mode = speed;
}

bool LabelSetting::routesWithDaughterRoutes()
{
	return typeRoute == MOTHER_DAUGHTER_ROUTES;
}

bool existIn(vector<int> &array, int value)
{
	for (vector<int>::iterator it = array.begin(); it != array.end(); ++it)
		if (*it == value)
			return true;
		else if (*it > value)
			return false;
	return false;
}

int LabelSetting::isNonDominated(vector<Label> &set, Label &label)
{
	int level = label.route.size();
	int startLevel = 0;
	double labelSailingTime, labelCost;
	double itSailingTime, itCost;

	if (levelSetLabels.size() > 0)
		startLevel = levelSetLabels[level - 1] + 1; //the starts is equal to the end of the previous level, plus one.

	for (LabelIterator it = set.begin() + startLevel; it != set.end();)
	{
		if (it->mod == label.mod && it->hub == label.hub) { //the same ports visited
			//if (!label.isLoop2)
			//{
			labelSailingTime = label.sailTime + label.returnSailTime;
			labelCost = pbm->vessels[label.vessel].charterCost + labelSailingTime * pbm->vessels[label.vessel].fuelConsumption;
			itSailingTime = it->sailTime + it->returnSailTime;
			itCost = pbm->vessels[it->vessel].charterCost + itSailingTime * pbm->vessels[it->vessel].fuelConsumption;
			//}
			//else
			//{
			//	labelSailingTime = label.sailTime + label.returnSailTime;
			//	labelCost = pbm->vessels[label.vessel].charterCost + labelSailingTime * pbm->vessels[label.vessel].fuelConsumption;
			//	itSailingTime = it->sailTime + it->returnSailTime;
			//	itCost = pbm->vessels[it->vessel].charterCost + labelSailingTime * pbm->vessels[it->vessel].fuelConsumption;
			//}
			if (itCost <= labelCost)
				//if (itSailingTime <= labelSailingTime + EPSILON && it->qNeed <= label.qNeed)
				return -2;	//label is dominated
			else if (itCost > labelCost)	//(itSailingTime > labelSailingTime - EPSILON && it->qNeed >= label.qNeed)	//remove dominated entries from setLabels
				it = set.erase(it);
			else
				it++;
		}
		else
			it++;
	}
	return -1;	//label is not dominated
}

int LabelSetting::isNonDominatedM(vector<Label> &set, Label &label)
{
	int level = label.route.size();
	int startLevel = 0;
	double labelSailingTime, labelCost;
	double itSailingTime, itCost;

	if (levelSetLabels.size() > 0)
		startLevel = levelSetLabels[level - 1] + 1; //the starts is equal to the end of the previous level, plus one.

	for (LabelIterator it = set.begin() + startLevel; it != set.end();)
	{
		if (it->mod == label.mod && it->hub == label.hub) { //the same ports visited
			labelSailingTime = label.sailTime + label.returnSailTime;
			labelCost = pbm->motherVessels[label.vessel].charterCost + labelSailingTime * pbm->motherVessels[label.vessel].fuelConsumption;
			itSailingTime = it->sailTime + it->returnSailTime;
			itCost = pbm->motherVessels[it->vessel].charterCost + itSailingTime * pbm->motherVessels[it->vessel].fuelConsumption;
			if (itCost <= labelCost)
				return -2;	//label is dominated
			else if (itCost > labelCost)	//(itSailingTime > labelSailingTime - EPSILON && it->qNeed >= label.qNeed)	//remove dominated entries from setLabels
				it = set.erase(it);
			else
				it++;
		}
		else
			it++;
	}
	return -1;	//label is not dominated
}

int LabelSetting::checkDominancePossibleRoutes(Label &label) // , vector<int> &indexDeleted)
{
	int nbDeleted = 0;
	int level = label.route.size();
	int startLevel = 0;

	if (levelSetPossibleRoutes.size() > 0)
		startLevel = levelSetPossibleRoutes[level - 1] + 1; //the starts is equal to the end of the previous level, plus one.

	for (LabelIterator it = setPossibleRoutes.begin() + startLevel; it != setPossibleRoutes.end();)
	{
		if (it->mod == label.mod && it->hub == label.hub &&
			!it->route.empty() && it->route.back() == label.route.back()) { //the same ports visited
			double labelSailingTime = label.sailTime + label.returnSailTime;
			double labelCost = pbm->vessels[label.vessel].charterCost + labelSailingTime * pbm->vessels[label.vessel].fuelConsumption;
			double itSailingTime = it->sailTime + it->returnSailTime;
			double itCost = pbm->vessels[it->vessel].charterCost + itSailingTime * pbm->vessels[it->vessel].fuelConsumption;

			//if (itSailingTime <= labelSailingTime + EPSILON && it->qNeed <= label.qNeed)
			if (itCost <= labelCost)
				return -1;
			//else if (itSailingTime > labelSailingTime - EPSILON && it->qNeed >= label.qNeed)
			else if (itCost > labelCost)
			{
				nbDeleted++;
				it = setPossibleRoutes.erase(it);
			}
			else
				it++;
		}
		else
			it++;
	}
	return nbDeleted;
}

int LabelSetting::checkDominancePossibleRoutesM(Label &label) // , vector<int> &indexDeleted)
{
	int nbDeleted = 0;
	int level = label.route.size();
	int startLevel = 0;

	if (levelSetPossibleRoutes.size() > 0)
		startLevel = levelSetPossibleRoutes[level - 1] + 1; //the starts is equal to the end of the previous level, plus one.

	for (LabelIterator it = setPossibleRoutes.begin() + startLevel; it != setPossibleRoutes.end();)
	{
		if (it->mod == label.mod && it->route.back() == label.route.back() && it->hub == label.hub) { //the same ports visited
			double labelSailingTime = label.sailTime + label.returnSailTime;
			double labelCost = pbm->motherVessels[label.vessel].charterCost + labelSailingTime * pbm->motherVessels[label.vessel].fuelConsumption;
			double itSailingTime = it->sailTime + it->returnSailTime;
			double itCost = pbm->motherVessels[it->vessel].charterCost + itSailingTime * pbm->motherVessels[it->vessel].fuelConsumption;

			//if (itSailingTime <= labelSailingTime + EPSILON && it->qNeed <= label.qNeed)
			if (itCost <= labelCost)
				return -1;
			//else if (itSailingTime > labelSailingTime - EPSILON && it->qNeed >= label.qNeed)
			else if (itCost > labelCost)
			{
				nbDeleted++;
				it = setPossibleRoutes.erase(it);
			}
			else
				it++;
		}
		else
			it++;
	}
	return nbDeleted;
}

void LabelSetting::generateDaughterLabelsForVesselType(int m_port)
{
	if (pbm->getPortType(m_port) != Ports::M_port && pbm->getPortType(m_port) != Ports::M_port_double_visit)
	{
		cout << "Port " << m_port << " is not a main port" << endl;
		return;
	}
	//vector<int> V({12, 13});

	levelSetLabels.clear();
	levelSetPossibleRoutes.clear();

	setPossibleRoutes.clear();

	for (int p = 1; p < portsToMain[m_port].size(); p++)
	{
		Label newLabel(pbm);
		newLabel.hub = m_port;
		newLabel.updateLabel(portsToMain[m_port][p]);
		if (newLabel.feasibleRoute())
		{
			setDaughterLabels[m_port].push_back(newLabel);
			setPossibleRoutes.push_back(newLabel);
		}
	}

	levelSetLabels.push_back(0);
	levelSetPossibleRoutes.push_back(0);

	int start = 0, end = setPossibleRoutes.size();
	levelSetPossibleRoutes.push_back(end - 1);
	levelSetLabels.push_back(end - 1);

	while (start < end)
	{
		//start --> end represents a LEVEL of a route (number of ports included)
		//Add a new port (when possible) to each route of this LEVEL.
		for (int i = start; i < end; i++)
			for (int p = 1; p < portsToMain[m_port].size(); p++)
				//if (!existIn(setPossibleRoutes[i].orderedRoute, feederToHub[h][p]))
				if ((setPossibleRoutes[i].mod % (int)pow(2, portsToMain[m_port][p] + 1)) < pow(2, portsToMain[m_port][p]))
				{
					Label newLabel = setPossibleRoutes[i];
					//if (newLabel.hub == 14 && newLabel.route == V)
					//	p = p;
					newLabel.updateLabel(portsToMain[m_port][p]);
					if (newLabel.feasibleRoute())
					{
						int result = isNonDominated(setDaughterLabels[m_port], newLabel);
						if (result == -1)	//new label is not dominated
							setDaughterLabels[m_port].push_back(newLabel);
						//Check dominance with setPossibleRoutes
						result = checkDominancePossibleRoutes(newLabel);
						if (result >= 0)
							setPossibleRoutes.push_back(newLabel);	//new entry
					}
				}
		start = end;
		end = setPossibleRoutes.size();
		levelSetPossibleRoutes.push_back(end - 1);
		levelSetLabels.push_back(setDaughterLabels[m_port].size() - 1);
	}
}

void LabelSetting::generateDaughterLabelsForVesselTypeHeuristic()
{
	int start, end;

	levelSetLabels.clear();
	levelSetPossibleRoutes.clear();
	setPossibleRoutes.clear();

	for (int start_port = pbm->nbPorts; start_port >= 3; start_port--)
	{
		cout << "Generating daughter routes for port " << start_port << endl;
		Label currentLabel(pbm);
		if (pbm->getPortType(start_port) == Ports::M_port)
			currentLabel.setHubPort(start_port);
		else
		{
			currentLabel.addPortHeuristic(start_port);
			currentLabel.hub = start_port;	//just to make the first loop correct
		}

		start = setPossibleRoutes.size();
		setPossibleRoutes.push_back(currentLabel);
		end = setPossibleRoutes.size();

		while (start < end)
		{
			for (int i = start; i < end; i++)
			{
				int currentPort = setPossibleRoutes[i].hub;	//port from which the route will be extended
				for (auto r : setPossibleRoutes[i].route)
					if (currentPort > r) currentPort = r;		//the currentport should always be the southernmost port visited.

				for (int p = 1; p < portsToMain[currentPort].size(); p++)
					if ((setPossibleRoutes[i].mod % (int)pow(2, portsToMain[currentPort][p] + 1)) < pow(2, portsToMain[currentPort][p]))
					{
						int port = portsToMain[currentPort][p];
						Label newLabel(setPossibleRoutes[i]);
						if (newLabel.hasHub())
						{
							newLabel.addPortHeuristic(port);
						//	display_vector(newLabel.route); cout << endl;
							if (pbm->getPortType(port) == Ports::M_port)	//If p is a main port, create a new label where p is the hub
							{
								Label nLabel(pbm);
								newLabel.copyToNewLabelHeuristic(nLabel);
								nLabel.setHubPort(port);
								nLabel.updateLabelHeuristic();								//Compute the costs?
								if (nLabel.feasibleRoute())
								{
									nLabel.nextPosition = 0;	//required for the next step. Later, the insertion should be at the first position as hub is the southernmost port.
									setDaughterLabelsHeuristic.push_back(nLabel);
									setPossibleRoutes.push_back(nLabel);	//new entry
								}
							}
						}
						else if (pbm->getPortType(port) == Ports::M_port)		//this is the first main port
							newLabel.setHubPort(port);
						else
							newLabel.addPortHeuristic(port);
						//These lines were used for the experimentation of ND/June
						if (newLabel.hasHub())
						{
							newLabel.updateLabelHeuristic();
							if (newLabel.feasibleRoute())
								setDaughterLabelsHeuristic.push_back(newLabel);
						}
						setPossibleRoutes.push_back(newLabel);	//new entry

						//if (newLabel.hasHub())
						//{
						//	newLabel.updateLabelHeuristic();
						//	//display_vector(newLabel.route); cout << endl;
						//	if (newLabel.feasibleRoute())
						//	{
						//		int result = isNonDominated(setDaughterLabelsHeuristic, newLabel);
						//		if (result == -1)	//new label is not dominated
						//			setDaughterLabelsHeuristic.push_back(newLabel);

						//		result = checkDominancePossibleRoutes(newLabel);
						//		if (result >= 0)
						//			setPossibleRoutes.push_back(newLabel);	//new entry
						//	}
						//}
						//else
						//	setPossibleRoutes.push_back(newLabel);	//new entry
					}
			}
			start = end;
			end = setPossibleRoutes.size();
		}
	}
}

void LabelSetting::setMotherNeighbors(int K)
{
	K_M = K;
}

void LabelSetting::generateDaughterLabels(cycle_type cycle)
{
	typeGeneratedRoutes = cycle;
	setDaughterLabels.resize(pbm->nbPorts + 1);
	//Generate the daughter vessel routes for the vessel with the highest capacity
	for (int p = 1; p <= pbm->nbPorts; p++)
		if (pbm->getPortType(p) == Ports::M_port || pbm->getPortType(p) == Ports::M_port_double_visit)
		{
			cout << "Generating the routes for main port " << p << endl;
			generateDaughterLabelsForVesselType(p);
			if (cycle == BUTTERFLY_CYCLE)
				generateSecondLoopForVesselType(p);
		}
	cout << "Indexing and computing the costs" << endl;
	indexLabels();
	computeDaughterCosts();
	cout << "Daughter routes generation completed" << endl;
}

void LabelSetting::generateDaughterLabelsHeuristic(cycle_type cycle)
{
	typeGeneratedRoutes = cycle;
	//Generate the daughter vessel routes for the vessel with the highest capacity
	generateDaughterLabelsForVesselTypeHeuristic();
	if (cycle == BUTTERFLY_CYCLE)
		generateSecondLoopForVesselTypeHeuristic();

	cout << "Indexing and computing the costs" << endl;
	indexLabelsHeuristic(setDaughterLabelsHeuristic);
	computeDaughterCostsHeuristic(setDaughterLabelsHeuristic);
	cout << "Daughter routes generation completed" << endl;
}

void LabelSetting::generateSecondLoopForVesselType(int m_port)
{
	//vector<int> V1({3, }), V2({4, 5 });
	int nbButterflyRoutes = 0;
	for (LabelIterator label = setDaughterLabels[m_port].begin(); label != setDaughterLabels[m_port].end(); label++)
	{
		setPossibleRoutes.clear(); setLabelsLoop2.clear(); levelSetLabels.clear(); levelSetPossibleRoutes.clear();
		//construct the list of candidates port
		//if (label->route == V1)
		//	nbButterflyRoutes = nbButterflyRoutes;
		vector<int> candidates;
		vector<int> markVisitedPorts(pbm->nbPorts + 1, 0);
		markVisitedPorts[label->hub] = 1;
		int minPort = label->route.front();
		for (int i = 0; i < label->route.size(); i++)
		{
			markVisitedPorts[label->route[i]] = 1;
			if (label->route[i] < minPort)
				minPort = label->route[i];
		}
		for (int i = 1/*minPort + 1*/; i <= pbm->nbPorts; i++)
			if (!markVisitedPorts[i])
				candidates.push_back(i);

		for (int p = 0; p < candidates.size(); p++)
		{
			Label lab2(pbm, label._Ptr);
			lab2.updateLabelLoop2(*label, candidates[p]);
			if (lab2.feasibleRoute())
			{
				//setLabels.push_back(lab2);
				setLabelsLoop2.push_back(lab2);
				setPossibleRoutes.push_back(lab2);
				nbButterflyRoutes++;
			}
		}

		//generate the other routes from candidates
		levelSetLabels.push_back(0);
		levelSetPossibleRoutes.push_back(0);

		int start = 0, end = setPossibleRoutes.size();
		levelSetPossibleRoutes.push_back(end - 1);
		levelSetLabels.push_back(end - 1);

		while (start < end)
		{
			//start --> end represents a LEVEL of a route (number of ports included)
			//Add a new port (when possible) to each route of this LEVEL.
			for (int i = start; i < end; i++)
				for (int p = 0; p < candidates.size(); p++)
					if ((setPossibleRoutes[i].mod % (int)pow(2, candidates[p] + 1)) < pow(2, candidates[p]))
					{
						Label newLabel = setPossibleRoutes[i];
						newLabel.updateLabelLoop2(*label, candidates[p]);
						//if (label->hub == 2 && label->route == V1 && newLabel.route == V2)
						//	p = p;
						if (newLabel.feasibleRoute())
						{
							int result = isNonDominated(setLabelsLoop2, newLabel);
							if (result == -1) {	//new label is not dominated
								setLabelsLoop2.push_back(newLabel);
								nbButterflyRoutes++;
							}
							//Check dominance with setPossibleRoutes
							result = checkDominancePossibleRoutes(newLabel);// , indexDeleted);
							if (result >= 0)
								setPossibleRoutes.push_back(newLabel);	//new entry
						}
					}
			start = end;
			end = setPossibleRoutes.size();
			levelSetPossibleRoutes.push_back(end - 1);
			levelSetLabels.push_back(setLabelsLoop2.size() - 1);
		}
		label->setLoop2 = setLabelsLoop2;
	}
}

void LabelSetting::generateSecondLoopForVesselTypeHeuristic()
{
	//vector<int> V1({ 16, 15 }), V2({ 13, 12 });
	vector<vector<Label*>> labelsOfPort;
	labelsOfPort.resize(pbm->nbPorts + 1);
	for (LabelIterator lab = setDaughterLabelsHeuristic.begin(); lab != setDaughterLabelsHeuristic.end(); lab++)
		labelsOfPort[lab->hub].push_back(lab._Ptr);

	for (int p = pbm->nbPorts; p >= 2; p--)
		for (auto lab1 = labelsOfPort[p].begin(); lab1 != (labelsOfPort[p].size() > 0 ? prev(labelsOfPort[p].end()) : labelsOfPort[p].end()); lab1++)
			for (auto lab2 = lab1 + 1; lab2 != labelsOfPort[p].end(); lab2++)
			{
				if ((*lab1)->differentPorts(**lab2))
				{
					double totTime1 = (*lab1)->time + (*lab1)->returnSailTime + (*lab1)->cargoTimeMPort;
					double totTime2 = (*lab2)->time + (*lab2)->returnSailTime + (*lab2)->cargoTimeMPort;
					//if ((*lab1)->route == V1 && (*lab2)->route == V2)
					//	V1.clear();
					if (totTime1 + totTime2 <= pbm->MAX_TIME)
					{
						(*lab1)->setLoop2.push_back(**lab2);
						(*lab1)->setLoop2.back().updateLabelLoop2Heuristic(**lab1);
					}
				}
			}
	labelsOfPort.clear();
}

void LabelSetting::computeDaughterCosts()
{
	//	vector<int> V1({ 2 }), V2({ 4, 6, 7, 9 });
		//for (int v = 1; v <= nbVesselType; v++)
	int route_time_limit = (frequency == TWICE_WEEK) ? (pbm->MAX_TIME / 2.0) : pbm->MAX_TIME;
	for (int m_port = 1; m_port <= pbm->nbPorts; m_port++)
	{
		if (pbm->getPortType(m_port) == Ports::M_port || pbm->getPortType(m_port) == Ports::M_port_double_visit)
			for (LabelIterator label = setDaughterLabels[m_port].begin(); label != setDaughterLabels[m_port].end(); label++)
				//for (LabelIterator label = setAllDaughterRoutes.begin(); label != setAllDaughterRoutes.end(); label++)
			{
				double sumToDeliver = 0, sumToPickUp = 0;
				double cargoHandledInPort = 0;
				double totSailTime = 0, fixedCost = Ports::FIXED_PORT_COST_DAUGHTER;	//cost at transshipment port
				double computeCost = 0;	//handling cost

				for (int p = 0; p < label->route.size(); p++)
				{
					sumToDeliver += pbm->getDeliver(label->route[p]);
					sumToPickUp += pbm->getPickUp(label->route[p]);
					cargoHandledInPort = pbm->getDeliver(label->route[p]) + pbm->getPickUp(label->route[p]);
					computeCost += cargoHandledInPort * Ports::CARGO_HANDLING_COST_PORT;
					fixedCost += Ports::FIXED_PORT_COST_DAUGHTER;

					if (p > 0)
						totSailTime += pbm->time_matrix[label->route[p - 1]][label->route[p]];
				}
				totSailTime += pbm->time_matrix[label->hub][label->route[0]];		//to go from the main port
				totSailTime += pbm->time_matrix[label->route.back()][label->hub];	//to go back to the main port
				double speed = pbm->vesselSpeed;
				//if (speed_mode == OPTIMIZE_SPEED)
				//	speed = getLowestVesselSpeed(indexVessel, totSailTime, route_time_limit);
				label->speed = speed;

				//Cost for loop 1
				label->cargoCost = computeCost + (sumToDeliver + sumToPickUp) * Ports::CARGO_HANDLING_COST_PORT;
				label->bunkerCost = computeVesselBunkerCost(label->vessel, totSailTime, speed);	//totSailTime * pbm->vessels[indexVessel].fuelConsumption * Ports::COST_BUNKER;
				label->costPorts = fixedCost;
				label->cost = label->cargoCost + label->bunkerCost + label->costPorts;

				//frequency
				label->nbVessels = 1;
				if (frequency == TWICE_WEEK)
					if (label->getWholeTime() <= route_time_limit)	//half a week
						label->cost *= 2;
					else
						label->nbVessels = 2;

				//Costs for second loops
				double totSailTime2, fixedCost2, sumToDeliver2, sumToPickUp2;
				for (LabelIterator loop2 = label->setLoop2.begin(); loop2 != label->setLoop2.end(); loop2++)
				{
					//if (label->hub == 3 && label->route == V1 && loop2->route == V2)
					//	speed = speed;
					loop2->cargoCost = computeCost;
					totSailTime2 = totSailTime;
					sumToDeliver2 = sumToDeliver;
					sumToPickUp2 = sumToPickUp;
					fixedCost2 = fixedCost + Ports::FIXED_PORT_COST_DAUGHTER;
					for (int p2 = 0; p2 < loop2->route.size(); p2++)
					{
						sumToDeliver2 += pbm->getDeliver(loop2->route[p2]);
						sumToPickUp2 += pbm->getPickUp(loop2->route[p2]);
						cargoHandledInPort = pbm->getDeliver(loop2->route[p2]) + pbm->getPickUp(loop2->route[p2]);
						loop2->cargoCost += cargoHandledInPort * Ports::CARGO_HANDLING_COST_PORT;
						fixedCost2 += Ports::FIXED_PORT_COST_DAUGHTER;
						if (p2 > 0)
							totSailTime2 += pbm->time_matrix[loop2->route[p2 - 1]][loop2->route[p2]];
					}
					totSailTime2 += pbm->time_matrix[label->hub][loop2->route[0]];
					totSailTime2 += pbm->time_matrix[loop2->route.back()][label->hub];
					//speed = getLowestVesselSpeed(indexVessel, totSailTime, route_time_limit);
					//if (speed_mode == OPTIMIZE_SPEED)
					//	speed = getLowestVesselSpeed(indexVessel, totSailTime, route_time_limit);
					label->speed = speed;

					loop2->cargoCost += (sumToDeliver2 + sumToPickUp2) * Ports::CARGO_HANDLING_COST_PORT;
					//double bunkerCost = totSailTime2 * pbm->vessels[indexVessel].fuelConsumption * Ports::COST_BUNKER;
					loop2->bunkerCost = computeVesselBunkerCost(loop2->vessel, totSailTime2, speed);	//totSailTime * pbm->vessels[indexVessel].fuelConsumption * Ports::COST_BUNKER;
					loop2->costPorts = fixedCost2;
					loop2->cost = loop2->cargoCost + loop2->bunkerCost + loop2->costPorts;
					//frequency
					if (frequency == TWICE_WEEK)
						if (label->getWholeTime() <= pbm->MAX_TIME / 2.0)	//half a week
							label->cost *= 2;
						else
							label->nbVessels = 2;
				}
			}
	}
}

void LabelSetting::computeDaughterCostsHeuristic(vector<Label>& setLabels, bool mother_vessels)
{
	//for (int v = 1; v <= nbVesselType; v++)
	int route_time_limit = (frequency == TWICE_WEEK) ? (pbm->MAX_TIME / 2.0) : pbm->MAX_TIME;
	for (LabelIterator label = setLabels.begin(); label != setLabels.end(); label++)
	{
		double sumToDeliver = 0, sumToPickUp = 0;
		double cargoHandledInPort = 0;
		double totSailTime = 0, fixedCost;	//cost at transshipment port
		double computeCost = 0;
		if (mother_vessels)
			fixedCost = Ports::FIXED_PORT_COST_MOTHER;
		else
			fixedCost = Ports::FIXED_PORT_COST_DAUGHTER;

		for (int p = 0; p < label->route.size(); p++)
		{
			sumToDeliver += pbm->getDeliver(label->route[p]);
			sumToPickUp += pbm->getPickUp(label->route[p]);
			cargoHandledInPort = pbm->getDeliver(label->route[p]) + pbm->getPickUp(label->route[p]);
			computeCost += cargoHandledInPort * Ports::CARGO_HANDLING_COST_PORT;
			if (mother_vessels)
				fixedCost += Ports::FIXED_PORT_COST_MOTHER;
			else
				fixedCost += Ports::FIXED_PORT_COST_DAUGHTER;

			if (p > 0)
				totSailTime += pbm->time_matrix[label->route[p - 1]][label->route[p]];
		}
		totSailTime += pbm->time_matrix[label->hub][label->route[0]];		//to go from the main port
		label->time = totSailTime;
		totSailTime += pbm->time_matrix[label->route.back()][label->hub];	//to go back to the main port
		double speed = pbm->vesselSpeed;
		if (speed_mode == OPTIMIZE_SPEED)
			speed = getLowestVesselSpeed(label->vessel, totSailTime, route_time_limit);
		label->speed = speed;

		//Cost for loop 1
		label->cargoCost = computeCost + (sumToDeliver + sumToPickUp) * Ports::CARGO_HANDLING_COST_PORT;
		if (mother_vessels)
			label->bunkerCost = totSailTime * Ports::COST_BUNKER;
		else
			label->bunkerCost = computeVesselBunkerCost(label->vessel, totSailTime, speed);	//totSailTime * pbm->vessels[indexVessel].fuelConsumption * Ports::COST_BUNKER;
		label->costPorts = fixedCost;
		label->cost = label->cargoCost + label->bunkerCost + label->costPorts;

		//frequency
		label->nbVessels = 1;
		if (frequency == TWICE_WEEK)
			if (label->getWholeTime() <= route_time_limit)	//half a week
				label->cost *= 2;
			else
				label->nbVessels = 2;

		//Costs for second loops
		double totSailTime2, fixedCost2, sumToDeliver2, sumToPickUp2;
		for (LabelIterator loop2 = label->setLoop2.begin(); loop2 != label->setLoop2.end(); loop2++)
		{
			loop2->cargoCost = computeCost;
			totSailTime2 = totSailTime;
			sumToDeliver2 = sumToDeliver;
			sumToPickUp2 = sumToPickUp;
			fixedCost2 = fixedCost + Ports::FIXED_PORT_COST_DAUGHTER;
			for (int p2 = 0; p2 < loop2->route.size(); p2++)
			{
				sumToDeliver2 += pbm->getDeliver(loop2->route[p2]);
				sumToPickUp2 += pbm->getPickUp(loop2->route[p2]);
				cargoHandledInPort = pbm->getDeliver(loop2->route[p2]) + pbm->getPickUp(loop2->route[p2]);
				loop2->cargoCost += cargoHandledInPort * Ports::CARGO_HANDLING_COST_PORT;
				fixedCost2 += Ports::FIXED_PORT_COST_DAUGHTER;
				if (p2 > 0)
					totSailTime2 += pbm->time_matrix[loop2->route[p2 - 1]][loop2->route[p2]];
			}
			totSailTime2 += pbm->time_matrix[label->hub][loop2->route[0]];
			totSailTime2 += pbm->time_matrix[loop2->route.back()][label->hub];
			//double speed = getLowestVesselSpeed(indexVessel, totSailTime, route_time_limit);
			label->speed = speed;

			loop2->cargoCost += (sumToDeliver2 + sumToPickUp2) * Ports::CARGO_HANDLING_COST_PORT;
			//double bunkerCost = totSailTime2 * pbm->vessels[indexVessel].fuelConsumption * Ports::COST_BUNKER;
			loop2->bunkerCost = computeVesselBunkerCost(loop2->vessel, totSailTime2, speed);	//totSailTime * pbm->vessels[indexVessel].fuelConsumption * Ports::COST_BUNKER;
			loop2->costPorts = fixedCost2;
			loop2->cost = loop2->cargoCost + loop2->bunkerCost + loop2->costPorts;
			//frequency
			if (frequency == TWICE_WEEK)
				if (label->getWholeTime() <= pbm->MAX_TIME / 2.0)	//half a week
					label->cost *= 2;
				else
					label->nbVessels = 2;
		}
	}
}

void LabelSetting::generateMotherRoutes()
{
	vector<int> candidates;
	int index = 1;
	//vector<int>	V({15, 14, 13, 12, 11, 10, 9, 8});

	if (cargoType == ROUTE_CARGO)
		cout << "Vessel capacity: Mother routes only mode -- the appropriate mother vessel with enough capacity to transport the route cargo is selected" << endl;
	else
		cout << "Vessel capacity: Transshipment mode -- the mother vessel has enough capacity to transport all cargoes" << endl;

	for (int p = 1; p <= pbm->nbPorts; p++)
		if (pbm->getPortType(p) == Ports::M_port || pbm->getPortType(p) == Ports::M_port_double_visit)
			candidates.push_back(p);

	levelSetPossibleRoutes.clear();
	for (int p = 0; p < candidates.size(); p++)
	{
		LabelMother newLabel(pbm, cargoType);
		newLabel.CM = 1;
		newLabel.addPort(candidates[p]);
		if (newLabel.feasibleRoute(LabelMother::PARTIAL_ROUTE))
		{
			newLabel.index = index++;
			newLabel.computeSailingTime();
			newLabel.computeTimesWithCM_Port();
			newLabel.computeCostsWithCM_Port();
			//Check the feasibility
			setPossibleMotherRoutes.push_back(newLabel);
			setMotherLabels.push_back(newLabel);
		}
	}

	levelSetPossibleRoutes.push_back(0);

	int start = 0, end = setPossibleMotherRoutes.size();
	levelSetPossibleRoutes.push_back(end - 1);

	while (start < end)
	{
		//start --> end represents a LEVEL of a route (number of ports included)
		//Add a new port (when possible) to each route of this LEVEL.
		for (int i = start; i < end; i++)
		{
			for (int p = 0; p < candidates.size(); p++)
			//for (auto candidatePort : pbm->neighborPorts[setPossibleMotherRoutes[i].route.back()])
			//for (int p = 0; p < pbm->neighborPorts[setPossibleMotherRoutes[i].route.back()].size(); p++)
			if (setPossibleMotherRoutes[i].route.back() != candidates[p])
			{
				LabelMother newLabel = setPossibleMotherRoutes[i];
				//if (newLabel.route == V)
				//	p = p;
				//if (p >= K_M)	//(++k <= K_M)
				//	break;
				//int candidatePort = pbm->neighborPorts[setPossibleMotherRoutes[i].route.back()][p];
				if (abs(newLabel.route.back() - candidates[p]) <= K_M)
				{
					newLabel.addPort(candidates[p]);
					//newLabel.addPort(candidatePort);
					if (newLabel.feasibleRoute(LabelMother::PARTIAL_ROUTE))
					{
						newLabel.computeSailingTime();
						newLabel.computeTimesWithCM_Port();
						newLabel.computeCostsWithCM_Port();
						setPossibleMotherRoutes.push_back(newLabel);
						if (newLabel.feasibleRoute(LabelMother::COMPLETE_ROUTE))
						{
							newLabel.index = index++;
							/*						if (newLabel.index == 14912)
														newLabel.computeTimesWithCM_Port();*/
														//	newLabel.addPort(0);
							setMotherLabels.push_back(newLabel);
							//display_vector(newLabel.route); cout << endl;
						}
					}
				}
			}
		}
		start = end;
		end = setPossibleMotherRoutes.size();
		levelSetPossibleRoutes.push_back(end - 1);
	}
}

void LabelSetting::generateMotherRoutesV2()	//generate the mother route in the same way as daughter route
											//this useful for the LIner-Lib instacnes where no south-north-bound can be set
{
	int start, end;
	int cp_port;

	for (int i = 1; cp_port = i, i <= pbm->nbPorts&&pbm->ports[i].type != "CP"; ++i);
	levelSetLabels.clear();
	levelSetPossibleRoutes.clear();
	setPossibleRoutes.clear();

	levelSetPossibleRoutes.clear();
	for (int p = 2; p <= pbm->nbPorts; p++)
	{
		Label currentLabel(pbm);
		currentLabel.setHubPort(cp_port);
		currentLabel.addPortHeuristic(p);
		currentLabel.updateLabelHeuristic();
		setMotherLabelsHeuristic.push_back(currentLabel);
		setPossibleRoutes.push_back(currentLabel);
	}
	start = 0;
	end = setPossibleRoutes.size();

	while (start < end)
	{
		for (int i = start; i < end; i++)
		{
			int currentPort = setPossibleRoutes[i].route.empty()?
							setPossibleRoutes[i].hub: setPossibleRoutes[i].route.back();	//port from which the route will be extended. Always = CP
			//for (auto r : setPossibleRoutes[i].route)
			//	if (currentPort > r) currentPort = r;		//the currentport should always be the southernmost port visited.

			for (int p = 1; p < portsToMain[currentPort].size(); p++)	//DEBUG check the ports for extension
				if ((setPossibleRoutes[i].mod % (int)pow(2, portsToMain[currentPort][p] + 1)) < pow(2, portsToMain[currentPort][p]))
				{
					int port = portsToMain[currentPort][p];
					Label newLabel(setPossibleRoutes[i]);
					newLabel.addPortHeuristic(port);
					newLabel.updateLabelHeuristic();
					if (newLabel.feasibleMotherRoute())
					{
						int result = isNonDominatedM(setMotherLabelsHeuristic, newLabel);
						if (result == -1)	//new label is not dominated
							setMotherLabelsHeuristic.push_back(newLabel);

						result = checkDominancePossibleRoutes(newLabel);
						if (result >= 0)
							setPossibleRoutes.push_back(newLabel);	//new entry
					}
				}
		}
		start = end;
		end = setPossibleRoutes.size();
	}
	indexLabelsHeuristic(setMotherLabelsHeuristic);
	computeDaughterCostsHeuristic(setMotherLabelsHeuristic, true);
	for (auto lblm : setMotherLabelsHeuristic)
	{
		LabelMother newLabel(pbm, cargoType);
		newLabel.index = lblm.index;
		newLabel.CM = lblm.hub;
		newLabel.route = lblm.route;
		newLabel.time = lblm.time;
		newLabel.maxCargoHandled = lblm.qNeed;
		newLabel.bunkerCost = lblm.bunkerCost;
		newLabel.bunkerCostCM = lblm.bunkerCost;
		newLabel.cargoCost = lblm.cargoCost;
		newLabel.costPorts = lblm.costPorts;
		setMotherLabels.push_back(newLabel);
	}
}

void LabelSetting::constructCombinedDaughterRoutes()
{
	if (typeGeneratedRoutes != SIMPLE_CYCLE)
	{
		cout << "constructCombinedDaughterRoutes is valid only for simple cycles" << endl;
		return;
	}
	if (setCombinedDSets.empty())
		setCombinedDSets.resize(nbVesselType + 1);

	for (int v = 1; v <= nbVesselType; v++)
	{
		int size = getNbDaughterRoutes();
		setCombinedDSets[v].resize(size + 1);
		for (int m = 1; m <= pbm->nbPorts; m++)
			if (pbm->getPortType(m) == Ports::M_port || pbm->getPortType(m) == Ports::M_port_double_visit)
			{
				//setCombinedDSets[v].resize(setCombinedDSets[v].size() + setDaughterLabels[v][m].size());
				for (int d = 0; d < setDaughterLabels[m].size(); d++)
				{
					setCombinedDSets[v][setDaughterLabels[m][d].index].push_back(&setDaughterLabels[m][d]);
					for (int d2 = 0; d2 < setDaughterLabels[m].size(); d2++)
					{
						if (d == d2) continue;
						//check if d and d2 can be combined in one cycle
						double totTime = setDaughterLabels[m][d].timeLoop1 + setDaughterLabels[m][d2].timeLoop1;
						if (totTime <= pbm->MAX_TIME)
							setCombinedDSets[v][setDaughterLabels[m][d].index].push_back(&setDaughterLabels[m][d2]);
					}
				}
			}
	}
}

int LabelSetting::getNbDaughterRoutes()
{
	int nbRoutes = 0;
	if (!setDaughterLabels.empty())
	{
		for (int m_port = 1; m_port <= pbm->nbPorts; m_port++)
			if (pbm->getPortType(m_port) == Ports::M_port || pbm->getPortType(m_port) == Ports::M_port_double_visit)
				for (LabelIterator label = setDaughterLabels[m_port].begin(); label != setDaughterLabels[m_port].end(); label++)
				{
					nbRoutes++;
					for (LabelIterator loop2 = label->setLoop2.begin(); loop2 != label->setLoop2.end(); loop2++)
						nbRoutes++;
				}
	}
	else if (!setDaughterLabelsHeuristic.empty())
	{
		for (LabelIterator label = setDaughterLabelsHeuristic.begin(); label != setDaughterLabelsHeuristic.end(); label++)
		{
			nbRoutes++;
			for (LabelIterator loop2 = label->setLoop2.begin(); loop2 != label->setLoop2.end(); loop2++)
				nbRoutes++;
		}
	}
	return nbRoutes;
}

int LabelSetting::getNbMotherRoutes()
{
	return setMotherLabels.size();
}

void LabelSetting::transformFromShipTypeToSpecificShips(int nbReplicates)
{
	this->nbReplicates = nbReplicates;
	//int nbVessels = nbVesselType * nbReplicates;
	//setDaughterLabels.resize(nbVessels + 1);
	//int end = nbVessels;
	//int start = end - nbReplicates + 1;
	//for (int indexCopy = nbVesselType; indexCopy >= 1; indexCopy--)
	//{
	//	for (int i = end; i >= start; i--)
	//	{
	//		//make a copy
	//		setDaughterLabels[i].resize(setDaughterLabels[indexCopy].size());
	//		for (int p = 1; p <= pbm->nbPorts; p++)
	//		{
	//			setDaughterLabels[i][p].resize(0);
	//			for (LabelIterator label = setDaughterLabels[indexCopy][p].begin(); label != setDaughterLabels[indexCopy][p].end(); label++)
	//				setDaughterLabels[i][p].push_back(*label);
	//		}
	//	}
	//	end -= nbReplicates;
	//	start -= nbReplicates;
	//	if (start == 1)
	//		start = 2;
	//}
}

void LabelSetting::writeDaughterRoutes(const char *fileName)
{
	FILE *f = fopen(fileName, "w");
	for (int v = 1; v <= nbVesselType; v++)
	{
		fprintf(f, "Vessel %d\n", v);
		for (int m_port = 1; m_port <= pbm->nbPorts; m_port++)
			for (LabelIterator label = setDaughterLabels[m_port].begin(); label != setDaughterLabels[m_port].end(); label++)
			{
				fprintf(f, "%d ", label->hub);
				for (vector<int>::iterator ii = label->route.begin(); ii != label->route.end(); ii++)
					fprintf(f, "%d ", *ii);
				fprintf(f, "%d ", label->hub);
				fprintf(f, "%.2f", label->cost);
				fprintf(f, "\n");

				for (LabelIterator it = label->setLoop2.begin(); it != label->setLoop2.end(); it++)
				{
					fprintf(f, "%d ", label->hub);
					for (vector<int>::iterator ii = label->route.begin(); ii != label->route.end(); ii++)
						fprintf(f, "%d ", *ii);
					fprintf(f, "%d ", label->hub);
					for (int k = 0; k < it->route.size(); k++)
						fprintf(f, "%d ", it->route[k]);
					fprintf(f, "%d ", label->hub);
					fprintf(f, "%.2f", it->cost);
					fprintf(f, "\n");
				}
			}
	}
	fclose(f);
}

void LabelSetting::indexLabels()
{
	int ind = 1;
	for (LabelMotherIterator lab = setMotherLabels.begin(); lab != setMotherLabels.end(); lab++)
		lab->index = ind++;

	//index the first set of vessel capacity
	ind = 1;
	for (int m_port = 1; m_port <= pbm->nbPorts; m_port++)
		for (LabelIterator label = setDaughterLabels[m_port].begin(); label != setDaughterLabels[m_port].end(); label++)
		{
			label->index = ind++;
			for (LabelIterator it = label->setLoop2.begin(); it != label->setLoop2.end(); it++)
				it->index = ind++;
		}
}

void LabelSetting::indexLabelsHeuristic(vector<Label>& setLabels)
{
	//index the first set of vessel capacity
	int ind = 1;
	for (LabelIterator label = setLabels.begin(); label != setLabels.end(); label++)
	{
		label->index = ind++;
		for (LabelIterator it = label->setLoop2.begin(); it != label->setLoop2.end(); it++)
			it->index = ind++;
	}
}

double LabelSetting::getLowestVesselSpeed(int indexVessel, double sailTimeWithStandardSpeed, double timeLimit)
{
	double sailTime = sailTimeWithStandardSpeed, speed = 12;

	for (int i = 0; i < pbm->vessels[indexVessel].speed_ratio.size(); i++)
	{
		sailTime = sailTimeWithStandardSpeed * 12.0 / pbm->vessels[indexVessel].speed_ratio[i].speed;
		if (sailTime > timeLimit)
			return speed;
		else if (speed > pbm->vessels[indexVessel].speed_ratio[i].speed)
			speed = pbm->vessels[indexVessel].speed_ratio[i].speed;
	}
	return speed;
}

double LabelSetting::computeVesselBunkerCost(int indexVessel, double sailTimeWithStandardSpeed, float speed)
{
	double ratio = 0, cost = 0;

	for (int i = 0; i < pbm->vessels[indexVessel].speed_ratio.size(); i++)
		if (pbm->vessels[indexVessel].speed_ratio[i].speed == speed)
		{
			ratio = pbm->vessels[indexVessel].speed_ratio[i].ratio;
			break;
		}

	if (ratio != 0)
	{
		double newSailingTime = sailTimeWithStandardSpeed * 12.0 / speed;
		cost = newSailingTime * pbm->vessels[indexVessel].fuelConsumption * ratio * Ports::COST_BUNKER;
	}
	else
	{
		cout << "LabelSetting::computeVesselBunkerCost Speed was not found, please check the parameters!" << endl;
		exit(-1);
	}
	return cost;
}

void LabelSetting::writeOPLFormat(const char *directory, const char *fileName, bool printDemand)
{
	string fullpath(directory);
	fullpath += "\\";
	fullpath += fileName;
	ofstream out(fullpath, ios::out);
	int line = 1;

	if (!out) {
		cout << "Error to write to " << fileName << endl;
		return;
	}
	out.setf(ios::fixed, ios::floatfield);
	out.setf(ios::showpoint);
	out << "fileName = \"" << fileName << "\";" << endl;
	if (!setDaughterLabels.empty() || !setDaughterLabelsHeuristic.empty())
	{
		out << "nbVessel = " << nbVesselType * nbReplicates << ";" << endl;
		//write the charter cost
		out << "charterCost = [" << endl;
		for (int v = 1, indexV = 1; v <= nbVesselType; v++)
			for (int cp = 1; cp <= nbReplicates; cp++, indexV++)
				out << pbm->vessels[v].charterCost << " ";
		out << "];" << endl;
	}
	if (cargoType == UNDEFINED_CAPACITY)
	{
		out << "motherVesselType = " << pbm->motherVessels.size() << ";" << endl;
		out << "MV = [" << endl;
		for (int m = 0; m < pbm->motherVessels.size(); m++)
			out << "<" << pbm->motherVessels[m].capacity << "	" << pbm->motherVessels[m].fuelConsumption << "	" << pbm->motherVessels[m].charterCost << ">" << endl;
		out << "];" << endl << endl;

	}
	if (printDemand)
	{
		out << "Capacity = [";
		for (int v = 1; v <= pbm->nbVessels; v++)
			out << pbm->vessels[v].capacity << "	";
		out << "];" << endl;
	}
	//Write main and small ports
	stringstream ss_m, ss_s;
	ss_m << "PM = {";
	ss_s << "PD = {";
	for (int p = 1; p <= pbm->nbPorts; p++)
	{
		if (pbm->getPortType(p) == Ports::S_port)
			ss_s << "P" << p << ", ";
		else
			ss_m << "P" << p << ", ";
	}
	ss_s << "};";
	ss_m << "};";
	out << ss_m.str() << endl;
	if (!setDaughterLabels.empty() || !setDaughterLabelsHeuristic.empty())
		out << ss_s.str() << endl;

	//write the set of mother routes
	out << "RM = {" << endl;
	for (LabelMotherIterator lab = setMotherLabels.begin(); lab != setMotherLabels.end(); lab++)
	{
		if (line++ % 20 == 0)
			out << endl;
		out << "m" << lab->index << ", ";
	}
	out << endl << "};" << endl;
	if (!setDaughterLabels.empty() || !setDaughterLabelsHeuristic.empty())
	{
		out << "RD = {" << endl;
		for (int cp = 1; cp <= nbReplicates; cp++)
		{
			line = 1;
			if (!setDaughterLabels.empty())
				for (int m_port = 1; m_port <= pbm->nbPorts; m_port++)	//	ALL LABELS
					for (LabelIterator label = setDaughterLabels[m_port].begin(); label != setDaughterLabels[m_port].end(); label++)
					{
						if (!label->toDelete)
						{
							out << "d" << label->index << ", ";
							if (line++ % 20 == 0)
								out << endl;
						}
						for (LabelIterator it = label->setLoop2.begin(); it != label->setLoop2.end(); it++)
							if (!it->toDelete)
							{
								out << "d" << it->index << ", ";
								if (line++ % 20 == 0)
									out << endl;
							}
					}
			else if (!setDaughterLabelsHeuristic.empty())			//	HEURISTIC
				for (LabelIterator label = setDaughterLabelsHeuristic.begin(); label != setDaughterLabelsHeuristic.end(); label++)
				{
					if (!label->toDelete)
					{
						out << "d" << label->index << ", ";
						if (line++ % 20 == 0)
							out << endl;
					}
					for (LabelIterator it = label->setLoop2.begin(); it != label->setLoop2.end(); it++)
					{
						if (!it->toDelete)
						{
							out << "d" << it->index << ", ";
							if (line++ % 20 == 0)
								out << endl;
						}
					}
				}
		}
		out << "};\n\n";
	}

	//write the routes
	out << "routesM =#[" << endl;
	//start with mother routes
	for (LabelMotherIterator lab = setMotherLabels.begin(); lab != setMotherLabels.end(); lab++)
	{
		out << "m" << lab->index << ": <{";
		for (int j = 0; j < lab->route.size(); j++)
			out << "P" << lab->route[j] << ", ";
		out << "P1}, ";
		if (cargoType == ROUTE_CARGO) {
			out << std::setprecision(1) << pbm->motherVessels[lab->indexVesselType].charterCost << ", ";
			out << std::setprecision(2) << pbm->motherVessels[lab->indexVesselType].fuelConsumption << std::setprecision(1);
			out << ", " << lab->bunkerCostCM << ", " << lab->cargoCost << ", " << lab->costPorts << ", " << lab->indexVesselType + 1 << ", " << lab->nbVessel << ", " << lab->costCM << ">" << endl;
		}
		else if (cargoType == UNDEFINED_CAPACITY) {
			double time = lab->time + pbm->time_matrix[lab->route.back()][lab->CM];
			out << std::setprecision(1) << time << ", " << (int)lab->maxCargoHandled << ", " << lab->bunkerCostCM << ", " << lab->cargoCost << ", " << lab->costPorts << ">" << endl;
		}
		else if (cargoType == SYSTEM_CARGO)
			out << std::setprecision(1) << pbm->TIME_CHARTER_MOTHER << ", " << pbm->FUEL_CONSUMPTION
			<< ", " << lab->bunkerCostCM << ", " << lab->cargoCost << ", " << lab->costPorts << ", " << lab->indexVesselType + 1 << ", " << lab->nbVessel << ", " << lab->costCM << ">" << endl;
	}
	out << "]#;" << endl << endl;

	//continue with daughter routes
	/////////////////////////////////////
	if (typeRoute != MOTHER_ROUTES_ONLY)
		out << "routesD =#[" << endl;
	////////     the vessel with highest capacity contains all daughter routes
	if (!setDaughterLabels.empty())							//	ALL LABELS
		for (int m_port = 1; m_port <= pbm->nbPorts; m_port++)
			for (LabelIterator label = setDaughterLabels[m_port].begin(); label != setDaughterLabels[m_port].end(); label++)
			{
				out << "d" << label->index << ": <" << label->vessel << ", " << "{P" << label->hub << ", ";
				for (vector<int>::iterator ii = label->route.begin(); ii != label->route.end(); ii++)
					out << "P" << *ii << ", ";
				out << "P" << label->hub << "}, " << label->nbVessels << ", " << label->qDelivery << ", " << label->qPickUp << ", "
					<< label->cost << ", " << label->bunkerCost << ", " << label->cargoCost << ", " << label->costPorts << ", 1>, " << endl;	//1 ==> simple route
				for (LabelIterator it = label->setLoop2.begin(); it != label->setLoop2.end(); it++)
				{
					out << "d" << it->index << ": <" << it->vessel << ", " << "{P" << label->hub << ", ";
					for (vector<int>::iterator ii = label->route.begin(); ii != label->route.end(); ii++)
						out << "P" << *ii << ", ";
					out << "P" << label->hub << ", ";
					for (int k = 0; k < it->route.size(); k++)
						out << "P" << it->route[k] << ", ";
					out << "P" << label->hub << "}, " << it->nbVessels << ", " << (label->qDelivery + it->qDelivery) << ", " << (label->qPickUp + it->qPickUp) << ", "
						<< it->cost << ", " << it->bunkerCost << ", " << it->cargoCost << ", " << it->costPorts << ", 2>," << endl;	// 2=> butterfly route
				}
			}
	else// if (!setDaughterLabelsHeuristic.empty())			//	HEURISTIC
		for (LabelIterator label = setDaughterLabelsHeuristic.begin(); label != setDaughterLabelsHeuristic.end(); label++)
		{
			out << "d" << label->index << ": <" << label->vessel << ", " << "{P" << label->hub << ", ";
			for (vector<int>::iterator ii = label->route.begin(); ii != label->route.end(); ii++)
				out << "P" << *ii << ", ";
			out << "P" << label->hub << "}, " << label->nbVessels << ", " << label->qDelivery << ", " << label->qPickUp << ", "
				<< label->cost << ", " << label->bunkerCost << ", " << label->cargoCost << ", " << label->costPorts << ", 1>, " << endl;	//1 ==> simple route

			for (LabelIterator it = label->setLoop2.begin(); it != label->setLoop2.end(); it++)
			{
				out << "d" << it->index << ": <" << it->vessel << ", " << "{P" << label->hub << ", ";
				for (vector<int>::iterator ii = label->route.begin(); ii != label->route.end(); ii++)
					out << "P" << *ii << ", ";
				out << "P" << label->hub << ", ";
				for (int k = 0; k < it->route.size(); k++)
					out << "P" << it->route[k] << ", ";
				out << "P" << label->hub << "}, " << it->nbVessels << ", " << (label->qDelivery + it->qDelivery) << ", " << (label->qPickUp + it->qPickUp) << ", "
					<< it->cost << ", " << it->bunkerCost << ", " << it->cargoCost << ", " << it->costPorts << ", 2>," << endl;	// 2=> butterfly route
			}
		}
	if (typeRoute != MOTHER_ROUTES_ONLY)
		out << "]#;" << endl << endl;

	//write the set of R_dv^R: set of the daughter routes that can be combined to represent a butterfly cycle
	if (!setDaughterLabels.empty())
		if (typeGeneratedRoutes == SIMPLE_CYCLE && setCombinedDSets.size() > 0)
		{
			out << "R_combine =#[" << endl;
			for (int v = 1, indexV = 1; v <= nbVesselType; v++)
				for (int cp = 1; cp <= nbReplicates; cp++, indexV++)
				{
					line = 1;
					out << indexV << ":#[" << endl;
					for (int d = 0; d < setCombinedDSets[v].size(); d++)
					{
						//int ind_d = setCombinedDSets[m_port][d][0];
						if (!setCombinedDSets[v][d].empty())
						{
							out << "d" << setCombinedDSets[v][d][0]->index << ": {";
							for (int d2 = 0; d2 < setCombinedDSets[v][d].size(); d2++)
							{
								//int ind_d2 = setCombinedDSets[m_port][d][d2];
								out << "d" << setCombinedDSets[v][d][d2]->index << ", ";
							}
							out << "}," << endl;
						}
					}
					out << "]#," << endl;
				}
			out << "]#;" << endl << endl;
		}
	//cout << "UPDATE WRITEOPLFORMAT METHOD TO INCLUDE THE CASE OF TWICE PER WEEK VISIT" << endl;
	//write the set of demands
	if (printDemand)
	{
		out << "Demand = {" << endl;
		for (int p = 2; p <= pbm->nbPorts; p++)
		{
			if (pbm->ports[p].toDeliver > 0)
				out << "<P1, P" << p << ", " << pbm->ports[p].toDeliver << ">," << endl;
			if (pbm->ports[p].toPickUp > 0)
				out << "<P" << p << ", P1, " << pbm->ports[p].toPickUp << ">," << endl;
		}
		out << "};" << endl;
	}
	if (cargoType == UNDEFINED_CAPACITY)
	{
		out << "Demand = #[" << endl;
		for (int p = 2; p <= pbm->nbPorts; p++)
			out << "P" << p << ": <" << pbm->ports[p].toDeliver << ", " << pbm->ports[p].toPickUp << ">," << endl;
		out << "]#;" << endl;
	}
	out.close();
}

Label *LabelSetting::searchDaughterLabelByIndex(int indexRoute)
{
	if (!setDaughterLabels.empty())
		for (int m_port = 1; m_port <= pbm->nbPorts; m_port++)
			for (LabelIterator label = setDaughterLabels[m_port].begin(); label != setDaughterLabels[m_port].end(); label++)
			{
				if (label->index == indexRoute)
					return label._Ptr;
				for (LabelIterator it = label->setLoop2.begin(); it != label->setLoop2.end(); it++)
					if (it->index == indexRoute)
						return it._Ptr;
			}
	else if (!setDaughterLabelsHeuristic.empty())
		for (LabelIterator label = setDaughterLabelsHeuristic.begin(); label != setDaughterLabelsHeuristic.end(); label++)
		{
			if (label->index == indexRoute)
				return label._Ptr;
			for (LabelIterator it = label->setLoop2.begin(); it != label->setLoop2.end(); it++)
				if (it->index == indexRoute)
					return it._Ptr;
		}

	return NULL;
}

void LabelSetting::testRoutes(vector<vector<int>> mother_routes, vector<vector<int>> daughter_routes)
{
	//mother routes
	for (auto rt : mother_routes)
	{
		Label newLabel(pbm);
		newLabel.hub = 1;
		for (auto p : rt)
		{
			newLabel.addPortHeuristic(p);
			newLabel.updateLabelHeuristic();
		}
		setMotherLabelsHeuristic.push_back(newLabel);
	}
	indexLabelsHeuristic(setMotherLabelsHeuristic);
	computeDaughterCostsHeuristic(setMotherLabelsHeuristic, true);
	//copy to mother labels
	for (auto lblm : setMotherLabelsHeuristic)
	{
		LabelMother newLabel(pbm, cargoType);
		newLabel.index = lblm.index;
		newLabel.CM = lblm.hub;
		newLabel.route = lblm.route;
		newLabel.time = lblm.time;
		newLabel.maxCargoHandled = lblm.qNeed;
		newLabel.bunkerCost = lblm.bunkerCost;
		newLabel.bunkerCostCM = lblm.bunkerCost;
		newLabel.cargoCost = lblm.cargoCost;
		newLabel.costPorts = lblm.costPorts;
		setMotherLabels.push_back(newLabel);
	}
	//Daughter routes
	for (auto rt : daughter_routes)
	{
		Label newLabel(pbm);
		newLabel.hub = rt[0];
		for (int i = 1; i < rt.size(); ++i)
		{
			newLabel.addPortHeuristic(rt[i]);
			newLabel.updateLabelHeuristic();
			if (!newLabel.feasibleRoute())
			{
				cout << "Infeasible daughter route";
				system("pause");
			}
		}
		setDaughterLabelsHeuristic.push_back(newLabel);
	}
	indexLabelsHeuristic(setDaughterLabelsHeuristic);
	computeDaughterCostsHeuristic(setDaughterLabelsHeuristic);
}

//double LabelSetting::computeTransitTime()
//{
//	//compute the transit time for the first demand
//	int indexM = 1 - 1;
//	int indexD = 15;
//	double waitingTime = 0;
//	Label *daughterLbl = searchDaughterLabelByIndex(1, indexD);
//	int transshipmentPort = daughterLbl->hub;
//	int deliveryPort = 10;
//
//	int pm;
//	for (pm = 0; pm < setMotherLabels[indexM].route.size(); pm++)
//	if (setMotherLabels[indexM].route[pm] == transshipmentPort)
//		break;
//
//	if (pm < setMotherLabels[indexM].route.size())
//	{
//		double time = setMotherLabels[indexM].arrivalTimesCM[pm];
//
//		int pd;
//		for (pd = 0; pd < daughterLbl->route.size(); pd++)
//		if (daughterLbl->route[pd] == deliveryPort)
//			break;
//		if (pd < daughterLbl->route.size())
//		{
//			double arrive = daughterLbl->arrivalTimes[pd];
//			double transitTime = time + waitingTime + arrive;
//			cout << "Transit time = " << transitTime << endl;
//		}
//	}
//	return 0;
//}
