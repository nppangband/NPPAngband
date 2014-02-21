#include "npp.h"
#include <QTextStream>

/*
 * Header and footer marker string for pref file dumps
 */
static QString dump_separator("#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#");

/*
 * Allow users to supply attr and char information in decimal, hexa-
 * decimal, octal, or even character form (the last of these three
 * being most familiar to many).  -LM-
 *
 * Also accept encoded tags for extended characters.
 */
static bool read_byte_or_char(const QString &zz, byte *a, QChar *c)
{
    *a = 0;  *c = '\0';

    if (zz.length() == 0) return (TRUE);

    /* First character is a digit or a '-' */
    if (zz[0].isDigit() || (zz[0] == '-'))
    {
        /* Read as unsigned byte */
        *a = (byte)zz.toInt();

        /* We are returning a byte */
        return (TRUE);
    }

    /* Usual case -- read it as a character */
    *c = zz[0];

    /* We are returning a char */
    return (FALSE);
}

/*
 * Remove old lines from pref files
 */
static void remove_old_dump(const QString &cur_fname, const QString &mark)
{
    bool between_marks = false;
    bool changed = false;

    QString buf;

    QString start_line;
    QString end_line;

    QString new_fname;

    QFile new_file, cur_file;

    /* Format up some filenames */
    new_fname = cur_fname;
    new_fname.append(".new");

    start_line = QString("%1 begin %2").arg(dump_separator).arg(mark);
    end_line = QString("%1 end %2").arg(dump_separator).arg(mark);

    /* Open current file */
    cur_file.setFileName(cur_fname);
    if (!cur_file.open(QIODevice::ReadOnly)) return;

    /* Open new file */
    new_file.setFileName(new_fname);
    if (!new_file.open(QIODevice::WriteOnly)) {
        cur_file.close();

        message(QString("Failed to create file %1").arg(new_fname));
        return;
    }

    QTextStream in(&cur_file);
    QTextStream out(&new_file);

    /* Loop for every line */
    while (!in.atEnd()) {
        buf = in.readLine();

        /* If we find the start line, turn on */
        if (buf.compare(start_line) == 0) {
            between_marks = true;
        }

        /* If we find the finish line, turn off */
        else if (buf.compare(end_line) == 0)
        {
            between_marks = false;
            changed = true;
        }

        if (!between_marks)
        {
            /* Copy orginal line */
            out << buf << endl;
        }
    }

    /* Close files */
    cur_file.close();
    new_file.close();

    /* If there are changes, move things around */
    if (changed) {
        QString old_fname = cur_fname;
        old_fname.append(".old");

        if (cur_file.rename(old_fname)) {
            new_file.rename(cur_fname);

            cur_file.setFileName(old_fname); // Paranoia

            cur_file.remove();
        }
    }
    /* Otherwise just destroy the new file */
    else {
        new_file.remove();
    }
}

/*
 * Output the header of a pref-file dump
 */
static void pref_header(QTextStream &fff, const QString mark)
{
    /* Start of dump */
    fff << QString("%1 begin %2").arg(dump_separator).arg(mark) << endl;

    fff << QString("# *Warning!*  The lines below are an automatic dump.") << endl;
    fff << QString("# Don't edit them; changes will be deleted and replaced automatically.") << endl;
}
