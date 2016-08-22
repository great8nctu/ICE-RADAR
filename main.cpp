///////////////////////////////////////////////////
// 2015/9/8 last modify
// main.cpp : main function
// Circuit.h : all circuit informations are in here
// TimeVec.h : all information of each pair FF 

#include<iostream>
#include<string.h>
//#include<fstream>
//#include<string>
//#include<string.h>
//#include<ext/hash_map>	        //ext/hash_map
//#include<vector>
//#include<ctime>
#include"Circuit.h"
//#include"Circuit.cpp"
using namespace std;
//using namespace __gnu_cxx;
int main(int argc, char *argv[])
{
	// reading information
	CIRCUIT Circuit;
	Circuit.ReadParameters("parameters.txt");
	float fix_tc= 0.0;
	int i = 1;
	while(i < argc)
	{
		char tmp[100];
		if(!strcmp(argv[i], "-circuit"))
		{
			if(Circuit.GetTimeBorrowing() == 0) { Circuit.SetSampleBoundary(Circuit.GetSampleBoundary() + 0.08); }
			strcpy(tmp, argv[i+1]);
			strcat(tmp, ".vg");
			Circuit.ReadIO(tmp);
			strcat(argv[i+1], ".rpt");
			Circuit.ReadData(argv[i+1]);
			if(fix_tc != 0) { Circuit.SetFixTc(fix_tc); }
		}
		if(!strcmp(argv[i], "-razor_limit"))	{ Circuit.SetRazorLimit(atoi(argv[i+1])); }
		if(!strcmp(argv[i], "-clock_aging"))	{ Circuit.SetCijAging(atof(argv[i+1])); }
		if(!strcmp(argv[i], "-logic_aging_worst"))	{ Circuit.SetLongAging(atof(argv[i+1])); }
		if(!strcmp(argv[i], "-logic_aging_best"))	{ Circuit.SetShortAging(atof(argv[i+1])); }
		if(!strcmp(argv[i], "-clock_margin"))	{ Circuit.SetAvgAging(atof(argv[i+1])); }
		if(!strcmp(argv[i], "-experiment"))	{ Circuit.SetExperiment(atoi(argv[i+1])); }
		if(!strcmp(argv[i], "-sample_boundary"))	{ Circuit.SetSampleBoundary(atof(argv[i+1])); }
		if(!strcmp(argv[i], "-time_borrowing"))	{ Circuit.SetTimeBorrowing(atoi(argv[i+1])); }
		if(!strcmp(argv[i], "-fix_clock")) { fix_tc = atof(argv[i+1]); }
		i += 2;
	}
	// initial set
	Circuit.SetBuf();
	if(Circuit.GetExperiment() == 1)
	{
		Circuit.BinarySearchTc();
		Circuit.ReadSatFile("myout.sat");
		cout << "Final Tc : " << Circuit.GetTc() << endl;
		cout << "Fresh Tc : " << Circuit.GetFreshTc() << endl;
		cout << "Aged Tc : " << Circuit.GetAgedTc() << endl;
		cout << "Number of FF : " << Circuit.No_FF() << endl;
		cout << "All number of FF : " << Circuit.No_AFF() << endl;
		cout << "Tc improvement : " << Circuit.Improvement() << "%" << endl;
		//Circuit.Simplify();
		//Circuit.ReadSatFile("myout_opt.sat");
	}
	if(Circuit.GetExperiment() == 2)
	{
		Circuit.SearchLimit();
		cout << "Final Tc : " << Circuit.GetFixTc() << endl;
	}
	return 0;
}

