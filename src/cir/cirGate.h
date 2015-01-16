/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"
#include <set>
#include <utility>
#include <algorithm>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.


typedef pair<unsigned,bool> net;

net make_net(unsigned &pin);
unsigned get_pin(net l);


class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   friend class CirMgr;

   // Basic access methods
   string getTypeStr() const { return ""; }
   unsigned getLineNo() const { return 0; }


   CirGate() : reachability(false){}
   CirGate(enum GateType gateType,unsigned int id,unsigned lineNo):gateType(gateType),id(id), lineNo(lineNo), reachability(false){}
   ~CirGate() {}

   // Printing functions
   //virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   //custom function
   void addFanIn(unsigned pin){ fanIn.push_back(make_net(pin)); }
   //void addFanIn(unsigned Id,bool inverse);
   void addFanOut(unsigned Id,bool inverse);
   static string gateTypeStr(enum GateType gt)
   {
      switch(gt)
      {
         case PI_GATE:
            return "PI";
         case PO_GATE:
            return "PO";
         case AIG_GATE:
            return "AIG";
         case CONST_GATE:
            return "CONST";
         case UNDEF_GATE:
            return "UNDEF";
         default:
            return "";
      }
   }
   struct netEraser
   {
      netEraser(unsigned id) : id(id) {}
      unsigned id;
      bool operator()(net i) const
      {
         return i.first != id;
      }
   };

private:
   void printUndef(unsigned inden, bool inverse, unsigned undefID) const;
   void printFanIn(unsigned inden, int level, bool inverse, set<unsigned>* &reported) const;
   void printFanOut(unsigned inden, int level, bool inverse, set<unsigned>* &reported) const;

protected:
   enum GateType gateType;
   vector<net> fanIn;
   vector<net> fanOut;
   unsigned int id;
   unsigned int lineNo;
   bool reachability;

   void removeFanInID(unsigned cid)
   {
      fanIn.erase(remove_if(fanIn.begin(),
                            fanIn.end(),
                            netEraser(cid)),
                  fanIn.end());
   }
   void removeFanOutID(unsigned cid)
   {
      fanOut.erase(remove_if(fanOut.begin(),
                            fanOut.end(),
                            netEraser(cid)),
                   fanOut.end());      
   }
   

};

#endif // CIR_GATE_H
