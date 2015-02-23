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
    fecSolver fs1, fs2;
    fs1.init();
    fs2.init();
    fs1.id = 0;
    fs2.id = 1;
    initFECTask();
    task.clear();
    thread mt1(std::ref(fs1));
    thread mt2(std::ref(fs2));
    while(taskFinish)
    {
        std::this_thread::yield();
    }
    mt1.join();
    mt2.join();
    for(vector<fraigTask>::iterator itr = task.begin();
        itr != task.end(); itr++)
    {
        merge(getGate(itr->merge),
              getGate(itr->parent),
              itr->invert,"Fraig");

    }
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
    cirMgr->readForSim[id] = false;
    bool meetUNSAT = false;
    counter = 0;
    while(cirMgr->getFECTask(this->itr))
    {
        do
        {
            gid1 = (*itr)[0];
            for(IdList::iterator ite = itr->begin()+1;
                ite != itr->end();)
            {
                gid2 = (*ite);
                solve();
                if(not result)
                {
                    itr->erase(ite);
                    cirMgr->setFraigTask(gid1,gid2,invert);
                    meetUNSAT = true;
                }
                else
                {
                    ite++;
                    for(IdList::const_iterator ita = cirMgr->PIs.begin();
                        ita != cirMgr->PIs.end(); ita++)
                    {
                        unsigned id = (*ita)/2;
                        unsigned modelValue = solver->getValue(varArray[id]);
                        (cirMgr->patternPool[id])[0] = 
                        (cirMgr->patternPool[id])[0]*2 + modelValue; 
                    }
                    if(meetUNSAT)
                        counter++;
                }
                if(cirMgr->ready)
                {
                    std::unique_lock<std::mutex> lck(cirMgr->simMutex) ;
                    cirMgr->readForSim[id] = true;
                    cirMgr->simCon.wait(lck); 
                    cirMgr->readForSim[id] = false;
                    goto endSection;    
                }
                else if(counter >= 31)
                {
                    cirMgr->ready = true;
                    cirMgr->readForSim[id] = true;
                    while(not (cirMgr->readForSim[0] and cirMgr->readForSim[1] and 
                               cirMgr->readForSim[2] and cirMgr->readForSim[3]))
                    {
                        std::this_thread::yield();
                    }
                    cirMgr->ready = false;
                    meetUNSAT = false;
                    counter = 0;
                    cirMgr->readForSim[id] = false;
                    cirMgr->roundSim(id);
                    cirMgr->fecGroupUpdate();
                    cirMgr->initFECTask();


                    cirMgr->simCon.notify_all();
                    goto endSection;
                }
            }
            itr->erase(itr->begin());
            


        }while(itr->size() > 1);
        cirMgr->fecGroupList->erase(itr);
        endSection:
        ;
    }
    cirMgr->readForSim[id] = true;
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
    for(auto &c:cirMgr->readForSim)
    {
        c = true;
    }
    cirMgr->ready = false;
}
