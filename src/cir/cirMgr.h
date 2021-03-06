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
#include "myHashMap.h"
#include "cirGate.h"
#include <mutex>
#include <thread>
#include <condition_variable>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class fraigTask
{
public:
    fraigTask(unsigned p, unsigned m, bool i):
    parent(p), merge(m), invert(i){}

    unsigned parent;
    unsigned merge;
    bool invert;
};
class fecEraser
{
public:
    set<unsigned> toRemove;
    bool operator () (unsigned g) const
    {
        return toRemove.find(g) == toRemove.end();
    }

};

class CirMgr
{
public:
   CirMgr()
   {
       simulate = false;
       _simLog = 0;
       satSolver = 0;
       fecGroupList = 0;
       gates = 0;
       fecHashMap = 0;
   }
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
       delete fecGroupList;
       if(fecHashMap != 0)
           delete fecHashMap;
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

   CirGate** cirGateBegin()         {return gates;}
   CirGate** cirOutputGateBegin()   {return gates + (M+1);}
   CirGate** cirGateEnd()           {return gates + (M+O+1);}

   void merge(CirGate* a, CirGate* b, bool invert ,string why);
   void checkhealth();
   void removeGate(unsigned gid);

   //for simulation
   map<unsigned,vector<uint32_t> > patternPool;
   grouplist* fecGroupList;
   HashMap<CirGate::PatternKey,grouplist::iterator>* fecHashMap;


   bool simulate;
   unsigned nSim;
   void gateSim(unsigned gid, unsigned &round, bool skipPISim = false);
   void roundSim(unsigned &round, unsigned bitPerRound = 32);
   void resetSim();
   void randomAddPattern();
   void fecGroupInit();
   bool fecGroupUpdate();
   void fecGroupPushToGate();
   static bool fecGroupListEraser(const IdList &i)
   {
       return i.size()==1;
   }

   //for fraig
   
   grouplist::iterator currentFECGroup;
   vector<fraigTask> task; 
   
   bool taskFinish;

   void initFECTask()
   {
       currentFECGroup = fecGroupList->begin();
       taskFinish = false;
       return;
   }   

   mutex cirMutex;

   bool getFECTask(grouplist::iterator &itr)
   {
       lock_guard<mutex> mLock(cirMutex);
       if(taskFinish)
           return false;
       if(currentFECGroup == fecGroupList->end())
       {
           taskFinish = true;
           return false;
       }
       itr = currentFECGroup;
       currentFECGroup ++;
       return true;
   }

   void setFraigTask(unsigned &parent, unsigned &merge, bool &invert)
   {
       lock_guard<mutex> mLock(cirMutex);
       task.push_back(fraigTask(parent, merge, invert));
   }
    
   void fraigTaskMerge()
   {
        for(vector<fraigTask>::iterator itr = task.begin();
            itr != task.end(); itr++)
        {
            merge(getGate(itr->merge),
                  getGate(itr->parent),
                  itr->invert,"Fraig");

        }
        task.clear();
   }

   SatSolver* satSolver;
   void satInitialize();
   bool solveBySat(unsigned gid1, unsigned gid2, bool &invert);


   bool readForSim[4];
   bool ready;
   std::condition_variable simCon;
   std::mutex simMutex;
    
   class fecSolver
   {
   public:
       fecSolver()
       {
           varArray = 0;
           solver = 0;
       }
       ~fecSolver()
       {
           if(varArray != 0)
               delete [] varArray;
           if(solver != 0)
               delete solver;
       }
       unsigned id;
       unsigned counter;
       Var* varArray;
       SatSolver* solver;
       unsigned gid1, gid2;
       bool invert, result;
       fecEraser eraser; 
       grouplist::iterator itr;

       void init();
       void solve();
       void operator () ();
   };
};


#endif 
