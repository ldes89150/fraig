/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2014 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <sstream>
#include <stack>

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
    fstream fin;
    fin.open(fileName.c_str(),ios::in);
    if(!fin)
    {
        cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
        fin.close();
        return false;
    }
    stringstream ss;
    unsigned lineNo = 1;
    unsigned ID;
    enum { header, input, latch, output, andGate, symbol, comment } state = header;
    string cirStr;
    unsigned andNum=0;

    while(true)
    {
        if(fin.eof() or state == comment)
        {
            break;
        }
        getline(fin,cirStr);
        ss.clear();
        ss.str(cirStr);
        if(*(cirStr.rbegin())==' ' or *(cirStr.begin()) == ' ')
        {
            parseError(EXTRA_SPACE);
            colNo++;
        }
        switch(state)
        {
            case header:
                if(5 == sscanf(cirStr.c_str(), "aag %d %d %d %d %d", &M, &I, &L, &O, &A))
                {
                    gates = new CirGate*[M+O+1];
                    for(unsigned i =0; i != M+O+1;i++)
                    {
                        gates[i] = 0;
                    }
                    state = input;
                    PIs.reserve(I);
                    POs.reserve(O);
                    gates[0] = new CirGate(CONST_GATE,0,0);
                }
                break;
            case input:
                if(PIs.size()== I)
                {
                    state = latch;
                }
                else
                {
                    unsigned i;
                    ss>>i;
                    #if DEBUG_MODE
                    assert(i%2 ==0);
                    #endif
                    ID = i/2;
                    if(ID>M)
                    {
                        errMsg = "PI index";
                        parseError(NUM_TOO_BIG);
                        colNo++;
                    }
                    gates[ID] = new CirGate(PI_GATE,ID,lineNo);
                    PIs.push_back(i);
                    break;
                }
            case latch:
                state = output;
            case output:
                if(POs.size() == O)
                {
                    state = andGate;
                }
                else
                {
                    unsigned o;
                    ss>>o;
                    ID = M+POs.size()+1;
                    gates[ID] = new CirGate(PO_GATE,ID,lineNo);
                    gates[ID]->addFanIn(o);
                    POs.push_back(o);
                    break;
                }
            case andGate:
                if(andNum == A)
                {
                    state = symbol;
                }
                else
                {
                    unsigned a, pin1, pin2;
                    ss>>a>>pin1>>pin2;
                    #if DEBUG_MODE
                    assert(a%2 == 0);
                    #endif
                    ID = a/2;
                    gates[ID] = new CirGate(AIG_GATE,ID,lineNo);
                    gates[ID]->addFanIn(pin1);
                    gates[ID]->addFanIn(pin2);
                    andNum++;
                    break;
                }
            case symbol:
                if(cirStr[0] != 'c')
                {
                    char io;
                    unsigned position;
                    string ioName;
                    ss.get(io);
                    ss>>position;
                    ss.get();
                    getline(ss,ioName);
                    if(io=='i')
                    {
                        if(position>I)
                            break;
                        ID = PIs[position]/2;
                    }
                    else
                    {
                        if(io != 'o')
                           break;
                        ID = M+position+1;
                    }
                    if(nameTable.find(ID)!=nameTable.end())
                        break;
                    nameTable[ID] = ioName;
                    break;
                }
                else
                {
                    state = comment;
                }
            case comment:
                break;
        }
        lineNo++;
    }
    fin.close();
    //find floatgate section
    for(unsigned i =0; i != M+O+1;++i )
    {
        if(gates[i]==0)
        {
            continue;
        }
        for(vector<net>::const_iterator itr = gates[i]->fanIn.begin();
            itr != gates[i]->fanIn.end();itr++)
        {
            ID = (*itr).first;
            if(gates[ID]==0)
            {
                gatesWithFloatFanin.insert(i);
                unDefs.insert(ID);
                gates[ID] = new CirGate(UNDEF_GATE,ID,0);
            }
        }
    }
    buildfanout();
    buildDFSList();

    for(vector<unsigned>::const_iterator itr = PIs.begin();
        itr != PIs.end();itr++)
    {
        patternPool[*itr /2] = vector<unsigned>();

    }
    #if CHECK_HEALTH
    checkhealth();
    #endif


    return true;

}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
    cout<<endl;
    cout<<"Circuit Statistics"<<endl
    <<"=================="<<endl
    <<"  PI"<<right<<setw(12)<<I<<endl
    <<"  PO"<<right<<setw(12)<<O<<endl
    <<"  AIG"<<right<<setw(11)<<A+L<<endl
    <<"------------------"<<endl
    <<"  Total"<<right<<setw(9)<<I+O+A+L<<endl;
}

void
CirMgr::printNetlist() const
{
    cout<<endl;
    unsigned i =0;
    CirGate* gate;
    map<unsigned,string>::const_iterator d;
    for(vector<unsigned>::const_iterator itr = dfsList.begin();
        itr != dfsList.end();itr++)
    {
        gate = gates[*itr];
        cout<<'['<<i<<"] ";
        if(gate->gateType == CONST_GATE)
        {
            cout<<"CONST0"<<endl;
            i++;
            continue;
        }
        cout<<left<<setw(4)<<CirGate::gateTypeStr(gate->gateType)
            <<( *itr);
        for(vector<net>::const_iterator ite =gate->fanIn.begin();
            ite != gate->fanIn.end();ite++)
        {
            cout<<' ';
            if(getGate(ite->first)==0)
            {
                cout<<'*';
            }
            else
            {
                if(getGate(ite->first)->gateType == UNDEF_GATE)
                    cout<<'*';
            }
            if(ite->second)
            {
                cout<<'!';
            }
            cout<<ite->first;
        }
        d = nameTable.find(*itr);
        if(d != nameTable.end())
            cout<<' '<<'('<<d->second<<')';
        cout<<endl;
        i++;
    }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(vector<unsigned>::const_iterator itr = PIs.begin();itr != PIs.end();itr++)
   {
       cout<<" "<<(*itr)/2;
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(unsigned i = M+1; i != M+O+1;++i)
   {
       cout<<" "<<i;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
    stringstream ssf;
    for(set<unsigned>::iterator itr = gatesWithFloatFanin.begin();
        itr != gatesWithFloatFanin.end();itr++)
    {
        ssf<<' '<<(*itr);
    }
    if(ssf.tellp() != 0)
    {
        cout<<"Gates with floating fanin(s):";
        cout<<ssf.str()<<endl;
    }
    stringstream ssu;
    for(unsigned i =0 ; i< M+O+1;i++)
    {
        if(gates[i] != 0)
        {
            if(gates[i]->fanOut.size()==0 and gates[i]->gateType == AIG_GATE)
                ssu<<' '<<gates[i]->id;
        }
    }
    if(ssu.tellp() != 0)
    {
        cout<<"Gates defined but not used  :";
        cout<<ssu.str()<<endl;
    }
}

void
CirMgr::printFECPairs() const
{
    if(not simulate)
        return;
    unsigned int count = 0;
    CirGate* gate;
    bool phase,phase2;
    for(grouplist::iterator it = fecGroupList->begin();
        it != fecGroupList->end();it++)
    {
        cout << "[" << count << "] ";
        gate = getGate(*(it->begin()));
        phase = (gate->pattern) %2 ==1;
        for(IdList::iterator it2 = (*it).begin();it2 != it->end();)
        {
            gate = getGate(*it2);
            phase2 = (gate->pattern)  %2 ==1;
            if(phase != phase2)
                cout<<'!';
            cout<< (*it2);
            if(++it2 != it->end())
            {
                cout << " ";
            }
        }
        cout << endl;
        count++;
    }
}

void
CirMgr::writeAag(ostream& outfile) const
{
    outfile<<"aag "<<M<<' '<<I<<' '<<L<<' '<<O<<' ';
    unsigned count = 0;
    for(unsigned i =0; i<M+O+1;i++)
    {
        if(gates[i] != 0)
        {
            if(gates[i]->reachability and gates[i]->gateType == AIG_GATE)
                count++;
        }
    }
    #if DEBUG_MODE
    assert(count>=0);
    #endif
    outfile<<count;
    for(vector<unsigned>::const_iterator itr = PIs.begin();
        itr != PIs.end();itr++ )
    {
        outfile<<endl<<(*itr);
    }
    for(vector<unsigned>::const_iterator itr = POs.begin();
        itr !=  POs.end(); itr++)
    {
        outfile<<endl<<(*itr);
    }
     for(vector<unsigned>::const_iterator ite = dfsList.begin();
        ite !=  dfsList.end(); ite++)
     {
         if(gates[*ite]->gateType==AIG_GATE)
         {
             outfile<<endl<<(gates[(*ite)]->id)*2;
             for(vector<net>::const_iterator itr = gates[(*ite)]->fanIn.begin();
                 itr != gates[(*ite)]->fanIn.end();itr++)
             {
                    outfile<<' '<<get_pin(*itr);
             }
         }

     }

    map<unsigned,string>::const_iterator mitr;
    count =0;
    for(vector<unsigned>::const_iterator itr = PIs.begin();
        itr != PIs.end();itr++)
    {
        mitr = nameTable.find((*itr)/2);
        if(mitr != nameTable.end())
        {
            outfile<<endl<<'i'<<count<<' '<<mitr->second;
            count++;
        }
    }
    count =0;
    for(unsigned i = M+1;i<M+O+1;i++)
    {
        mitr = nameTable.find(i);
        if(mitr != nameTable.end())
        {
            outfile<<endl<<'o'<<count<<' '<<mitr->second;
            count++;
        }
    }
    outfile.flush();
}

void CirMgr::buildfanout()
{
    for(CirGate** ptr= cirGateBegin();ptr != cirGateEnd(); ptr++)
    {
        if(*ptr != 0)
        {
            (*ptr)->fanOut.clear();
        }
    }

    for(unsigned i = 0; i != M+O+1; i++)
    {
        if(gates[i] != 0)
        {
            for(vector<net>::const_iterator itr = gates[i]->fanIn.begin();
                itr != gates[i]->fanIn.end();itr++)
            {
                CirGate* gate = getGate(itr->first);
                if(gate != 0)
                {
                    gate->addFanOut(i,itr->second);
                }
            }

        }
    }
}

void
CirMgr::buildDFSList()
{
    dfsList.clear();
    for(CirGate** ptr = cirGateBegin();ptr != cirGateEnd(); ptr++)
    {
        if((*ptr) == 0)
            continue;
        (*ptr)->reachability = false;

    }
    stack<unsigned> trace;
    unsigned gateID;
    CirGate* gate;
    CirGate* tmp;
    bool store;
    for(unsigned i = M+1;i < M+O+1;i++)
    {
        gateID = i;
        while(true)
        {
            store = true;
            gate = getGate(gateID);
            for(vector<net>::const_iterator itr = gate->fanIn.begin();
                itr != gate->fanIn.end();itr++)
            {
                tmp = getGate(itr->first);

                if(tmp !=0)
                {
                    if(!tmp->reachability and tmp->gateType!=UNDEF_GATE)
                    {
                        trace.push(gateID);
                        gateID = itr->first;
                        store = false;
                        break;
                    }
                }
            }
            if(store)
            {
                gate->reachability = true;
                dfsList.push_back(gateID);
                if(!trace.empty())
                {
                    gateID = trace.top();
                    trace.pop();
                    continue;

                }
                else
                {
                    break;
                }
            }
        }

    }
}


void CirMgr::checkhealth()
{
    cerr<<"==================start checking========================="<<endl;
    CirGate* target;
    bool phase;
    for(CirGate** gate = cirGateBegin(); gate != cirGateEnd(); gate++)
    {
        if(*gate == 0)
            continue;
        for(std::vector<net>::iterator itr = (*gate)->fanIn.begin();
            itr != (*gate)->fanIn.end(); itr++)
        {
            target = getGate(itr->first);
            if(target == 0)
            {
                cerr<<(*gate)->id<<" s fanIn "<<itr->first<<" doesnt exist"<<endl;
                continue;
            }
            phase = itr->second;
            if(std::find(target->fanOut.begin(), target->fanOut.end(), net((*gate)->id, itr->second))
                == target->fanOut.end())
                cerr<<(*gate)->id<<" is not in "<<target->id<<" s fanOut"<<endl;
        }
        for(std::vector<net>::iterator itr = (*gate)->fanOut.begin();
            itr != (*gate)->fanOut.end(); itr++)
        {
            target = getGate(itr->first);
            if(target == 0)
            {
                cerr<<(*gate)->id<<" s fanOut "<<itr->first<<" doesnt exist"<<endl;
                continue;
            }
            phase = itr->second;
            if(std::find(target->fanIn.begin(), target->fanIn.end(), net((*gate)->id, itr->second))
                == target->fanOut.end())
                cerr<<(*gate)->id<<" is not in "<<target->id<<" s fanIn"<<endl;
        }

    }
    cerr<<"========================================================="<<endl;
    return;
}

void CirMgr::removeGate(unsigned gid)
{
    CirGate** ptr  = gates + gid;
    if(*ptr == 0)
        return;
    enum GateType g = (*ptr)->gateType;
    if(g == PI_GATE)
       I--;
    if(g == PO_GATE)
       O--;
    if(g == AIG_GATE)
       A--;

   delete (*ptr);
   (*ptr) = 0;

}
