#include "CDict.h"
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QFile>
#include <QTextStream>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QDebug>
#include <iostream>
#include <string.h>

#define OPEN_DB(DbName) \
    QSqlDatabase DB = QSqlDatabase::addDatabase("QSQLITE");\
    DB.setDatabaseName("..//"+getDbName()+".db");\
    if (!DB.open()){\
        qDebug()<<"Can't open "+getDbName()+".db";\
        return false;\
    }
#define CLOSE_DB() DB.close();\
    QSqlDatabase::removeDatabase(DB.connectionName());

CWord NULL_WORD;

bool CDict::DropTables(EDictTable tables)
{
    bool bResult=true;
    {
        QSqlQuery query(getDB());
        QString Query="drop table ";
        QStringList Tables=this->getTables(tables);
        for (const auto &Table:Tables){
            query.prepare(Query+Table);
            //query.bindValue(":Table",Table);
            if (query.exec()){
                qDebug()<<getDbName()+"."+Table+" was dropped";
            }else{
                bResult&=false;
                qDebug()<<getDbName()+"."+Table+" wasn't dropped";
            }
        }
    }
    return bResult;
}



bool CDict::CreateTables(EDictTable tables)
{
    bool bResult=true;
    {
        QStringList Tables=this->getTables(tables);
        if (Tables.contains("Params")){
            bResult&=CreateTableFromRsc("Params");
        }
        if (Tables.contains("Words")){
            bResult&=CreateTableFromRsc("Words");
        }
        if (Tables.contains("Global")){
            bResult&=CreateVectorTable("Global",false);
        }
        if (Tables.contains("Sense")){
            bResult&=CreateVectorTable("Sense",true);
        }
        if (Tables.contains("Centre")){
            bResult&=CreateVectorTable("Centre",true);
        }
    }
    return bResult;
}

bool CDict::CreateTableFromRsc(const QString &strTableName)
{
    bool bResult=true;
    try{
        QSqlQuery query;
        QFile ifile(":/sql/Rsc/"+strTableName+".sql");
        ifile.open(QIODevice::ReadOnly);
        QTextStream ts(&ifile);
        QString strQuery=ts.readAll();
        query.prepare(strQuery);
        //query.addBindValue(getTable(strTableName));
        //query.bindValue(":Table",strTableName);
        if (query.exec()){
            qDebug()<<"table "+strTableName+" was created";
            bResult=true;
        }
        else{
            qDebug()<<"table "+strTableName+" wasn't' created";
            qDebug()<<query.lastError().text();
            bResult=false;
        }
    }
    catch(...){
        bResult=false;
    }
    return bResult;
}

bool CDict::CreateVectorTable(const QString &strTableName, bool bNeedSensesCount)
{
    bool bResult=true;
    {
        QSqlQuery query(getDB());
        QString strQuery("CREATE TABLE %1 ( "+getField(dtfVector_Word_id)+" INTEGER REFERENCES Words (id) NOT NULL, ");
        if (bNeedSensesCount){
            strQuery.append(""+getField(dtfVector_SenseNum)+" INTEGER NOT NULL, ");
        }
        for (uint column=0;column<getDictParams().m_dim;column++){
            strQuery.append(getField(dtfVector_column).arg(column)+" REAL NOT NULL");
            if (column!=getDictParams().m_dim-1){
                strQuery.append(", ");
            }
        }
        if (bNeedSensesCount){
            strQuery.append(", PRIMARY KEY ("+getField(dtfVector_Word_id)+", "+getField(dtfVector_SenseNum)+")");
        }else{
            strQuery.append(", PRIMARY KEY ("+getField(dtfVector_Word_id)+")");
        }
        strQuery.append(" )");

        query.prepare(strQuery.arg(strTableName));
        //qDebug()<<strQuery;
        //query.bindValue(":Table",strTableName);
        if (query.exec()){
            qDebug()<<"table "+strTableName+" was created";
        }
        else{
            qDebug()<<"table "+strTableName+" wasn't' created";
            qDebug()<<query.lastError();
            bResult&=false;
        }
    }
    return bResult;
}

bool CDict::AddWordtoVectorTable(CDict::EDictTable table,uint minindex, uint maxindex, const vector<vector<valarray<ereal> >> &Values)
{
    Q_ASSERT(Values.size()>0);
    bool bResult=true;
    {
        QSqlTableModel stm(nullptr,getDB());
        stm.setTable(getTable(table));
        stm.setEditStrategy(QSqlTableModel::OnManualSubmit);
        QSqlRecord rec=stm.record();
        for (uint index=minindex;index<maxindex;index++){
            for (uint sense=0;sense<Values[index].size();sense++){
                rec.setValue(getField(dtfVector_Word_id),index);
                if (table!=dtGlobal){
                    rec.setValue(getField(dtfVector_SenseNum),sense);
                }
                for (uint ival=0;ival<getDictParams().m_dim;ival++){
                    rec.setValue(getField(dtfVector_column).arg(ival),Values[index][sense][ival]);
                }
                stm.insertRecord(-1,rec);
                rec.clearValues();
            }
        }
        if(stm.submitAll()){
            //qDebug()<<QString("Word_id = %1 was inserted in ").arg(index)+getTable(table,true);
        } else{
            qDebug()<<QString("Words from %1 to %2 wasn't' inserted in ").arg(minindex).arg(maxindex-1)+getTable(table);
            qDebug()<<stm.lastError();
            bResult&=false;
        }
        //qDebug()<<query.executedQuery();
    }
    return bResult;
}

QString CDict::getTable(CDict::EDictTable table, bool withDbName) const
{
    QString Result;
    switch(table){
        case dtParams: Result="Params"; break;
        case dtWords: Result="Words"; break;
        case dtGlobal: Result="Global"; break;
        case dtSense: Result="Sense"; break;
        case dtCentre: Result="Centre"; break;
        default: return "";
    }
    if (withDbName){
        Result=getDbName()+'.'+Result;
    }
    return Result;
}

QString CDict::getField(CDict::EDictTablesField field,EDictTable table/*=dtWords*/, bool bwithTable/*==false*/) const
{
    QString res;
    switch(field){
        case dtfWords_id: res="id"; break;
        case dtfWords_Word: res="Word"; break;
        case dtfWords_SensesCount: res="SensesCount"; break;
        case dtfVector_Word_id:res="Word_id"; break;
        case dtfVector_SenseNum:res="SenseNum";break;
        case dtfVector_column:res="Column_%1"; break;
        case dtfParams_dim: res="dim";break;
    }
    if (bwithTable){
        res=getTable(table)+'.'+res;
    }
    return res;
}

bool CDict::AddWordstoDB(uint minindex, uint maxindex, const vector<QString> &strWords, const vector<vector<valarray<ereal>> > &globals,
                        const vector<vector<valarray<ereal> >> &senses, const vector<vector<valarray<ereal> > > &centres)
{
    Q_ASSERT(strWords.size()==globals.size());
    Q_ASSERT(globals.size()==senses.size());
    Q_ASSERT(senses.size()==centres.size());
    bool bResult=true;
    {        
        QSqlTableModel tmWords(nullptr,getDB());
        tmWords.setTable(getTable(dtWords));
        tmWords.setEditStrategy(QSqlTableModel::OnManualSubmit);
        QSqlRecord WordsRec=tmWords.record();
        for (uint i=minindex;i<maxindex;i++){
            WordsRec.setValue(getField(dtfWords_id),i);
            WordsRec.setValue(getField(dtfWords_Word),strWords[i-minindex]);
            WordsRec.setValue(getField(dtfWords_SensesCount),senses[i-minindex].size());
            qDebug()<<tmWords.insertRecord(i,WordsRec);
        }
        if (tmWords.submitAll()){
            //qDebug()<<strWord+" was inserted in "+getTable(curTable);
            for (uint i=minindex;i<maxindex;i++){
                m_CachedWords[i]=CSimpleWord(i,strWords[i-minindex],senses[i-minindex].size());
            }
        }else{
            qDebug()<<QString("Words from %1 to %2 wasn't' inserted in ").arg(strWords[0]).arg(strWords[maxindex-1-minindex])+getTable(dtWords);
            bResult&=false;
        }
        //query.clear();
    }
    bResult&=AddWordtoVectorTable(dtGlobal,minindex,maxindex,globals);
    bResult&=AddWordtoVectorTable(dtSense,minindex,maxindex,senses);
    bResult&=AddWordtoVectorTable(dtCentre,minindex,maxindex,centres);
    return bResult;
}

bool CDict::AddDictParamstoDB(const CDictParams &obj)
{
//    QSqlQuery query(getDB());
//    QString strQuery("insert into %1 values(:dim)");
//    strQuery=strQuery.arg(getTable(dtParams));
//    query.prepare(strQuery);
//    query.bindValue(":dim",obj.m_dim);
//    if(query.exec()){

//    }
    QSqlTableModel qtm;
    qtm.setTable(getTable(dtParams));
    //qtm.select();
    QSqlRecord rec=qtm.record();
    rec.setValue(getField(dtfParams_dim),obj.m_dim);
    qDebug()<<qtm.insertRecord(0,rec);
    return qtm.submitAll();
}

CDictParams CDict::getDictParams(bool fromDb/*=false*/) const
{
    if (fromDb){
        CDictParams Result;
        {
            QSqlQuery query(getDB());
            if (!query.exec("select * from Params")){
                Result=CDictParams();
            }
            query.next();
            QSqlRecord rec(query.record());
            Result.m_dim=query.value(rec.indexOf("dim")).toInt();
        }
        return Result;
    }else{
        return CDictParams(m_dim);
    }
}

bool CDict::OpenDB()
{
    m_DB = QSqlDatabase::addDatabase("QSQLITE");
    m_DB.setDatabaseName("..//"+getDbName()+".db");
    if (!m_DB.open()){
        qDebug()<<"Can't open "+getDbName()+".db";
        return false;
    }
    return true;
}

const QSqlDatabase &CDict::getDB() const
{
    return m_DB;
}

bool CDict::CloseDB()
{
    m_DB.close();
    QSqlDatabase::removeDatabase(m_DB.connectionName());
    return !m_DB.isOpen();
}

QStringList CDict::getTables(CDict::EDictTable tables)
{
    QStringList Result;
    if ((tables & EDictTable::dtParams)!=0)
        Result.append("Params");
    if ((tables & EDictTable::dtWords)!=0)
        Result.append("Words");
    if ((tables & EDictTable::dtGlobal)!=0)
        Result.append("Global");
    if ((tables & EDictTable::dtSense)!=0)
        Result.append("Sense");
    if ((tables & EDictTable::dtCentre)!=0)
        Result.append("Centre");
    return Result;
}

void CDict::setDictParams(const CDictParams &obj)
{
    m_dim=obj.m_dim;
}

CDict::CDict(QString strDbName):m_strDbName(strDbName),m_dim(0),m_size(0),m_BlockLenght(10000)
{

}

CDict::CDict():CDict("")
{

}

void CDict::setDbName(const QString &strDbName)
{
    m_strDbName=strDbName;
}

QString CDict::getDbName() const
{
    return m_strDbName;
}

uint CDict::getBlockLenght() const
{
    return min(m_BlockLenght,m_size);
}

void CDict::setBlockLenght(uint Blocklenght)
{
    m_BlockLenght=Blocklenght;
}

bool CDict::ClearDb()
{
    return false;
}

bool CDict::FillDb(const QString &strFileName, bool bFromBinary/*=false*/)
{
    if (getDbName().isEmpty())
        return false;
    if (bFromBinary){
        return false;
    }
    else
    {
        bool bResult=true;
        QFile file(strFileName);
        file.open(QIODevice::ReadOnly);
        QTextStream ts(&file);
        ts>>m_size>>m_dim;
        //if (m_dim!=getDictParams(true))
        {
            EDictTable tables=dtAll;
            DropTables(tables);
            CreateTables(tables);
        }
        vector<QString> words(getBlockLenght());
        vector<uint> scount(getBlockLenght());
        vector<vector<valarray<ereal>>> senses(getBlockLenght()), centres(getBlockLenght()), globals(getBlockLenght());
        for (uint i=0;i<m_size;){
            uint BlockLenght=min(getBlockLenght(),m_size-i);
            words.resize(BlockLenght);
            scount.resize(BlockLenght);
            globals.resize(BlockLenght);
            senses.resize(BlockLenght);
            centres.resize(BlockLenght);
            for (uint j=0;j<BlockLenght;j++){
                ts>>words[j]>>scount[j];
                senses[j].resize(scount[j]);
                centres[j].resize(scount[j]);
                globals[j].resize(1);
                globals[j][0].resize(m_dim);
                for (uint k=0;k<m_dim;k++){
                    ts>>globals[j][0][k];
                }
                for(uint isense=0;isense<scount[j];isense++){
                    senses[j][isense].resize(m_dim);
                    centres[j][isense].resize(m_dim);
                    for (uint k=0;k<m_dim;k++){
                        ts>>senses[j][isense][k];
                    }
                    for (uint k=0;k<m_dim;k++){
                        ts>>centres[j][isense][k];
                    }
                }
            }
            bResult&=AddWordstoDB(i,i+getBlockLenght(),words,globals,senses,centres);
            i+=BlockLenght;
            QTextStream(stdout)<<i*1.0/m_size<<"  of words inserted";
            if (i!=m_size-1)
                QTextStream(stdout)<<"\r";
            else
                QTextStream(stdout)<<"\n";
        }
        AddDictParamstoDB(getDictParams());
        return bResult;
    }
}

bool CDict::LoadCache()
{
    {
        QSqlTableModel stm(nullptr,getDB());
        stm.setTable(getTable(dtWords));
        stm.select();
        QSqlRecord rec=stm.record();
        uint id,str,scount;
        id=rec.indexOf(getField(dtfWords_id));
        str=rec.indexOf(getField(dtfWords_Word));
        scount=rec.indexOf(getField(dtfWords_SensesCount));
        for (int row=0;row<stm.rowCount();row++){
            rec=stm.record(row);
            m_CachedWords[rec.value(id).toInt()]=CSimpleWord(rec.value(id).toInt(),rec.value(str).toString(),rec.value(scount).toInt());
        }
    }
    {
        setDictParams(getDictParams(true));
    }
    return true;
}


CWord &CDict::getNewWordObj(uint index) const
{
    return m_CachedWordObjs[index];
}

const CWord &CDict::getWordfromDB(const QString &strWord, bool bUpdateCache/*=true*/) const
{
    QSqlTableModel stm(nullptr,getDB());
    stm.setTable(getTable(dtWords));
    stm.setFilter(getField(dtfWords_Word)+"='"+strWord+"'");
    if(stm.select()){
        QSqlRecord rec=stm.record(0);
        uint index=rec.value(getField(dtfWords_id)).toInt();
        uint scount=rec.value(getField(dtfWords_SensesCount)).toInt();
        CWord &obj=getNewWordObj(index);
        obj.m_index=index;
        obj.m_scount=scount;
        obj.m_str=strWord;
        getWordVectorfromDB(obj,dtGlobal);
        getWordVectorfromDB(obj,dtSense);
        getWordVectorfromDB(obj,dtCentre);
        return obj;
    }else
        return NULL_WORD;
}

void CDict::getWordVectorfromDB(CWord &obj, CDict::EDictTable table) const
{
    Q_ASSERT(table == dtGlobal || table == dtCentre || table == dtSense);
    QSqlTableModel stm(nullptr,getDB());
    stm.setTable(getTable(table));
    stm.setFilter(getField(dtfVector_Word_id)+"="+QString::number(obj.m_index));
    stm.select();
    vector<valarray<ereal>> values;
    valarray<ereal> value;
    uint dim=getDictParams().m_dim;
    value.resize(dim);
    values.resize(obj.m_scount);
    QSqlRecord rec;
    for (int row=0;row<stm.rowCount();row++){
        rec=stm.record(row);
        value.resize(dim);
        for (uint ival=0;ival<dim;ival++){
            value[ival]=rec.value(getField(dtfVector_column).arg(ival)).toFloat();
        }
        if (table==dtGlobal){
            obj.m_global=std::move(value);
        }else{
            values[rec.value(getField(dtfVector_SenseNum)).toInt()]=std::move(value);
        }
    }
    if (table==dtSense){
        obj.m_senses=std::move(values);
    }else if(table==dtCentre){
        obj.m_centres=std::move(values);
    }
}

CRefWord CDict::getWord(uint index, bool *ok) const
{
    auto it=m_CachedWordObjs.find(index);
    if (it!=m_CachedWordObjs.cend()){
        return CRefWord(it->second);
    }
    const CWord &obj=getWordfromDB(m_CachedWords.at(index).m_str);
    if (obj==NULL_WORD && ok)
        *ok=false;
    if (obj!=NULL_WORD && ok)
        *ok=true;
    return CRefWord(obj);
}

CRefWord CDict::getWord(const QString &Word, bool *ok) const
{
    using namespace std::placeholders;
    auto index=std::find_if(m_CachedWords.begin(),m_CachedWords.end(),
                            [Word](const pair<uint,CSimpleWord> &pair)->bool {
            return (pair.second.m_str.compare(Word,Qt::CaseInsensitive)==0);
            });
    if (index!=m_CachedWords.end()){
        return getWord(index->first,ok);
    }else{
        if (ok) *ok=false;
        return CRefWord();
    }
}
