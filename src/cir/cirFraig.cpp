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
#include <unistd.h>
#include <thread>
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
    n = A/100 + 100;

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
    fecSolver fs1, fs2, fs3;
    fs1.init();
    initFECTask();
    task.clear();
    thread mt1(std::ref(fs1));
    while(currentFECGroup != fecGroupList->end())
    {
        usleep(100000);
        ;
    }
    if(mt1.joinable())
        mt1.join();
    cout<<"mt1 finisg"<<endl;
    for(vector<fraigTask>::iterator itr = task.begin();
        itr != task.end(); itr++)
    {
        merge(getGate(itr->merge),
              getGate(itr->parent),
              itr->invert,"Fraig");

    }
    checkhealth();
    buildfanout();
    buildDFSList();
    optimize();
    strash();
    

} 
/********************************************/
/*   Private member functions about fraig   */
/********************************************/


void CirMgr::fecSolver::operator () ()
{
    while(cirMgr->getFECTask(this->itr))
    {
        do
        {
            this->eraser.toRemove.clear();
            gid1 = (*itr)[0];
            this->eraser.toRemove.insert(gid1);
            for(IdList::iterator ite = itr->begin()+1;
                ite != itr->end(); ite++)
            {
                gid2 = (*ite);
                solve();
                if(not result)
                {
                    eraser.toRemove.insert(gid2);
                    cirMgr->setFraigTask(gid1,gid2,invert);
                }
            }
            itr->erase(remove_if(itr->begin(),
                                 itr->end(),
                                 this->eraser),
                       itr->end());
        }while(itr->size() > 1);
    }
    return;
}


void CirMgr::fecSolver::solve()
{
    if(solver == 0)
        init();

    CirGate* gate1 = cirMgr->getGate(gid1);
    CirGate* gate2 = cirMgr->getGate(gid2);
    solver->assumeRelease();
    solver->assumeProperty(varArray[gid1],false);
    invert = (gate1->phase != gate2->phase);

    if(gid1 == 0)
    {
        //might have problem
        #if PRINT_PROVING_RESULT
        cerr << "Proving " << gid2 << " = " << (invert ?"false":"true") << "...";
        #endif
        solver->assumeProperty(varArray[gid2],~invert);
    }
    else
    {
        #if PRINT_PROVING_RESULT
        cerr << "Proving (" << gid1 << ", "
        << (invert ? "":"!") << gid2 << ")...";
        #endif
        Var f = solver->newVar();
        solver->addXorCNF(f, varArray[gid1], false, varArray[gid2], invert);
        solver->assumeProperty(f, true);
    }
    result = solver->assumpSolve();
    #if PRINT_PROVING_RESULT
    cerr << (result?"SAT!!":"UNSAT!!")<<endl;
    #endif

}



void CirMgr::fecSolver::init()
{
    if(solver != 0)
        return;
    this->solver = new SatSolver();
    solver->initialize();
    CirGate* gate;
    varArray = new Var[cirMgr->M];

    varArray[0] = solver->newVar();
    for(IdList::const_iterator itr = cirMgr->dfsList.begin();
    itr != cirMgr->dfsList.end();itr++)
    {
        gate =  cirMgr->getGate(*itr);
        if(gate->gateType != AIG_GATE and
            gate->gateType != PI_GATE)
        {
            continue;
        }
        varArray[*itr] = solver->newVar();
        if(gate->gateType == AIG_GATE)
        {
            solver -> addAigCNF(varArray[*itr],
            varArray[gate->fanIn[0].first],gate->fanIn[0].second,
            varArray[gate->fanIn[1].first],gate->fanIn[1].second);
        }
    }
}
