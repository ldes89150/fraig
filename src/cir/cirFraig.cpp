/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
void
CirMgr::strash()
{
    unsigned n;
    if(A>100)
        n = A/100;
    else
        n = 100;

    HashMap<CirGate::FanInKey, unsigned> gatesHashMap((size_t) n);
    unsigned match;
    CirGate::FanInKey key;
    CirGate* gate;
    for(vector<unsigned>::iterator itr = dfsList.begin();
        itr != dfsList.end(); itr++)
    {
        gate = getGate(*itr);
        if(gate->gateType !=AIG_GATE)
            continue;

        key = gate->getKey();
        //cout<<"id:"<<gate->id<<" k():"<<key()<<endl;
        if(gatesHashMap.retrive(key,match))
        {
            merge(gate,getGate(match),false,"Strashing");
            removeGate(*itr);
        }
        else
        {
            gatesHashMap.quickInsert(key,*itr);
        }
    }
    buildfanout();
    this->buildDFSList();
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
