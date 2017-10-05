#include <QString>
#include <QTextStream>
#include <QtSql>
#include <QSqlQuery>
#include <QDebug>
#include <iostream>
#include "CDict.h"

int main(int argc, char *argv[])
{
    const QString vecs="myvecs";//"vecs50_MSSG";
    CDict dict(vecs);
    dict.setBlockLenght(10);
    dict.OpenDB();
//    QSqlRelationalTableModel rtm(nullptr, dict.getDB());
//    rtm.setTable("Global");
//    rtm.setRelation(0,QSqlRelation("Words","id","Word_id"));
//    for (int i=0;i<rtm.record().count();i++)
//        qDebug()<<rtm.record().fieldName(i);
//    return 0;
//    QSqlTableModel tmWords(nullptr,dict.getDB());
//    tmWords.setTable("Words");
//    tmWords.setEditStrategy(QSqlTableModel::OnManualSubmit);
//    QSqlRecord WordsRec=tmWords.record();
//    qDebug()<<WordsRec.fieldName(0);
//    qDebug()<<WordsRec.fieldName(1);
//    qDebug()<<WordsRec.fieldName(2);
//    dict.FillDb(QString("..//%1.txt").arg(vecs));
//    dict.CloseDB();
//    return 0;
    dict.LoadCache();
    QString str;
    QTextStream(stdin)>>str;
    //qDebug()<<(str=="Anton");
    auto w=dict.getWord(str);
    qDebug()<<w.getStr()<<endl;
}
