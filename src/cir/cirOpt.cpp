/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#define DEBUG 1
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <queue>

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
void
CirMgr::sweep()
{
    set<unsigned> removeId;
    queue<CirGate**> removePtr;
    GateType g;
    CirGate** ptr;
    for(ptr = cirGateBegin()+1; // skip const gate
        ptr != cirOutputGateBegin(); ptr++)
    {
        if(*ptr == 0)
            continue;
        if((*ptr)->reachability)
            continue;
        g = (*ptr)->gateType;
        if(g == PI_GATE ) 
            continue;
        if(g == AIG_GATE)
        {
           A--;
        }
        else
        {
            #if DEBUG
            assert(g == UNDEF_GATE);
            #endif
            bool c = false;
            //keep "good" undef gate
            for(vector<net>::const_iterator itr = (*ptr)->fanOut.begin();
                itr != (*ptr)->fanOut.end();itr++)
            {
                if(getGate(itr->first)->reachability)
                {
                    c = true;
                    break;
                }                
            }
            if(c) continue;
        }
        removeId.insert((*ptr)->id);
        removePtr.push(ptr);
    }
    //If the fanIn gate of the gate to be removed would be kept after sweep,
    //modify its fanOut.
    while(not removePtr.empty())
    {
        ptr = removePtr.front();

        for(vector<net>::const_iterator itr = (*ptr)->fanIn.begin();
            itr != (*ptr)->fanIn.end(); itr++)
        {
            CirGate* gate = getGate(itr->first);
            if(removeId.find(gate->id) == removeId.end())
                continue;
            for(vector<net>::iterator ite = gate->fanOut.begin();
                ite != gate->fanOut.end(); ite++)
            {
                if((ite->first) == (*ptr)->id)
                {
                    gate->fanOut.erase(ite);
                    break;
                }
            }
        }
        cout<<"Sweeping: "<<g
             <<"("<<(*ptr)->id<<") removed..."<<endl;
        delete (*ptr);
        (*ptr) = 0;

        removePtr.pop();
    }
}

void
CirMgr::optimize()
{
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/



