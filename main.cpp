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

#include <QFile>
#include <cstdio>
#include <QTimer>
#include <QTextStream>
#include <QCoreApplication>

#include "blendtoxml.h"

namespace {
class BlendToXmlApplication : public QCoreApplication
{
public:
    BlendToXmlApplication(int &argc, char **argv) :
        QCoreApplication(argc, argv)
    {}

    static int exec()
    {
        QTextStream qerr(stderr);

        auto args = QCoreApplication::arguments();
        if (args.count() != 2) {
            qerr << "Usage: blend2xml input.blend > output.xml\n";
            return 1;
        }

        QFile file(args[1]);
        if (!file.open(QIODevice::ReadOnly)) {
            qerr << "open: unable to open file " << args[1] << "\n";
            return 1;
        }

        QFile outFile;
        outFile.open(stdout, QIODevice::WriteOnly);
        BlendToXml *task = new BlendToXml(&file, &outFile);
        QObject::connect(task, SIGNAL(finished()), instance(), SLOT(quit()));
        QTimer::singleShot(0, task, SLOT(run()));

        return QCoreApplication::exec();
    }
};

}

int main(int argc, char *argv[])
{
    BlendToXmlApplication app(argc, argv);
    return app.exec();
}
