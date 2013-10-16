//qtRemoteSignals auto generated file
//do not edit this file; if needed subclass it or create a decorator
#include "TestDataServer.h"

TestDataServer::TestDataServer(QIODevice* readDevice, QIODevice* writeDevice, bool initialize)
	: RemoteSignals(Q_UINT64_C(0xe7a9f0687899fa6),Q_UINT64_C(0x44a1ab36a7026130), readDevice, writeDevice, initialize)
{}

void TestDataServer::echoAnswer(const QString& text)
{
	QByteArray msgData;
	QDataStream stream(&msgData, QIODevice::WriteOnly);
	stream << RemoteSignals::version() << RemoteSignals::gid1() << RemoteSignals::gid2() << (int)4;
	stream << text;
	transmitSignal(msgData);
}

void TestDataServer::processRemoteInputs(const QByteArray& data)
{
	QDataStream stream(data);
	int remoteSignalVersion = 0;
	quint64 remoteSignalGid1 = 0;
	quint64 remoteSignalGid2 = 0;
	int remoteSignalMethodId = 0;
	stream >> remoteSignalVersion >> remoteSignalGid1 >> remoteSignalGid2 >> remoteSignalMethodId;
	RemoteSignals::checkId(remoteSignalVersion, remoteSignalGid1, remoteSignalGid2);

	if(remoteSignalMethodId == 1) {
		QString data;
		stream >> data;
		emit logMessage(data);
		return;
	}
	if(remoteSignalMethodId == 2) {
		QString data;
		stream >> data;
		emit logError(data);
		return;
	}
	if(remoteSignalMethodId == 3) {
		QString text;
		stream >> text;
		emit echo(text);
		return;
	}

	RemoteSignals::handleError(remoteSignalMethodId);
}
