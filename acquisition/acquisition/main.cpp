/*
    Acquisition is a stash management program for Path of Exile

    Copyright(C) 2024 Gerwaric

    This program is free software : you can redistribute it and /or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < https://www.gnu.org/licenses/>.
*/

#include <acquisition/main_window.h>

#include <acquisition/constants.h>
#include <acquisition/utils/utils.h>

#include <QsLog/QsLog.h>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QStandardPaths>

#ifdef _DEBUG
constexpr QsLogging::Level DefaultLogging = QsLogging::DebugLevel;
#else
constexpr QsLogging::Level DefaultLogging = QsLogging::InfoLevel;
#endif

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setApplicationVersion(APP_VERSION_STRING);

    QCommandLineOption data_directory_option(
        { "d", "data-directory" },
        "Location for data files",
        "data_directory",
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));

    QCommandLineOption logging_level_option(
        { "l", "logging-level" },
        "off, fatal, error, warn, info, debug, trace",
        "logging_level",
        utils::logLevelName(DefaultLogging));

    QCommandLineParser p;
    p.setApplicationDescription("Inventory and forum shop management for Path of Exile");
    p.addHelpOption();
    p.addVersionOption();
    p.addOption(data_directory_option);
    p.addOption(logging_level_option);
    p.process(app);

    const auto logging_level = utils::logLevel(p.value(logging_level_option));
    const QString data_directory = p.value(data_directory_option);
    const QString log_filename = data_directory + QDir::separator() + "log-new.txt";

    QsLogging::Logger& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(logging_level);
    logger.addDestination(QsLogging::DestinationFactory::MakeDebugOutputDestination());
    logger.addDestination(QsLogging::DestinationFactory::MakeFileDestination(log_filename,
        QsLogging::EnableLogRotation,
        QsLogging::MaxSizeBytes(10 * 1024 * 1024),
        QsLogging::MaxOldLogCount(0)));

    QLOG_INFO() << "Starting " APP_NAME " version " APP_VERSION_STRING;

    MainWindow main_window(data_directory);
    return app.exec();
}
