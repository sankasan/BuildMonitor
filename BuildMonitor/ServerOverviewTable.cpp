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

#include "ServerOverviewTable.h"

#include "ProjectInformation.h"

#include <qdatetime.h>
#include <qheaderview.h>
#include <qmenu.h>
#include <QMouseEvent>
#include <qtablewidget.h>
#include <qtextstream.h>
#include <qtooltip.h>

ServerOverviewTable::ServerOverviewTable(QWidget* parent) :
	QTableWidget(parent),
	succeeded(nullptr),
	succeededBuilding(nullptr),
	failed(nullptr),
	failedBuilding(nullptr),
	projectInformation(nullptr)
{
	headerLabels.push_back("Status");
	headerLabels.push_back("Project");
	headerLabels.push_back("Remaining Time");
	headerLabels.push_back("Duration");
	headerLabels.push_back("Last Successful Build");
	headerLabels.push_back("Volunteer");
	headerLabels.push_back("Initiated By");

	setColumnCount(headerLabels.size());
	setHorizontalHeaderLabels(headerLabels);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &ServerOverviewTable::customContextMenuRequested, this, &ServerOverviewTable::openContextMenu);
}

void ServerOverviewTable::setIcons(const QIcon* inSucceeded, const QIcon* inSucceededBuilding,
	const QIcon* inFailed, const QIcon* inFailedBuilding)
{
	succeeded = inSucceeded;
	succeededBuilding = inSucceededBuilding;
	failed = inFailed;
	failedBuilding = inFailedBuilding;
}

void ServerOverviewTable::setProjectInformation(const class std::vector<class ProjectInformation>& inProjectInformation)
{
	projectInformation = &inProjectInformation;

	for (auto& item : itemPool)
	{
		delete item;
	}
	itemPool.clear();
	clearContents();

	for (const ProjectInformation& info : inProjectInformation)
	{
		QTableWidgetItem* statusItem = new QTableWidgetItem();
		statusItem->setText(projectStatus_toString(info.status));
		if (projectStatus_isFailure(info.status))
		{
			if (info.isBuilding && failedBuilding != nullptr)
			{
				statusItem->setIcon(*failedBuilding);
			}
			else if (!info.isBuilding && failed != nullptr)
			{
				statusItem->setIcon(*failed);
			}
		}
		else
		{
			if (info.isBuilding && succeededBuilding != nullptr)
			{
				statusItem->setIcon(*succeededBuilding);
			}
			else if (!info.isBuilding && succeeded != nullptr)
			{
				statusItem->setIcon(*succeeded);
			}
		}
		itemPool.push_back(statusItem);

		itemPool.push_back(new QTableWidgetItem(info.projectName));

		if (info.isBuilding)
		{
			qint32 estimatedRemainingTime = info.estimatedRemainingTime / 1000;
			const char* timeUnit = "minute(s)";
			if (estimatedRemainingTime < 60 && estimatedRemainingTime > -60)
			{
				timeUnit = "second(s)";
			}
			else
			{
				estimatedRemainingTime /= 60;
			}

			QString estimatedRemainingTimeFull;
			QTextStream estimatedRemainingTimeStream(&estimatedRemainingTimeFull);
			if (estimatedRemainingTime < 0)
			{
				estimatedRemainingTime = abs(estimatedRemainingTime);
				estimatedRemainingTimeStream << "Taking " << estimatedRemainingTime << " " << timeUnit << " longer";
			}
			else
			{
				estimatedRemainingTimeStream << estimatedRemainingTime << " " << timeUnit;
			}
			estimatedRemainingTimeStream.flush();

			itemPool.push_back(new QTableWidgetItem(estimatedRemainingTimeFull));
		}
		else
		{
			itemPool.push_back(new QTableWidgetItem("-"));
		}
		itemPool.push_back(new QTableWidgetItem(QString::number(info.inProgressFor / 60 / 1000) + " minutes"));

		if (info.lastSuccessfulBuildTime != -1)
		{
			QDateTime lastSuccessfulBuild;
			lastSuccessfulBuild.setMSecsSinceEpoch(info.lastSuccessfulBuildTime);
			itemPool.push_back(new QTableWidgetItem(lastSuccessfulBuild.toLocalTime().toString("hh:mm dd-MM-yyyy")));
		}
		else
		{
			itemPool.push_back(new QTableWidgetItem("Unavailable"));
		}

		itemPool.push_back(new QTableWidgetItem(info.volunteer));

		QString initiators;
		for (size_t i = 0; i < info.initiatedBy.size(); ++i)
		{
			if (i != 0)
			{
				if (i == info.initiatedBy.size() - 1)
				{
					initiators += " and ";
				}
				else
				{
					initiators += ", ";
				}
			}
			initiators += info.initiatedBy[i];
		}

		itemPool.push_back(new QTableWidgetItem(initiators));
	}

	const qint32 numProjects = static_cast<qint32>(inProjectInformation.size());
	setRowCount(numProjects);
	const qint32 numHeaders = headerLabels.size();
	for (qint32 row = 0; row < numProjects; ++row)
	{
		for (qint32 column = 0; column < numHeaders; ++column)
		{
			const qint32 itemLocationInArray = row * numHeaders + column;
			QTableWidgetItem* item = itemPool[itemLocationInArray];
			item->setToolTip(item->text());
			setItem(row, column, item);
		}
	}

	resizeColumnsToContents();
	horizontalHeader()->setSectionResizeMode(headerLabels.size() - 1, QHeaderView::Stretch);
}

QString ServerOverviewTable::getProjectName(qint32 row)
{
	if (row == -1)
	{
		return "";
	}
	return item(row, 1)->text();
}

void ServerOverviewTable::openContextMenu(const QPoint& location)
{
	QPoint globalLocation = viewport()->mapToGlobal(location);

	QMenu contextMenu;

	QAction* volunteerToFixAction = contextMenu.addAction("Volunteer to Fix");
	QString projectName = getProjectName(currentIndex().row());
	bool volunteerOptionEnabled = false;
	if (projectInformation)
	{
		const std::vector<ProjectInformation>::const_iterator pos = std::find_if(projectInformation->begin(), projectInformation->end(),
			[&projectName](const ProjectInformation& projectInformation) { return projectInformation.projectName == projectName; });
		volunteerOptionEnabled = pos != projectInformation->end() && projectStatus_isFailure(pos->status);
	}
	volunteerToFixAction->setEnabled(volunteerOptionEnabled);

	QAction* viewBuildLogAction = contextMenu.addAction("View Build Log");
	bool viewBuildLogActionEnabled = false;
	if (projectInformation)
	{
		const std::vector<ProjectInformation>::const_iterator pos = std::find_if(projectInformation->begin(), projectInformation->end(),
			[&projectName](const ProjectInformation& projectInformation) { return projectInformation.projectName == projectName; });
		viewBuildLogActionEnabled = pos != projectInformation->end() && pos->buildNumber != 0;
	}
	viewBuildLogAction->setEnabled(viewBuildLogActionEnabled);

	const QAction* selectedContextMenuItem = contextMenu.exec(globalLocation);
	if (selectedContextMenuItem)
	{
		if (selectedContextMenuItem == volunteerToFixAction)
		{
			volunteerToFix(getProjectName(currentIndex().row()));
		}
		else if (selectedContextMenuItem == viewBuildLogAction)
		{
			viewBuildLog(getProjectName(currentIndex().row()));
		}
	}
}
