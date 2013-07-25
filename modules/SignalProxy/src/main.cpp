// OFFIS Automation Framework
// Copyright (C) 2013 OFFIS e.V.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http:#www.gnu.org/licenses/>.

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QUuid>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>

struct Parameter
{
    QString name;
    QString type;
    QString signature() const
    {
        return QString("%1 %2").arg(type, name);
    }
};

struct Method
{
    QString signature() const
    {
        QString sig;
        sig += name;
        sig += "(";
        QStringList paramSigs;
        foreach(const Parameter& param, params)
            paramSigs << param.signature();
        sig += paramSigs.join(", ");
        sig += ")";
        return sig;
    }
    QStringList paramNames() const {
        QStringList names;
        foreach(const Parameter& param, params)
            names << param.name;
        return names;
    }

    QUuid id;
    bool isReverse;
    QString name;
    QList<Parameter> params;
};

Method parseLine(const QString& line)
{
    Method method;
    method.id = QUuid::createUuid();
    int start = line.indexOf('(');
    int end = line.indexOf(')');
    method.name = line.mid(0, start);
    method.isReverse = false;
    if(method.name.startsWith('-'))
    {
        method.isReverse = true;
        method.name.remove(0,1);
    }
    QString paramsLine = line.mid(start+1, end-start-1);
    QStringList params = paramsLine.split(",");
    foreach(QString param, params)
    {
        Parameter paramType;
        param = param.trimmed();
        int split = param.indexOf(' ');
        paramType.name = param.mid(split+1).trimmed();
        paramType.type = param.mid(0, split).trimmed();
        method.params << paramType;
    }
    return method;
}

void writeMethod(QTextStream& stream, Method method, QString ns)
{
    stream << "void " << ns << method.signature() << endl;
    stream << "{" << endl;
    stream << "\tQByteArray msgData;" << endl;
    stream << "\tQDataStream stream(&msgData, QIODevice::WriteOnly);" << endl;
    stream << "\tstream << QUuid(\"" << method.id.toString() << "\");" << endl;
    foreach(const Parameter& param, method.params)
        stream << "\tstream << " << param.name << ";" << endl;
    stream << "\ttransmitSignal(msgData);" << endl;
    stream << "}" << endl << endl;
}


void writeMethodParsing(QTextStream& stream, Method method)
{
    stream << "\tif(id == QUuid(\"" << method.id << "\")) {" << endl;
    foreach(const Parameter& param, method.params)
        stream << "\t\t" << param.type << " " << param.name << ";" << endl;
    foreach(const Parameter& param, method.params)
        stream << "\t\tstream >> " << param.name << ";" << endl;
    stream << "\t\temit " << method.name << "(" << method.paramNames().join(", ") << ");" << endl;
    stream << "\t\treturn;" << endl;
    stream << "\t}" << endl;
}


void writeImplementation(const QList<Method>& methods, QDir dir, const QString& className, bool reverse)
{
    QString ns = className + "::";
    QFile file(dir.absoluteFilePath(QString("%1.cpp").arg(className)));
    file.open(QFile::WriteOnly);
    QTextStream stream(&file);
    stream << "//autogenerated file SignalProxy" << endl;
    stream << "//created at:" << QDateTime::currentDateTime().toString() << endl << endl;
    stream << "#include \"" << className << ".h\"" << endl << endl;
    stream << ns << className << "(QIODevice& readDevice, QIODevice& writeDevice)" << endl;
    stream << "\t: SignalProxy(readDevice, writeDevice)"  << endl;
    stream << "{}" << endl << endl;
    foreach(const Method& method, methods)
    {
        if(method.isReverse != reverse)
            writeMethod(stream, method, ns);
    }
    stream << "void " << ns << "processRemoteInputs(const QByteArray& data)" << endl;
    stream << "{" << endl;
    stream << "\tQDataStream stream(data);" << endl;
    stream << "\tQUuid id;" << endl;
    stream << "\tstream >> id;" << endl << endl;
    foreach(const Method& method, methods)
    {
        if(method.isReverse == reverse)
            writeMethodParsing(stream, method);
    }
    stream << endl;
    stream << "\tSignalProxy::handleError(id);" << endl;
    stream << "}" << endl;

}

void writeHeader(const QList<Method>& methods, QDir dir, const QString& className, bool reverse)
{
    QFile file(dir.absoluteFilePath(QString("%1.h").arg(className)));
    file.open(QFile::WriteOnly);
    QTextStream stream(&file);
    stream << "//autogenerated file SignalProxy" << endl;
    stream << "//created at:" << QDateTime::currentDateTime().toString() << endl << endl;
    stream << "#ifndef " << className.toUpper() << "_H" << endl;
    stream << "#define " << className.toUpper() << "_H" << endl << endl;
    stream << "#include \"SignalProxy.h\"" << endl << endl;
    stream << "class " << className << " : public SignalProxy" << endl;
    stream << "{" << endl;
    stream << "\tQ_OBJECT" << endl;
    stream << "public:" << endl;
    stream << "\t" << className << "(QIODevice& readDevice, QIODevice& writeDevice);" << endl << endl;
    stream << "signals:" << endl;
    foreach(const Method& method, methods)
    {
        if(method.isReverse == reverse)
            stream << "\tvoid " << method.signature() << ";" << endl;
    }
    stream << endl;
    stream << "public slots:" << endl;
    foreach(const Method& method, methods)
    {
        if(method.isReverse != reverse)
            stream << "\tvoid " << method.signature() << ";" << endl;
    }
    stream << endl;
    stream << "private:" << endl;
    stream << "\tvoid processRemoteInputs(const QByteArray& data);" << endl;
    stream << "};" << endl << endl;
    stream << "#endif //" << className.toUpper() << "_H" << endl;

}
void writeClass(const QList<Method>& methods, QDir dir, const QString& className, bool reverse)
{
    writeHeader(methods, dir, className, reverse);
    writeImplementation(methods, dir, className, reverse);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList args = a.arguments();
    if(args.size() != 3)
    {
        QTextStream(stdout) << "Usage: SignalProxy definitionFile codeTargetDir" << endl;
        return 1;
    }
    QString definitionFile = args[1];
    QDir targetDir(args[2]);
    QString className = QFileInfo(definitionFile).baseName();
    QFile file(definitionFile);
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);
    QList<Method> methods;
    while(!stream.atEnd())
    {
        QString line = stream.readLine();
        if(!line.startsWith('#'))
            methods << parseLine(line);
    }
    file.close();
    writeClass(methods, targetDir, className + "Client", false);
    writeClass(methods, targetDir, className + "Server", true);
    QDir srcDir(a.applicationDirPath());
    QFile::copy(srcDir.absoluteFilePath("SignalProxy.h"), targetDir.absoluteFilePath("SignalProxy.h"));
    QFile::copy(srcDir.absoluteFilePath("SignalProxy.cpp"), targetDir.absoluteFilePath("SignalProxy.cpp"));
    return 0;
}

