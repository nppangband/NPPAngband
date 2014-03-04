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
        if (zz.startsWith("0x")) {
            bool ok;
            *a = (byte)zz.toInt(&ok, 16);
        }
        else {
            *a = (byte)zz.toInt();
        }

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

static int read_number(QString text)
{
    bool ok;
    if (text.startsWith("0x")) return text.toInt(&ok, 16);
    return text.toInt();
}

/*
 * Parse a sub-file of the "extra info" (format shown below)
 *
 * Each "action" line has an "action symbol" in the first column,
 * followed by a colon, followed by some command specific info,
 * usually in the form of "tokens" separated by colons or slashes.
 *
 * Blank lines, lines starting with white space, and lines starting
 * with pound signs ("#") are ignored (as comments).
 *
 * Note the use of "tokenize()" to allow the use of both colons and
 * slashes as delimeters, while still allowing final tokens which
 * may contain any characters including "delimiters".
 *
 *
 * Note that "monster zero" is used for the "player" attr/char, "object
 * zero" will be used for the "stack" attr/char, and "feature zero" is
 * used for the "nothing" attr/char.
 *
 * Specify the attr/char values for "monsters" by race index.
 *   R:<num>:<a>/<c>
 *
 * Specify the attr/char values for "objects" by kind index.
 *   K:<num>:<a>/<c>
 *
 * Specify the attr/char values for "features" by feature index.
 *   F:<num>:<a>/<c>
 *
 * Specify the attr/char values for "special" things.
 *   S:<num>:<a>/<c>
 *
 * Specify the attribute values for inventory "objects" by kind tval.
 *   E:<tv>:<a>
 *
 * Define a macro action, given an encoded macro action.
 *   A:<str>
 *
 * Create a macro, given an encoded macro trigger.
 *   P:<str>
 *
 * Create a keymap, given an encoded keymap trigger.
 *   C:<num>:<str>
 *
 * Turn an option off, given its name.
 *   X:<str>
 *
 * Turn an option on, given its name.
 *   Y:<str>
 *
 * Turn a window flag on or off, given a window, flag, and value.
 *   W:<win>:<flag>:<value>
 *
 * Specify visual information, given an index, and some data.
 *   V:<num>:<kv>:<rv>:<gv>:<bv>
 *
 * Specify colors for message-types.
 *   M:<type>:<attr>
 *
 * Specify the attr/char values for "flavors" by flavors index.
 *   L:<num>:<a>/<c>
 */
int process_pref_file_command(QString buf)
{
    long i, j, n1, n2, sq;

    char c;
    byte a;

    QList<QString> tokens;

    /* Skip "empty" lines */
    if (buf.length() == 0) return (0);

    /* Skip "blank" lines */
    if (buf[0].isSpace()) return (0);

    /* Skip comments */
    if (buf[0] == '#') return (0);

    /* Require "?:*" format */
    if ((buf.length() < 2) || (buf[1] != ':')) return (1);

    /* Process "R:<num>:<a>/<c>" -- attr/char for monster races */
    if (buf[0] == 'R')
    {
        tokens = buf.mid(2).split(":");

        if (tokens.size() == 3)
        {
            monster_race *r_ptr;
            i = tokens.at(0).toInt();
            if ((i < 0) || (i >= (long)z_info->r_max)) return (1);
            r_ptr = &r_info[i];

            int y = read_number(tokens.at(1)) & 0x7F;
            int x = read_number(tokens.at(2)) & 0x7F;

            r_ptr->tile_id = QString("%1x%2").arg(y).arg(x);

            return (0);
        }
    }

    /* Process "F:<num>:<a>/<c>" -- attr/char for terrain features */
    else if (buf[0] == 'F')
    {
        tokens = buf.mid(2).split(":");

        if (tokens.size() == 3)
        {
            feature_type *f_ptr;
            i = tokens.at(0).toInt();
            if ((i < 0) || (i >= (long)z_info->f_max)) return (1);
            f_ptr = &f_info[i];

            int y = read_number(tokens.at(1)) & 0x7F;
            int x = read_number(tokens.at(2)) & 0x7F;

            f_ptr->tile_id = QString("%1x%2").arg(y).arg(x);

            return (0);
        }
    }

    /* Process "K:<num>:<a>/<c>"  -- attr/char for object kinds */
    else if (buf[0] == 'K')
    {
        tokens = buf.mid(2).split(":");

        if (tokens.size() == 3)
        {
            object_kind *k_ptr;
            i = tokens.at(0).toInt();
            if ((i < 0) || (i >= (long)z_info->k_max)) return (1);
            k_ptr = &k_info[i];

            int y = read_number(tokens.at(1)) & 0x7F;
            int x = read_number(tokens.at(2)) & 0x7F;

            k_ptr->tile_id = QString("%1x%2").arg(y).arg(x);

            return (0);
        }
    }

    /* Process "L:<num>:<a>/<c>" -- attr/char for flavors */
    else if (buf[0] == 'L')
    {
        tokens = buf.mid(2).split(":");

        if (tokens.size() == 3)
        {
            flavor_type *flavor_ptr;
            i = tokens.at(0).toInt();
            if ((i < 0) || (i >= (long)z_info->flavor_max)) return (1);
            flavor_ptr = &flavor_info[i];
            int y = read_number(tokens.at(1)) & 0x7F;
            int x = read_number(tokens.at(2)) & 0x7F;
            flavor_ptr->tile_id = QString("%1x%2").arg(y).arg(x);
            return (0);
        }
    }

    return 0;

#if 0
    /* Process "B:<k_idx>:inscription */
    else if (buf[0] == 'B')
    {
        if(2 == tokenize(buf + 2, 2, zz))
        {
            add_autoinscription(strtol(zz[0], NULL, 0), zz[1]);
            return (0);
        }
    }

    /* Process "Q:<idx>:<tval>:<sval>:<y|n>"  -- squelch bits   */
    /* and     "Q:<idx>:<val>"                -- squelch levels */
    /* and     "Q:<val>"                      -- auto_destroy   */
    else if (buf[0] == 'Q')
    {
        i = tokenize(buf+2, 4, zz);
        if (i == 2)
        {
            n1 = strtol(zz[0], NULL, 0);
            n2 = strtol(zz[1], NULL, 0);
            squelch_level[n1] = n2;
            return(0);
        }
        else if (i == 4)
        {
            i = strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            sq = strtol(zz[3], NULL, 0);
            if ((k_info[i].tval == n1) && (k_info[i].sval == n2))
            {
                k_info[i].squelch = sq;
                return(0);
            }
            else
            {
                for (i = 1; i < z_info->k_max; i++)
                {
                    if ((k_info[i].tval == n1) && (k_info[i].sval == n2))
                    {
                        k_info[i].squelch = sq;
                        return(0);
                    }
                }
            }
        }
    }

    /* Process "S:<num>:<a>:<c>:<a>:<c>" -- attr/char for ball spells and bolt spells */
    else if (buf[0] == 'S')
    {

        if (tokenize(buf+2, 5, zz) == 5)
        {
            long n3, n4;

            i = strtol(zz[0], NULL, 0);
            n1 = strtol(zz[1], NULL, 0);
            n2 = strtol(zz[2], NULL, 0);
            n3 = strtol(zz[3], NULL, 0);
            n4 = strtol(zz[4], NULL, 0);
            if ((i < 0) || (i > (long)MAX_COLOR_USED)) return (1);
            color_to_attr[TILE_BALL_INFO][i] = (byte)n1;
            color_to_char[TILE_BALL_INFO][i] = (char)n2;
            color_to_attr[TILE_BOLT_INFO][i] = (byte)n3;
            color_to_char[TILE_BOLT_INFO][i] = (char)n4;
            return (0);
        }
    }

    /* Process "E:<tv>:<a>" -- attribute for inventory objects */
    else if (buf[0] == 'E')
    {
        if (tokenize(buf+2, 2, zz) == 2)
        {
            i = strtol(zz[0], NULL, 0) % 128;
            n1 = strtol(zz[1], NULL, 0);
            if ((i < 0) || (i >= (long)N_ELEMENTS(tval_to_attr))) return (1);
            if (n1) tval_to_attr[i] = (byte)n1;
            return (0);
        }
    }


    /* Process "A:<str>" -- save an "action" for later */
    else if (buf[0] == 'A')
    {
        text_to_ascii(macro_buffer, sizeof(macro_buffer), buf+2);
        return (0);
    }

    /* Process "P:<str>" -- create macro */
    else if (buf[0] == 'P')
    {
        char tmp[1024];
        text_to_ascii(tmp, sizeof(tmp), buf+2);
        macro_add(tmp, macro_buffer);
        return (0);
    }

    /* Process "C:<num>:<str>" -- create keymap */
    else if (buf[0] == 'C')
    {
        long mode;

        char tmp[1024];

        if (tokenize(buf+2, 2, zz) != 2) return (1);

        mode = strtol(zz[0], NULL, 0);
        if ((mode < 0) || (mode >= KEYMAP_MODES)) return (1);

        text_to_ascii(tmp, sizeof(tmp), zz[1]);
        if (!tmp[0] || tmp[1]) return (1);
        i = (long)tmp[0];

        string_free(keymap_act[mode][i]);

        keymap_act[mode][i] = string_make(macro_buffer);

        return (0);
    }


    /* Process "V:<num>:<kv>:<rv>:<gv>:<bv>" -- visual info */
    else if (buf[0] == 'V')
    {
        if (tokenize(buf+2, 5, zz) == 5)
        {
            i = strtol(zz[0], NULL, 0);
            if ((i < 0) || (i >= 256)) return (1);
            angband_color_table[i][0] = (byte)strtol(zz[1], NULL, 0);
            angband_color_table[i][1] = (byte)strtol(zz[2], NULL, 0);
            angband_color_table[i][2] = (byte)strtol(zz[3], NULL, 0);
            angband_color_table[i][3] = (byte)strtol(zz[4], NULL, 0);
            return (0);
        }
    }

    /* set macro trigger names and a template */
    /* Process "T:<trigger>:<keycode>:<shift-keycode>" */
    /* Process "T:<template>:<modifier chr>:<modifier name>:..." */
    else if (buf[0] == 'T')
    {
        int tok;

        tok = tokenize(buf + 2, MAX_MACRO_MOD + 2, zz);

        /* Trigger template */
        if (tok >= 4)
        {
            int i;
            int num;

            /* Free existing macro triggers and trigger template */
            macro_trigger_free();

            /* Clear template done */
            if (*zz[0] == '\0') return 0;

            /* Count modifier-characters */
            num = strlen(zz[1]);

            /* One modifier-character per modifier */
            if (num + 2 != tok) return 1;

            /* Macro template */
            macro_template = string_make(zz[0]);

            /* Modifier chars */
            macro_modifier_chr = string_make(zz[1]);

            /* Modifier names */
            for (i = 0; i < num; i++)
            {
                macro_modifier_name[i] = string_make(zz[2+i]);
            }
        }
        /* Macro trigger */
        else if (tok >= 2)
        {
            char *buf;
            cptr s;
            char *t;

            if (max_macrotrigger >= MAX_MACRO_TRIGGER)
            {
                msg_print("Too many macro triggers!");
                return 1;
            }

            /* Buffer for the trigger name */
            buf = C_ZNEW(strlen(zz[0]) + 1, char);

            /* Simulate strcpy() and skip the '\' escape character */
            s = zz[0];
            t = buf;

            while (*s)
            {
                if ('\\' == *s) s++;
                *t++ = *s++;
            }

            /* Terminate the trigger name */
            *t = '\0';

            /* Store the trigger name */
            macro_trigger_name[max_macrotrigger] = string_make(buf);

            /* Free the buffer */
            FREE(buf);

            /* Normal keycode */
            macro_trigger_keycode[0][max_macrotrigger] = string_make(zz[1]);

            /* Special shifted keycode */
            if (tok == 3)
            {
                macro_trigger_keycode[1][max_macrotrigger] = string_make(zz[2]);
            }
            /* Shifted keycode is the same as the normal keycode */
            else
            {
                macro_trigger_keycode[1][max_macrotrigger] = string_make(zz[1]);
            }

            /* Count triggers */
            max_macrotrigger++;
        }

        return 0;
    }


    /* Process "X:<str>" -- turn option off */
    else if (buf[0] == 'X')
    {
        /* Check non-adult options */
        for (i = 0; i < OPT_ADULT; i++)
        {
            if (options[i].name && streq(options[i].name, buf + 2))
            {
                op_ptr->opt[i] = FALSE;
                return (0);
            }
        }

        /* Ignore unknown options */
        return (0);

    }

    /* Process "Y:<str>" -- turn option on */
    else if (buf[0] == 'Y')
    {
        /* Check non-adult options */
        for (i = 0; i < OPT_ADULT; i++)
        {
            if (options[i].name && streq(options[i].name, buf + 2))
            {
                op_ptr->opt[i] = TRUE;
                return (0);
            }
        }

        /* Ignore unknown options */
        return (0);

    }


    /* Process "W:<win>:<flag>:<value>" -- window flags */
    else if (buf[0] == 'W')
    {
        long win, flag, value;

        if (tokenize(buf + 2, 3, zz) == 3)
        {
            win = strtol(zz[0], NULL, 0);
            flag = strtol(zz[1], NULL, 0);
            value = strtol(zz[2], NULL, 0);

            /* Ignore illegal windows */
            /* Hack -- Ignore the main window */
            if ((win <= 0) || (win >= ANGBAND_TERM_MAX)) return (1);

            /* Ignore illegal flags */
            if ((flag < 0) || (flag >= 32)) return (1);

            /* Require a real flag */
            if (window_flag_desc[flag])
            {
                if (value)
                {
                    /* Turn flag on */
                    op_ptr->window_flag[win] |= (1L << flag);
                }
                else
                {
                    /* Turn flag off */
                    op_ptr->window_flag[win] &= ~(1L << flag);
                }
            }

            /* Success */
            return (0);
        }
    }


    /* Process "M:<type>:<attr>" -- colors for message-types */
    else if (buf[0] == 'M')
    {
        if (tokenize(buf+2, 2, zz) == 2)
        {
            byte color;
            /* Get tval */
            j = strtol(zz[0], NULL, 0);

            if ((j < 0) || (j >= MSG_MAX)) return (1);

            /* Handle 'r', or '4', or '0x04' for "red" */
            if (read_byte_or_char(zz[1], &a, &c))   color = a;
            else color = (byte)color_char_to_attr(c);

            message_color_define(j, color);

            /* Success */
            return (0);
        }
    }

#endif

    /* Failure */
    return (1);
}

class Parser
{
public:
    QString buffer;
    QString cur_token;
    QString saved_token;

    Parser(QString _buffer);
    bool get_token();
    QString parse();
    void unget_token();
};

void Parser::unget_token()
{
    saved_token = cur_token;
}

Parser::Parser(QString _buffer)
{
    buffer = _buffer;
}

bool Parser::get_token()
{
    // Return previous token
    if (saved_token.length() > 0) {
        cur_token = saved_token;
        saved_token.clear();
        return true;
    }

    int n = buffer.length();
    int i = 0;

    // Discard whitespace
    while ((i < n) && buffer[i].isSpace()) i++;    
    if (i >= n) return false;
    buffer = buffer.mid(i);

    // Delimiters
    if (buffer[0] == '[' || buffer[0] == ']') {
        cur_token = QString(buffer[0]);
        buffer = buffer.mid(1);
        return true;
    }

    // Collect token chars
    i = 1;
    while ((i < n) && (buffer[i] != '[') && (buffer[i] != ']') && !buffer[i].isSpace()) {
        i++;
    }

    cur_token = buffer.mid(0, i);
    buffer = buffer.mid(i);

    // Special tokens
    if (cur_token.compare("$RACE") == 0) {
        cur_token = rp_ptr->pr_name;
    }
    else if (cur_token.compare("$CLASS") == 0) {
        cur_token = cp_ptr->cl_name;
    }

    return true;
}

QString Parser::parse()
{
    if (!get_token()) return "0";

    if (cur_token[0] == '[') {
        if (!get_token()) return "0";

        QString op = cur_token;

        QList<QString> args;

        while (get_token() && (cur_token[0] != ']')) {
            unget_token();
            QString text = parse();
            args.append(text);
        }

        if (op.compare("EQU") == 0) {
            if (args.size() < 2) return "0";
            QString x = args.at(0);
            for (int i = 1; i < args.size(); i++) {
                QString y = args.at(i);
                if (x.compare(y)) return "0";
            }
            return "1";
        }

        if (op.compare("AND") == 0) {
            if (args.size() < 2) return "0";
            for (int i = 0; i < args.size(); i++) {
                QString x = args.at(i);
                if (x.compare("0") == 0) return "0";
            }
            return "1";
        }

        if (op.compare("OR") == 0) {
            if (args.size() < 2) return "0";
            for (int i = 0; i < args.size(); i++) {
                QString x = args.at(i);
                if (x.compare("0") != 0) return "1";
            }
            return "0";
        }

        if (op.compare("NOT") == 0) {
            if (args.size() != 1) return "0";
            for (int i = 0; i < args.size(); i++) {
                QString x = args.at(i);
                if (x.compare("0") == 0) return "1";
            }
            return "0";
        }

        return "0";
    }

    return cur_token;
}

/*
 * Open the "user pref file" and parse it.
 */
static int process_pref_file_aux(QString name)
{
    QFile fp(name);

    QString buf;
    QString old;

    int line = -1;

    int err = 0;

    bool bypass = FALSE;

    /* Open the file */
    if (!fp.open(QIODevice::ReadOnly)) return (-1);

    QTextStream in(&fp);

    /* Process the file */
    while (!in.atEnd())
    {
        buf = in.readLine();

        /* Count lines */
        line++;

        /* Skip "empty" lines */
        if (buf.length() == 0) continue;

        /* Skip "blank" lines */
        if (buf[0].isSpace()) continue;

        /* Skip comments */
        if (buf[0] == '#') continue;

        /* Save a copy */
        old = buf;

        /* Process "?:<expr>" */
        if (buf.startsWith("?:"))
        {
            buf = buf.mid(2);

            Parser p(buf);

            QString v = p.parse();

            /* Set flag */
            bypass = (v.compare("0") == 0);

            continue;
        }

        /* Apply conditionals */
        if (bypass) continue;

        /* Process "%:<file>" */
        if (buf.startsWith("%:"))
        {
            /* Process that file if allowed */
            (void)process_pref_file(buf.mid(2));

            /* Continue */
            continue;
        }

        /* Process the line */
        err = process_pref_file_command(buf);

        /* Oops */
        if (err) break;
    }

    /* Error */
    if (err)
    {
        /* Print error message */
        /* ToDo: Add better error messages */
        /*
        msg_format("Error %1 in line %2 of file '%3'.", err, line, name);
        msg_format("Parsing '%s'", old);
        message_flush();
        */
    }

    /* Close the file */
    fp.close();

    /* Result */
    return (err);
}

/*
 * Process the "user pref file" with the given name
 *
 * See the functions above for a list of legal "commands".
 *
 * We also accept the special "?" and "%" directives, which
 * allow conditional evaluation and filename inclusion.
 */
int process_pref_file(QString name)
{    
    QString buf(NPP_DIR_PREF);

    int err = 0;

    /* Build the filename */
    buf.append(name);

    /* Process the pref file */
    err = process_pref_file_aux(buf);

    /* Stop at parser errors, but not at non-existing file */
    if (err < 1)
    {
        /* Build the filename */
        buf = NPP_DIR_USER;
        buf.append(name);

        /* Process the pref file */
        err = process_pref_file_aux(buf);

    }

    /* Result */
    return (err);
}
