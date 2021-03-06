/* BuildMonitor - Monitor the state of projects in CI.
 * Copyright (C) 2017 Sander Brattinga

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <qstandarditemmodel.h>
#include <qtablewidget.h>

class ServerOverviewTable : public QTableWidget
{
	Q_OBJECT

public:
	ServerOverviewTable(QWidget* parent);

	void setIcons(const QIcon* inSucceeded, const QIcon* inSucceededBuilding,
		const QIcon* inFailed, const QIcon* inFailedBuilding);
	void setProjectInformation(const class std::vector<class ProjectInformation>& inProjectInformation);

	QString getProjectName(qint32 row);

Q_SIGNALS:
	void volunteerToFix(const QString& projectName);
	void viewBuildLog(const QString& projectName);

private:
	void openContextMenu(const QPoint& location);

	const QIcon* succeeded;
	const QIcon* succeededBuilding;
	const QIcon* failed;
	const QIcon* failedBuilding;

	const class std::vector<class ProjectInformation>* projectInformation;
	std::vector<class QTableWidgetItem*> itemPool;
	QStringList headerLabels;
};
