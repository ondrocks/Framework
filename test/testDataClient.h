//qtRemoteSignals auto generated file
//do not edit this file; if needed subclass it or create a decorator
//md5 hash: e7a9f0687899fa644a1ab36a7026130
#ifndef TESTDATACLIENT_H
#define TESTDATACLIENT_H

#include "RemoteSignals.h"

class TestDataClient : public RemoteSignals
{
	Q_OBJECT
public:
	TestDataClient(QIODevice* writeDevice, QIODevice* readDevice = 0, bool initialize = false);
signals:
	void echoAnswer(const QString& text);

public slots:
	void logMessage(const QString& data);
	void logError(const QString& data);
	void echo(const QString& text);

private:
	void processRemoteInputs(const QByteArray& data);
};

#endif //TESTDATACLIENT_H
