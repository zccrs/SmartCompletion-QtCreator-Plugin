#ifndef SMARTCOMPLETIONPLUGIN_GLOBAL_H
#define SMARTCOMPLETIONPLUGIN_GLOBAL_H

#include <QtGlobal>
#include <QRegularExpression>
#include <QDebug>

#if defined(SMARTCOMPLETIONPLUGIN_LIBRARY)
#  define SMARTCOMPLETIONPLUGINSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SMARTCOMPLETIONPLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif

#define LS(str) QLatin1String(str)
#define LC(ch) QLatin1Char(ch)
#define RX_CLASSNAME LS("(?<=class\\s+)\\w+(?=\\s+)")
#define RX_SYMBOL "[\\$_a-zA-Z][\\$a-zA-Z0-9]*"

#define STR_PROPERTY LS("Q_PROPERTY")
#define STR_CLASS LS("class")

class Global
{
public:
    enum WordType{
        UnknowType,
        PropertyType,
        ClassNameType
    };

    enum BlockType{
        UnknowBlock,
        CodeBlock,
        CharBlock,
        StringBlock,
        CommentedOutBlock,
        CommentedOutLine
    };

    struct CodeInfo{
        WordType type;
        QString word;
    };

    struct Block{
        BlockType type = UnknowBlock;
        int fromPosition = -1;
        int length = 0;
    };

    struct Property{
        QString type;
        QString name;
        QString read;
        QString write;
        QString member;
        QString reset;
        QString notify;
    };

    static inline Block createBlock(BlockType type, int from = -1, int length = 0)
    {
        Block block;

        block.fromPosition = from;
        block.length = length;
        block.type = type;

        return block;
    }

    static inline QString getStrByBlock(const QString &str, const Block &block)
    {
        return str.mid(block.fromPosition, block.length);
    }

    /// split into blocks of c++ code.
    static QList<Block> codeToBlocks(const QString &code, int end_position = -1);
    static int getBlockByPosition(const QList<Block> &list, int current_position);
    /// skip commented out and empty char
    static QString prevSymbolByPosition(const QString &code,
                                        const QList<Block> &list,
                                        int current_position);
    /// skip commented out and empty string
    static QString nextSymbolByPosition(const QString &code,
                                      const QList<Block> &list,
                                      int current_position);
    /// get vaild symbol(such as class name, variable name) from cursor position.
    static QString getSymbolByPosition(const QString &text, int position,
                                       int *start_pos = nullptr, int *end_pos = nullptr);
    /// parse qt code, get cursor position code type(such as type is property or class defind)
    static CodeInfo codeParse(const QString &str, int cursor_position);
    /// parse Q_PROPERTY code. get property type&value name&get fun name&set fun name|signal name...
    static bool propertyParse(const QString &str, Property &property);
    /// get vaild c++ type name(such as QList<int*>*) from current position.
    static QString getVaildTypeName(const QString &code, int from_position,
                                    int *start_pos = nullptr, int *end_pos = nullptr);
};

QDebug operator<<(QDebug deg, const Global::Block &block)
{
    deg << LS("type:") << block.type << LS("begin position:") << block.fromPosition
        << LS("length:") << block.length;

    return deg;
}

QDebug operator<<(QDebug deg, const Global::CodeInfo &symbol)
{
    deg << LS("type:") << symbol.type << LS("word:") << symbol.word;

    return deg;
}

QDebug operator<<(QDebug deg, const Global::Property &property)
{
    deg << LS("type:") << property.type
        << LS("name:") << property.name
        << LS("read:") << property.read
        << LS("write:") << property.write
        << LS("reset:") << property.reset
        << LS("notify:") << property.notify;

    return deg;
}

QList<Global::Block> Global::codeToBlocks(const QString &code, int end_position)
{
    if(end_position == -1)
        end_position = code.count();

    int i = -1;
    int begin_pos = 0;

    QList<Block> blocks;

    while(++i < code.count()) {
        QChar ch = code.at(i);

        switch (ch.toLatin1()) {
        case '\'':/// intentional
        case '\"':{
            blocks << createBlock(CodeBlock, begin_pos, i - begin_pos);

            if(i >= end_position) {
                return blocks;/// return
            }

            int j = i;

            while(++j < code.count()) {
                switch (code.at(j).toLatin1()) {
                case '"':{
                    if(ch != LC('"'))
                        break;
                    goto next;
                }
                case '\'':{
                    if(ch != LC('\''))
                        break;
                    goto next;
                }
                case '\\':{
                    ++j;
                    break;
                }
                case '\n':{
                    goto next;
                }
                default:
                    break;
                }
            }

            next:
            blocks << createBlock((ch == LC('"') ? StringBlock : CharBlock), i, j - i + 1);
            i = j;
            begin_pos = j + 1;
            break;
        }
        case '/':{
            QChar next_ch = code.at(i + 1);
            int j = 0;

            if(next_ch == LC('*')) {
                blocks << createBlock(CodeBlock, begin_pos, i - begin_pos);

                if(i >= end_position) {
                    return blocks;/// return
                }

                j = code.indexOf(LS("*/"), i + 2);

                if(j < 0)
                    j = code.count() - 1;
                else
                    ++j;
            } else if(next_ch == LC('/')) {
                blocks << createBlock(CodeBlock, begin_pos, i - begin_pos);

                if(i >= end_position) {
                    return blocks;/// return
                }

                j = code.indexOf(LC('\n'), i + 2);

                if(j < 0)
                    j = code.count() - 1;
            } else {
                break;
            }

            blocks << createBlock((next_ch == LC('/') ? CommentedOutLine : CommentedOutBlock), i, j - i + 1);
            i = j;
            begin_pos = j + 1;
            break;
        }
        case '\\':{
            QChar next_ch = code.at(i + 1);

            if(next_ch == LC('"') || next_ch == LC('\''))
                ++i;
            break;
        }
        default:
            break;
        }
    }

    blocks << createBlock(CodeBlock, begin_pos, i - begin_pos);

    return blocks;
}

int Global::getBlockByPosition(const QList<Block> &list, int current_position)
{
    for(int i = 0; i < list.count(); ++i) {
        const Block &block = list.at(i);

        if(block.fromPosition + block.length > current_position) {
            return i;
        }
    }

    return -1;
}

QString Global::prevSymbolByPosition(const QString &code,
                                     const QList<Block> &list,
                                     int current_position)
{
    QRegularExpression not_word_rx(LS("\\S\\s"));
    QString tmp_str;

    int index = getBlockByPosition(list, current_position);

    const Block &block = list.at(index);

    int pos_offset = block.length - current_position + block.fromPosition;

    do {
        const Block &block = list.at(index);

        if(block.type == CodeBlock) {
            tmp_str = getStrByBlock(code, block) + tmp_str;

            int pos = tmp_str.lastIndexOf(not_word_rx, qMax(tmp_str.count() - pos_offset - 1, 0));

            if(pos >= 0) {
                return getSymbolByPosition(tmp_str, pos);
            }
        } else if(block.type != CommentedOutBlock
                  && block.type != CommentedOutLine) {
            break;
        }
    }while(index--);

    return LS("");
}

QString Global::nextSymbolByPosition(const QString &code,
                                   const QList<Block> &list,
                                   int current_position)
{
    QRegularExpression not_word_rx(LS("\\s\\S"));
    QString tmp_str;

    int index = getBlockByPosition(list, current_position);

    const Block &block = list.at(index);

    int pos_offset = current_position - block.fromPosition;

    do {
        const Block &block = list.at(index);

        if(block.type == CodeBlock) {
            tmp_str += getStrByBlock(code, block);

            int pos = tmp_str.indexOf(not_word_rx, pos_offset + 1);

            if(pos >= 0) {
                return getSymbolByPosition(tmp_str, pos + 1);
            }
        } else if(block.type != CommentedOutBlock
                  && block.type != CommentedOutLine) {
            break;
        }

        ++index;
    }while(index < list.count());

    return LS("");
}

QString Global::getSymbolByPosition(const QString &code, int current_position,
                                    int *start_pos, int *end_pos)
{
    if(current_position < 0)
        return LS("");

    QRegularExpression not_word_rx(LS("[^\\w\\$]"));

    int word_begin_position = code.lastIndexOf(not_word_rx, current_position);
    int word_end_position = code.indexOf(not_word_rx, current_position);

    if(word_begin_position <= current_position
            && word_begin_position != word_end_position) {

        if(start_pos)
            *start_pos = word_begin_position + 1;

        if(word_end_position == -1) {
            if(end_pos)
                *end_pos = code.length();

            return code.mid(word_begin_position + 1);
        }

        if(end_pos)
            *end_pos = word_end_position;

        return code.mid(word_begin_position + 1,
                        word_end_position - word_begin_position - 1);
    }

    return LS("");
}

Global::CodeInfo Global::codeParse(const QString &str, int cursor_position)
{
    if(str.isEmpty())
        return CodeInfo{UnknowType, LS("")};

    const QList<Block> &blocks = codeToBlocks(str, cursor_position);
    const Block block = blocks.last();

    if(block.type != CodeBlock)
        return CodeInfo{UnknowType, LS("")};

    const QString word = getSymbolByPosition(getStrByBlock(str, block),
                                             cursor_position - block.fromPosition);
    WordType type = UnknowType;

    if(word == STR_PROPERTY) {
        type = PropertyType;
    } else {
        const QString &left_word = prevSymbolByPosition(str, blocks, cursor_position);

        qDebug() << "left:" << left_word << "right:" << nextSymbolByPosition(str, blocks, cursor_position);

        if(left_word == STR_CLASS)
            type = ClassNameType;
    }

    return CodeInfo{type, word};
}

bool Global::propertyParse(const QString &str, Global::Property &property)
{
    int offset = str.indexOf(LC('(')) + 1;

    property.type = getVaildTypeName(str, offset, nullptr, &offset);

    if(!property.type.isEmpty()) {
        property.type = property.type.trimmed();
        property.name = getSymbolByPosition(str, offset, nullptr, &offset);

        if(property.name.isEmpty())
            return false;

        const QStringList list1 = QStringList() << LS("READ") << LS("WRITE") << LS("MEMBER")
                                                << LS("RESET") << LS("NOTIFY");
        const QList<QString*> list2 = QList<QString*>() << &property.read << &property.write
                                                        << &property.member << &property.reset
                                                        << &property.notify;

        QRegularExpression rx;

        for(int i = 0; i < list1.count(); ++i) {
            rx.setPattern(QString(LS("\\s*%1\\s+(?<value>%2)")).arg(list1[i]).arg(LS(RX_SYMBOL)));

            const QRegularExpressionMatch &match = rx.match(str, offset);

            if(match.isValid())
                *list2.at(i) = match.captured(LS("value"));
        }
    }

    return false;
}

QString Global::getVaildTypeName(const QString &code, int offset, int *start_pos, int *end_pos)
{
    QString typeName;

    if(offset < 0)
        return typeName;

    QRegularExpression rx(LS(RX_SYMBOL));

    const QRegularExpressionMatch &match = rx.match(code, offset);

    if(match.isValid()) {
        if(start_pos)
            *start_pos = match.capturedStart();

        offset = match.capturedEnd() - 1;
        typeName = match.captured();

        if(typeName.isEmpty())
            return typeName;

        while(++offset < code.length()) {
            const QChar &ch = code.at(offset);

            switch (ch.toLatin1()) {
            case ' ':/// intentional
            case '*':
                typeName.append(ch);
                break;
            case ':':
                if(offset >= code.length() || code.at(++offset) != LC(':')) {
                    return LS("");
                }
                typeName.append(ch);
                /// intentional
            case ',':/// intentional
            case '<':{
                typeName.append(ch);

                int endPos = code.indexOf(QRegularExpression(LS("[^ ]")), offset + 1);

                const QString &str = getVaildTypeName(code, endPos, nullptr, &endPos);

                if(!str.isEmpty()) {
                    if(endPos < code.length())
                        endPos = code.indexOf(QRegularExpression(LS("[^ ]")), endPos);

                    if(endPos > 0) {
                        if(ch != LC('<')) {
                            --endPos;
                        } else if(endPos >= code.length() || code.at(endPos) != LC('>')) {
                            return LS("");
                        }

                        typeName.append(code.mid(offset + 1, endPos - offset));
                        offset = endPos;
                        break;
                    }
                }

                return LS("");
            }
            default:
                if(end_pos)
                    *end_pos = offset;

                return typeName;
            }
        }
    }

    if(end_pos)
        *end_pos = offset;

    return typeName;
}

#endif // SMARTCOMPLETIONPLUGIN_GLOBAL_H
