#include "CDict.h"
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QFile>
#include <QTextStream>
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
            qDebug()<<"table "+getTable(strTableName)+" was created";
            bResult=true;
        }
        else{
            qDebug()<<"table "+getTable(strTableName)+" wasn't' created";
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
        QString strQuery("CREATE TABLE %1 ( Word_id INTEGER REFERENCES Words (id) NOT NULL, ");
        if (bNeedSensesCount){
            strQuery.append("SensesNum INTEGER NOT NULL, ");
        }
        for (uint column=0;column<getDictParams().m_dim;column++){
            strQuery.append("Column_"+QString::number(column)+" REAL NOT NULL");
            if (column!=getDictParams().m_dim-1){
                strQuery.append(", ");
            }
        }
        if (bNeedSensesCount){
            strQuery.append(", PRIMARY KEY (Word_id, SensesNum)");
        }else{
            strQuery.append(", PRIMARY KEY (Word_id)");
        }
        strQuery.append(" )");

        query.prepare(strQuery.arg(strTableName));
        //qDebug()<<strQuery;
        //query.bindValue(":Table",strTableName);
        if (query.exec()){
            qDebug()<<"table "+getTable(strTableName)+" was created";
        }
        else{
            qDebug()<<"table "+getTable(strTableName)+" wasn't' created";
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
    bool bNeedSenseNum=table==dtSense || table==dtCentre;
    {
        QSqlQuery query(getDB());
        query.clear();
        QString strQuery("INSERT INTO %1 VALUES");
        strQuery=strQuery.arg(getTable(table));
        for (uint index=minindex;index<maxindex;index++){
            for (uint i=0;i<Values[index-minindex].size();i++){
                strQuery.append(QString("(:Word_id_%1,").arg(index));
                if (bNeedSenseNum){
                    strQuery.append(QString(":SenseNum_%3_%4, ").arg(index).arg(i));
                }
                for (uint column=0;column<getDictParams().m_dim;column++){
                    strQuery.append(QString(":column_%5_%6").arg(index).arg(column));
                    if (column!=getDictParams().m_dim-1)
                        strQuery.append(',');
                }
                strQuery.append(")");
                if (i!=Values[index-minindex].size()-1)
                    strQuery.append(", ");;
            }
            if (index!=maxindex-1)
                strQuery.append(',');
        }
        //qDebug()<<strQuery;
        query.prepare(strQuery);
        for (uint index=minindex;index<maxindex;index++){
            for (uint i=0;i<Values[index-minindex].size();i++){
                query.bindValue(QString("(:Word_id_%1").arg(index),index);
                if (bNeedSenseNum)
                    query.bindValue(QString(":SenseNum_%3_%4").arg(index).arg(i),i);
                for (uint column=0;column<getDictParams().m_dim;column++){
                    query.bindValue(QString(":column_%5_%6").arg(index).arg(column),Values[index-minindex][i][column]);
                }
            }
        }

        if(query.exec()){
            //qDebug()<<QString("Word_id = %1 was inserted in ").arg(index)+getTable(table,true);
        } else{
            qDebug()<<QString("Words from %1 to %2 wasn't' inserted in ").arg(minindex).arg(maxindex-1)+getTable(table);
            qDebug()<<query.lastError();
            bResult&=false;
        }
    }
    return bResult;
}

QString CDict::getTable(CDict::EDictTable table, bool withDbName)
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

bool CDict::AddWordstoDB(uint minindex, uint maxindex, const vector<QString> &strWords, const vector<vector<valarray<ereal>> > &globals,
                        const vector<vector<valarray<ereal> >> &senses, const vector<vector<valarray<ereal> > > &centres)
{
    Q_ASSERT(strWords.size()==globals.size());
    Q_ASSERT(globals.size()==senses.size());
    Q_ASSERT(senses.size()==centres.size());
    bool bResult=true;
    {
        QString curTable(getTable(dtWords));
        QSqlQuery query(getDB());
        QString strQuery("INSERT INTO %1(id,Word,SensesCount) VALUES ");
        for (uint i=minindex;i<maxindex;i++){
            strQuery.append(QString("(:id_%1,:Word_%2,:SensesCount_%3)").arg(i).arg(i).arg(i));
            if (i!=maxindex-1)
                strQuery.append(", ");
        }
        query.prepare(strQuery.arg(curTable));
        for (uint i=minindex;i<maxindex;i++){
            query.bindValue(QString(":id_%1").arg(i),i);
            query.bindValue(QString(":Word_%1").arg(i),strWords[i-minindex]);
            query.bindValue(QString(":SensesCount_%1").arg(i),uint(senses[i-minindex].size()));
        }

        if (query.exec()){
            //qDebug()<<strWord+" was inserted in "+getTable(curTable);
        }else{
            qDebug()<<QString("Words from %1 to %2 wasn't' inserted in ").arg(strWords[0]).arg(strWords[maxindex-1-minindex])+getTable(curTable);
            qDebug()<<query.lastError();
            bResult&=false;
        }
        query.clear();

        bResult&=AddWordtoVectorTable(dtGlobal,minindex,maxindex,globals);
        bResult&=AddWordtoVectorTable(dtSense,minindex,maxindex,senses);
        bResult&=AddWordtoVectorTable(dtCentre,minindex,maxindex,centres);
    }
    return bResult;
}

CDictParams CDict::getDictParams(bool fromDb/*=false*/) const
{
    if (fromDb){
        CDictParams Result;
        {
            QSqlQuery query(getDB());
            if (!query.exec("select * from "+getTable("Params"))){
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

QString CDict::getTable(const QString &strTableName) const
{
    return getDbName()+"."+strTableName;
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
            QTextStream(stdout)<<i*1.0/m_size<<"  of words inserted\r";
        }
        return bResult;
    }
}