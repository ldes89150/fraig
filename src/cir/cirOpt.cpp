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
    GateType g;
    for(CirGate** ptr = cirGateBegin();
        ptr != cirGateEnd(); ptr++)
    {
        if((*ptr) != 0)
        {
            if(!((*ptr)->reachability))
            {
                g = (*ptr)->gateType;
                if(g == AIG_GATE)
                {
                    A--;
                }
                else if(g == CONST_GATE or g == PI_GATE)
                {
                   continue;
                } 
                //Assert Undefine Gates have false reachability
                #if DEBUG
                assert(g != PO_GATE); // Asser that all PO gates are reachable.
                assert(g == AIG_GATE or g == UNDEF_GATE);
                #endif
                cout<<"Sweeping: "<<g
                    <<"("<<(*ptr)->id<<") removed..."<<endl;
                delete (*ptr);
                (*ptr) = 0;
            }
        }
    }
}

void
CirMgr::optimize()
{
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

