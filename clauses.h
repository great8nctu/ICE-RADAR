#ifndef CLAUSES_H
#define CLAUSES_H
#include<vector>
using namespace std;
typedef vector<int>* VIPTR;
class CLAUSES 
{
	private:
		vector<int> idi;
		vector<int> idj;                    // useless
        	vector<int> ffid;
		vector<int> i1;
		vector<int> i0;
		vector<int> j1;
		vector<int> j0;
		vector<int> rj;
		vector<int> rti;
	public:
		VIPTR GetIdi() { return &idi; }
		VIPTR GetIdj() { return &idj; }     // useless
        VIPTR GetFFid() { return & ffid; }
		VIPTR GetI1() { return &i1; }
		VIPTR GetI0() { return &i0; }
		VIPTR GetJ1() { return &j1; }
		VIPTR GetJ0() { return &j0; }
		VIPTR GetRj() { return &rj; }
		VIPTR GetRti() { return &rti; }
		void clear()
		{
			idi.clear();
			idj.clear();
		 	ffid.clear();
			i1.clear();
			i0.clear();
			j1.clear();
			j0.clear();
			rj.clear();
			rti.clear();
		}
};

#endif
