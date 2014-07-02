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

#include "blendtoxml.h"

#include <memory>
#include <QRegExp>
#include <QXmlStreamWriter>
#include <QRegularExpression>

CombType::CombType(const QString &name)
    : isPointer(false), width(1), height(1)
{
    isPointer = name.startsWith('*');

    if (isPointer) {
        printType = "*";
    }

    QRegExp rx_name("^\\*?(\\w+)");
    rx_name.indexIn(name);
    shortname = rx_name.cap(1);

    QRegExp rx_wh("\\[(\\d+)\\]\\[(\\d+)\\]");
    if (rx_wh.indexIn(name) > -1) {
        width = rx_wh.cap(1).toInt();
        height = rx_wh.cap(2).toInt();
        printType += rx_wh.cap(0);
        return;
    }
    else {
        QRegExp rx_w("\\[(\\d+)\\]");
        if (rx_w.indexIn(name) > -1) {
            width = rx_w.cap(1).toInt();
            printType += rx_w.cap(0);
            return;
        }
    }
}

BlendToXml::BlendToXml(QIODevice *in, QIODevice *out, bool notypes, bool nodata, bool printRawPointers, QObject *parent) :
    QObject(parent), m_in(in), m_out(out),
    notypes(notypes), nodata(nodata), printRawPointers(printRawPointers), ptrSize(0)
{}

void BlendToXml::run()
{
    stream.setDevice(m_in);
    stream.setByteOrder(QDataStream::LittleEndian);

    QXmlStreamWriter out(m_out);
    out.setAutoFormatting(true);
    out.writeStartDocument();

    out.writeStartElement("blend");

    {
        auto identifier = readString<7>();

        if (identifier != "BLENDER") {
            qFatal("This tool does not support compressed files.\n"
                   "Rename .blend-file to .blend.gz and unpack before running this tool again\n");
        }

        ptrSize = (readType<uint8_t>() == '_') ? 4 : 8;
        auto endianness = readType<uint8_t>();

        if (endianness == 'V') {
            stream.setByteOrder(QDataStream::BigEndian);
        }

        out.writeAttribute("identifier", identifier);
        out.writeAttribute("pointer-size", QString::number(ptrSize));
        out.writeAttribute("endianness", QString(endianness));
        out.writeAttribute("version-number", readString<3>());
    }

    while (!stream.atEnd()) {
        Block b;
        b.name = readString<4>();
        b.size = readType<uint32_t>();
        b.oldMemoryAddress = readAddress();
        b.sdnaIndex = readType<uint32_t>();
        b.count = readType<uint32_t>();
        b.pos = m_in->pos();

        if (b.name == "DNA1") {
            readAlignedIdent("SDNA");

            readAlignedIdent("NAME");
            auto namesCount = readType<uint32_t>();
            for (size_t i = 0; i < namesCount; i++) {
                names << readString();
            }

            readAlignedIdent("TYPE");
            auto types = readType<uint32_t>();
            for (size_t i = 0; i < types; i++) {
                typenames << readString();
            }

            readAlignedIdent("TLEN");
            for (size_t i = 0; i < types; i++) {
                auto typelength = readType<uint16_t>();
#if 0
                if (typelength % 4) {
                    qFatal(qPrintable(QString("Invalid structure length %1").arg(typelength)));
                }
                typelength /= 4;
#endif
                typelengths << typelength;
            }

            for (size_t i = 0; i < types; i++) {
                typestructures.append(NOTYPE);
            }

            readAlignedIdent("STRC");
            auto structuresCount = readType<uint32_t>();
            for (uint32_t i = 0; i < structuresCount; i++) {
                Structure s;
                s.type = readType<uint16_t>();
                Q_ASSERT(typestructures[s.type] == NOTYPE);
                typestructures[s.type] = i;
                auto fields = readType<uint16_t>();
                for (size_t j = 0; j < fields; j++) {
                    Field f;
                    f.type = readType<uint16_t>();
                    f.name = readType<uint16_t>();
                    s.fields.append(f);
                }
                structures.append(s);
            }
            break;
        } else {
            skipBytes(b.size);
            blocks.append(b);
        }
    }

    if (!notypes) {
        out.writeStartElement("types");
        for (int i = 0; i < typenames.length(); i++) {
            out.writeStartElement("type");
            out.writeAttribute("name", typenames[i]);
            out.writeAttribute("length", QString::number(typelengths[i]));
            out.writeEndElement();
        }
        out.writeEndElement();

        out.writeStartElement("structures");
        for (const auto &structure : structures) {
            out.writeStartElement("structure");
            out.writeAttribute("type", typenames[structure.type]);
            out.writeAttribute("size", QString::number(typelengths[structure.type]));
            for (const auto &field : structure.fields) {
                out.writeStartElement("field");
                out.writeAttribute("type", typenames[field.type]);
                out.writeAttribute("name", names[field.name]);
                out.writeEndElement();
            }
            out.writeEndElement();
        }
        out.writeEndElement();
    }

    if (!nodata) {
        for (const Block &block : blocks) {
            m_in->seek(block.pos);
            out.writeStartElement(typenames[structures[block.sdnaIndex].type]);
            out.writeAttribute("block", block.name);

            if (printRawPointers) {
                out.writeAttribute("old-memory-address", QString::number(block.oldMemoryAddress, 16));
            }

            for (size_t i = 0; i < block.count; i++) {
                if (block.count != 1) {
                    out.writeStartElement("elem");
                }
                printStructure(out, structures[block.sdnaIndex].type, CombType());
                if (block.count != 1) {
                    out.writeEndElement();
                }
            }
            out.writeEndElement();
        }
    }

    out.writeEndElement();
    out.writeEndDocument();

    emit finished();
}


QString BlendToXml::readString(std::size_t len)
{
    std::unique_ptr<char[]> data(new char[len + 1]);
    stream.readRawData(data.get(), static_cast<int>(len));
    data[len] = '\0';
    return QString(data.get());
}

QString BlendToXml::readString()
{
    QString str;
    char c;
    while ((c = readType<uint8_t>())) {
        str.append(QChar(c));
    }
    return str;
}

uint64_t BlendToXml::readAddress()
{
    if (ptrSize == 4) {
        return readType<quint32>();
    } else {
        return readType<quint64>();
    }
}

void BlendToXml::skipBytes(std::size_t len)
{
    stream.skipRawData(static_cast<int>(len));
}

QByteArray BlendToXml::readBytes(std::size_t len)
{
    std::unique_ptr<char[]> data(new char[len]);
    stream.readRawData(data.get(), static_cast<int>(len));
    return QByteArray(data.get(), static_cast<int>(len));
}

quint16 BlendToXml::readBytesCrc(std::size_t len)
{
    std::unique_ptr<char[]> data(new char[len]);
    stream.readRawData(data.get(), static_cast<int>(len));
    return qChecksum(data.get(), static_cast<int>(len));
}

void BlendToXml::readAlignedIdent(const char *ident)
{
    for (int i = 0; i < 4; i++) {
        char c = readType<uint8_t>();
        if (c) {
            Q_ASSERT(QChar(c) + readString<3>() == ident);
            return;
        }
    }
    Q_ASSERT(!"Unable to find aligned word");
}

void BlendToXml::printStructure(QXmlStreamWriter &out, int typeId, const CombType &type)
{
    if (type.isPointer) {
        for (size_t i = 0; i < type.height; i++) {
            for (size_t j = 0; j < type.width; j++) {
                auto address = readAddress();
                if (address) {
                    if (printRawPointers) {
                        out.writeCharacters(QString::number(address, 16));
                    } else {
                        out.writeCharacters("0xDEADBEEF");
                    }
                } else {
                    out.writeCharacters("NULL");
                }
            }
        }
        return;
    }

    if (typestructures[typeId] == NOTYPE) {
        bool isSingle = type.height == 1 && type.width == 1;
        bool isFlag = isSingle && (type.shortname.contains("flag") || type.shortname.contains("type"));

        if (typenames[typeId] == "char") {
            if (isSingle) {
                out.writeAttribute("mode", "flag");
                out.writeCharacters(QString("0b%1").arg(readType<quint8>(), 0, 2));
                return;
            }

            QByteArray bytes = readBytes(type.height * type.width);

            bool fullprint = true;
            for (size_t i = 0; i < type.height; i++) {
                for (size_t j = 0; j < type.width; j++) {
                    uint8_t c = bytes.at(static_cast<int>(i * type.width + j));
                    QChar qc(c);
                    if (qc.toLatin1() != c || !qc.isPrint()) {
                        for (; j < type.width; j++) {
                            uint8_t nullc = bytes.at(static_cast<int>(i * type.width + j));
                            if (nullc != '\0') {
                                fullprint = false;
                                break;
                            }
                        }
                    }
                }
                if (!fullprint) {
                    break;
                }
            }
            if (fullprint) {
                out.writeAttribute("mode", "ascii");
                for (size_t i = 0; i < type.height; i++) {
                    out.writeCharacters(QString(bytes.data() + i * type.width));
                }
            } else if (type.shortname.contains("name") ||
                       type.shortname.contains("title") ||
                       type.shortname.contains("filepath") ||
                       type.shortname.contains("string") ||
                       type.shortname == "dir" ||
                       type.shortname == "file" ||
                       type.shortname.endsWith("str")) {
                out.writeAttribute("mode", "mixed");
                for (size_t i = 0; i < type.height; i++) {

                    size_t nonNullCharacters = type.width;
                    while (nonNullCharacters) {
                        if (bytes.at(static_cast<int>(i * type.width + nonNullCharacters - 1))) {
                            break;
                        }
                        nonNullCharacters--;
                    }

                    for (size_t j = 0; j < nonNullCharacters; j++) {
                        char byte = bytes.at(static_cast<int>(i * type.width + j));
                        auto ascii = QChar::fromLatin1(byte);
                        if (byte == '\0') {
                            out.writeCharacters("\\0");
                        } else if (byte == '\\') {
                            out.writeCharacters("\\\\");
                        } else if (ascii.unicode() <= 127 && ascii.isPrint()) {
                            out.writeCharacters(ascii);
                        } else {
                            out.writeCharacters(QString("\\%1").arg((quint8)byte, 3, 8, QLatin1Char('0')));
                        }
                    }
                }
            } else {
                out.writeAttribute("mode", "data");
                for (size_t i = 0; i < type.height; i++) {
                    for (size_t j = 0; j < type.width; j++) {
                        char byte = bytes.at(static_cast<int>(i * type.width + j));
                        out.writeCharacters(QString("%1").arg((quint8)byte, 2, 16, QLatin1Char('0')));
                    }
                }
            }
            return;
        }

        for (size_t i = 0; i < type.height; i++) {
            for (size_t j = 0; j < type.width; j++) {
                switch (typelengths[typeId]) {
                case 0:
                    out.writeCharacters("???");
                    break;
                case 1:
                    if (isFlag)
                        out.writeCharacters(QString("0b%1").arg(readType<quint8>(), 0, 2));
                    else
                        out.writeCharacters(QString::number(readType<quint8>()));
                    break;
                case 2:
                    if (isFlag)
                        out.writeCharacters(QString("0b%1").arg(readType<quint16>(), 0, 2));
                    else
                        out.writeCharacters(QString::number(readType<quint16>()));
                    break;
                case 4:
                    if (typenames[typeId] == "float") {
                        out.writeCharacters(QString::number(readType<float>()));
                    } else {
                        if (isFlag)
                            out.writeCharacters(QString("0b%1").arg(readType<quint32>(), 0, 2));
                        else
                            out.writeCharacters(QString::number(readType<quint32>()));
                    }
                    break;
                case 8:
                    if (typenames[typeId] == "double") {
                        out.writeCharacters(QString::number(readType<double>()));
                    } else {
                        if (isFlag)
                            out.writeCharacters(QString("0b%1").arg(readType<quint64>(), 0, 2));
                        else
                            out.writeCharacters(QString::number(readType<quint64>()));
                    }
                    break;
                default: Q_ASSERT(!"Too long typelength");
                }
                if (i != type.height - 1 || j != type.width - 1) {
                    out.writeCharacters(" ");
                }
            }
        }
        return;
    }

    for (size_t i = 0; i < type.height; i++) {
        for (size_t j = 0; j < type.width; j++) {
            if (type.width != 1 || type.height != 1) {
                out.writeStartElement("elem");
            }
            for (const Field &field : structures[typestructures[typeId]].fields) {
                CombType ct(names[field.name]);

                if (ct.shortname.contains(QRegularExpression("^_*pad\\d*$")))
                    continue;

                out.writeStartElement(ct.shortname);
                out.writeAttribute("type", typenames[field.type] + ct.printType);
                printStructure(out, field.type, ct);
                out.writeEndElement();
            }
            if (type.width != 1 || type.height != 1) {
                out.writeEndElement();
            }
        }
    }
}
