/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr()
   {
       
       CirGate * gate;
       if(gates==0)
           return;
       for(unsigned i =0;i<M+O+1;i++)
       {
           gate = getGate(i);
           if(gate !=0)
               delete gate;
       }
       delete gates;
       
   }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const
   {
       if(gid >= (M+O+1))
       {
           return 0;
       }
       return gates[gid];
   }
   // My Custom Function
   string getGateName(unsigned gid) const
   {
       map<unsigned,string>::const_iterator itr = nameTable.find(gid);
       if(itr == nameTable.end())
           return "";
       else
           return (*itr).second;
   }
   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;

private:
   ofstream           *_simLog;

   typedef map<unsigned,CirGate*> cirDict;
   unsigned int M;
   unsigned int I;
   unsigned int L;
   unsigned int O;
   unsigned int A;
   CirGate** gates;
   vector<unsigned> PIs;
   vector<unsigned> POs;
   //vector<unsigned> Ls;
   //vector<unsigned> As;
   set<unsigned> unDefs;//change name (floatFanInID)
   set<unsigned> gatesWithFloatFanin;
   vector<unsigned> dfsList;
   //set<unsigned> reachableID;
   map<unsigned,string> nameTable;
   void buildfanout();
   void buildDFSList();

};

#endif // CIR_MGR_H
