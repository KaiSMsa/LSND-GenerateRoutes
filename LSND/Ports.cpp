#include "stdafx.h"
#include "Ports.h"

const double Ports::CARGO_HANDLING_TIME_PORT = 15;
const double Ports::CARGO_HANDLING_TIME_M_PORT = 15;
const double Ports::CARGO_HANDLING_TIME_CP = 20;
const double Ports::FIXED_PORT_COST_MOTHER = 200;
double Ports::FIXED_PORT_COST_DAUGHTER = 100;
const double Ports::CARGO_HANDLING_COST_PORT = 30;
const double Ports::CARGO_HANDLING_COST_CP = 30;
//const double Ports::COST_CONTAINER_OH = 0;
//const double Ports::COST_CONTAINER_REST = 120;
double Ports::FUEL_CONSUMPTION = 1.9;
const double Ports::COST_BUNKER = 600;
double Ports::TIME_CHARTER_MOTHER = 70000;
const double Ports::REVENUE_CONTAINER = 1000;
//const double Ports::CARGO_HANDLING_COST_OH = 0;

Ports::Ports()
{
	Vessel mother;
	mother.capacity = 700;	mother.fuelConsumption = 0.5; mother.charterCost = 49000;
	motherVessels.push_back(mother);
	//mother.capacity = 800;	mother.fuelConsumption = 0.54; mother.charterCost = 53000;
	//motherVessels.push_back(mother);
	mother.capacity = 900;	mother.fuelConsumption = 0.58; mother.charterCost = 56000;
	motherVessels.push_back(mother);
	mother.capacity = 1100;	mother.fuelConsumption = 0.65;	mother.charterCost = 63000;
	motherVessels.push_back(mother);
	mother.capacity = 1500;	mother.fuelConsumption = 0.73;	mother.charterCost = 70000;
	motherVessels.push_back(mother);
	//mother.capacity = 2000;	mother.fuelConsumption = 0.75;	mother.charterCost = 73500;
	//motherVessels.push_back(mother);
	//mother.capacity = 2550;	mother.fuelConsumption = 0.80;	mother.charterCost = 77000;
	//motherVessels.push_back(mother);
	//mother.capacity = 3500;	mother.fuelConsumption = 0.85;	mother.charterCost = 87500;
	//motherVessels.push_back(mother);

	//mother.capacity = 450;	mother.fuelConsumption = 0.78;	mother.charterCost = 35000;
	//motherVessels.push_back(mother);
	//mother.capacity = 800;	mother.fuelConsumption = 0.62;	mother.charterCost = 56000;
	//motherVessels.push_back(mother);

	//cout << "Mother vessels set to 700-900 TEU" << endl;
	//system("pause");
}


Ports::~Ports()
{
}

void Ports::setDefautMotherVessel(int capacity)
{
	int m;
	for (m = 0; m < motherVessels.size(); m++)
		if (motherVessels[m].capacity == capacity)
		{
			TIME_CHARTER_MOTHER = motherVessels[m].charterCost;
			FUEL_CONSUMPTION = motherVessels[m].fuelConsumption;
			break;
		}
	if (m >= motherVessels.size())
	{
		cout << "Error in setting the mother vessel parameters, please check the caapacity" << endl;
		exit(-2);
	}
}

void Ports::setAllPortsToMain()
{
	for (int p = 1; p <= nbPorts; p++)
		if (ports[p].type == "S")
			ports[p].type = "M1";
}

void Ports::readData(string fileName, route_type route_t, cycle_type cycle_t) {
	ifstream input(fileName, ios::in);
	char line[1024], buffer[51];
	//Vérifier si le fichier existe ou non
	if (!input) {
		cout << "Error to open : " << fileName << endl;
		exit(-1);
	}
	//Read the number of vessels.
	input >> buffer >> nbVessels;
	input.getline(line, 1024);
	vessels.push_back(Vessel());
	for (int v = 1; v <= nbVessels; v++)
	{
		Vessel vv;
		input.getline(line, 1024);
		sscanf(line, "%d %d %lf", &vv.capacity, &vv.charterCost, &vv.fuelConsumption);
		char *ss = line;
		while (*ss)
		{
			if (*ss == '<')
			{
				SpeedRatio sr;
				sscanf(ss, "<%lf%lf>", &sr.speed, &sr.ratio);
				vv.speed_ratio.push_back(sr);
			}
			ss++;
		}
		//Read speed-ratio
		vessels.push_back(vv);
	}
	//Read the number of ports.
	input >> buffer >> nbPorts;
	Port null_port;
	portsM.push_back(null_port); portsS.push_back(null_port); ports.push_back(null_port);
	indexPorts.resize(nbPorts + 1);

	//	int index[] = { 0,
	//	1,
	//2,
	//3,
	//4,
	//5,
	//6,
	//7,
	//8,
	//9,
	//10,
	//11,
	//12,
	//13,
	//14,
	//16,
	//15,
	//17,
	//18,
	//19,
	//20,
	//21,
	//22
	//
	//	};

		//ports.resize(nbPorts + 1);

	for (int p = 1; p <= nbPorts; p++) {
		Port pp;
		input >> pp.index >> pp.name >> pp.type >> pp.toDeliver >> pp.toPickUp;
		if (pp.type == "M1" || pp.type == "M2")
			portsM.push_back(pp);
		else if (pp.type == "S")
			portsS.push_back(pp);
		ports.push_back(pp);
		//ports[index[p]] = pp;
		indexPorts[p] = p;
	}

	distance_matrix.resize(nbPorts + 1);
	for (int p = 1; p <= nbPorts; p++)
		distance_matrix[p].resize(nbPorts + 1);

	for (int p = 1; p <= nbPorts; p++)
		for (int q = 1; q <= nbPorts; q++)
			//input >> distance_matrix[index[p]][index[q]];
			input >> distance_matrix[p][q];

	input.getline(line, 1024);
	input.getline(line, 1024);
	neighborPorts.resize(nbPorts + 1);
	for (int p = 1; p <= nbPorts; p++)
	{
		input.getline(line, 1024);
		stringstream ss;
		int index, next;
		ss << line;
		ss >> index;
		while (ss >> next)
			neighborPorts[index].push_back(next);
		//neighborPorts[index].push_back(n2);
	}

	//check triangular inequality
	bool inequality;
	int it = 0, change = 0;
	do {
		inequality = true;
		for (int i = 1; i <= nbPorts; i++)
			for (int j = 1; j <= nbPorts; j++)
				if (i != j && distance_matrix[i][j] != distance_matrix[j][i])
				{
					distance_matrix[i][j] = distance_matrix[j][i] = max(distance_matrix[i][j], distance_matrix[j][i]);
					cout << "Distance are not symmetric " << ports[i].name << " " << ports[j].name << endl;
					system("pause");
				}

		for (int i = 1; i <= nbPorts; i++)
			for (int j = 1; j <= nbPorts; j++)
				for (int k = 1; k <= nbPorts; k++)
					if (i != j && i != k && k != j && distance_matrix[i][j] - 0.1 > distance_matrix[i][k] + distance_matrix[k][j])
					{
						cout << distance_matrix[i][j] << "	" << distance_matrix[i][j] << endl;
						distance_matrix[i][j] = distance_matrix[i][k] + distance_matrix[k][j];
						inequality = false;
						change++;
						cout << "Trinagular inequality detected for ports: " << ports[i].name << " " << ports[j].name << " " << ports[k].name << endl;
						system("pause");
					}
		it++;
	} while (!inequality);
	if (change)
	{
		cout << "Iterations: " << it << "  change:" << change << endl;
		ofstream out("new_distance.txt", ios::out);
		for (int p = 1; p <= nbPorts; p++)
		{
			for (int q = 1; q <= nbPorts; q++)
				out << distance_matrix[p][q] << "	";
			out << endl;
		}
		out.close();
	}
	time_matrix.resize(nbPorts + 1);
	for (int p = 1; p <= nbPorts; p++)
		time_matrix[p].resize(nbPorts + 1);

	for (int p = 1; p <= nbPorts; p++)
		for (int q = 1; q <= nbPorts; q++)
			time_matrix[p][q] = round(10 * distance_matrix[p][q] / vesselSpeed) / 10;

	totalDelivery = totalPickUp = 0;
	for (int p = 1; p <= nbPorts; p++)
	{
		totalDelivery += ports[p].toDeliver;
		totalPickUp += ports[p].toPickUp;
	}
	//transform the instance
	for (int p = 1; p <= nbPorts; p++) {
		if (cycle_t == BUTTERFLY_CYCLE)
			if (ports[p].type == "M1")
				ports[p].type = "M2";

		if (route_t == MOTHER_ROUTES_ONLY)
			if (ports[p].type == "S")
				ports[p].type = "M1";
	}
}

void selectWithoutRepitition(vector<int> &tab, int size, int min, int max)
{
	//fabrication d'une urne contenant les valeurs de min à max, dans l'ordre croissant
	int tailleUrne = max - min + 1;
	int i;
	int positionTiree;
	//	assert(tailleTab <= tailleUrne);
	vector<int> urne;
	urne.resize(tailleUrne);
	//srand((unsigned int) time(NULL));
	for (i = 0; i < tailleUrne; i++)
		urne[i] = min + i;

	for (i = 0; i < size; i++)
	{
		positionTiree = rand() % tailleUrne;
		tab[i] = urne[positionTiree];
		--tailleUrne;
		urne[positionTiree] = urne[tailleUrne];
	}
}

void Ports::readDataWithIndex(string fileName, route_type route_t, cycle_type cycle_t) {
	ifstream input(fileName, ios::in);
	char line[1024], buffer[51];
	//Vérifier si le fichier existe ou non
	if (!input) {
		cout << "Error to open : " << fileName << endl;
		exit(-1);
	}
	//Read the number of vessels.

	input >> buffer >> nbVessels;
	input.getline(line, 1024);
	vessels.push_back(Vessel());
	for (int v = 1; v <= nbVessels; v++)
	{
		Vessel vv;
		input.getline(line, 1024);
		sscanf(line, "%d %d %lf", &vv.capacity, &vv.charterCost, &vv.fuelConsumption);
		char *ss = line;
		while (*ss)
		{
			if (*ss == '<')
			{
				SpeedRatio sr;
				sscanf(ss, "<%lf%lf>", &sr.speed, &sr.ratio);
				vv.speed_ratio.push_back(sr);
			}
			ss++;
		}
		//Read speed-ratio
		vessels.push_back(vv);
	}
	//Read the number of ports.
	input >> buffer >> nbPorts;
	Port null_port;
	null_port.index = null_port.toDeliver = null_port.toPickUp = NULL;
	null_port.name = ""; null_port.type = "";
	portsM.push_back(null_port); portsS.push_back(null_port); ports.push_back(null_port);

	//	int index[] = { 0,
	//1,
	//2,
	//3,
	//4,
	//5,
	//6,
	//7,
	//8, 
	//9,
	//10,
	//11,
	//12, 
	//13,
	//14,
	//15,
	//16,
	//17,
	//18,
	//19,
	//20,
	//21,
	//22
	//	};
	//	int indexS = sizeof(index) / sizeof(*index) - 1;
	vector<int> index(15);
	selectWithoutRepitition(index, 15, 2, 22);
	index.insert(index.begin(), 0);
	index.insert(index.begin(), 1);
	sort(index.begin(), index.end());
	int indexS = index.size() - 1;

	indexPorts.resize(indexS + 1);
	nbPorts = 22;
	int ii = 1;
	for (int p = 1; p <= nbPorts; p++) {
		Port pp;
		input >> pp.index >> pp.name >> pp.type >> pp.toDeliver >> pp.toPickUp;
		if (pp.type == "M1" || pp.type == "M2")
			portsM.push_back(pp);
		else if (pp.type == "S")
			portsS.push_back(pp);
		if (index[ii] == p)
		{
			pp.index = ii;
			ports.push_back(pp);
			ii++;
		}
	}

	for (int p = 1; p <= ports.size() - 1; p++)
		indexPorts[p] = p;
	distance_matrix.resize(ports.size());
	for (int p = 1; p < ports.size(); p++)
		distance_matrix[p].resize(ports.size());

	int pp = 0, qq = 1;
	for (int p = 1; p <= nbPorts; p++)
	{
		if (index[pp + 1] == p)
			pp++;
		qq = 1;
		for (int q = 1; q <= nbPorts; q++)
		{
			double val;
			input >> val;
			if (index[pp] == p && index[qq] == q)
			{
				distance_matrix[pp][qq] = val;
				qq++;
			}
		}
	}
	input.close();
	nbPorts = (int)ports.size() - 1;

	time_matrix.resize(nbPorts + 1);
	for (int p = 1; p <= nbPorts; p++)
		time_matrix[p].resize(nbPorts + 1);

	for (int p = 1; p <= nbPorts; p++)
		for (int q = 1; q <= nbPorts; q++)
			time_matrix[p][q] = round(10 * distance_matrix[p][q] / vesselSpeed) / 10;

	totalDelivery = totalPickUp = 0;
	for (int p = 1; p <= nbPorts; p++)
	{
		totalDelivery += ports[p].toDeliver;
		totalPickUp += ports[p].toPickUp;
	}
	//transform the instance
	for (int p = 1; p <= nbPorts; p++) {
		if (cycle_t == BUTTERFLY_CYCLE)
			if (ports[p].type == "M1")
				ports[p].type = "M2";

		if (route_t == MOTHER_ROUTES_ONLY)
			if (ports[p].type == "S")
				ports[p].type = "M1";
	}
}

int Ports::getPickUp(int indexPort)
{
	return ports[indexPort].toPickUp;
}

int Ports::getDeliver(int indexPort)
{
	return ports[indexPort].toDeliver;
}

Ports::port_type Ports::getPortType(int indexPort)
{
	//if (ports[indexPort].type == 100)
	//	return M_port;
	//else if (ports[indexPort].type == 200)
	//	return DF_port;
	//else if (ports[indexPort].type == 300)
	//	return LM_port;
	//else if (ports[indexPort].type == 400)
	//	return OH_port;
	if (ports[indexPort].type == "CP")
		return CP_port;
	else if (ports[indexPort].type == "S")
		return S_port;
	else if (ports[indexPort].type == "M1")
		return M_port;
	else if (ports[indexPort].type == "M2")
		return M_port_double_visit;
	else
	{
		cout << "Error in port index " << indexPort << endl;
		exit(-1);
	}
}

void Ports::randomDemand()
{
	cout << "+++++++++++++++ Modifying demand ++++++++++++++" << endl;
	float sumDelivery, sumPickUp;
	bool stop;
	vector<int> delivery, pickup;
	delivery.resize(ports.size() + 1);
	pickup.resize(ports.size() + 1);
	//srand((unsigned int)time(NULL));
	for (int p = 1; p <= nbPorts; p++)
	{
		delivery[p] = ports[p].toDeliver;
		pickup[p] = ports[p].toPickUp;
	}
	do
	{
		sumDelivery = sumPickUp = 0;
		for (int p = 1; p <= nbPorts; p++)
		{
			//ports[p].toDeliver = (int)(delivery[p] * (1.3 + (rand() % 130) / 100.0));	//generate demand between 100 - 160%
			//ports[p].toPickUp = (int)(pickup[p] * (1.3 + (rand() % 130) / 100.0));
			ports[p].toDeliver = (int)(delivery[p] * (0.5 + (rand() % 130) / 100.0));	//generate demand between 50 - 180%
			ports[p].toPickUp = (int)(pickup[p] * (0.5 + (rand() % 130) / 100.0));
			sumDelivery += ports[p].toDeliver;
			sumPickUp += ports[p].toPickUp;
		}
		//stop = sumDelivery >= 1400 && sumDelivery <= 1450 && sumPickUp >= 1450 && sumPickUp <= 1500;	//Increase of 30 %
		stop = sumDelivery < sumPickUp;	// sumDelivery >= 1740 && sumDelivery <= 1750 && sumPickUp >= 1810 && sumPickUp <= 1820;
	} while (!stop);
}

void Ports::displayDemand()
{
	for (int p = 1; p <= nbPorts; p++)
	{
		cout << p << "	" << ports[p].toDeliver << "	" << ports[p].toPickUp << endl;
	}
}

void Ports::writeInstance(int IDfile)
{
	stringstream fileName;
	fileName << "B-test-" << IDfile << ".txt";
	ofstream out(fileName.str(), ios::out);

	out << "#Vessels 3" << endl;
	out << "100	25000	0.23		<8	0.305>	<9	0.403>	<10	0.531>	<11	0.729> < 12	1.000>" << endl;
	out << "200	30000	0.28		<8	0.310>	<9	0.407>	<10	0.533>	<11	0.727> < 12	1.000>" << endl;
	out << "300	35000	0.33		<8	0.320>	<9	0.419>	<10	0.547>	<11	0.738>	<12	1.000>" << endl;

	out << "#Ports	" << nbPorts << endl;

	for (int p = 1; p <= nbPorts; p++) {
		out << p << "	" << ports[p].name << "	" << ports[p].type << "	" << ports[p].toDeliver << "	" << ports[p].toPickUp << endl;
	}
	out << endl;

	for (int p = 1; p <= nbPorts; p++)
	{
		for (int q = 1; q <= nbPorts; q++)
			out << distance_matrix[p][q] << "	";
		out << endl;
	}
	out.close();
}