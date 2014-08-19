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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef OLVISPLUGIN_H
#define OLVISPLUGIN_H

#include <filter/FilterPluginInterface.h>
#include <QObject>

class OlvisPlugin : public QObject,  public FilterPluginInterface
{
public:
    explicit OlvisPlugin();
    virtual int numFilters();
    virtual SrcFilter* createFilter(int num);
private:

#if QT_VERSION < 0x050000
    Q_OBJECT
    Q_INTERFACES(FilterPluginInterface)
#endif
};



#endif // OLVISPLUGIN_H
