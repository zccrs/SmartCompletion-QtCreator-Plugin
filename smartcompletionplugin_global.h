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
#define RX_SYMBOL LS("[\\$_a-zA-Z][\\$a-zA-Z0-9]*")

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

    struct Symbol{
        WordType type;
        QString word;
    };

    struct Block{
        BlockType type = UnknowBlock;
        int fromPosition = -1;
        int length = 0;
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
    static QString getSymbolByPosition(const QString &text, int position);
    static Symbol getSymbolByString(const QString &str, int cursor_position);
};

QDebug operator<<(QDebug deg, const Global::Block &block)
{
    deg << LS("type:") << block.type << LS("begin position:") << block.fromPosition
        << LS("length:") << block.length;

    return deg;
}

QDebug operator<<(QDebug deg, const Global::Symbol &symbol)
{
    deg << LS("type:") << symbol.type << LS("word:") << symbol.word;

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

QString Global::getSymbolByPosition(const QString &code, int current_position)
{
    if(current_position < 0)
        return LS("");

    QRegularExpression not_word_rx(LS("[^\\w\\$]"));

    int word_begin_position = code.lastIndexOf(not_word_rx, current_position);
    int word_end_position = code.indexOf(not_word_rx, current_position);

    if(word_begin_position <= current_position && word_begin_position != word_end_position) {
        if(word_end_position == -1)
            return code.mid(word_begin_position + 1);

        return code.mid(word_begin_position + 1,
                        word_end_position - word_begin_position - 1);
    }

    return LS("");
}

Global::Symbol Global::getSymbolByString(const QString &str, int cursor_position)
{
    if(str.isEmpty())
        return Symbol{UnknowType, LS("")};

    const QList<Block> &blocks = codeToBlocks(str, cursor_position);
    const Block block = blocks.last();

    if(block.type != CodeBlock)
        return Symbol{UnknowType, LS("")};

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

    return Symbol{type, word};
}

#endif // SMARTCOMPLETIONPLUGIN_GLOBAL_H
