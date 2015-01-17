/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
    randomAddPattern();
    roundSim(0);
}

void
CirMgr::fileSim(ifstream& patternFile)
{
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/




    
void
CirMgr::roundSim(unsigned round)
{
    for(vector<unsigned>::const_iterator itr = dfsList.begin();
        itr != dfsList.end(); itr++)
    {
        gateSim(*itr, round);
    }
}


    
void
CirMgr::gateSim(unsigned gid, unsigned round)
{
    CirGate* gate = getGate(gid);
    assert(gate != 0);
    switch(gate->gateType)
    {
case AIG_GATE:
    {
    CirGate* pin1 = getGate(gate->fanIn[0].first);
    CirGate*pin2 = getGate(gate->fanIn[1].first);
    unsigned pattern1, pattern2; 
    if(pin1->gateType != UNDEF_GATE) 
        pattern1 = pin1->pattern[round];
    else
        pattern1 = 0;

    if(pin2->gateType != UNDEF_GATE)
        pattern2 = pin2->pattern[round];
    else
        pattern2 = 0;

    if(gate->fanIn[0].second)
        pattern1 = ~pattern1;

    if(gate->fanIn[1].second)
        pattern2 = ~pattern2;
    
    gate->pattern.push_back(pattern1 & pattern2);
    return;
    }
case PI_GATE:
    {
    gate->pattern.push_back(patternPool[gid][round]);
    return;
    }
case PO_GATE:
    {
    CirGate* pin = getGate(gate->fanIn[0].first);
    unsigned pattern;
    if(pin->gateType != UNDEF_GATE)
        pattern = pin->pattern[round];
    else 
        pattern = 0;

    if(gate->fanIn[0].second)
        pattern = ~pattern;
    
    gate->pattern.push_back(pattern);
    return;
    }
case CONST_GATE:
    {
    gate->pattern.push_back(0);
    return;
    }
default:
    {
    assert(false);
    }
    };
}


void CirMgr::resetSim()
{
    for(map<unsigned,vector<uint32_t> > ::iterator itr = patternPool.begin();
        itr != patternPool.end(); itr++)
    {
        itr->second.clear();
    }
    for(CirGate** ptr = cirGateBegin(); ptr != cirGateEnd(); ptr++)
    {
        if(*ptr == 0)
            continue;
        (*ptr)->pattern.clear();
    }
}


void CirMgr::randomAddPattern()
{
    for(map<unsigned,vector<uint32_t> > ::iterator itr = patternPool.begin();
        itr != patternPool.end(); itr++)
    {
        itr->second.push_back((rand()<<16)+rand());
    }
}





