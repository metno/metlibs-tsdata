
#include "DynamicFunction.h"
#include <iostream>
#include <math.h>
#include <stdlib.h>

using namespace std;

namespace pets {
namespace math {


DynamicFunction::DynamicFunction(string nf, double f)
    : factor(f)
    , next(0)
{
  txt=nf;
  if(!nf.empty()) {

    int pos     = nf.find_first_of("+-*/",0);
    int nextpos = nf.find_first_of("+-*/",pos+1);

    if(pos >= 0 && pos < nf.length()) {

      char   opr = nf[pos];
      string nxt = "";
      int    end = nf.length();;

      if(nextpos > 0 && nextpos < nf.length()) {
        end = nextpos;
        nxt = nf.substr(nextpos,nf.length()-1);
      }

      double fac = atof(nf.substr(pos+1,end).c_str() );

      if( opr=='*' ) next = new Multiply(  nxt, fac );
      if( opr=='/' ) next = new Divide(    nxt, fac );
      if( opr=='+' ) next = new Add(       nxt, fac );
      if( opr=='-' ) next = new Substract( nxt, fac );
    }
  }
}



void DynamicFunction::calc(double& res)
{
  if(next)
    next->calc(res);
}


void Add::calc(double& res)
{
  res+=factor;
  if(next)
    next->calc(res);
}


void Divide::calc(double& res)
{
  res/=factor;
  if(next)
    next->calc(res);
}


void Multiply::calc(double& res)
{
  res*=factor;
  if(next)
    next->calc(res);
}


void Substract::calc(double& res)
{
  res-=factor;
  if(next)
    next->calc(res);
}


}
};





