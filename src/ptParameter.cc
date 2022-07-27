#include "ptParameter.h"

#include <iostream>

#define MILOGGER_CATEGORY "metlibs.tsdata.Parameter"
#include <miLogger/miLogging.h>

std::string Parameter::alias() const
{
  METLIBS_LOG_WARN("deprecated");
  return alias_;
}

std::string Parameter::name() const
{
  METLIBS_LOG_WARN("deprecated");
  return name_;
}

std::string Parameter::unit() const
{
  METLIBS_LOG_WARN("deprecated");
  return unit_;
}

int Parameter::num() const
{
  METLIBS_LOG_WARN("deprecated");
  return num_;
}

int Parameter::scale() const
{
  METLIBS_LOG_WARN("deprecated");
  return scale_;
}

int Parameter::size() const
{
  METLIBS_LOG_WARN("deprecated");
  return size_;
}

int Parameter::order() const
{
  METLIBS_LOG_WARN("deprecated");
  return order_;
}

int Parameter::datatype() const
{
  METLIBS_LOG_WARN("deprecated");
  return datatype_;
}

int Parameter::plottype() const
{
  METLIBS_LOG_WARN("deprecated");
  return plottype_;
}

void Parameter::printName() const
{
  METLIBS_LOG_WARN("deprecated");
  std::cout << name_ << std::endl;
}
