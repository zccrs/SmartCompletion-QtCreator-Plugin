#include <QCoreApplication>
#include <QFile>
#include <QDebug>

#include "../smartcompletionplugin_global.h"
class CookieJar;
class QNetworkAccessManager;
class Tower : public QObject
{};
int main()
{
    //QCoreApplication a(argv, args);

//    QFile file("/home/zhang/projects/SmartCompletion/test/test.cpp");

//    if(file.open(QIODevice::ReadOnly)) {
//        const QString &str = file.readAll();

//        for(const Global::Block &block : Global::codeToBlocks(str)) {
//            qDebug().noquote() << block;
//            qDebug().noquote().nospace() << '"' + Global::getStrByBlock(str, block) + '"'
//                                         << endl;
//        }

//        qDebug() << Global::getSymbolByString(str, 165);
//    }

    Global::Property property;

    Global::propertyParse("Q_PROPERTY(QList < int < aaa >* > *   test READ getTest WRITE setTest NOTIFY testChanged)", property);

    qDebug() << property;

    //qDebug() << Global::getVaildTypeName("QList::q <  int::a<b>::nn * >::bbb  *      aaa*", 0);//

    return 0;
}
