#ifndef CDICT_H
#define CDICT_H

#include <QString>
#include <valarray>
#include <functional>
#include <vector>
#include <QSqlDatabase>
#include <list>
#include "CWord.h"
#include <map>
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

    enum EDictTablesField{
        dtfWords_id,
        dtfWords_Word,
        dtfWords_SensesCount,
        dtfVector_Word_id,
        dtfVector_SenseNum,
        dtfVector_column
    };

    QString m_strDbName;
    uint m_dim, m_size;
    uint m_BlockLenght;
    QSqlDatabase m_DB;
    mutable map<uint, QString> m_CachedWords;
    mutable map<uint, CWord> m_CachedWordObjs;
    bool DropTables(EDictTable tables);

    bool CreateTables(EDictTable tables);
    bool CreateTableFromRsc(const QString &strTableName);
    bool CreateVectorTable(const QString &strTableName, bool bNeedSensesCount);
    bool AddWordtoVectorTable(EDictTable table, uint minindex, uint maxindex, const vector<vector<valarray<ereal> > > &Values);

    bool AddWordstoDB(uint minindex, uint index, const vector<QString> &strWords, const vector<vector<valarray<ereal> > > &globals,
                     const vector<vector<valarray<ereal> > > &senses, const vector<vector<valarray<ereal>> > &centres);

    CWord &getNewWordObj(uint index) const;


    const CWord &getWordfromDB(const QString &strWord, bool bUpdateCache=true) const;
    void getWordVectorfromDB(CWord &obj, EDictTable table) const;

    QString getTable(EDictTable table, bool withDbName=false) const;
    QString getField(EDictTablesField field,EDictTable table=dtWords, bool bwithTable=false) const;
    //QString getTable(const QString &strTableName) const;
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

    CRefWord getWord(uint index, bool *ok=nullptr) const;
    CRefWord getWord(const QString &Word, bool *ok) const;
};

#endif // CDICT_H
