/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include <cstdio>

#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QCoreApplication>
#include <QCommandLineParser>

#include "blendtoxml.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("blend2xml");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", "Source .blend file.");

    QCommandLineOption outputOption(QStringList() << "o" << "output", "Destination .xml file.", "file");
    parser.addOption(outputOption);

    QCommandLineOption notypesOption("notypes", "Disable type info.");
    parser.addOption(notypesOption);

    QCommandLineOption nodataOption("nodata", QCoreApplication::translate("main", "Disable data info."));
    parser.addOption(nodataOption);

    QCommandLineOption printRawPointersOption("rawpointers", QCoreApplication::translate("main", "Print raw pointers."));
    parser.addOption(printRawPointersOption);

    parser.process(*qApp);

    const QStringList args = parser.positionalArguments();
    if (args.count() != 1) {
        parser.showHelp(1);
    }

    QTextStream qerr(stderr);

    QFile file(args[0]);
    if (!file.open(QIODevice::ReadOnly)) {
        qerr << QString("open %1: %2\n").arg(args[0], file.errorString());
        return 1;
    }

    QString outputPath = parser.value(outputOption);

    QFile outFile;

    if (outputPath.isEmpty()) {
        outFile.open(stdout, QIODevice::WriteOnly);
    } else {
        outFile.setFileName(outputPath);
        outFile.open(QIODevice::WriteOnly);
    }

    if (outFile.error() != QFileDevice::NoError) {
        qerr << "open:" << outFile.errorString() << "\n";
        return 1;
    }

    bool notypes = parser.isSet(notypesOption);
    bool nodata = parser.isSet(nodataOption);
    bool printRawPointers = parser.isSet(printRawPointersOption);

    BlendToXml *task = new BlendToXml(&file, &outFile, notypes, nodata, printRawPointers);
    QObject::connect(task, &BlendToXml::finished, &app, &QCoreApplication::quit);
    QTimer::singleShot(0, task, SLOT(run()));

    return app.exec();
}
