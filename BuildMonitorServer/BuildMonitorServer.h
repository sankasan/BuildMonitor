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

#include "Server.h"

#include <QtWidgets/QMainWindow>

#include "ui_BuildMonitorServer.h"

class BuildMonitorServer : public QMainWindow
{
	Q_OBJECT

public:
	BuildMonitorServer(QWidget *parent = Q_NULLPTR);

private:
	void onFixInfoChanged(const std::vector<FixInfo>& fixInfo);

	Ui::BuildMonitorServerClass ui;
	Server server;
};
