
#include "ptParameter.h"

#include <cstring>
#include <iostream>

Parameter::Parameter(const parameter& p)
{
    par=new parameter;
    
    par->num=p.num;
    strcpy(par->name,p.name);
    strcpy(par->alias,p.alias);
    strcpy(par->unit,p.unit);
  //  strcpy(par->robsname,p.robsname);
    par->scale=p.scale;
    par->size=p.size;
    par->order=p.order;
    par->datatype=p.datatype;
    par->plottype=p.plottype;
}

Parameter::Parameter(const Parameter& p)
{
    par=new parameter;

    par->num=p.par->num;
    strcpy(par->name,p.par->name);
    strcpy(par->alias,p.par->alias);
    strcpy(par->unit,p.par->unit);
    strcpy(par->robsname,p.par->robsname);
    par->scale=p.par->scale;
    par->size=p.par->size;
    par->order=p.par->order;
    par->datatype=p.par->datatype;
    par->plottype=p.par->plottype;
}

Parameter& Parameter::operator=(const Parameter& p)
{
    if (this!=&p) {
      delete par;
      par=new parameter;

      par->num=p.par->num;
      strcpy(par->name,p.par->name);
      strcpy(par->alias,p.par->alias);
      strcpy(par->unit,p.par->unit);
      strcpy(par->robsname,p.par->robsname);
      par->scale=p.par->scale;
      par->size=p.par->size;
      par->order=p.par->order;
      par->datatype=p.par->datatype;
      par->plottype=p.par->plottype;
    }
    return *this;
}

Parameter& Parameter::operator=(const parameter& p)
{
    delete par;
    par=new parameter;

    par->num=p.num;
    strcpy(par->name,p.name);
    strcpy(par->alias,p.alias);
    strcpy(par->unit,p.unit);
    strcpy(par->robsname,p.robsname);
    par->scale=p.scale;
    par->size=p.size;
    par->order=p.order;
    par->datatype=p.datatype;
    par->plottype=p.plottype;

    return *this;
}

Parameter::~Parameter() {
    delete par;
}

void Parameter::printName() const {
    std::cout << par->name << std::endl;
}
