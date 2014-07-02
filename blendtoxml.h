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

#ifndef BLENDTOXML_H
#define BLENDTOXML_H

#include <inttypes.h>

#include <QString>
#include <QDataStream>
#include <QStringList>

class QIODevice;
class QXmlStreamWriter;

struct Block
{
    QString name;
    uint32_t size;
    uint64_t oldMemoryAddress;
    uint32_t sdnaIndex;
    uint32_t count;
    qint64 pos;
};

struct Field
{
    uint16_t type;
    uint16_t name;
};

struct Structure
{
    uint16_t type;
    QList<Field> fields;
};

struct CombType
{
    bool isPointer;
    size_t width, height;
    QString shortname;
    QString printType;

    CombType() : isPointer(false), width(1), height(1) {}
    CombType(const QString &name);
};

class BlendToXml : public QObject
{
    Q_OBJECT
public:
    explicit BlendToXml(QIODevice *in, QIODevice *out, bool notypes, bool nodata, bool printRawPointers, QObject *parent = 0);
    
public slots:
    void run();

signals:
    void finished();

private:
    const uint32_t NOTYPE = 0xFFFFFFFF;

    QIODevice *m_in;
    QIODevice *m_out;
    bool notypes;
    bool nodata;
    bool printRawPointers;

    QDataStream stream;
    uint8_t ptrSize;

    QList<Block> blocks;
    QStringList names;
    QStringList typenames;
    QList<uint16_t> typelengths;
    QList<uint32_t> typestructures;
    QList<Structure> structures;

    QString readString(std::size_t len);

    template<std::size_t N>
    inline QString readString()
    {
        char data[N + 1];
        stream.readRawData(data, N);
        data[N] = '\0';
        return QString(data);
    }

    QString readString();

    template<typename T>
    inline T readType()
    {
        T value;
        stream >> value;
        return value;
    }

    uint64_t readAddress();
    void skipBytes(std::size_t len);

    template<std::size_t N>
    inline QByteArray readBytes()
    {
        char data[N];
        stream.readRawData(data, N);
        return QByteArray(data, N);
    }

    QByteArray readBytes(std::size_t len);
    quint16 readBytesCrc(std::size_t len);
    void readAlignedIdent(const char *ident);
    void printStructure(QXmlStreamWriter &out, int typeId, const CombType &type);
};

#endif // BLENDTOXML_H
