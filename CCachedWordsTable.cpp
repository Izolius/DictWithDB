#include "CCachedWordsTable.h"
#include "CDict.h"

CCachedWordsTable::CCachedWordsTable()
{

}

bool CCachedWordsTable::init()
{
    setTable(CDict::dtWords);
    select();
}
