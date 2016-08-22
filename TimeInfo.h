#ifndef TIMEINFO_H
#define TIMEINFO_H
#include<string>
#include<vector>
#include"clauses.h"
using namespace std;
class TIMEINFO
{
	private:
		string SP;
		string EP;
		string type;
		string clk;
		int IDi;			// unused
		int IDj;			// unused
		float LongPath;
		float ShortPath;
		float At;
		float Rt;
		float Slack;			// unused
		float ci;
		float cj;
		float Tcq;
		float Tsu;
		float Th;
		float Tun;
		float retiming;
		//CLAUSES sop;
		CLAUSES pos;
	public:
		void Init()
		{
			SP.clear();
			EP.clear();
			type.clear();
			IDi = -1;
			IDj = -1;
			LongPath = 0;
			ShortPath = 0;
			At = -1;
			Rt = -1;
			Slack = -1;
			ci = -1;
			cj = -1;
			Tcq = -999;
			Tsu = -999;
			Th = -999;
			retiming=0;
		}

		int GetIdi() { return IDi; }
		int GetIdj() { return IDj; }
		float GetAt() { return At; }
		float GetRt() { return Rt; }
		float GetCi() { return ci; }
		float GetCj() { return cj; }
		float GetTcq() { return Tcq; }
		float GetTsu() { return Tsu; }
		float GetTh() { return Th; }
		float GetLongPath() { return LongPath; }
		float GetShortPath() { return ShortPath; }
        float GetSlk() { return Slack; }
		float GetTun() { return Tun; }
		string GetSP() { return SP; }
		string GetEP() { return EP; }
		string GetType() { return type; }
		string GetClk() { return clk; }
		//CLAUSES* GetSOP() { return &sop; }
		CLAUSES* GetPOS() { return &pos; }
		float GetRetiming() { return retiming; }

		void SetSP(string x) { SP = x; }
		void SetEP(string x) { EP = x; }
		void SetType(string x) { type = x; }
		void SetAt(float x) { At = x; }
		void SetRt(float x) { Rt = x; }
		void SetSlk(float x) { Slack = x; }
		void SetIdi(int x) { IDi = x; }			// useless
		void SetIdj(int x) { IDj = x; }			// useless
		void SetCi(float x) { ci = x; }
		void SetCj(float x) { cj = x; }
		void SetTcq(float x) { Tcq = x; }
		void SetTsu(float x) { Tsu = x; }
		void SetTh(float x) { Th = x; }
		void SetTun(float x) { Tun = x; }
		void SetLongPath(float x) { LongPath = x; }
		void SetShortPath(float x) { ShortPath = x; }
		void SetClk(string x) { clk = x; }
		void SetRetiming(float t) { retiming = t; }
};
/*
void TIMEINFO::Init()
{
	SP.clear();
	EP.clear();
	type.clear();
	IDi = -1;
	IDj = -1;
	LongPath = 0;
	ShortPath = 0;
	At = -1;
	Rt = -1;
	Slack = -1;
	ci = -1;
	cj = -1;
	Tcq = -999;
	Tsu = -999;
	Th = -999;
}
*/
#endif
