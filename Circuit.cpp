#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>
#include "Circuit.h"
using namespace std;
using namespace __gnu_cxx;
typedef pair<string, int> P;
typedef pair<int, string> IP;
typedef vector<int> VEC;
typedef vector<VEC>* VECPTR;
// above encoding method may have problem -> FFi-FFj-FFk path
enum INIT { LAGING, SAGING, AAGING, CIJAGING, TCQAGING, RDELAY, RPENALTY, RLIMIT, MODE, EXPERIMENT, BUFCONTROL, BOUNDARY};

void CIRCUIT::ReadIO(string name)
{
	char tmp[2048];
	string contmp;
	fstream file;
	file.open(strdup(name.c_str()), ios::in);
	if(!file)
	{
		cout << name << " can not open." << endl;
		exit(-1);
	}
	cout << "Reading the " << name << endl;
	while(file.getline(tmp, sizeof(tmp)))
	{
		contmp.assign(tmp);
		if(contmp.find("input") <= contmp.size())
		{
			char *t, *tt;
			strtok_r(strdup(contmp.c_str()), " ", &t);
			tt = strtok_r(NULL, ";", &t);
			inputport.insert(P(tt, inputport.size()));
		}
		if(contmp.find("output") <= contmp.size())
		{
			char *t, *tt;
			strtok_r(strdup(contmp.c_str()), " ", &t);
			tt = strtok_r(NULL, ";", &t);
			outputport.insert(P(tt, outputport.size()));
		}
	}
	file.close();
	cout << "End" << endl;
}
void CIRCUIT::ReadParameters(string name)
{
	char tmp[1024];
	vector<string> stmp;
	fstream file;
	file.open(strdup(name.c_str()), ios::in);
	if(!file)
	{
		cout << name << " can not open." << endl;
		exit(-1);
	}
	while(file.getline(tmp, sizeof(tmp)))
	{
		char *t, *tt;
		tt = strtok_r(tmp, " ", &t);
		stmp.push_back(t);
	}
	LongAging = atof(stmp[LAGING].c_str());
	ShortAging = atof(stmp[SAGING].c_str());
	AvgAging = atof(stmp[AAGING].c_str());
	CijAging = atof(stmp[CIJAGING].c_str());
	TcqAging = atof(stmp[TCQAGING].c_str());
	RDelay = atof(stmp[RDELAY].c_str());
	RPenalty = atof(stmp[RPENALTY].c_str());
	constraint = atoi(stmp[RLIMIT].c_str());
	mode = atoi(stmp[MODE].c_str());
	experiment = atoi(stmp[EXPERIMENT].c_str());
	buf_control = atoi(stmp[BUFCONTROL].c_str());
	sample_boundary = atof(stmp[BOUNDARY].c_str());
}
void CIRCUIT::ReadData(string name)
{
	char tmp[1024];
	TIMEINFO timetmp;
	int phase = 0;
	bool chk = false;	// chk is true mean that is input-FF path
	fstream file;
	file.open(strdup(name.c_str()), ios::in);
	if(!file)
	{
		cout << name << " can not open." << endl;
		exit(-1);
	}
	cout << "Reading the " << name << endl;

	timetmp.Init();
	while(file.getline(tmp, sizeof(tmp)))
	{
		string contmp(tmp);
		if(contmp.find("Startpoint") <= contmp.size() && phase == 0)
		{
			char *t;
			strtok_r(tmp, " ", &t);
			string stmp(t);
			if(stmp.find("input port") <= stmp.size())	t = strtok_r(NULL, " ", &t);
			if(stmp.find("(") <= stmp.size()) t = strtok_r(NULL, " ", &t);
			if(fftable.find(t) == fftable.end())	fftable.insert(P(t, (int)fftable.size())); // fftable ID start from 0
			timetmp.SetSP(t);
			timetmp.SetRetiming(0);
			timetmp.SetIdi(fftable.find(timetmp.GetSP())->second);
			if(inputport.find(t) != inputport.end())	// start is input case
			{
				if(mode == 0)		// contain input-FF path
				{
					timetmp.SetCi(0);
					timetmp.SetTcq(0);
					chk = true;
				}
				else	continue;	// don't catch input-FF path
			}
			phase++;
		}
		if(contmp.find("Endpoint") <= contmp.size() && phase == 1)
		{
			char *t;
			strtok_r(tmp, " ", &t);
			string stmp(t);
			if(stmp.find("(") <= stmp.size())	t = strtok_r(NULL, " ", &t);
			if(fftable.find(t) == fftable.end())	fftable.insert(P(t, (int)fftable.size()));
			if(outputport.find(t) != outputport.end())		// FF is output
			{
				phase = 0;
				continue;
			}
			if(ffset.find(t) == ffset.end())
			{
				ffset.insert(P(t, (int)ffset.size()+1));	// RFF ID start from 1
				iffset.insert(IP((int)iffset.size()+1, t));
			}
			timetmp.SetEP(t);
			timetmp.SetIdj(fftable.find(timetmp.GetEP())->second);
			phase++;
		}
		if(contmp.find("Path Group") <= contmp.size() && phase == 2)
		{
			char *t, *tt;
			tt = strtok_r(tmp, ":", &t);
			tt = strtok_r(NULL, " ", &t);
			timetmp.SetClk(tt);
			phase++;
		}
		if(contmp.find("Path Type") <= contmp.size() && phase == 3)
		{
			char *t, *tt;
			tt = strtok_r(tmp, " ", &t);
			while((tt = strtok_r(NULL, " ", &t)) != NULL)
				if(strcmp(tt, "Type:"))	timetmp.SetType(tt);
			if(chk)	phase = 6;		// if true, go to calc arrival time
			else phase++;
		}
		if(contmp.find(timetmp.GetSP()) <= contmp.size() && contmp.find("/CP") <= contmp.size() && phase == 4)
		{
			char *t, *tt;
			vector<string> stmp;
			tt = strtok_r(strdup(contmp.c_str()), " ", &t);
			while((tt = strtok_r(NULL, " ", &t)) != NULL)
				stmp.push_back(tt);
			timetmp.SetCi(atof(strdup(stmp[4].c_str())) - atof(strdup(stmp[2].c_str())));
			timetmp.SetTcq(atof(strdup(stmp[2].c_str())));
			phase++;
		}
		if(contmp.find(timetmp.GetSP()) <= contmp.size() && contmp.find("/Q") <= contmp.size() && phase == 5)
		{
			char *t, *tt;
			vector<string> stmp;
			tt = strtok_r(strdup(contmp.c_str()), " ", &t);
			while((tt = strtok_r(NULL, " ", &t)) != NULL)
				stmp.push_back(tt);
			(timetmp.GetType() == "max") ? timetmp.SetLongPath(atof(strdup(stmp[4].c_str()))) : timetmp.SetShortPath(atof(strdup(stmp[4].c_str())));
			timetmp.SetTcq(timetmp.GetTcq() + atof(strdup(stmp[2].c_str())));
			phase++;
			file.getline(tmp, sizeof(tmp));//FF output net
			file.getline(tmp, sizeof(tmp));//next gate(input)
			contmp.assign(tmp);
            		// 
			int in=0;
			char* p[50];
            		p[in] = strtok_r(strdup(contmp.c_str()), " ", &t);
			while(p[in]){
				in++;
				p[in] = strtok_r(NULL," ",&t);
			}
            		timetmp.SetRetiming(atof(p[3]));
            		file.getline(tmp, sizeof(tmp));//next gate(output)
			contmp.assign(tmp);
           		in=0;
            		p[in] = strtok_r(strdup(contmp.c_str()), " ", &t);
			while(p[in]){
				in++;
				p[in]=strtok_r(NULL," ",&t);
			}
            		timetmp.SetRetiming(timetmp.GetRetiming()+atof(p[3]));
			//cout << "retiming :  "<<timetmp.GetRetiming() << endl;
		}
		if(contmp.find("arrival time") <= contmp.size() && phase == 6)
		{
			char *t, *tt;
			tt = strtok_r(strdup(contmp.c_str()), " ", &t);
			while((tt = strtok_r(NULL, " ", &t)) != NULL)
				if(strcmp(tt, "arrival") && strcmp(tt, "time"))	timetmp.SetAt(atof(tt));
			(timetmp.GetType() == "max") ? timetmp.SetLongPath(timetmp.GetAt() - timetmp.GetLongPath()) : timetmp.SetShortPath(timetmp.GetAt() - timetmp.GetShortPath());
			phase++;
		}
		if(contmp.find(timetmp.GetClk()) <= contmp.size() && contmp.find("rise") <= contmp.size() && phase == 7)        // here have problem
		{
			char *t, *tt;
			vector<string> stmp;
			tt= strtok_r(strdup(contmp.c_str()), " ", &t);
			while((tt = strtok_r(NULL, " ", &t)) != NULL)
				stmp.push_back(tt);
			timetmp.SetCj(atof(strdup(stmp[4].c_str())));
			phase++;
		}
		if(contmp.find(timetmp.GetEP()) <= contmp.size() && contmp.find("/CP") <= contmp.size() && phase == 8)
		{
			char *t, *tt;
			vector<string> stmp;
			tt = strtok_r(strdup(contmp.c_str()), " ", &t);
			while((tt = strtok_r(NULL, " ", &t)) != NULL)
				stmp.push_back(tt);
			timetmp.SetCj(atof(strdup(stmp[4].c_str())) - atof(strdup(stmp[2].c_str())) - timetmp.GetCj());
			phase++;
		}
		if(contmp.find("uncertainty") <= contmp.size() && phase == 9)
		{
			char *t, *tt;
			vector<string> stmp;
			tt = strtok_r(tmp, " ", &t);
			while((tt = strtok_r(NULL, " ", &t)) != NULL)
				stmp.push_back(tt);
			timetmp.SetTun(atof(strdup(stmp[1].c_str())));
			phase++;
		}
		if(contmp.find("library") <= contmp.size() && phase == 10)
		{
			char *t, *tt;
			vector<string> stmp;
			tt = strtok_r(tmp, " ", &t);
			while((tt = strtok_r(NULL, " ", &t)) != NULL)
				stmp.push_back(tt);
			(stmp[0] == "setup") ? timetmp.SetTsu(atof(strdup(stmp[2].c_str()))*-1) : timetmp.SetTh(atof(strdup(stmp[2].c_str())));
			timetmp.SetRt(atof(strdup(stmp[3].c_str())) - timetmp.GetTun());
			phase++;
		}
		if(contmp.find("slack") <= contmp.size() && phase == 11)
		{
			if(timevec.size() == 0)
			{
				SlkLimit = (timetmp.GetTcq() + timetmp.GetLongPath() + timetmp.GetTsu() - (timetmp.GetCj()-timetmp.GetCi())) * (float)(1.0 - sample_boundary);
				timetmp.SetSlk(timetmp.GetTcq() + timetmp.GetLongPath() + timetmp.GetTsu() - (timetmp.GetCj()-timetmp.GetCi()));
				Tc = (timetmp.GetTcq() + timetmp.GetLongPath() + timetmp.GetTsu() - (timetmp.GetCj()-timetmp.GetCi())) * 1.2;
				fresh_tc = (timetmp.GetTcq() + timetmp.GetLongPath() + timetmp.GetTsu() - (timetmp.GetCj()-timetmp.GetCi()));
				aged_tc = (timetmp.GetTcq()*TcqAging + timetmp.GetLongPath()*LongAging + timetmp.GetTsu() - (timetmp.GetCj()-timetmp.GetCi())*CijAging);
				fix_tc = (timetmp.GetTcq() + AvgAging*timetmp.GetLongPath() + timetmp.GetTsu() - (timetmp.GetCj()-timetmp.GetCi())) + 0.0000006;
				timevec.push_back(timetmp);
			}
			else
			{
				timetmp.SetSlk(timetmp.GetTcq() + timetmp.GetLongPath() + timetmp.GetTsu() - (timetmp.GetCj()-timetmp.GetCi()));
				if(timetmp.GetSlk() > SlkLimit)
				{
					timevec.push_back(timetmp);
				}
				else
				{
					ffset.erase(timetmp.GetEP());
					break;
				}
			}
			timetmp.Init();
			chk = false;
			phase = 0;
		}
	}
	cout << "End" << endl;
	file.close();
}
void CIRCUIT::SetBuf()	// done
{
	buf[0] = 1.00;
	buf[1] = 1.02;
	buf[2] = 1.04;
	buf[3] = 1.08;
}
bool CIRCUIT::SetupConstraint(int ffi1, int ffi0, int ffj1, int ffj0, int rj, int idx) //done
{
	int bufi = ffi1 * 2 + ffi0;
	int bufj = ffj1 * 2 + ffj0;
	float si = timevec[idx].GetCi() * buf[bufi];
	float sj = timevec[idx].GetCj() * buf[bufj];
	float LHS = CijAging*si + TcqAging*timevec[idx].GetTcq() + LongAging*timevec[idx].GetLongPath() + timevec[idx].GetTsu() + (float)rj*RPenalty;
	float RHS = CijAging*sj + (1 + (float)rj*RDelay)*Tc;
	float LHS2 = si + timevec[idx].GetTcq() + AvgAging*timevec[idx].GetLongPath() + timevec[idx].GetTsu() + rj*RPenalty;
	float RHS2 = sj + Tc;
	if((LHS < RHS) && (LHS2 < RHS2)) return true;
	else	return false;
}
bool CIRCUIT::SetupConstraint2(int rti, int i1, int i0, int j1, int j0, int rj, int idx)
{
	int bufi = i1 * 2 + i0;
	int bufj = j1 * 2 + j0;
	float si = timevec[idx].GetCi() * buf[bufi];
	float sj = timevec[idx].GetCj() * buf[bufj];
	float LHS = CijAging*si + TcqAging*timevec[idx].GetTcq() + LongAging*(timevec[idx].GetLongPath() - (float)rti*timevec[idx].GetRetiming()) + timevec[idx].GetTsu() + (float)rj*RPenalty;
	float RHS = CijAging*sj + (1 + (float)rj*RDelay)*Tc;
	float LHS2 = si + timevec[idx].GetTcq() + AvgAging*(timevec[idx].GetLongPath() - (float)rti*timevec[idx].GetRetiming()) + timevec[idx].GetTsu();
	float RHS2 = sj + Tc;
	if ((LHS < RHS) && (LHS2 < RHS2)) return true;
	else	return false;
}
void CIRCUIT::CalcTTable(int idx)
{
	int record[32] = {0};
	for(int i1 = 0; i1 < 2; i1++)
	{
		for(int i0 = 0; i0 < 2; i0++)
		{
			for(int j1 = 0; j1 < 2; j1++)
			{
				for(int j0 = 0; j0 < 2; j0++)
				{
					for(int r = 0; r < 2; r++)
					{
						if(SetupConstraint(i1, i0, j1, j0, r, idx) == false)
						{
							record[i1*16 + i0*8 + j1*4 + j0*2 + r] = 1;
							//AddInfo(idx, i1, i0, j1, j0, r);
						}
					}
				}
			}
		}
	}
	if(record[0] == 1)	AddInfo(idx, 3, 3, 0, 0, 0);	// 00000 -> XX000
	if(record[1] == 1)	AddInfo(idx, 3, 3, 0, 0, 3);	// 00001 -> XX00X
	if(record[2] == 1)	AddInfo(idx, 3, 3, 0, 3, 0);	// 00010 -> XX0X0
	if(record[3] == 1) 	AddInfo(idx, 3, 3, 0, 3, 3);	// 00011 -> XX0XX
	if(record[4] == 1)	AddInfo(idx, 3, 3, 3, 0, 0);	// 00100 -> XXX00
	if(record[5] == 1) 	AddInfo(idx, 3, 3, 3, 0, 3);	// 00101 -> XXX0X
	if(record[6] == 1)	AddInfo(idx, 3, 3, 3, 3, 0);	// 00110 -> XXXX0
	if(record[7] == 1)	AddInfo(idx, 0, 0, 1, 1, 3);	// 00111 
	if(record[8] == 1)	AddInfo(idx, 3, 1, 0, 0, 0);	// 01000 -> X1000
	if(record[9] == 1)	AddInfo(idx, 3, 1, 0, 0, 3);	// 01001 -> X100X
	if(record[10] == 1)	AddInfo(idx, 3, 1, 0, 3, 0);	// 01010 -> X10X0
	if(record[11] == 1)	AddInfo(idx, 3, 1, 0, 3, 3);	// 01011 -> X10XX
	if(record[12] == 1)	AddInfo(idx, 3, 1, 3, 0, 0);	// 01100 -> X1X00
	if(record[13] == 1)	AddInfo(idx, 3, 1, 3, 0, 3);	// 01101 -> X1X0X
	if(record[14] == 1)	AddInfo(idx, 3, 1, 3, 3, 0);	// 01110 -> X1XX0
	if(record[15] == 1)	AddInfo(idx, 3, 1, 3, 3, 3);	// 01111 -> X1XXX
	if(record[16] == 1)	AddInfo(idx, 1, 3, 0, 0, 0);	// 10000 -> 1X000
	if(record[17] == 1)	AddInfo(idx, 1, 3, 0, 0, 3);	// 10001 -> 1X00X
	if(record[18] == 1)	AddInfo(idx, 1, 3, 0, 3, 0);	// 10010 -> 1X0X0
	if(record[19] == 1)	AddInfo(idx, 1, 3, 0, 3, 3);	// 10011 -> 1X0XX
	if(record[20] == 1)	AddInfo(idx, 1, 3, 3, 0, 0);	// 10100 -> 1XX00
	if(record[21] == 1)	AddInfo(idx, 1, 3, 3, 0, 3);	// 10101 -> 1XX0X
	if(record[22] == 1)	AddInfo(idx, 1, 3, 3, 3, 0);	// 10110 -> 1XXX0
	if(record[23] == 1)	AddInfo(idx, 1, 3, 3, 3, 3);	// 10111 -> 1XXXX
	if(record[24] == 1)	AddInfo(idx, 1, 1, 0, 0, 0);	// 11000
	if(record[25] == 1)	AddInfo(idx, 1, 1, 0, 0, 3);	// 11001 -> 1100X
	if(record[26] == 1)	AddInfo(idx, 1, 1, 0, 3, 0);	// 11010 -> 110X0
	if(record[27] == 1)	AddInfo(idx, 1, 1, 0, 3, 3);	// 11011 -> 110XX
	if(record[28] == 1)	AddInfo(idx, 1, 1, 3, 0, 0);	// 11100 -> 11X00
	if(record[29] == 1)	AddInfo(idx, 1, 1, 3, 0, 3);	// 11101 -> 11X0X
	if(record[30] == 1)	AddInfo(idx, 1, 1, 3, 3, 0);	// 11110 -> 11XX0
	if(record[31] == 1)	AddInfo(idx, 1, 1, 3, 3, 3);	// 11111 -> 11XXX
}
void CIRCUIT::CalcTTable2(int idx)
{
	int record[64] = {0};
	for (int rti = 0; rti < 2; rti++)
	{
		for (int i1 = 0; i1 < 2; i1++)
		{
			for (int i0 = 0; i0 < 2; i0++)
			{
				for (int j1 = 0; j1 < 2; j1++)
				{
					for (int j0 = 0; j0 < 2; j0++)
					{
						for (int r = 0; r < 2; r++)
						{
							if (SetupConstraint2(idx, i1, i0, j1, j0, r, idx) == false)
							{
								record[rti * 32 + i1 * 16 + i0 * 8 + j1 * 4 + j0 * 2 + r] = 1;
							}
						}
					}
				}
			}
		}
	}
	if(record[0] == 1)	AddInfo2(idx, 0, 3, 3, 0, 0, 0);	// 00000 -> XX000
	if(record[1] == 1)	AddInfo2(idx, 0, 3, 3, 0, 0, 3);	// 00001 -> XX00X
	if(record[2] == 1)	AddInfo2(idx, 0, 3, 3, 0, 3, 0);	// 00010 -> XX0X0
	if(record[3] == 1) 	AddInfo2(idx, 0, 3, 3, 0, 3, 3);	// 00011 -> XX0XX
	if(record[4] == 1)	AddInfo2(idx, 0, 3, 3, 3, 0, 0);	// 00100 -> XXX00
	if(record[5] == 1) 	AddInfo2(idx, 0, 3, 3, 3, 0, 3);	// 00101 -> XXX0X
	if(record[6] == 1)	AddInfo2(idx, 0, 3, 3, 3, 3, 0);	// 00110 -> XXXX0
	if(record[7] == 1)	AddInfo2(idx, 0, 3, 3, 3, 3, 3);
	if(record[8] == 1)	AddInfo2(idx, 0, 3, 1, 0, 0, 0);	// 01000 -> X1000
	if(record[9] == 1)	AddInfo2(idx, 0, 3, 1, 0, 0, 3);	// 01001 -> X100X
	if(record[10] == 1)	AddInfo2(idx, 0, 3, 1, 0, 3, 0);	// 01010 -> X10X0
	if(record[11] == 1)	AddInfo2(idx, 0, 3, 1, 0, 3, 3);	// 01011 -> X10XX
	if(record[12] == 1)	AddInfo2(idx, 0, 3, 1, 3, 0, 0);	// 01100 -> X1X00
	if(record[13] == 1)	AddInfo2(idx, 0, 3, 1, 3, 0, 3);	// 01101 -> X1X0X
	if(record[14] == 1)	AddInfo2(idx, 0, 3, 1, 3, 3, 0);	// 01110 -> X1XX0
	if(record[15] == 1)	AddInfo2(idx, 0, 3, 1, 3, 3, 3);	// 01111 -> X1XXX
	if(record[16] == 1)	AddInfo2(idx, 0, 1, 3, 0, 0, 0);	// 10000 -> 1X000
	if(record[17] == 1)	AddInfo2(idx, 0, 1, 3, 0, 0, 3);	// 10001 -> 1X00X
	if(record[18] == 1)	AddInfo2(idx, 0, 1, 3, 0, 3, 0);	// 10010 -> 1X0X0
	if(record[19] == 1)	AddInfo2(idx, 0, 1, 3, 0, 3, 3);	// 10011 -> 1X0XX
	if(record[20] == 1)	AddInfo2(idx, 0, 1, 3, 3, 0, 0);	// 10100 -> 1XX00
	if(record[21] == 1)	AddInfo2(idx, 0, 1, 3, 3, 0, 3);	// 10101 -> 1XX0X
	if(record[22] == 1)	AddInfo2(idx, 0, 1, 3, 3, 3, 0);	// 10110 -> 1XXX0
	if(record[23] == 1)	AddInfo2(idx, 0, 1, 3, 3, 3, 3);	// 10111 -> 1XXXX
	if(record[24] == 1)	AddInfo2(idx, 0, 1, 1, 0, 0, 0);	// 11000
	if(record[25] == 1)	AddInfo2(idx, 0, 1, 1, 0, 0, 3);	// 11001 -> 1100X
	if(record[26] == 1)	AddInfo2(idx, 0, 1, 1, 0, 3, 0);	// 11010 -> 110X0
	if(record[27] == 1)	AddInfo2(idx, 0, 1, 1, 0, 3, 3);	// 11011 -> 110XX
	if(record[28] == 1)	AddInfo2(idx, 0, 1, 1, 3, 0, 0);	// 11100 -> 11X00
	if(record[29] == 1)	AddInfo2(idx, 0, 1, 1, 3, 0, 3);	// 11101 -> 11X0X
	if(record[30] == 1)	AddInfo2(idx, 0, 1, 1, 3, 3, 0);	// 11110 -> 11XX0
	if(record[31] == 1)	AddInfo2(idx, 0, 1, 1, 3, 3, 3);	// 11111 -> 11XXX
	
	if(record[32] == 1)	AddInfo2(idx, 3, 3, 3, 0, 0, 0);	// 00000 -> XX000
	if(record[33] == 1)	AddInfo2(idx, 3, 3, 3, 0, 0, 3);	// 00001 -> XX00X
	if(record[34] == 1)	AddInfo2(idx, 3, 3, 3, 0, 3, 0);	// 00010 -> XX0X0
	if(record[35] == 1) 	AddInfo2(idx, 3, 3, 3, 0, 3, 3);	// 00011 -> XX0XX
	if(record[36] == 1)	AddInfo2(idx, 3, 3, 3, 3, 0, 0);	// 00100 -> XXX00
	if(record[37] == 1) 	AddInfo2(idx, 3, 3, 3, 3, 0, 3);	// 00101 -> XXX0X
	if(record[38] == 1)	AddInfo2(idx, 3, 3, 3, 3, 3, 0);	// 00110 -> XXXX0
	if(record[39] == 1)	AddInfo2(idx, 3, 3, 3, 3, 3, 1);	// 01000 -> X1000
	if(record[40] == 1)	AddInfo2(idx, 3, 3, 1, 0, 0, 0);	// 01001 -> X100X
	if(record[41] == 1)	AddInfo2(idx, 3, 3, 1, 0, 0, 3);	// 01010 -> X10X0
	if(record[42] == 1)	AddInfo2(idx, 3, 3, 1, 0, 3, 0);	// 01011 -> X10XX
	if(record[43] == 1)	AddInfo2(idx, 3, 3, 1, 0, 3, 3);	// 01100 -> X1X00
	if(record[44] == 1)	AddInfo2(idx, 3, 3, 1, 3, 0, 0);	// 01101 -> X1X0X
	if(record[45] == 1)	AddInfo2(idx, 3, 3, 1, 3, 0, 3);	// 01110 -> X1XX0
	if(record[46] == 1)	AddInfo2(idx, 3, 3, 1, 3, 3, 0);	// 01111 -> X1XXX
	if(record[47] == 1)	AddInfo2(idx, 3, 3, 1, 3, 3, 3);	// 10000 -> 1X000
	if(record[48] == 1)	AddInfo2(idx, 3, 1, 3, 0, 0, 0);	// 10001 -> 1X00X
	if(record[49] == 1)	AddInfo2(idx, 3, 1, 3, 0, 0, 3);	// 10010 -> 1X0X0
	if(record[50] == 1)	AddInfo2(idx, 3, 1, 3, 0, 3, 0);	// 10011 -> 1X0XX
	if(record[51] == 1)	AddInfo2(idx, 3, 1, 3, 0, 3, 3);	// 10100 -> 1XX00
	if(record[52] == 1)	AddInfo2(idx, 3, 1, 3, 3, 0, 3);	// 10101 -> 1XX0X
	if(record[53] == 1)	AddInfo2(idx, 3, 1, 3, 3, 0, 1);	// 10110 -> 1XXX0
	if(record[54] == 1)	AddInfo2(idx, 3, 1, 3, 3, 3, 0);	// 10111 -> 1XXXX
	if(record[55] == 1)	AddInfo2(idx, 3, 1, 3, 3, 3, 3);	// 11000
	if(record[56] == 1)	AddInfo2(idx, 3, 1, 1, 0, 0, 0);	// 11001 -> 1100X
	if(record[57] == 1)	AddInfo2(idx, 3, 1, 1, 0, 0, 3);	// 11010 -> 110X0
	if(record[58] == 1)	AddInfo2(idx, 3, 1, 1, 0, 3, 0);	// 11011 -> 110XX
	if(record[59] == 1)	AddInfo2(idx, 3, 1, 1, 0, 3, 3);	// 11100 -> 11X00
	if(record[60] == 1)	AddInfo2(idx, 3, 1, 1, 3, 0, 0);	// 11101 -> 11X0X
	if(record[61] == 1)	AddInfo2(idx, 3, 1, 1, 3, 0, 3);	// 11110 -> 11XX0
	if(record[62] == 1)	AddInfo2(idx, 3, 1, 1, 3, 3, 0);	// 11111 -> 11XXX
	if(record[63] == 1)	AddInfo2(idx, 3, 1, 1, 3, 3, 3);

}
void CIRCUIT::SetPOS(int idx)
{
	for(int j = 0; j < (int)timevec[idx].GetPOS()->GetI1()->size(); j++)
	{
		vector<int> tmp;
		tmp.push_back(timevec[idx].GetPOS()->GetI1()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetI0()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetJ1()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetJ0()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetRj()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetIdi()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetIdj()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetFFid()->at(j));
		pos.push_back(tmp);
	}
}
void CIRCUIT::SetPOS2(int idx)
{
	for (int j = 0; j < (int)timevec[idx].GetPOS()->GetI1()->size(); j++)
	{
		vector<int> tmp;
		tmp.push_back(timevec[idx].GetPOS()->GetI1()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetI0()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetJ1()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetJ0()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetRj()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetIdi()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetIdj()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetFFid()->at(j));
		tmp.push_back(timevec[idx].GetPOS()->GetRti()->at(j));
		pos.push_back(tmp);
	}
}

void CIRCUIT::WriteCNF()
{
	int cvar = pos.size()*4 + ffset.size();
	int ttc = pos.size();
	int svar = ffset.size() + ffset.size()*constraint;
	int sc = 2*constraint*ffset.size() + ffset.size() - 3*constraint - 1;
	int I1 = fftable.size()*1 + 1;		// fftable ID start from 0
	int I0 = fftable.size()*2 + 1;
	int S = fftable.size()*3 + 1;		// Si,j = S + (i-1)k + j
	fstream file;
	file.open("mycnf.cnf", ios::out);
	if(!file)	cout << "file can not open" << endl;
	file << "p cnf " << cvar + svar << " " << ttc + sc << endl;

	if(constraint == 0)
	{
		for(int i = 1; i <= (int)fftable.size(); i++)
			file << -1 * i << " 0" << endl;
	}
	else
	{
		// sequenctial counter initial, (!X1, S1,1)(!S1,j), for 1<j<=k
		file << -1 << " " << (S + (1-1)*constraint + 1) << " " << 0 << endl;	// (!X1, S1,1)
		for(int i = 2; i <= constraint; i++)					// (!S1,j), for 1<j<=k
			file << -1 * (S + (1-1)*constraint + i) << " " << 0 << endl;

		// sequenctial counter iterative
		for(int i = 2; i < (int)fftable.size(); i++)
		{
			file << -1 * i << " " << S + (i-1)*constraint + 1 << " " << 0 << endl;	// (!Xi, Si,1), for 1<i<n
			file << -1 * (S + (i-1-1)*constraint + 1) << " " << S + (i-1)*constraint + 1 << " " << 0 << endl; // (!Si-1,1, Si,1)
			for(int j = 2; j <= constraint; j++)
			{
				file << -1 * i << " " << -1 * (S + (i-1-1)*constraint + (j-1)) << " " << S + (i-1)*constraint + j << " " << 0 << endl; // (!Xi, !Si-1,j-1, Si,j)
				file << -1 * (S + (i-1-1)*constraint + j) << " " << S + (i-1)*constraint + j << " " << 0 << endl; // (!Si-1,j, Si,j)
			}
		}
		// sequenctial counter final check
		for(int i = 2; i <= (int)fftable.size(); i++)
			file << -1 * i << " " << -1 * (S + (i-1-1)*constraint + constraint) << " " << 0 << endl; // (!Xi, !Si-1,k)
	}
	/*
	I1 = 0
	I0 = 1
	J1 = 2
	J0 = 3
	Rj() = 4
	Idi() = 5
	Idj() = 6
	FFid() = 7
	*/
	// truth table part
	for(int i = 0; i < (int)pos.size(); i++)
	{
		for(int j = 0; j < (int)pos[i].size() - 2; j++)
		{
			if(j == 0)
			{
				if(pos[i].at(j) != -2)
				{
					if(pos[i].at(j) > 0)	file << I1 + pos[i].at(5) << " ";
					else	file << -1 * (I1 + pos[i].at(5)) << " ";
				}
			}
			if(j == 1)
			{
				if(pos[i].at(j) != -2)
				{
					if(pos[i].at(j) > 0)	file << I0 + pos[i].at(5) << " ";
					else	file << -1 * (I0 + pos[i].at(5)) << " ";
				}
			}
			if(j == 2)
			{
				if(pos[i].at(j) != -2)
				{
					if(pos[i].at(j) > 0)	file << I1 + pos[i].at(6) << " ";
					else	file << -1 * (I1 + pos[i].at(6)) << " ";
				}
			}
			if(j == 3)
			{
				if(pos[i].at(j) != -2)
				{
					if(pos[i].at(j) > 0)	file << I0 + pos[i].at(6) << " ";
					else	file << -1 * (I0 + pos[i].at(6)) << " ";
				}
			}
			if(j == 4)
			{
				if(pos[i].at(j) != -2)
				{
					if(pos[i].at(j) > 0)	file << pos[i].at(6)+1 << " ";//0" << endl;
					else	file << -1 * (pos[i].at(6)+1) << " ";//0" << endl;
				}
			}
			if(j == 5)	file << "0" << endl;
		}
	}

	// speedup
	if(buf_control == 0)
	{
		for(int i = 0; i < (int)fftable.size(); i++)
		{
			file << -1*(I0 + i) << " 0" << endl;
			file << -1*(I1 + i) << " 0" << endl;
		}
	}

	file.close();
}
void CIRCUIT::WriteCNF2()
{
	int cvar = pos.size() * 4 + ffset.size();
	int ttc = pos.size();
	int svar = ffset.size() + ffset.size()*constraint;
	int sc = 2 * constraint*ffset.size() + ffset.size() - 3 * constraint - 1;
	int I1 = fftable.size() * 1 + 1;		// fftable ID start from 0
	int I0 = fftable.size() * 2 + 1;
	int RT = fftable.size() * 3 + 1;
	int S = fftable.size() * 4 + 1;		// Si,j = S + (i-1)k + j
	fstream file;
	file.open("mycnf.cnf", ios::out);
	if (!file)	cout << "file can not open" << endl;
	file << "p cnf " << cvar + svar << " " << ttc + sc << endl;

	if (constraint == 0)
	{
		for (int i = 1; i <= (int)fftable.size(); i++)
			file << -1 * i << " 0" << endl;
	}
	else
	{
		// sequenctial counter initial, (!X1, S1,1)(!S1,j), for 1<j<=k
		file << -1 << " " << (S + (1 - 1)*constraint + 1) << " " << 0 << endl;	// (!X1, S1,1)
		for (int i = 2; i <= constraint; i++)					// (!S1,j), for 1<j<=k
			file << -1 * (S + (1 - 1)*constraint + i) << " " << 0 << endl;

		// sequenctial counter iterative
		for (int i = 2; i < (int)fftable.size(); i++)
		{
			file << -1 * i << " " << S + (i - 1)*constraint + 1 << " " << 0 << endl;	// (!Xi, Si,1), for 1<i<n
			file << -1 * (S + (i - 1 - 1)*constraint + 1) << " " << S + (i - 1)*constraint + 1 << " " << 0 << endl; // (!Si-1,1, Si,1)
			for (int j = 2; j <= constraint; j++)
			{
				file << -1 * i << " " << -1 * (S + (i - 1 - 1)*constraint + (j - 1)) << " " << S + (i - 1)*constraint + j << " " << 0 << endl; // (!Xi, !Si-1,j-1, Si,j)
				file << -1 * (S + (i - 1 - 1)*constraint + j) << " " << S + (i - 1)*constraint + j << " " << 0 << endl; // (!Si-1,j, Si,j)
			}
		}
		// sequenctial counter final check
		for (int i = 2; i <= (int)fftable.size(); i++)
			file << -1 * i << " " << -1 * (S + (i - 1 - 1)*constraint + constraint) << " " << 0 << endl; // (!Xi, !Si-1,k)
	}
	/*
	I1 = 0
	I0 = 1
	J1 = 2
	J0 = 3
	Rj() = 4
	Idi() = 5
	Idj() = 6
	FFid() = 7
	Rti() = 8
	*/
	// truth table part
	for (int i = 0; i < (int)pos.size(); i++)
	{
		for (int j = 0; j <= (int)pos[i].size(); j++)
		{
			if (j == 0)
			{
				if (pos[i].at(j) != -2)
				{
					if (pos[i].at(j) > 0)	file << I1 + pos[i].at(5) << " ";
					else	file << -1 * (I1 + pos[i].at(5)) << " ";
				}
			}
			if (j == 1)
			{
				if (pos[i].at(j) != -2)
				{
					if (pos[i].at(j) > 0)	file << I0 + pos[i].at(5) << " ";
					else	file << -1 * (I0 + pos[i].at(5)) << " ";
				}
			}
			if (j == 2)
			{
				if (pos[i].at(j) != -2)
				{
					if (pos[i].at(j) > 0)	file << I1 + pos[i].at(6) << " ";
					else	file << -1 * (I1 + pos[i].at(6)) << " ";
				}
			}
			if (j == 3)
			{
				if (pos[i].at(j) != -2)
				{
					if (pos[i].at(j) > 0)	file << I0 + pos[i].at(6) << " ";
					else	file << -1 * (I0 + pos[i].at(6)) << " ";
				}
			}
			if (j == 4)
			{
				if (pos[i].at(j) != -2)
				{
					if (pos[i].at(j) > 0)	file << pos[i].at(6) + 1 << " ";
					else	file << -1 * (pos[i].at(6) + 1) << " ";
				}
			}
			if (j == 8)
			{
				if (pos[i].at(j) != -2)
				{
					if (pos[i].at(j) > 0)	file << RT + pos[i].at(6) << " ";
					else	file << -1 * (RT + pos[i].at(6)) << " ";
				}
			}
			if (j == 9)	file << "0" << endl;
		}
	}

	// speedup
	if (buf_control == 0)
	{
		for (int i = 0; i < (int)fftable.size(); i++)
		{
			file << -1 * (I0 + i) << " 0" << endl;
			file << -1 * (I1 + i) << " 0" << endl;
		}
	}

	file.close();

}
void CIRCUIT::ReadSatFile(string name)		// done
{
	char tmp[1000000];
	int count = 0, chk = 0;
	vector<int> satchk;
	fstream file;
	file.open(strdup(name.c_str()), ios::in);
	if(!file)	cout << ".sat can not open." << endl;
	file.getline(tmp, sizeof(tmp));		// SAT or UNSAT
	file.getline(tmp, sizeof(tmp));		// soluction
	char *t, *tt;
	tt = strtok_r(tmp, " ", &t);
	satchk.push_back(atoi(tt));		// the first flip-flip, (Ri, i=0)
	while(count < fftable.size() - 1)
	{
		tt = strtok_r(NULL, " ", &t);
		satchk.push_back(atoi(tt));
		count++;
	}
	for(int i = 0; i < satchk.size(); i++)
	{
		if(satchk[i] > 0)	chk++;
	}
	if(chk <= constraint)	cout << "Number of RFF : " << chk << endl;
	else	cout << "constraint is not satisfiable !!!!!!" << endl;
	file.close();
}
void CIRCUIT::BinarySearchTc()
{
	char stmp[50];
	int fcount = 0, tcount = 0;
	float upper = Tc;
	float tmptc = Tc;
	float lower = fresh_tc, mid = 0, pre = 0;
	char inst[50] = "./minisat mycnf.cnf myout.sat";

	cout << "Binary Search Tc & Set the POS" << endl;
	while(fcount < 10)
	{
		bool chk = false;
		mid = (lower + upper) / 2;
		Tc = mid;
		if(pre == Tc)	break;
		if(Tc == fresh_tc)	break;
		for(int i = 0; i < (int)timevec.size(); i++)	// calculate all truth table
		{
			CalcTTable(i);
			if(timevec[i].GetPOS()->GetI1()->size() == 32)
			{
				chk = true;
				break;
			}
		}
		if(chk)
		{
			lower = mid;
			chk = false;
			fcount++;
		}
		else
		{
			for(int i = 0; i <(int)timevec.size(); i++)
				SetPOS(i);
			WriteCNF();

			system(inst);                           // call minisat
			fstream file;
			file.open("myout.sat", ios::in);
			if(!file)	cout << "Sat File can not open" << endl;
			file.getline(stmp, sizeof(stmp));
			string ctmp(stmp);
			if(ctmp.find("UNSAT") <= ctmp.size())
			{
				lower = mid;
				chk = false;
				fcount++;
			}
			else
			{
				upper = mid;
				tmptc = Tc;
				fcount = 0;
			}
			file.close();
		}
		pre = Tc;

		for(int i = 0; i <(int)timevec.size(); i++)
			timevec[i].GetPOS()->clear();
		pos.clear();
	}

	// final Tc and POS setting
	Tc = tmptc;
	for(int i = 0; i < (int)timevec.size(); i++)	// calculate all truth table
	{
		CalcTTable(i);
		SetPOS(i);
	}
	WriteCNF();                                     // write into .cnf file
	system(inst);
	cout << "End" << endl;
}

void CIRCUIT::SearchLimit()
{
	bool unsat = false, check = false;
	char tmp[50];
	int upper = ffset.size(), lower = 0, limit_tmp = ffset.size(), fcount = 0, pre = 0;
	char inst[50] = "./minisat mycnf.cnf myout.sat";
	Tc = fix_tc;

	while(fcount <= 10)
	{
		constraint = (upper + lower) / 2;
		cout << constraint << endl;
		cout << Tc << endl;
		if(constraint == pre)	break;
		for(int i = 0; i < (int)timevec.size(); i++)	// calculate all truth table
		{
			CalcTTable(i);
			SetPOS(i);
		}
		WriteCNF();
		system(inst);

		fstream file;
		file.open("myout.sat", ios::in);
		file.getline(tmp, sizeof(tmp));
		string stmp(tmp);
		file.close();

		if(stmp.find("UNSAT") <= stmp.size())
		{
			lower = constraint;
			fcount++;
		}
		else
		{
			limit_tmp = constraint;
			upper = constraint;
			fcount = 0;
			check = true;
		}

		for(int i = 0; i <(int)timevec.size(); i++)
			timevec[i].GetPOS()->clear();
		pos.clear();

		pre = constraint;
	}

	constraint = limit_tmp;
	for(int i = 0; i < (int)timevec.size(); i++)	// calculate all truth table
	{
		CalcTTable(i);
		SetPOS(i);
	}
	WriteCNF();
	system(inst);

	fstream file;
	file.open("myout.sat", ios::in);
	file.getline(tmp, sizeof(tmp));
	string stmp(tmp);
	file.close();

	if(!(stmp.find("UNSAT") <= stmp.size()) && check == false)
	{
		check = true;
		limit_tmp = ffset.size();
	}

	if(!check) cout << "no soluction" << endl;
	cout << "final limit : " << limit_tmp << endl;
	if(buf_control == 0) cout << "clock skew : close" << endl;
	else 	cout << "clock skew : open" << endl;		// 1 is open
}

void CIRCUIT::CalcBuf(string name, int* value, int* razor)
{
	char tmp[1000000];
	int calctmp[4]={0};
	int I1 = fftable.size()*1 + 1;
	int I0 = fftable.size()*2 + 1;
	vector<int> i1tmp, i0tmp, i1_bool, i0_bool, rjtmp, rj_bool;
	fstream file, output;
	file.open(strdup(name.c_str()), ios::in);
	if(!file)	cout << ".sat can not open." << endl;
	file.getline(tmp, sizeof(tmp));			// SAT or UNSAT
	file.getline(tmp, sizeof(tmp));			// solution
	char *t, *tt;
	tt = strtok_r(tmp, " ", &t);
	rjtmp.push_back(atoi(tt));			// thr first flip-flop, (Ri, i=0)

	while((tt = strtok_r(NULL, " ", &t)))
	{
		if(abs(atoi(tt)) > 0 && abs(atoi(tt)) <= fftable.size())	// Rj: [1,fftable.size]
			rjtmp.push_back(atoi(tt));
		if(abs(atoi(tt)) >= I1 && abs(atoi(tt)) < I1+fftable.size())	// I1 part, I1: [fftable.size*1+1,fftable.size*2]
			i1tmp.push_back(atoi(tt));
		if(abs(atoi(tt)) >= I0 && abs(atoi(tt)) < I0+fftable.size())	// I0 part, I0: [fftable.size*2+1,fftable.size*3]
			i0tmp.push_back(atoi(tt));
	}
	file.close();
	for(int i = 0; i < (int)fftable.size(); i++)
	{
		if(rjtmp[i] > 0)	razor[i] = 1;
		else	razor[i] = 0;
	}
	for(int i = 0; i < (int)fftable.size(); i++)
	{
		if(i1tmp[i] > 0)	i1_bool.push_back(1);
		else	i1_bool.push_back(0);
		if(i0tmp[i] > 0)	i0_bool.push_back(1);
		else	i0_bool.push_back(0);

		int VALUE = i1_bool[i]*2 + i0_bool[i];
		value[i] = VALUE;
		calctmp[VALUE]++;
	}

	for(int i = 0; i < 4; i++)
		cout << "type of buffer " << i << " : " << calctmp[i] << endl;
}

void CIRCUIT::Simplify()
{
	float avg_ori = 0.0, avg_opt = 0.0;
	char tmp[1000000];
	int* value = new int[fftable.size()];
	int* razor = new int[fftable.size()];
	int I1 = fftable.size()*1 + 1;
	int I0 = fftable.size()*2 + 1;
	CalcBuf("myout.sat", value, razor);

	for(int i = 0; i < (int)fftable.size(); i++)
		avg_ori += buf[value[i]]-1;
	avg_ori /= fftable.size();

	for(int i = 0; i < (int)timevec.size(); i++)
	{
		int tmpi = timevec[i].GetIdi(), tmpj = timevec[i].GetIdj();
		int vtmpi = value[tmpi], vtmpj = value[tmpj];
		value[tmpi] = 0;
		value[tmpj] = 0;
		int i1 = value[tmpi]/2;
		int i0 = value[tmpi]%2;
		int j1 = value[tmpj]/2;
		int j0 = value[tmpj]%2;
		int rj = razor[tmpj];
		if(!SetupConstraint(i1, i0, j1, j0, rj, i))
		{
			value[tmpj] = vtmpj;
			j1 = vtmpj/2;
			j0 = vtmpj%2;
		}
		if(!SetupConstraint(i1, i0, j1, j0, rj, i))
			value[tmpi] = vtmpi;
	}

	for(int i = 0; i < (int)fftable.size(); i++)
		avg_opt += buf[value[i]]-1;
	avg_opt /= fftable.size();

	// rewrite
	fstream file, output;
	file.open("mycnf.cnf", ios::in);
	output.open("mycnf_opt.cnf", ios::out);

	while(file.getline(tmp, sizeof(tmp)))
	{
		output << tmp << endl;
	}
	for(int i = 0; i < (int)fftable.size(); i++)
	{
		if(value[i] == 0)
		{
			output << -1*(I1+i) << " 0" << endl;
			output << -1*(I0+i) << " 0" << endl;
		}
		else if(value[i] == 1)
		{
			output << -1*(I1+i) << " 0" << endl;
			output << I0+i << " 0" << endl;
		}
		else if(value[i] == 2)
		{
			output << I1+i << " 0" << endl;
			output << -1*(I0+i) << " 0" << endl;
		}
		else
		{
			output << I1+i << " 0" << endl;
			output << I0+i << " 0" << endl;
		}
	}
	file.close();
	output.close();

	system("./minisat mycnf_opt.cnf myout_opt.sat");

	file.open("myout_opt.sat", ios::in);
	file.getline(tmp, sizeof(tmp));
	string stmp(tmp);
	file.close();

	if(!(stmp.find("UNSAT") <= stmp.size()))
	{
		CalcBuf("myout_opt.sat", value, razor);
		cout << "avg_ori : " << avg_ori * 100 << "%" << endl;
		cout << "avg_opt : " << avg_opt * 100 << "%" << endl;
	}
	else	cout << "optimal is UNSAT." << endl;
}

void CIRCUIT::AddInfo(int idx, int i1, int i0, int j1, int j0, int r)
{
	string name = timevec[idx].GetEP();
	timevec[idx].GetPOS()->GetIdi()->push_back(timevec[idx].GetIdi());
	timevec[idx].GetPOS()->GetIdj()->push_back(timevec[idx].GetIdj());
	timevec[idx].GetPOS()->GetFFid()->push_back(ffset.find(name)->second);	// useless
	timevec[idx].GetPOS()->GetI1()->push_back((i1 - 1) * -1);
	timevec[idx].GetPOS()->GetI0()->push_back((i0 - 1) * -1);
	timevec[idx].GetPOS()->GetJ1()->push_back((j1 - 1) * -1);
	timevec[idx].GetPOS()->GetJ0()->push_back((j0 - 1) * -1);
	timevec[idx].GetPOS()->GetRj()->push_back((r - 1) * -1);
}
void CIRCUIT::AddInfo2(int idx, int rti, int i1, int i0, int j1, int j0, int r)
{
	string name = timevec[idx].GetEP();
	timevec[idx].GetPOS()->GetIdi()->push_back(timevec[idx].GetIdi());
	timevec[idx].GetPOS()->GetIdj()->push_back(timevec[idx].GetIdj());
	timevec[idx].GetPOS()->GetFFid()->push_back(ffset.find(name)->second);	// useless
	timevec[idx].GetPOS()->GetI1()->push_back((i1 - 1) * -1);
	timevec[idx].GetPOS()->GetI0()->push_back((i0 - 1) * -1);
	timevec[idx].GetPOS()->GetJ1()->push_back((j1 - 1) * -1);
	timevec[idx].GetPOS()->GetJ0()->push_back((j0 - 1) * -1);
	timevec[idx].GetPOS()->GetRj()->push_back((r - 1) * -1);
	timevec[idx].GetPOS()->GetRti()->push_back((rti - 1) * -1);
}
