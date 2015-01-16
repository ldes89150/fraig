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
#include <algorithm>

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
    net pinA, pinB;
    CirGate** ptr;
    for(std::vector<unsigned>::const_iterator itr = dfsList.begin() ;
        itr != dfsList.end(); itr++)
    {
        ptr = gates + (*itr);
        if(*ptr == 0)
            continue;
        
        if((*ptr)->gateType != AIG_GATE)
            continue;
        
        
        pinA = (*ptr)->fanIn[0];
        pinB = (*ptr)->fanIn[1];
        
        //cout<<getGate(pinA.first)<<":"<<pinA.first<<" "<<pinA.second<<endl;
        //cout<<getGate(pinB.first)<<":"<<pinB.first<<" "<<pinB.second<<endl;
        
        if(pinA.first == pinB.first)
        {
            if(pinA.second == pinB.second)
            {
                merge(*ptr,getGate(pinA.first),pinA.second, "Simplifying");
            }
            else
            {
                merge(*ptr,getGate(0),false ,"Simplifying");// connect to zero
            }
        }
        else
        {
            if(pinA.first == 0)
            {
                if(pinA.second)//pinA = 1
                    merge(*ptr,getGate(pinB.first),pinB.second, "Simplifying");
                else
                    merge(*ptr,getGate(0),false ,"Simplifying");
            }
            else{
            if(pinB.first == 0)
            {
                if(pinB.second)//pinB = 1
                    merge(*ptr,getGate(pinA.first),pinA.second, "Simplifying");
                else
                    merge(*ptr,getGate(0),false ,"Simplifying");
            }
            else
            {
                continue;
            }}
        }
        //cout<<"delete:"<<(*ptr)->id<<endl;
        delete *ptr;
        *ptr = 0;
        A--;
        
    }
    buildDFSList();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/


void
CirMgr::merge(CirGate* a, CirGate* b, bool invert ,string why)
{
    //replace gate a with b
    //Warning: this function would not maintain the dfslist and Cirgates,
    //you should maintain it by yourself.
    cout << why << ": " << b->id << " merging " ;
    if(invert)
        cout<<"!";
        
    cout<<a->id<< "..." << endl;

//ref: http://stackoverflow.com/questions/347441/erasing-elements-from-a-vector
    for(std::vector<net> ::iterator itr = a->fanIn.begin();
        itr != a->fanIn.end(); itr++)
    {;
        getGate(itr->first)->removeFanOutID(a->id);
    }
    
    
    for(std::vector<net> ::iterator itr = a->fanOut.begin();
        itr != a->fanOut.end(); itr++)
    {
        CirGate* gate = getGate(itr->first);
        for(std::vector<net>::iterator ite = gate->fanIn.begin() ;
            ite != gate->fanIn.end();ite++)
        {
            if(ite->first != a->id)
                continue;
            
            ite->first = b->id;
            if(invert)
            {
                ite->second = not ite->second;
                b->fanOut.push_back(net(gate->id, not ite->second));
            }
            else
            {
                ite->second = ite->second;
                b->fanOut.push_back(net(gate->id, ite->second));
            }
            
        }
        
    }
}
