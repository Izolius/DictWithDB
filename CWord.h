#ifndef CWORD_H
#define CWORD_H

#include <QString>
#include <QStringRef>
#include <vector>
#include <valarray>
using namespace std;


typedef float ereal;
#define USE_IWORD 0
#if (USE_IWORD>0)
    #define OVERRIDE(method) virtual method override
#else
    #define OVERRIDE(method) method
#endif

class IWord
{
    //virtual const valarray<ereal> &Global() const =0;
    virtual const vector<valarray<ereal> > &Senses() const =0;
    virtual const vector<valarray<ereal> > &Centres() const =0;
};
class CRefWord;
class CWord :IWord
{
    friend class CRefWord;
    friend class CDict;
    mutable uint m_refs;
    QString m_str;
    uint m_scount;
    int m_index;
    valarray<ereal> m_global;
    vector<valarray<ereal>> m_senses;
    vector<valarray<ereal>> m_centres;
public:
    CWord();
    CWord(const QString &str, const valarray<ereal> &global,const vector<valarray<ereal>> &senses,const vector<valarray<ereal>> &centres);
    OVERRIDE(const valarray<ereal> &Global() const);
    OVERRIDE(const vector<valarray<ereal> > &Senses() const);
    OVERRIDE(const vector<valarray<ereal> > &Centres() const);
    CRefWord getRef() const;

    bool operator ==(const CWord &obj) const;
    bool operator !=(const CWord &obj) const;
};

class CRefWord :IWord
{
    const CWord *m_RealObj;
    QStringRef m_str;
    uint m_scount, m_index;
    bool m_bSelfCreatedRealObj;
    const valarray<ereal> &m_global;
    const vector<valarray<ereal>> &m_senses;
    const vector<valarray<ereal>> &m_centres;
public:
    //CRefWord() = delete;
    CRefWord(const CWord &obj);
    CRefWord(const CWord *obj);
    CRefWord();
    ~CRefWord();
    CRefWord(const QStringRef &str, uint index, const valarray<ereal> &global,const vector<valarray<ereal>> &senses,const vector<valarray<ereal>> &centres);
    OVERRIDE(const valarray<ereal> &Global() const );
    OVERRIDE(const vector<valarray<ereal> > &Senses() const );
    OVERRIDE(const vector<valarray<ereal> > &Centres() const );
    OVERRIDE(const QStringRef &getStr() const);
};

#endif // CWORD_H
