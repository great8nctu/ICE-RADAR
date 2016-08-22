#ifndef CIRCUIT_H
#define CIRCUIT_H

#include<string>
#include<ext/hash_map>
#include"TimeInfo.h"		// include truthTable.h inside
using namespace __gnu_cxx;
using namespace std;
typedef vector<int> VEC;
typedef vector<VEC>* VECPTR;
typedef hash_map<string, int> HASH;
namespace __gnu_cxx
{
	template<> struct hash<string>	
	{
		size_t operator()(const string& s) const
		{
			return __stl_hash_string(s.c_str());
		}
	};
}
class CIRCUIT
{
	private:
		hash_map<string, int> fftable;		// hash table			maybe is useless
		hash_map<string, int> inputport;	// input port hash table
		hash_map<string, int> outputport;	// output port hash table
		hash_map<string, int> ffset;		// temp the set of flip flop
		hash_map<int, string> iffset;
		vector<TIMEINFO> timevec;		// save each critical path information include max and min
		vector<VEC> pos;
		float Tc;				// optimal tc
		float fix_tc;				// experiment 2, to find the optimal razor limit
		float aged_tc;			            
		float fresh_tc;
		float RDelay;				// Razor clock delay
		float RPenalty;			        // Razor extra cost (penalty)
		float buf[4];			        // buffer delay type
		float LongAging;	        	// long path aging level
		float ShortAging;		        // short path aging level
		float SlkLimit;
		float AvgAging;
		float CijAging;
		float TcqAging;
		float sample_boundary;
		int mode;				// wheither contain input-FF path
		int constraint;                 	// constraint limit k
		int experiment;
		int buf_control;
	public:
		//hash_map<string, int> HT() { return fftable; }
		TIMEINFO TimeVec(int idx) { return timevec[idx]; }
		int GetVecSize() { return timevec.size(); }
		int No_FF() { return ffset.size(); }
		int No_AFF() { return fftable.size(); }
		int No_PI() { return inputport.size(); }		// test used
		int No_PO() { return outputport.size(); }		// test used
		void SetTc(float x) { Tc = x; }					// test used
		void SetSlkLimit(float x) { SlkLimit = x; }		// test used
		void SetAvgAging(float x) { AvgAging = x; }		// test used
		void SetCijAging(float x) { CijAging = x; }		// test used
		void SetConstraint(int x) { constraint = x; }	// test used
		void SetRDelay(float x) { RDelay = x;}			// test used
		void SetRPenalty(float x) { RPenalty = x;}		// test used
		void SetLongAging(float x) { LongAging = x; }	// test used
		void SetShortAging(float x) { ShortAging = x; }	// test used
		void SetRazorLimit(int x) { constraint = x; }
		void SetSampleBoundary(float x) { sample_boundary = x; }
		void SetExperiment(int x) { experiment = x; }
		void SetClockAging(float x) { CijAging = x; }
		void SetTimeBorrowing(int x) { buf_control = x; }
		void SetFixTc(float x) { fix_tc = x; }
		float GetTc() { return Tc; }
		float GetFreshTc() { return fresh_tc; }
		float GetAgedTc() { return aged_tc; }
		float GetFixTc() { return fix_tc; }
		int GetConstraint() { return constraint; }
		float GetRDelay() { return RDelay; }
		float GetRPenalty() { return RPenalty; }
		float GetBuf(int idx) { return buf[idx]; }
		float GetLongAging() { return LongAging; }
		float GetShortAging() { return ShortAging; }
		float GetSlkLimit() { return SlkLimit; }
		float GetAvgAging() { return AvgAging; }
		float GetCijAging() { return CijAging; }
		float GetSampleBoundary() { return sample_boundary; }
		int GetTimeBorrowing() { return buf_control; }
		int GetExperiment() { return experiment; }
		int GetRazorLimit() { return constraint; }
		float Improvement() { return (aged_tc - Tc)/(aged_tc - fresh_tc) * 100; }
		//VECPTR GetPOS() { return &pos; }
		HASH GetHT() { return ffset; }					// test used
		//vector<TIMEINFO> GetTV() { return timevec; }
		int pos_size() { return pos.size(); }			// test used

		// Circuit.cpp
		void ReadData(string name);
		void ReadIO(string name);
		void ReadParameters(string name);
		void ReadSatFile(string name);
		void SetBuf();
		bool SetupConstraint(int ffi1, int ffi0, int ffj1, int ffj0, int rj, int idx);
		bool SetupConstraint2(int rti, int ffi1, int ffi0, int ffj1, int ffj0, int rj, int idx);
		bool HoldConstraint(int ffi1, int ffi0, int ffj1, int ffj0, int rj, int idx);
		void CalcTTable(int idx);
		void CalcTTable2(int idx);
		void SetPOS(int idx);
		void SetPOS2(int idx);
		void BinarySearchTc();
		void SearchLimit();
		void WriteCNF();
		void WriteCNF2();
		void CalcBuf(string name, int* value, int* razor);
		void Simplify();
		//void Derivation(int idx);
		void AddInfo(int idx, int i1, int i0, int j1, int j0, int r);
		void AddInfo2(int idx, int rti, int i1, int i0, int j1, int j0, int r);
};
#endif
