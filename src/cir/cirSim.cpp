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
    resetSim();
    fecGroupInit();
    randomAddPattern();
    roundSim(0);
    fecGroupUpdate();
    fecGroupPushToGate();

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
        (*ptr)->infecg = false;
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


void CirMgr::fecGroupInit()
{
    grouplist*  nfecGroupList = new grouplist();
    CirGate* gate;
    IdList group;
    for(IdList::iterator itr = dfsList.begin();itr != dfsList.end();itr++)
    {
        gate = getGate(*itr);
        if(gate->gateType != AIG_GATE)
            continue;
        
        group.push_back(*itr);
    }
    nfecGroupList->push_back(group);
    fecGroupList = nfecGroupList;
}




void CirMgr::fecGroupUpdate()
{
    unsigned n;
    if(A>100)
        n = A/100;
    else
        n = 100;
    HashMap<CirGate::PatternKey,grouplist::iterator> fecHashMap((size_t) n); 
    CirGate* gate; 
    CirGate::PatternKey key;
    grouplist::iterator group;
    grouplist* nfecGroupList = new grouplist();
    for(grouplist::iterator itr = fecGroupList->begin();
        itr != fecGroupList->end(); itr++)
    {
    for(IdList::iterator ite = itr->begin();
        ite != itr->end();ite++)
    {
        gate = getGate(*ite);
        key = gate->getPatternKey();
        cerr<<(*ite)<<' '<<key()<<' '<<key.pat<<endl;
        if(fecHashMap.retrive(key,group))
        {
            group->push_back(*ite);
        }
        else
        {
            nfecGroupList->push_back(IdList());
            group = (--(nfecGroupList->end()));
            group->push_back(*ite);
            fecHashMap.quickInsert(key,group);
        }
    }
    }

    nfecGroupList->erase(remove_if(nfecGroupList->begin(),
                                   nfecGroupList->end(),
                                   fecGroupListEraser),
                         nfecGroupList->end());
    fecGroupList = nfecGroupList;
}

void CirMgr::fecGroupPushToGate()
{
    CirGate* gate;
    for(grouplist::iterator itr = fecGroupList->begin();
        itr != fecGroupList->end(); itr++)
    {
    for(IdList::iterator ite = itr->begin();
        ite != itr->end();ite++)
    {
        cout<<(*ite)<<' ';
        gate = getGate(*ite);
        gate->infecg = true;
        gate->fecg = itr;
        if(*(gate->pattern.end()-1) %2 ==1)
            gate->fectype=true;
        else
            gate->fectype=false;
    }
    }
}




