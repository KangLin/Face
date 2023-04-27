#include "Database.h"
#include "RabbitCommonDir.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QFile>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QVariant>

#include <stdexcept>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logDB, "DB")

CDatabase::CDatabase(QObject *parent)
    : QObject(parent)
{
    m_szFile = RabbitCommon::CDir::Instance()->GetDirUserDatabaseFile();
    if(InitDatabase())
    {
        throw std::runtime_error("Init database fail");
    }
}

CDatabase::~CDatabase()
{
    m_Database.close();
}

int CDatabase::InitDatabase()
{
    m_Database = QSqlDatabase::addDatabase("QSQLITE");
    m_Database.setDatabaseName(m_szFile);
    QDir d;
    if(!d.exists(m_szFile))
    {
        if(!m_Database.open())
        {
            qCritical(logDB) << "Open database fail:"
                             << m_Database.lastError().text()
                             << m_szFile;
            return m_Database.lastError().nativeErrorCode().toInt();
        }
#if defined (_DEBUG) || defined(DEBUG)
        QFile file(":/database/database");
#else
        QFile file(RabbitCommon::CDir::Instance()->GetDirDatabase()
                   + QDir::separator() + "database.sql");
#endif
        if(file.open(QFile::ReadOnly))
        {
            QSqlQuery query(m_Database);
            QString szSql(file.readAll());
            QStringList sql = szSql.split(";");
            for(int i = 0; i < sql.size(); i++)
            {
                qCritical(logDB) << "sql:" << sql[i];
                if(!query.exec(sql[i]) && m_Database.lastError().type() != QSqlError::NoError)
                {
                    qCritical(logDB) << "Create database fail:"
                                     << m_Database.lastError().text();
                    file.close();
                    m_Database.close();
                    QDir d;
                    d.remove(m_szFile);
                    return -1;
                }
            }
            file.close();
        }
        m_Database.close();
    }

    if(!m_Database.open())
    {
        qCritical(logDB) << "Open database fail:"
                   << m_Database.lastError().text();
        return -2;
    }

    m_TableRegister.SetDatabase(m_Database);
    return 0;
}

CTableRegister* CDatabase::GetTableRegister()
{
    return &m_TableRegister;
}

QString CDatabase::getDbFile()
{
    return m_szFile;
}

int CDatabase::setDbFile(const QString &szFile)
{
    m_szFile = szFile;
    return 0;
}
