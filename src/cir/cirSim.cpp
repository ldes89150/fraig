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
#include <math.h>

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
    if(not simulate)
    {
        fecGroupInit();
    }
    resetSim();
    unsigned maxFail = (unsigned)(3+ 3*(int)log((double)(A+I+O)));
    cout<<"MAX_FAIL = "<<maxFail<<endl;
    unsigned fail = 0;
    unsigned round;
    for(round = 0; fail <maxFail; round++)
    {
        randomAddPattern();
        roundSim(round);
        if(not fecGroupUpdate())
        {
            fail++;
        }
        else
        {
            ;
        }

    }

    cout<<round*32<<" patterns simulated."<<endl;
    fecGroupPushToGate();
    simulate = true;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
    if(not simulate)
    {
        fecGroupInit();
    }
    resetSim();


    cout << "\n"; // mysterious \n in ref???
    unsigned nSim = 0;
    uint32_t *patbuff = new uint32_t[I];
    for(unsigned i = 0; i<I; i++)
    {
        patbuff[i]=0;
    }
    while(true)
    {
        string curLine;
        patternFile>>curLine;
        if(curLine.length() != this->I)
        {
            if(!curLine.empty())
            {
                cerr << "\nError: Pattern(" << curLine <<  ") length(" << curLine.length()
                     << ") does not match the number of inputs(" << this->I << ") in a circuit!!" << endl;
            }
            break;
        }
        size_t pos = 0;
        // http://stackoverflow.com/questions/8888748/how-to-check-if-given-c-string-or-char-contains-only-digits
        if((pos = curLine.find_first_not_of("01")) != string::npos)
        {
            cerr << "Error: Pattern(" << curLine << ") contains a non-0/1 character(\'"
                 << curLine[pos] << "\')." << endl;
            break;
        }
        for(unsigned int i = 0; i < this->I; i++)
        {
            unsigned long long int tmpBit = (curLine[i] == '1');
            tmpBit <<= (nSim%32);
            patbuff[i] += tmpBit; // http://stackoverflow.com/questions/5369770/bool-to-int-conversion
        }
        nSim++;
        // start simulation
        if(nSim%32 == 0)
        {
            for(unsigned i = 0; i<I; i++)
            {
                unsigned gid = PIs[i]/2;
                patternPool[gid].push_back(patbuff[i]);
                patbuff[i]=0;
            }
        }
        if(patternFile.eof())
        {
            break;
        }
    }
    unsigned round =0;
    unsigned roundMAX =nSim/32;
    if(nSim%32 != 0)
    {
        roundMAX++;
        for(unsigned i = 0; i<I; i++)
        {
            unsigned gid = PIs[i]/2;
            patternPool[gid].push_back(patbuff[i]);
        }
    }
    for(round = 0; round<roundMAX; round++)
    {
        randomAddPattern();
        roundSim(round);
        fecGroupUpdate();
    }
    cout << ((PIs.size() != 0 && POs.size() != 0)?nSim:0) << " patterns simulated." << endl;
    fecGroupPushToGate();
    simulate = true;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/





void
CirMgr::roundSim(unsigned &round)
{
    for(vector<unsigned>::const_iterator itr = dfsList.begin();
            itr != dfsList.end(); itr++)
    {
        gateSim(*itr, round);
    }
}



void
CirMgr::gateSim(unsigned gid, unsigned &round)
{
    CirGate* gate = getGate(gid);
    assert(gate != 0);
    switch(gate->gateType)
    {
    case AIG_GATE:
    {
        CirGate* pin1 = getGate(gate->fanIn[0].first);
        CirGate* pin2 = getGate(gate->fanIn[1].first);
        unsigned pattern1, pattern2;
        if(pin1->gateType != UNDEF_GATE)
            pattern1 = pin1->pattern;
        else
            pattern1 = 0;

        if(pin2->gateType != UNDEF_GATE)
            pattern2 = pin2->pattern;
        else
            pattern2 = 0;

        if(gate->fanIn[0].second)
            pattern1 = ~pattern1;

        if(gate->fanIn[1].second)
            pattern2 = ~pattern2;

        gate->pattern = (pattern1 & pattern2);
        break;
    }
    case PI_GATE:
    {
        gate->pattern = patternPool[gid][round];
        break;
    }
    case PO_GATE:
    {
        unsigned pattern;
        if(gate->fanIn.empty())
        {
            pattern = 0;
            break;
        }
        CirGate* pin = getGate(gate->fanIn[0].first);
        if(pin->gateType != UNDEF_GATE)
            pattern = pin->pattern;
        else
            pattern = 0;

        if(gate->fanIn[0].second)
            pattern = ~pattern;

        gate->pattern = pattern;
        break;
    }
    case CONST_GATE:
    {
        gate->pattern = 0;
        break;
    }
    default:
    {
        assert(false);
    }
    };
    gate->phase = (gate->pattern % 2 ==1);
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
        (*ptr)->pattern = 0;
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
    unsigned n;
    if(A>100)
        n = A/10+100;
    else
        n = 100;
    if(fecHashMap != 0)
        delete fecHashMap;

    fecHashMap = new HashMap<CirGate::PatternKey,grouplist::iterator>((size_t) n);

    grouplist*  nfecGroupList = new grouplist();
    CirGate* gate;
    IdList group;
    group.push_back(0);
    for(IdList::iterator itr = dfsList.begin(); itr != dfsList.end(); itr++)
    {
        gate = getGate(*itr);
        if(gate->gateType != AIG_GATE)
            continue;

        group.push_back(*itr);
    }
    nfecGroupList->push_back(group);
    fecGroupList = nfecGroupList;
}




bool CirMgr::fecGroupUpdate()
{
    if(fecGroupList->empty())
        return false;

    fecHashMap->clear();
    CirGate* gate;
    CirGate::PatternKey key;
    grouplist::iterator group;
    grouplist* nfecGroupList = new grouplist();


    for(grouplist::iterator itr = fecGroupList->begin();
        itr != fecGroupList->end();)
    {
        for(IdList::iterator ite = itr->begin();
                ite != itr->end(); ite++)
        {
            gate = getGate(*ite);
            key = gate->getPatternKey();
            //cerr<<(*ite)<<' '<<key()<<' '<<key.pat<<endl;
            if(fecHashMap->retrive(key,group))
            {
                group->push_back(*ite);
            }
            else
            {
                nfecGroupList->push_back(IdList());
                group = (--(nfecGroupList->end()));
                group->push_back(*ite);
                fecHashMap->quickInsert(key,group);
            }
        }
        itr++;
        if(itr == fecGroupList->end())
            break;
        //fecHashMap->init(itr->size()/10+10);
        fecHashMap->resize(itr->size()/10+10);

    }
    size_t before, after;
    before = nfecGroupList->size();
    nfecGroupList->remove_if(fecGroupListEraser);
    /*nfecGroupList->erase(remove_if(nfecGroupList->begin(),
                                   nfecGroupList->end(),
                                   fecGroupListEraser),
                         nfecGroupList->end());*/
    after = nfecGroupList->size();
    delete fecGroupList;
    fecGroupList = nfecGroupList;
    return (before != after);
}

void CirMgr::fecGroupPushToGate()
{
    CirGate* gate;
    for(grouplist::iterator itr = fecGroupList->begin();
            itr != fecGroupList->end(); itr++)
    {
        std::sort(itr->begin(), itr->end());
        for(IdList::iterator ite = itr->begin();
                ite != itr->end(); ite++)
        {
            //cout<<(*ite)<<' ';
            gate = getGate(*ite);
            gate->infecg = true;
            gate->fecg = itr;
            if((gate->pattern) %2 ==1)
                gate->fectype=true;
            else
                gate->fectype=false;
        }
    }
}
