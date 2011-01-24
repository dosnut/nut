/*
    CErrorCodeEvaluator
    Copyright (C) 2010  Oliver Groß <z.o.gross@gmx.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef CERRORCODEEVALUATOR_H
#define CERRORCODEEVALUATOR_H

#include <QHash>
#include <QString>

class QWidget;

class CErrorCodeEvaluator {
public:
	CErrorCodeEvaluator();
	
	QString evaluate(unsigned long int code) const;
	void evaluate(QStringList & target, unsigned long int & errorFlags) const;
	void evaluate(QString & target, const QString & separator, unsigned long int & errorFlags) const;
	
	void registerErrorCode(unsigned long int code, const QString & string);
	void unregisterErrorCode(unsigned long int code);
protected:
	QHash<unsigned long int, QString> m_ErrorHash;
};
#endif // CERRORCODEEVALUATOR_H
