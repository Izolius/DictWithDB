#include "CWord.h"

CWord::CWord():m_str(""),m_senses(0), m_refs(0), m_index(-1)
{

}

CWord::CWord(const QString &str, const valarray<ereal> &global,const vector<valarray<ereal>> &senses,const vector<valarray<ereal>> &centres):
    m_str(str),m_global(global), m_senses(senses), m_centres(centres), m_refs(0)
{
    Q_ASSERT(senses.size()==centres.size());
    Q_ASSERT(senses.size()>0);
    Q_ASSERT(senses[0].size()==global.size());
    Q_ASSERT(senses[0].size()==centres[0].size());
}

CRefWord CWord::getRef() const
{
    return CRefWord(this);
}

bool CWord::operator ==(const CWord &obj) const
{
    return m_index==obj.m_index;
}

bool CWord::operator !=(const CWord &obj) const
{
    return m_index!=obj.m_index;
}

const valarray<ereal> &CWord::Global() const
{
    return m_global;
}

const vector<valarray<ereal> > &CWord::Senses() const
{
    return m_senses;
}

const vector<valarray<ereal> > &CWord::Centres() const
{
    return m_centres;
}

CRefWord::CRefWord(const CWord &obj):
    CRefWord(obj.m_str.leftRef(-1),obj.m_index,obj.m_global,obj.m_senses,obj.m_centres)
{
    m_RealObj=&obj;
    obj.m_refs++;
}

CRefWord::CRefWord(const CWord *obj):
    CRefWord(*obj)
{

}

CRefWord::CRefWord():CRefWord(new CWord())
{
    m_bSelfCreatedRealObj=true;
}

CRefWord::~CRefWord()
{
    if (m_bSelfCreatedRealObj){
        delete m_RealObj;
        return;
    }
    if (m_RealObj)
        m_RealObj->m_refs--;
}

CRefWord::CRefWord(const QStringRef &str,uint index, const valarray<ereal> &global,const vector<valarray<ereal>> &senses,const vector<valarray<ereal>> &centres):
    m_str(str),m_global(global), m_senses(senses), m_centres(centres),m_RealObj(nullptr), m_index(index), m_bSelfCreatedRealObj(false)
{
    Q_ASSERT(senses.size()==centres.size());
    //Q_ASSERT(senses.size()>0);
    //Q_ASSERT(senses[0].size()==global.size());
    //Q_ASSERT(senses[0].size()==centres[0].size());
}

const QStringRef &CRefWord::getStr() const
{
    return m_str;
}

const valarray<ereal> &CRefWord::Global() const
{
    return m_global;
}

const vector<valarray<ereal> > &CRefWord::Senses() const
{
    return m_senses;
}

const vector<valarray<ereal> > &CRefWord::Centres() const
{
    return m_centres;
}
