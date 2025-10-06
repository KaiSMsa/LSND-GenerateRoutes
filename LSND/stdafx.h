// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <list>
#include <vector>
#include <string>
#include <ostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <iomanip>

using namespace std;

enum cycle_type
{
	SIMPLE_CYCLE,
	BUTTERFLY_CYCLE
};
enum route_type
{
	MOTHER_ROUTES_ONLY,
	MOTHER_DAUGHTER_ROUTES
};
enum cargo_type
{
	ROUTE_CARGO,	//the mother route capacity is limited (to be used when only mother routes are generated)
	SYSTEM_CARGO,	//the mother route transports the cargo of the whole system
	UNDEFINED_CAPACITY	//the capacity of the mother route is decided in the LP
};
enum visit_frequency
{
	WEEKLY,
	TWICE_WEEK		//twice visits per week
};
enum control_speed
{
	STANDARD_SPEED,
	OPTIMIZE_SPEED
};
enum generation_mode
{
	ALL_ROUTES,
	HEURISTIC_ROUTES
};

#define EPSILON 1.e-4
// TODO: reference additional headers your program requires here
