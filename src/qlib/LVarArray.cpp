#include <common.h>

#include "LVarArray.hpp"

using namespace qlib;

LVarArray::~LVarArray()
{
  if (m_pData!=NULL)
    delete [] m_pData;
}

void LVarArray::copyFrom(const LVarArray &a)
{
  MB_ASSERT(false);
}

