
#include "ptError.h"

bool setErrorFlag(bool ok, ErrorFlag* ef)
{
  if (ef) {
    *ef = ok ? OK : UNKNOWN_ERROR;
  }
  return ok;
}
