#include "stdafx.h"
#include "Solution.h"
#include "Ports.h"
#include "LabelSetting.h"

Solution::Solution(LabelSetting *lbl)
{
	this->lbl = lbl; 
	pbm = lbl->pbm;
	motherArrivalTimes.resize(pbm->nbPorts + 1, 0);
	motherDepartureTimes.resize(pbm->nbPorts + 1, 0);
	waitDeliveryCargo.resize(pbm->nbPorts + 1, 0);	//each port has a demand to be delivered
	waitPickedCargo.resize(pbm->nbPorts + 1, 0);	//each port has a demand to be picked up
};

Solution::~Solution()
{
}

void Solution::setMotherRoute(int indexM, int type, int nb)
{
	//this->indexMRoute = indexM - 1;
	indexMRoutes.push_back(indexM - 1);
	motherVessels.push_back(make_pair(type, nb));
}

void Solution::addDaughterRoute(int vessel, int indexD)
{
	indexDRoutes.push_back(make_pair(vessel, indexD));
}

//void Solution::getVisitIndexMPort(int port, int &index1, int &index2)
//{
//	vector<int> motherRoute(lbl->setMotherLabels[indexMRoute].route);
//	index1 = index2 = -1;
//	for (int p = 0; p < motherRoute.size(); p++)
//	if (motherRoute[p] == port)
//	if (index1 == -1)
//		index1 = p;
//	else
//		index2 = p;
//}

void Solution::computeMotherRouteTimes(int indexMRoute)
{
	LabelMother *lblMother = &lbl->setMotherLabels[indexMRoute];
	vector<int> motherRoute(lblMother->route);
	vector<int> nbVisits(pbm->nbPorts + 1, 0);	
	int nbVisitedPorts = 0, i = 0;

	mainPorts.clear();
	mainPorts.push_back(MainPort(1, SINGLE_VISIT));	//Departure from the continental port
	for (vector<int>::iterator p = motherRoute.begin(); p != motherRoute.end(); p++, i++) {
		nbVisits[*p] ++;
		MainPort portToAdd(*p, SINGLE_VISIT);
		if (nbVisits[*p] == 2) {
			//double visit ==> update the status
			for (int j = 0; j < mainPorts.size(); j++)
			if (mainPorts[j].port == *p) {
				mainPorts[j].status = NORTH_VISIT;
				break;
			}
			portToAdd.status = SOUTH_VISIT;
		}
		mainPorts.push_back(portToAdd);
	}
	mainPorts.push_back(MainPort(1, SINGLE_VISIT));	//Arrival to the continental port

	for (vector<MainPort>::iterator mPort = mainPorts.begin() + 1; mPort != mainPorts.end(); mPort++)
	{
		int amountToDeliver = pbm->getDeliver(mPort->port);
		int amountToPickUp = pbm->getPickUp(mPort->port);
		for (int d = 0; d < indexDRoutes.size(); d++)
		{
			//is m_port visited by each daughter route
			Label *daughterLbl = lbl->searchDaughterLabelByIndex(indexDRoutes[d].second);
			if (daughterLbl->hub == mPort->port)
			{
				amountToDeliver += daughterLbl->getDelivery();
				amountToPickUp += daughterLbl->getPickUp();
			}
		}
		if (mPort->status == SINGLE_VISIT)
		{
			mPort->deliveredAmount = amountToDeliver;
			mPort->pickedUpAmount = amountToPickUp;
			mainPorts.front().departureTime += (double)amountToDeliver / pbm->CARGO_HANDLING_TIME_CP;	//the continental port
		}
		else if (mPort->status == NORTH_VISIT)
		{
			mPort->deliveredAmount = amountToDeliver;
			//look for the south visit of mPort and update it
			for (vector<MainPort>::iterator p = mPort + 1; p != mainPorts.end(); p++)
			if (p->port == mPort->port)
			{
				p->pickedUpAmount = amountToPickUp;
				break;
			}
			mainPorts.front().departureTime += (double)amountToDeliver / pbm->CARGO_HANDLING_TIME_CP;	//the continental port
		}
	}
	double time = mainPorts.front().departureTime;	//motherDepartureTimes[1];
	int lastPort = 1;
	for (vector<MainPort>::iterator mPort = mainPorts.begin() + 1; mPort != mainPorts.end(); mPort++)
	{
		mPort->arrivalTime = time + pbm->time_matrix[lastPort][mPort->port];
		if (next(mPort) != mainPorts.end())	//not the last port visited
		{
			if (mPort->status == SINGLE_VISIT)
				mPort->departureTime = mPort->arrivalTime + (mPort->deliveredAmount + mPort->pickedUpAmount) / pbm->CARGO_HANDLING_TIME_M_PORT;
			else if (mPort->status == NORTH_VISIT)
				mPort->departureTime = mPort->arrivalTime + mPort->deliveredAmount / pbm->CARGO_HANDLING_TIME_M_PORT;
			else if (mPort->status == SOUTH_VISIT)
				mPort->departureTime = mPort->arrivalTime + mPort->pickedUpAmount / pbm->CARGO_HANDLING_TIME_M_PORT;
		}
		else
			mPort->departureTime = mPort->arrivalTime + pbm->totalPickUp / pbm->CARGO_HANDLING_TIME_CP;
		time = mPort->departureTime;
		lastPort = mPort->port;
	}
}

int  Solution::getVisitedVessel(int indexMRoute, int port)
{
	for (vector<int>::iterator m_port = lbl->setMotherLabels[indexMRoute].route.begin(); m_port != lbl->setMotherLabels[indexMRoute].route.end(); m_port++)
	if (*m_port == port)
		return -1;		//visited by the mother route

	for (int d = 0; d < indexDRoutes.size(); d++)
	{
		Label *daughterLbl = lbl->searchDaughterLabelByIndex(indexDRoutes[d].second);
		if (daughterLbl->isPortVisited(port))
			return d;
		//for (int p = 0; p < daughterLbl->route.size(); p++, indexPort++)
		//if (daughterLbl->route[p] == port)
		//	return d;
	}
	return -2;
}

int Solution::numberMotherRoute()
{
	return motherVessels.size();
}

int Solution::numberDaughterRoutes()
{
	return indexDRoutes.size();
}

bool Solution::includeButterfly()
{
	for (auto rt : indexDRoutes)
	{
		Label * daughterLbl = lbl->searchDaughterLabelByIndex(rt.second);
		if (daughterLbl->isLoop2)
			return true;
	}
	return false;
}

void Solution::computeTransitTimes()
{
	for (auto indexM : indexMRoutes)
	{
		computeMotherRouteTimes(indexM);
		for (int p = 2; p <= pbm->nbPorts; p++)
		{
			int indexVessel = getVisitedVessel(indexM, p);	//which vessel has visited this port? mother or daughter vessel
			if (indexVessel == -1) //mother route visit
			{
				for (vector<MainPort>::iterator mPort = mainPorts.begin() + 1; mPort != mainPorts.end(); mPort++)
				{
					if (mPort->port == p)
						if (mPort->status == SINGLE_VISIT)
						{
							waitDeliveryCargo[p] = mPort->arrivalTime;
							waitPickedCargo[p] = mainPorts.back().arrivalTime - mPort->departureTime;	//To recall, last element = return back time to the CP
							break;
						}
						else if (mPort->status == NORTH_VISIT)
							waitDeliveryCargo[p] = mPort->arrivalTime;
						else if (mPort->status == SOUTH_VISIT)
						{
							waitPickedCargo[p] = mainPorts.back().arrivalTime - mPort->departureTime;	//To recall, last element = return back time to the CP
							break;
						}
				}
			}
			else if (indexVessel >= 0)
			{
				//delivered by a daughter vessel
				Label *daughterLbl = lbl->searchDaughterLabelByIndex(indexDRoutes[indexVessel].second);
				int main_port = daughterLbl->hub;
				double departureVessel;

				//if (pbm->ports[p].toDeliver != 0)
					for (vector<MainPort>::iterator mPort = mainPorts.begin() + 1; mPort != mainPorts.end(); mPort++)
						if (mPort->port == main_port)
						{
							departureVessel = mPort->departureTime;
							waitDeliveryCargo[p] = mPort->departureTime + daughterLbl->getArrivalTime(p);
							break;
						}

				if (pbm->ports[p].toPickUp != 0)
					for (vector<MainPort>::iterator mPort = mainPorts.begin() + 1; mPort != mainPorts.end(); mPort++)
						if (mPort->port == main_port && (mPort->status == SINGLE_VISIT || mPort->status == SOUTH_VISIT))
						{
							double departureFromPort = mPort->departureTime, arrivalToMPort;
							//if (daughterLbl->isVistedInLoop1(p))
							departureFromPort = /*mPort->departureTime*/departureVessel + daughterLbl->getDepartureTime(p);
							//else
							//	departureFromPort = /*mPort->departureTime*/departureVessel + daughterLbl->getDepartureTime(p);

							if (daughterLbl->isLoop2)
								arrivalToMPort = /*mPort->departureTime*/departureVessel + daughterLbl->timeLoop2;					//the arrival of the vessel to the main port
							else
								arrivalToMPort = /*mPort->departureTime*/departureVessel + daughterLbl->timeLoop1;			//the arrival of the vessel to the main port

							if (mPort->arrivalTime >= arrivalToMPort)	//the cargo is unloaded from the daugh. vessel before the arrival of the mother vessel
								waitPickedCargo[p] = mainPorts.back().arrivalTime - departureFromPort;
							else	//wait for the next mother vessel
							{
								int nbWeeks = (int)(arrivalToMPort - mPort->arrivalTime) / 168 + 1;
								waitPickedCargo[p] = 168 * nbWeeks + mainPorts.back().arrivalTime - departureFromPort;
							}
							break;
						}
			}
		}
	}
}

double Solution::avgTransitTimeDelivery()
{
	double sumWeighted = 0, sumCargo = 0;
	for (int p = 2; p <= pbm->nbPorts; p++)
	{
		sumCargo += pbm->getDeliver(p);
		sumWeighted += pbm->getDeliver(p) * waitDeliveryCargo[p];
	}
	return sumWeighted / sumCargo;
}

double Solution::avgTransitTimePickUp()
{
	double sumWeighted = 0, sumCargo = 0;
	for (int p = 2; p <= pbm->nbPorts; p++)
	{
		sumCargo += pbm->getPickUp(p);
		sumWeighted += pbm->getPickUp(p) * waitPickedCargo[p];
	}
	return sumWeighted / sumCargo;
}

double Solution::agTransitTimeCargoes()
{
	double sumWeighted = 0, sumCargo = 0;
	for (int p = 2; p <= pbm->nbPorts; p++)
	{
		sumCargo += pbm->getDeliver(p) + pbm->getPickUp(p);
		sumWeighted += pbm->getDeliver(p) * waitDeliveryCargo[p] + pbm->getPickUp(p) * waitPickedCargo[p];
	}
	return sumWeighted / sumCargo;
}

void Solution::printUsedVessels()
{
	for (int t = 1; t <= 3; t++)	//vessel type
	{
		int nb = 0;
		for (int i = 0; i < indexDRoutes.size(); i++)
		if (indexDRoutes[i].first == t)
			nb ++;
		if (nb>0)
		{
			cout << nb << " ship";
			if (nb > 1) cout << "s";
			switch (t)
			{
			case 1: cout << " 100-TEU "; break;
			case 2: cout << " 200-TEU "; break;
			case 3: cout << " 300-TEU "; break;
			}
		}
	}
	cout << endl;
}

void Solution::printMotherVessels()
{
	for (auto v : motherVessels)
	{
		int ind = v.first - 1;	// 4 - v.first;
		cout << v.second << " ship " << pbm->motherVessels[ind].capacity << "-TEU ; ";
	}
	cout << endl;
}

void Solution::printRoutes()
{
	for (auto indexM : indexMRoutes)
	{
		LabelMother *lblMother = &lbl->setMotherLabels[indexM];
		cout << "m" << indexM + 1 << ": Rotterdam ";
		for (auto p : lblMother->route)
		{
			cout << pbm->ports[p].name << " ";
		}
		cout << "Duration: " << lblMother->timeCM;
		cout << endl;
	}

	for (auto rt : indexDRoutes)
	{
		cout << "V" << rt.first << ", d" << rt.second << " ";
		Label * daughterLbl = lbl->searchDaughterLabelByIndex(rt.second);

		if (daughterLbl->isLoop2)
		{
			cout << "(" << daughterLbl->timeLoop2 << "): ";
			cout << pbm->ports[daughterLbl->hub].name << " ";
			for (auto p : daughterLbl->loop1->route)
				cout << pbm->ports[p].name << " ";
		}
		else
			cout << "(" << daughterLbl->timeLoop1 << "): ";
		cout << pbm->ports[daughterLbl->hub].name << " ";
		for (auto p:daughterLbl->route)
				cout << pbm->ports[p].name << " ";
		cout << pbm->ports[daughterLbl->hub].name << endl;
	}
}