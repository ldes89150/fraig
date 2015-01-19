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

class fecEraser;
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


class fraigTask
{
public:
    fraigTask(unsigned p, unsigned m, bool i):
    parent(p), merge(m), invert(i){}

    unsigned parent;
    unsigned merge;
    bool invert;
};



void
CirMgr::fraig()
{
    bool invert;
    unsigned ref;
    fecEraser eraser;
    typedef std::vector<fraigTask> taskList;
    taskList task;
    for(grouplist::iterator itr = fecGroupList->begin();
        itr != fecGroupList->end(); itr++)
    {
        do
        {
            eraser.toRemove.clear();
            ref = (*itr)[0];
            eraser.toRemove.insert(ref);
            for(IdList::iterator ite = itr->begin()+1;
                ite != itr->end();ite++)
            {
                if(not solveBySat(ref,*ite,invert))
                {
                    eraser.toRemove.insert(ref);
                    task.push_back(fraigTask(ref,*ite,invert));
                }
            }
            itr->erase(remove_if(itr->begin(),
                                 itr->end(),
                                 eraser),
                       itr->end());
        }while(itr->size() > 1);
    }

    for(taskList::iterator itr = task.begin();
        itr != task.end();itr++)
    {
        merge(getGate(itr->merge),
              getGate(itr->parent),
              itr->invert, "Fraig");
    }
    #if CHECK_HEALTH
    checkhealth();
    #endif
    buildfanout();
    buildDFSList();

}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

bool CirMgr::solveBySat(unsigned gid1, unsigned gid2, bool &invert)
{
    if(satSolver == 0)
        satInitialize();

    CirGate* gate1 = getGate(gid1);
    CirGate* gate2 = getGate(gid2);
    satSolver->assumeRelease();
    satSolver->assumeProperty(gate1->satVar,false);
    invert = (gate1->phase != gate2->phase);

    if(gid1 == 0)
    {
        //might have problem
        cout << "\nProving " << gid2 << " = " << (invert ?"0":"1") << "...";
        satSolver->assumeProperty(gate2->satVar,invert);
    }
    else
    {
        cout << "\nProving (" << gid1 << ", "
             << (invert ? "":"!") << gid2 << ")...";
        Var f = satSolver->newVar();
        satSolver->addXorCNF(f, gate1->satVar, false, gate2->satVar, invert);
        satSolver->assumeProperty(f, true);
    }
    bool result = satSolver->assumpSolve();
    cout << (result?"SAT!!":"UNSAT!!")<<endl;
    return result;
}



void CirMgr::satInitialize()
{
    if(satSolver != 0)
        return;
    this->satSolver = new SatSolver();
    satSolver->initialize();
    CirGate* gate;
    gate = getGate(0);
    gate->satVar = satSolver->newVar();
    for(IdList::const_iterator itr = dfsList.begin();
        itr != dfsList.end();itr++)
    {
        gate = getGate(*itr);
        if(gate->gateType != AIG_GATE and
           gate->gateType != PI_GATE)
        {
           continue;
        }
        gate->satVar = satSolver->newVar();
        if(gate->gateType == AIG_GATE)
        {
            satSolver -> addAigCNF(gate->satVar,
            getGate(gate->fanIn[0].first)->satVar,gate->fanIn[0].second,
            getGate(gate->fanIn[1].first)->satVar,gate->fanIn[1].second);
        }
    }
}
