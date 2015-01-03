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
#include <utility>
#include <set>


using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
    friend class CirMgr;

    public:
        typedef pair<unsigned, bool> net;
        net make_net(unsigned &pin)
        {
            return net(pin/2,(pin%2)==1);
        }
        static unsigned getPin(net &l)
        {
            if(l.second)
                return l.first*2+1;
            else
                return l.first*2;
        }
        
        CirGate() {}
        CirGate(GateType g, unsigned i, unsigned l): gateType(g), id(i), lineNo(i){}
        ~CirGate() {}

        // Basic access methods
        string getTypeStr() const { return ""; }
        unsigned getLineNo() const { return lineNo; }

        // Printing functions
        virtual void printGate() const = 0;
        void reportGate() const;
        void reportFanin(int level) const;
        void reportFanout(int level) const;
        
        //custom function
        void addFanIn(net);
        void addFanOut(net);
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

    private:
        void printUndef(unsigned inden, bool inverse, unsigned undefID) const;
        void printFanIn(unsigned inden, int level, bool inverse, set<unsigned>* &reported) const;
        void printFanOut(unsigned inden, int level, bool inverse, set<unsigned>* &reported) const;
    protected:
        enum GateType gateType;
        bool reachable;
        unsigned id;
        unsigned lineNo; 
        vector<net> fanIn;
        vector<net> fanOut;
};

#endif // CIR_GATE_H
