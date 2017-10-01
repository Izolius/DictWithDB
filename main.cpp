#include <QString>
#include <QTextStream>
#include <QtSql>
#include <QSqlQuery>
#include <QDebug>
#include "CDict.h"

int main(int argc, char *argv[])
{
    QTextStream(stdout);
    const QString vecs="myvecs";//"vecs50_MSSG";
    CDict dict(vecs);
    dict.setBlockLenght(10);
    dict.OpenDB();
    qDebug()<<dict.FillDb(QString("..//%1.txt").arg(vecs));
    dict.CloseDB();
}
