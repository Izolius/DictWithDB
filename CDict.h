#ifndef CDICT_H
#define CDICT_H

#include <QString>
#include <valarray>
#include <functional>
#include <vector>
#include <QSqlDatabase>
#include <list>
#include "CWord.h"
using namespace std;

struct CDictParams{
    uint m_dim;
    CDictParams(uint dim):m_dim(dim){}
    CDictParams():CDictParams(-1){}
};

class CDict
{
    enum EDictTable{
        dtParams=0x1,
        dtWords=0x2,
        dtGlobal=0x4,
        dtSense=0x8,
        dtCentre=0x10,
        dtAll=dtParams|dtWords|dtGlobal|dtSense|dtCentre
    };

    static struct SdtWordsFields{
        const QString id="id";
        const QString strWord="Word";
        const QString sensesCount="SensesCount";
    } dtWordsFields;
    static struct StdGlobalFields{
        const QString word_id="Word_id";
        const QString column="Column_%1";
    } dtGlobalFields;
    static struct StdSenseFileds{
        const StdGlobalFields GlobalFields;
        const QString senseNum="SenseNum";
    } dtSenseFields;

    QString m_strDbName;
    uint m_dim, m_size;
    uint m_BlockLenght;
    QSqlDatabase m_DB;
    mutable list<CWord> m_CachedWords;
    bool DropTables(EDictTable tables);

    bool CreateTables(EDictTable tables);
    bool CreateTableFromRsc(const QString &strTableName);
    bool CreateVectorTable(const QString &strTableName, bool bNeedSensesCount);
    bool AddWordtoVectorTable(EDictTable table, uint minindex, uint maxindex, const vector<vector<valarray<ereal> > > &Values);

    bool AddWordstoDB(uint minindex, uint index, const vector<QString> &strWords, const vector<vector<valarray<ereal> > > &globals,
                     const vector<vector<valarray<ereal> > > &senses, const vector<vector<valarray<ereal>> > &centres);

    const CWord &getWordfromDB(const QString &strWord, bool bUpdateCache=true) const;

    QString getTable(EDictTable table, bool withDbName=false) const;
    QString getTable(const QString &strTableName) const;
    static QStringList getTables(EDictTable tables);
public:
    CDict(QString strDbName);
    CDict();
    void setDbName(const QString &strDbName);
    QString getDbName() const;
    uint getBlockLenght() const;
    void setBlockLenght(uint Blocklenght);
    CDictParams getDictParams(bool fromDb=false) const;
    bool OpenDB();
    const QSqlDatabase &getDB() const;
    bool CloseDB();
    bool ClearDb();
    bool FillDb(const QString &strFileName, bool bFromBinary=false);

    CRefWord getWord() const;
};

#endif // CDICT_H
