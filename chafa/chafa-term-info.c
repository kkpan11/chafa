/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/* Copyright (C) 2020-2025 Hans Petter Jansson
 *
 * This file is part of Chafa, a program that shows pictures on text terminals.
 *
 * Chafa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Chafa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Chafa.  If not, see <http://www.gnu.org/licenses/>. */

#include "config.h"

#include <stdarg.h>

#include "chafa.h"
#include "internal/chafa-private.h"
#include "internal/chafa-string-util.h"

/**
 * SECTION:chafa-term-info
 * @title: ChafaTermInfo
 * @short_description: Describes a particular terminal type
 *
 * A #ChafaTermInfo describes the characteristics of one particular kind
 * of display terminal. It stores control sequences that can be used to
 * move the cursor, change text attributes, mark the beginning and end of
 * sixel graphics data, etc.
 *
 * #ChafaTermInfo also implements an efficient low-level API for formatting
 * these sequences with marshaled arguments so they can be sent to the
 * terminal.
 **/

/**
 * ChafaTermSeq:
 * @CHAFA_TERM_SEQ_RESET_TERMINAL_SOFT: Reset the terminal to configured defaults.
 * @CHAFA_TERM_SEQ_RESET_TERMINAL_HARD: Reset the terminal to factory defaults.
 * @CHAFA_TERM_SEQ_RESET_ATTRIBUTES: Reset active graphics rendition (colors and other attributes) to terminal defaults.
 * @CHAFA_TERM_SEQ_CLEAR: Clear the screen.
 * @CHAFA_TERM_SEQ_INVERT_COLORS: Invert foreground and background colors (disable with RESET_ATTRIBUTES).
 * @CHAFA_TERM_SEQ_CURSOR_TO_TOP_LEFT: Move cursor to top left of screen.
 * @CHAFA_TERM_SEQ_CURSOR_TO_BOTTOM_LEFT: Move cursor to bottom left of screen.
 * @CHAFA_TERM_SEQ_CURSOR_TO_POS: Move cursor to specific position.
 * @CHAFA_TERM_SEQ_CURSOR_UP_1: Move cursor up one cell.
 * @CHAFA_TERM_SEQ_CURSOR_UP: Move cursor up N cells.
 * @CHAFA_TERM_SEQ_CURSOR_DOWN_1: Move cursor down one cell.
 * @CHAFA_TERM_SEQ_CURSOR_DOWN: Move cursor down N cells.
 * @CHAFA_TERM_SEQ_CURSOR_LEFT_1: Move cursor left one cell.
 * @CHAFA_TERM_SEQ_CURSOR_LEFT: Move cursor left N cells.
 * @CHAFA_TERM_SEQ_CURSOR_RIGHT_1: Move cursor right one cell.
 * @CHAFA_TERM_SEQ_CURSOR_RIGHT: Move cursor right N cells.
 * @CHAFA_TERM_SEQ_CURSOR_UP_SCROLL: Move cursor up one cell. Scroll area contents down when at the edge.
 * @CHAFA_TERM_SEQ_CURSOR_DOWN_SCROLL: Move cursor down one cell. Scroll area contents up when at the edge.
 * @CHAFA_TERM_SEQ_INSERT_CELLS: Insert blank cells at cursor position.
 * @CHAFA_TERM_SEQ_DELETE_CELLS: Delete cells at cursor position.
 * @CHAFA_TERM_SEQ_INSERT_ROWS: Insert rows at cursor position.
 * @CHAFA_TERM_SEQ_DELETE_ROWS: Delete rows at cursor position.
 * @CHAFA_TERM_SEQ_SET_SCROLLING_ROWS: Set scrolling area extents.
 * @CHAFA_TERM_SEQ_ENABLE_INSERT: Enable insert mode.
 * @CHAFA_TERM_SEQ_DISABLE_INSERT: Disable insert mode.
 * @CHAFA_TERM_SEQ_ENABLE_CURSOR: Show the cursor.
 * @CHAFA_TERM_SEQ_DISABLE_CURSOR: Hide the cursor.
 * @CHAFA_TERM_SEQ_ENABLE_ECHO: Make the terminal echo input locally.
 * @CHAFA_TERM_SEQ_DISABLE_ECHO: Don't echo input locally.
 * @CHAFA_TERM_SEQ_ENABLE_WRAP: Make cursor wrap around to the next row after output in the final column.
 * @CHAFA_TERM_SEQ_DISABLE_WRAP: Make cursor stay in place after output to the final column.
 * @CHAFA_TERM_SEQ_SET_COLOR_FG_DIRECT: Set foreground color (directcolor/truecolor).
 * @CHAFA_TERM_SEQ_SET_COLOR_BG_DIRECT: Set background color (directcolor/truecolor).
 * @CHAFA_TERM_SEQ_SET_COLOR_FGBG_DIRECT: Set foreground and background color (directcolor/truecolor).
 * @CHAFA_TERM_SEQ_SET_COLOR_FG_256: Set foreground color (256 colors).
 * @CHAFA_TERM_SEQ_SET_COLOR_BG_256: Set background color (256 colors).
 * @CHAFA_TERM_SEQ_SET_COLOR_FGBG_256: Set foreground and background colors (256 colors).
 * @CHAFA_TERM_SEQ_SET_COLOR_FG_16: Set foreground color (16 colors).
 * @CHAFA_TERM_SEQ_SET_COLOR_BG_16: Set background color (16 colors).
 * @CHAFA_TERM_SEQ_SET_COLOR_FGBG_16: Set foreground and background colors (16 colors).
 * @CHAFA_TERM_SEQ_BEGIN_SIXELS: Begin sixel image data.
 * @CHAFA_TERM_SEQ_END_SIXELS: End sixel image data.
 * @CHAFA_TERM_SEQ_REPEAT_CHAR: Repeat previous character N times.
 * @CHAFA_TERM_SEQ_BEGIN_KITTY_IMMEDIATE_IMAGE_V1: Begin upload of Kitty image for immediate display at cursor.
 * @CHAFA_TERM_SEQ_END_KITTY_IMAGE: End of Kitty image upload.
 * @CHAFA_TERM_SEQ_BEGIN_KITTY_IMAGE_CHUNK: Begin Kitty image data chunk.
 * @CHAFA_TERM_SEQ_END_KITTY_IMAGE_CHUNK: End Kitty image data chunk.
 * @CHAFA_TERM_SEQ_BEGIN_ITERM2_IMAGE: Begin iTerm2 image data.
 * @CHAFA_TERM_SEQ_END_ITERM2_IMAGE: End of iTerm2 image data.
 * @CHAFA_TERM_SEQ_ENABLE_SIXEL_SCROLLING: Enable sixel scrolling.
 * @CHAFA_TERM_SEQ_DISABLE_SIXEL_SCROLLING: Disable sixel scrolling.
 * @CHAFA_TERM_SEQ_ENABLE_BOLD: Enable boldface (disable with RESET_ATTRIBUTES).
 * @CHAFA_TERM_SEQ_SET_COLOR_FG_8: Set foreground color (8 colors).
 * @CHAFA_TERM_SEQ_SET_COLOR_BG_8: Set background color (8 colors).
 * @CHAFA_TERM_SEQ_SET_COLOR_FGBG_8: Set foreground and background colors (8 colors).
 * @CHAFA_TERM_SEQ_RESET_DEFAULT_FG: Reset the default FG color to the terminal's default.
 * @CHAFA_TERM_SEQ_SET_DEFAULT_FG: Set the default FG to a specific RGB color.
 * @CHAFA_TERM_SEQ_QUERY_DEFAULT_FG: Query the default FG color.
 * @CHAFA_TERM_SEQ_RESET_DEFAULT_BG: Reset the default BG color to the terminal's default.
 * @CHAFA_TERM_SEQ_SET_DEFAULT_BG: Set the default BG to a specific RGB color.
 * @CHAFA_TERM_SEQ_QUERY_DEFAULT_BG: Query the default BG color.
 * @CHAFA_TERM_SEQ_QUERY_PRIMARY_DEVICE_ATTRIBUTES: Query the primary device attributes.
 * @CHAFA_TERM_SEQ_PRIMARY_DEVICE_ATTRIBUTES: Indicate the primary device attributes.
 * @CHAFA_TERM_SEQ_QUERY_CELL_SIZE_PX: Query the character cell size in pixels.
 * @CHAFA_TERM_SEQ_CELL_SIZE_PX: Indicate the character cell size in pixels.
 * @CHAFA_TERM_SEQ_QUERY_TEXT_AREA_SIZE_CELLS: Query the text area size in character cells.
 * @CHAFA_TERM_SEQ_TEXT_AREA_SIZE_CELLS: Indicate the text area size in character cells.
 * @CHAFA_TERM_SEQ_QUERY_TEXT_AREA_SIZE_PX: Query the text area size in pixels.
 * @CHAFA_TERM_SEQ_TEXT_AREA_SIZE_PX: Indicate the text area size in pixels.
 * @CHAFA_TERM_SEQ_RETURN_KEY: Return key.
 * @CHAFA_TERM_SEQ_BACKSPACE_KEY: Backspace key.
 * @CHAFA_TERM_SEQ_TAB_KEY: Tab key.
 * @CHAFA_TERM_SEQ_TAB_SHIFT_KEY: Shift + tab key.
 * @CHAFA_TERM_SEQ_UP_KEY: Up key.
 * @CHAFA_TERM_SEQ_UP_CTRL_KEY: Ctrl + up key.
 * @CHAFA_TERM_SEQ_UP_SHIFT_KEY: Shift + up key.
 * @CHAFA_TERM_SEQ_DOWN_KEY: Down key.
 * @CHAFA_TERM_SEQ_DOWN_CTRL_KEY: Ctrl + down key.
 * @CHAFA_TERM_SEQ_DOWN_SHIFT_KEY: Shift + down key.
 * @CHAFA_TERM_SEQ_LEFT_KEY: Left key.
 * @CHAFA_TERM_SEQ_LEFT_CTRL_KEY: Ctrl + left key.
 * @CHAFA_TERM_SEQ_LEFT_SHIFT_KEY: Shift + left key.
 * @CHAFA_TERM_SEQ_RIGHT_KEY: Right key.
 * @CHAFA_TERM_SEQ_RIGHT_CTRL_KEY: Ctrl + right key.
 * @CHAFA_TERM_SEQ_RIGHT_SHIFT_KEY: Shift + right key.
 * @CHAFA_TERM_SEQ_PAGE_UP_KEY: Page up key.
 * @CHAFA_TERM_SEQ_PAGE_UP_CTRL_KEY: Ctrl + page up key.
 * @CHAFA_TERM_SEQ_PAGE_UP_SHIFT_KEY: Shift + page up key.
 * @CHAFA_TERM_SEQ_PAGE_DOWN_KEY: Page down key.
 * @CHAFA_TERM_SEQ_PAGE_DOWN_CTRL_KEY: Ctrl + page down key.
 * @CHAFA_TERM_SEQ_PAGE_DOWN_SHIFT_KEY: Shift + page down key.
 * @CHAFA_TERM_SEQ_HOME_KEY: Home key.
 * @CHAFA_TERM_SEQ_HOME_CTRL_KEY: Ctrl + home key.
 * @CHAFA_TERM_SEQ_HOME_SHIFT_KEY: Shift + home key.
 * @CHAFA_TERM_SEQ_END_KEY: End key.
 * @CHAFA_TERM_SEQ_END_CTRL_KEY: Ctrl + end key.
 * @CHAFA_TERM_SEQ_END_SHIFT_KEY: Shift + end key.
 * @CHAFA_TERM_SEQ_INSERT_KEY: Insert key.
 * @CHAFA_TERM_SEQ_INSERT_CTRL_KEY: Ctrl + insert key.
 * @CHAFA_TERM_SEQ_INSERT_SHIFT_KEY: Shift + insert key.
 * @CHAFA_TERM_SEQ_DELETE_KEY: Delete key.
 * @CHAFA_TERM_SEQ_DELETE_CTRL_KEY: Ctrl + delete key.
 * @CHAFA_TERM_SEQ_DELETE_SHIFT_KEY: Shift + delete key.
 * @CHAFA_TERM_SEQ_F1_KEY: F1 key.
 * @CHAFA_TERM_SEQ_F1_CTRL_KEY: Ctrl + F1 key.
 * @CHAFA_TERM_SEQ_F1_SHIFT_KEY: Shift + F1 key.
 * @CHAFA_TERM_SEQ_F2_KEY: F2 key.
 * @CHAFA_TERM_SEQ_F2_CTRL_KEY: Ctrl + F2 key.
 * @CHAFA_TERM_SEQ_F2_SHIFT_KEY: Shift + F2 key.
 * @CHAFA_TERM_SEQ_F3_KEY: F3 key.
 * @CHAFA_TERM_SEQ_F3_CTRL_KEY: Ctrl + F3 key.
 * @CHAFA_TERM_SEQ_F3_SHIFT_KEY: Shift + F3 key.
 * @CHAFA_TERM_SEQ_F4_KEY: F4 key.
 * @CHAFA_TERM_SEQ_F4_CTRL_KEY: Ctrl + F4 key.
 * @CHAFA_TERM_SEQ_F4_SHIFT_KEY: Shift + F4 key.
 * @CHAFA_TERM_SEQ_F5_KEY: F5 key.
 * @CHAFA_TERM_SEQ_F5_CTRL_KEY: Ctrl + F5 key.
 * @CHAFA_TERM_SEQ_F5_SHIFT_KEY: Shift + F5 key.
 * @CHAFA_TERM_SEQ_F6_KEY: F6 key.
 * @CHAFA_TERM_SEQ_F6_CTRL_KEY: Ctrl + F6 key.
 * @CHAFA_TERM_SEQ_F6_SHIFT_KEY: Shift + F6 key.
 * @CHAFA_TERM_SEQ_F7_KEY: F7 key.
 * @CHAFA_TERM_SEQ_F7_CTRL_KEY: Ctrl + F7 key.
 * @CHAFA_TERM_SEQ_F7_SHIFT_KEY: Shift + F7 key.
 * @CHAFA_TERM_SEQ_F8_KEY: F8 key.
 * @CHAFA_TERM_SEQ_F8_CTRL_KEY: Ctrl + F8 key.
 * @CHAFA_TERM_SEQ_F8_SHIFT_KEY: Shift + F8 key.
 * @CHAFA_TERM_SEQ_F9_KEY: F9 key.
 * @CHAFA_TERM_SEQ_F9_CTRL_KEY: Ctrl + F9 key.
 * @CHAFA_TERM_SEQ_F9_SHIFT_KEY: Shift + F9 key.
 * @CHAFA_TERM_SEQ_F10_KEY: F10 key.
 * @CHAFA_TERM_SEQ_F10_CTRL_KEY: Ctrl + F10 key.
 * @CHAFA_TERM_SEQ_F10_SHIFT_KEY: Shift + F10 key.
 * @CHAFA_TERM_SEQ_F11_KEY: F11 key.
 * @CHAFA_TERM_SEQ_F11_CTRL_KEY: Ctrl + F11 key.
 * @CHAFA_TERM_SEQ_F11_SHIFT_KEY: Shift + F11 key.
 * @CHAFA_TERM_SEQ_F12_KEY: F12 key.
 * @CHAFA_TERM_SEQ_F12_CTRL_KEY: Ctrl + F12 key.
 * @CHAFA_TERM_SEQ_F12_SHIFT_KEY: Shift + F12 key.
 * @CHAFA_TERM_SEQ_RESET_COLOR_FG: Reset foreground color to the default.
 * @CHAFA_TERM_SEQ_RESET_COLOR_BG: Reset background color to the default.
 * @CHAFA_TERM_SEQ_RESET_COLOR_FGBG: Reset foreground and background colors to the default.
 * @CHAFA_TERM_SEQ_RESET_SCROLLING_ROWS: Reset scrolling area extent to that of the entire screen.
 * @CHAFA_TERM_SEQ_SAVE_CURSOR_POS: Save the cursor's position.
 * @CHAFA_TERM_SEQ_RESTORE_CURSOR_POS: Move cursor to the last saved position.
 * @CHAFA_TERM_SEQ_SET_SIXEL_ADVANCE_DOWN: After showing a sixel image, leave cursor somewhere on the row below the image.
 * @CHAFA_TERM_SEQ_SET_SIXEL_ADVANCE_RIGHT: After showing a sixel image, leave cursor to the right of the last row.
 * @CHAFA_TERM_SEQ_ENABLE_ALT_SCREEN: Switch to the alternate screen buffer.
 * @CHAFA_TERM_SEQ_DISABLE_ALT_SCREEN: Switch back to the regular screen buffer.
 * @CHAFA_TERM_SEQ_BEGIN_SCREEN_PASSTHROUGH: Begins OSC passthrough for the GNU Screen multiplexer.
 * @CHAFA_TERM_SEQ_END_SCREEN_PASSTHROUGH: Ends OSC passthrough for the GNU Screen multiplexer.
 * @CHAFA_TERM_SEQ_BEGIN_TMUX_PASSTHROUGH: Begins OSC passthrough for the tmux multiplexer.
 * @CHAFA_TERM_SEQ_END_TMUX_PASSTHROUGH: Ends OSC passthrough for the tmux multiplexer.
 * @CHAFA_TERM_SEQ_BEGIN_KITTY_IMMEDIATE_VIRT_IMAGE_V1: Begin upload of virtual Kitty image for immediate display at cursor.
 * @CHAFA_TERM_SEQ_QUERY_PRIMARY_DEVICE_ATTRIBUTES: Queries the terminal for its primary device attributes.
 * @CHAFA_TERM_SEQ_PRIMARY_DEVICE_ATTRIBUTES: The terminal's primary device attributes.
 * @CHAFA_TERM_SEQ_MAX: Last control sequence plus one.
 *
 * An enumeration of the control sequences supported by #ChafaTermInfo.
 **/

#define ARG_INDEX_SENTINEL 255

typedef struct
{
    guint8 is_varargs : 1;
    guint8 pre_len : 7;
    guint8 arg_index;
}
SeqArgInfo;

struct ChafaTermInfo
{
    gint refs;
    gchar *name;
    gchar seq_str [CHAFA_TERM_SEQ_MAX] [CHAFA_TERM_SEQ_LENGTH_MAX];
    SeqArgInfo seq_args [CHAFA_TERM_SEQ_MAX] [CHAFA_TERM_SEQ_ARGS_MAX];
    gchar *unparsed_str [CHAFA_TERM_SEQ_MAX];
    guint8 pixel_passthrough_needed [CHAFA_PIXEL_MODE_MAX];
    guint8 inherit_seq [CHAFA_TERM_SEQ_MAX];
    ChafaTermQuirks quirks;
    ChafaSymbolTags safe_symbol_tags;
};

typedef enum
{
    SEQ_TYPE_FIXARGS,
    SEQ_TYPE_VARARGS
}
SeqType;

typedef struct
{
    guint seq_type : 8;
    guint n_args : 8;
    guint type_size : 8;
}
SeqMeta;

static const SeqMeta seq_meta [CHAFA_TERM_SEQ_MAX] =
{
#define CHAFA_TERM_SEQ_DEF(name, NAME, n_args, arg_proc, arg_type, ...)  \
    [CHAFA_TERM_SEQ_##NAME] = { SEQ_TYPE_FIXARGS, n_args, sizeof (arg_type) },
#define CHAFA_TERM_SEQ_DEF_VARARGS(name, NAME, arg_type)  \
    [CHAFA_TERM_SEQ_##NAME] = { SEQ_TYPE_VARARGS, CHAFA_TERM_SEQ_ARGS_MAX - 1, sizeof (arg_type) },
#include <chafa-term-seq-def.h>
#undef CHAFA_TERM_SEQ_DEF
#undef CHAFA_TERM_SEQ_DEF_VARARGS
};

/* Avoid memcpy() because it inlines to a sizeable amount of code (it
 * doesn't know our strings are always short). We also take the liberty
 * of unconditionally copying a byte even if n=0. This simplifies the
 * generated assembly quite a bit. */
static inline void
copy_bytes (gchar *out, const gchar *in, gint n)
{
    gint i = 0;

    do
    {
        *(out++) = *(in++);
        i++;
    }
    while (i < n);
}

#if 0
static void
clear_seq (ChafaTermInfo *ti, gint seq)
{
    g_free (ti->unparsed_str [seq]);
    ti->unparsed_str [seq] = NULL;

    memset (&ti->seq_str [seq] [0], 0, CHAFA_TERM_SEQ_LENGTH_MAX);
    memset (&ti->seq_args [seq] [0], 0, CHAFA_TERM_SEQ_ARGS_MAX * sizeof (SeqArgInfo));
}
#endif

static void
copy_seq (const ChafaTermInfo *src, ChafaTermInfo *dest, gint seq)
{
    g_free (dest->unparsed_str [seq]);
    dest->unparsed_str [seq] = NULL;

    if (src->unparsed_str [seq])
        dest->unparsed_str [seq] = g_strdup (src->unparsed_str [seq]);
    memcpy (&dest->seq_str [seq] [0], &src->seq_str [seq] [0],
            CHAFA_TERM_SEQ_LENGTH_MAX);
    memcpy (&dest->seq_args [seq] [0], &src->seq_args [seq] [0],
            CHAFA_TERM_SEQ_ARGS_MAX * sizeof (SeqArgInfo));

    dest->inherit_seq [seq] = src->inherit_seq [seq];
}

static gboolean
parse_seq_args (gchar *out, SeqArgInfo *arg_info, const gchar *in,
                gint n_args, gint arg_len_max, GError **error)
{
    gint i, j, k;
    gint pre_len = 0;
    gint result = FALSE;

    g_assert (n_args < CHAFA_TERM_SEQ_ARGS_MAX);

    for (k = 0; k < CHAFA_TERM_SEQ_ARGS_MAX; k++)
    {
        arg_info [k].is_varargs = FALSE;
        arg_info [k].pre_len = 0;
        arg_info [k].arg_index = ARG_INDEX_SENTINEL;
    }

    for (i = 0, j = 0, k = 0;
         j < CHAFA_TERM_SEQ_LENGTH_MAX && k < CHAFA_TERM_SEQ_ARGS_MAX && in [i];
         i++)
    {
        gchar c = in [i];

        if (c == '%')
        {
            i++;
            c = in [i];

            if (c == '%')
            {
                /* "%%" -> literal "%" */
                out [j++] = '%';
                pre_len++;
            }
            else if (c >= '1' && c <= '7')  /* arg # 0-6 */
            {
                /* "%n" -> argument n-1 */

                arg_info [k].arg_index = c - '1';
                arg_info [k].pre_len = pre_len;

                if (arg_info [k].arg_index >= n_args)
                {
                    /* Bad argument index (out of range) */
                    g_set_error (error, CHAFA_TERM_INFO_ERROR, CHAFA_TERM_INFO_ERROR_BAD_ARGUMENTS,
                                 "Control sequence had too many arguments.");
                    goto out;
                }

                pre_len = 0;
                k++;
            }
            else if (c == 'v')
            {
                /* Variable number of arguments separated by ';' */

                arg_info [k].is_varargs = TRUE;
                arg_info [k].arg_index = 0;  /* FIXME: Should be highest seen + 1 */
                arg_info [k].pre_len = pre_len;

                if (arg_info [k].arg_index >= CHAFA_TERM_SEQ_ARGS_MAX)
                {
                    /* Bad argument index (out of range) */
                    g_set_error (error, CHAFA_TERM_INFO_ERROR, CHAFA_TERM_INFO_ERROR_BAD_ARGUMENTS,
                                 "Control sequence had too many arguments.");
                    goto out;
                }

                pre_len = 0;
                k++;
            }
            else
            {
                /* Bad "%?" escape */
                goto out;
            }
        }
        else
        {
            out [j++] = c;
            pre_len++;
        }
    }

    if (k == CHAFA_TERM_SEQ_ARGS_MAX)
    {
        /* Too many argument expansions */
        g_set_error (error, CHAFA_TERM_INFO_ERROR, CHAFA_TERM_INFO_ERROR_BAD_ARGUMENTS,
                     "Control sequence had too many arguments.");
        goto out;
    }

    /* Reserve an extra byte for copy_byte() and chafa_format_dec_u8() excess. */
    if (j + k * arg_len_max + 1 > CHAFA_TERM_SEQ_LENGTH_MAX)
    {
        /* Formatted string may be too long */
        g_set_error (error, CHAFA_TERM_INFO_ERROR, CHAFA_TERM_INFO_ERROR_SEQ_TOO_LONG,
                     "Control sequence too long.");
        goto out;
    }

    arg_info [k].pre_len = pre_len;
    arg_info [k].arg_index = ARG_INDEX_SENTINEL;

    result = TRUE;

out:
    return result;
}

#define EMIT_SEQ_DEF(name, inttype, intformatter)                       \
    static gchar *                                                      \
    emit_seq_##name (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, \
                     inttype *args, gint n_args)                        \
    {                                                                   \
        const gchar *seq_str;                                           \
        const SeqArgInfo *seq_args;                                     \
        gint ofs = 0;                                                   \
        gint i;                                                         \
                                                                        \
        seq_str = &term_info->seq_str [seq] [0];                        \
        seq_args = &term_info->seq_args [seq] [0];                      \
                                                                        \
        if (seq_args [0].arg_index == ARG_INDEX_SENTINEL)               \
            return out;                                                 \
                                                                        \
        for (i = 0; i < n_args; i++)                                    \
        {                                                               \
            copy_bytes (out, &seq_str [ofs], seq_args [i].pre_len);     \
            out += seq_args [i].pre_len;                                \
            ofs += seq_args [i].pre_len;                                \
            out = intformatter (out, args [seq_args [i].arg_index]);    \
        }                                                               \
                                                                        \
        copy_bytes (out, &seq_str [ofs], seq_args [i].pre_len);         \
        out += seq_args [i].pre_len;                                    \
                                                                        \
        return out;                                                     \
    }

#define EMIT_VARARGS_SEQ_DEF(name, inttype, intformatter)               \
    static gchar *                                                      \
    emit_seq_##name (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, \
                     inttype *args, gint n_args)                        \
    {                                                                   \
        const gchar *seq_str;                                           \
        const SeqArgInfo *seq_args;                                     \
        gboolean found_varargs = FALSE;                                 \
        gint ofs = 0;                                                   \
        gint i, j;                                                      \
                                                                        \
        seq_str = &term_info->seq_str [seq] [0];                        \
        seq_args = &term_info->seq_args [seq] [0];                      \
                                                                        \
        if (seq_args [0].arg_index == ARG_INDEX_SENTINEL)               \
            return out;                                                 \
                                                                        \
        for (i = 0, j = 0; i + j < n_args; )                            \
        {                                                               \
            if (found_varargs)                                          \
            {                                                           \
                *(out++) = ';';                                         \
            }                                                           \
            else                                                        \
            {                                                           \
                if (seq_args [i].is_varargs)                            \
                    found_varargs = TRUE;                               \
                copy_bytes (out, &seq_str [ofs], seq_args [i].pre_len); \
                out += seq_args [i].pre_len;                            \
                ofs += seq_args [i].pre_len;                            \
            }                                                           \
                                                                        \
            out = intformatter (out, args [seq_args [i].arg_index + j]);\
                                                                        \
            if (found_varargs)                                          \
                j++;                                                    \
            else                                                        \
                i++;                                                    \
        }                                                               \
                                                                        \
        if (found_varargs)                                              \
            i++;                                                        \
                                                                        \
        copy_bytes (out, &seq_str [ofs], seq_args [i].pre_len);         \
        out += seq_args [i].pre_len;                                    \
                                                                        \
        return out;                                                     \
    }

EMIT_SEQ_DEF(guint, guint, chafa_format_dec_uint_0_to_9999)
EMIT_SEQ_DEF(guint8, guint8, chafa_format_dec_u8)
EMIT_SEQ_DEF(guint16_hex, guint16, chafa_format_dec_u16_hex)
EMIT_VARARGS_SEQ_DEF(guint_varargs, guint, chafa_format_dec_uint_0_to_9999)

static gchar *
emit_seq_0_args_uint (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq)
{
    copy_bytes (out, &term_info->seq_str [seq] [0], term_info->seq_args [seq] [0].pre_len);
    return out + term_info->seq_args [seq] [0].pre_len;
}

static gchar *
emit_seq_1_args_uint (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint arg0)
{
    return emit_seq_guint (term_info, out, seq, &arg0, 1);
}

static gchar *
emit_seq_1_args_uint8 (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint8 arg0)
{
    return emit_seq_guint8 (term_info, out, seq, &arg0, 1);
}

static gchar *
emit_seq_2_args_uint (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint arg0, guint arg1)
{
    guint args [2];

    args [0] = arg0;
    args [1] = arg1;
    return emit_seq_guint (term_info, out, seq, args, 2);
}

static gchar *
emit_seq_2_args_uint8 (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint arg0, guint arg1)
{
    guint8 args [2];

    args [0] = arg0;
    args [1] = arg1;
    return emit_seq_guint8 (term_info, out, seq, args, 2);
}

static gchar *
emit_seq_3_args_uint (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint arg0, guint arg1, guint arg2)
{
    guint args [3];

    args [0] = arg0;
    args [1] = arg1;
    args [2] = arg2;
    return emit_seq_guint (term_info, out, seq, args, 3);
}

static gchar *
emit_seq_5_args_uint (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint arg0, guint arg1, guint arg2, guint arg3, guint arg4)
{
    guint args [5];

    args [0] = arg0;
    args [1] = arg1;
    args [2] = arg2;
    args [3] = arg3;
    args [4] = arg4;
    return emit_seq_guint (term_info, out, seq, args, 5);
}

static gchar *
emit_seq_6_args_uint (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint arg0, guint arg1, guint arg2, guint arg3, guint arg4, guint arg5)
{
    guint args [6];

    args [0] = arg0;
    args [1] = arg1;
    args [2] = arg2;
    args [3] = arg3;
    args [4] = arg4;
    args [5] = arg5;
    return emit_seq_guint (term_info, out, seq, args, 6);
}

static gchar *
emit_seq_3_args_uint8 (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint8 arg0, guint8 arg1, guint8 arg2)
{
    guint8 args [3];

    args [0] = arg0;
    args [1] = arg1;
    args [2] = arg2;
    return emit_seq_guint8 (term_info, out, seq, args, 3);
}

static gchar *
emit_seq_6_args_uint8 (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint8 arg0, guint8 arg1, guint8 arg2, guint8 arg3, guint8 arg4, guint8 arg5)
{
    guint8 args [6];

    args [0] = arg0;
    args [1] = arg1;
    args [2] = arg2;
    args [3] = arg3;
    args [4] = arg4;
    args [5] = arg5;
    return emit_seq_guint8 (term_info, out, seq, args, 6);
}

static gchar *
emit_seq_3_args_uint16_hex (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint16 arg0, guint16 arg1, guint16 arg2)
{
    guint16 args [3];

    args [0] = arg0;
    args [1] = arg1;
    args [2] = arg2;
    return emit_seq_guint16_hex (term_info, out, seq, args, 3);
}

static gchar *
emit_seq_varargs_uint (const ChafaTermInfo *term_info, gchar *out, ChafaTermSeq seq, guint *args, gint n_args)
{
    return emit_seq_guint_varargs (term_info, out, seq, args, n_args);
}

/* Stream parsing */

static gint
parse_dec (const gchar *in, gint in_len, guint *args_out)
{
    gint i = 0;
    guint result = 0;

    while (in_len > 0 && *in >= '0' && *in <= '9')
    {
        result *= 10;
        result += *in - '0';
        in++;
        in_len--;
        i++;
    }

    *args_out = result;
    return i;
}

static gint
parse_hex4 (const gchar *in, gint in_len, guint *args_out)
{
    gint i = 0;
    guint result = 0;

    while (in_len > 0)
    {
        gchar c = g_ascii_tolower (*in);

        if (c >= '0' && c <= '9')
        {
            result *= 16;
            result += c - '0';
        }
        else if (c >= 'a' && c <= 'f')
        {
            result *= 16;
            result += c - 'a' + 10;
        }
        else
            break;

        in++;
        in_len--;
        i++;
    }

    *args_out = result;
    return i;
}

static ChafaParseResult
try_parse_seq (const ChafaTermInfo *term_info, ChafaTermSeq seq,
               gchar **input, gint *input_len, guint *args_out, gint *n_args_out)
{
    gchar *in = *input;
    gint in_len = *input_len;
    const gchar *seq_str;
    const SeqArgInfo *seq_args;
    gboolean parsed_varargs = FALSE;
    gint pofs = 0;
    guint i = 0;

    seq_str = &term_info->seq_str [seq] [0];
    seq_args = &term_info->seq_args [seq] [0];

    memset (args_out, 0, seq_meta [seq].n_args * sizeof (guint));

    for ( ; ; i++)
    {
        gint len;
        guint j;

        if (memcmp (in, &seq_str [pofs], MIN (in_len, seq_args [i].pre_len)))
            return CHAFA_PARSE_FAILURE;
        if (in_len < seq_args [i].pre_len)
            return CHAFA_PARSE_AGAIN;
        in += seq_args [i].pre_len;
        in_len -= seq_args [i].pre_len;
        pofs += seq_args [i].pre_len;

        if (parsed_varargs || i >= seq_meta [seq].n_args)
            break;

        if (seq_args [i].is_varargs)
            parsed_varargs = TRUE;

        for (j = 0; ; j++)
        {
            guint arg_ofs = seq_args [i].arg_index + j;

            if (arg_ofs > CHAFA_TERM_SEQ_ARGS_MAX - 1)
                return CHAFA_PARSE_FAILURE;
            if (in_len == 0)
                return CHAFA_PARSE_AGAIN;

            *n_args_out = arg_ofs + 1;

            if (seq_meta [seq].type_size == 1)
                len = parse_dec (in, in_len, args_out + arg_ofs);
            else if (seq_meta [seq].type_size == 2)
                len = parse_hex4 (in, in_len, args_out + arg_ofs);
            else
                len = parse_dec (in, in_len, args_out + arg_ofs);

            /* Blank args are interpreted to mean zero */
            if (len == 0)
                *(args_out + arg_ofs) = 0;

            in += len;
            in_len -= len;

            if (!seq_args [i].is_varargs)
                break;
            if (in_len > 0)
            {
                if (*in != ';')
                    break;
                in++;
                in_len--;
            }
        }
    }

    if (*input == in)
        return CHAFA_PARSE_FAILURE;

    *input = in;
    *input_len = in_len;
    return CHAFA_PARSE_SUCCESS;
}

/* Public */

G_DEFINE_QUARK (chafa-term-info-error-quark, chafa_term_info_error)

/**
 * chafa_term_info_new:
 *
 * Creates a new, blank #ChafaTermInfo.
 *
 * Returns: The new #ChafaTermInfo
 *
 * Since: 1.6
 **/
ChafaTermInfo *
chafa_term_info_new (void)
{
    ChafaTermInfo *term_info;
    gint i;

    term_info = g_new0 (ChafaTermInfo, 1);
    term_info->refs = 1;

    for (i = 0; i < CHAFA_TERM_SEQ_MAX; i++)
    {
        term_info->seq_args [i] [0].arg_index = ARG_INDEX_SENTINEL;
    }

    return term_info;
}

/**
 * chafa_term_info_copy:
 * @term_info: A #ChafaTermInfo to copy.
 *
 * Creates a new #ChafaTermInfo that's a copy of @term_info.
 *
 * Returns: The new #ChafaTermInfo
 *
 * Since: 1.6
 **/
ChafaTermInfo *
chafa_term_info_copy (const ChafaTermInfo *term_info)
{
    ChafaTermInfo *new_term_info;
    gint i;

    g_return_val_if_fail (term_info != NULL, NULL);

    new_term_info = g_new (ChafaTermInfo, 1);
    memcpy (new_term_info, term_info, sizeof (ChafaTermInfo));
    new_term_info->refs = 1;

    for (i = 0; i < CHAFA_TERM_SEQ_MAX; i++)
    {
        if (new_term_info->unparsed_str [i])
            new_term_info->unparsed_str [i] = g_strdup (new_term_info->unparsed_str [i]);
    }

    new_term_info->name = g_strdup (new_term_info->name);

    return new_term_info;
}

/**
 * chafa_term_info_ref:
 * @term_info: #ChafaTermInfo to add a reference to.
 *
 * Adds a reference to @term_info.
 *
 * Since: 1.6
 **/
void
chafa_term_info_ref (ChafaTermInfo *term_info)
{
    gint refs;

    g_return_if_fail (term_info != NULL);
    refs = g_atomic_int_get (&term_info->refs);
    g_return_if_fail (refs > 0);

    g_atomic_int_inc (&term_info->refs);
}

/**
 * chafa_term_info_unref:
 * @term_info: #ChafaTermInfo to remove a reference from.
 *
 * Removes a reference from @term_info.
 *
 * Since: 1.6
 **/
void
chafa_term_info_unref (ChafaTermInfo *term_info)
{
    gint refs;

    g_return_if_fail (term_info != NULL);
    refs = g_atomic_int_get (&term_info->refs);
    g_return_if_fail (refs > 0);

    if (g_atomic_int_dec_and_test (&term_info->refs))
    {
        gint i;

        for (i = 0; i < CHAFA_TERM_SEQ_MAX; i++)
            g_free (term_info->unparsed_str [i]);

        g_free (term_info->name);
        g_free (term_info);
    }
}

/**
 * chafa_term_info_get_name:
 * @term_info: A #ChafaTermInfo
 *
 * Gets the name associated with the @term_info. This may be NULL.
 * The returned string belongs to @term_info, and is only valid until the
 * next operation on this data structure. Make a copy using g_strdup() if
 * you need to keep it around.
 *
 * Returns: The name for this #ChafaTermInfo, or %NULL
 *
 * Since: 1.16
 **/
const gchar *
chafa_term_info_get_name (ChafaTermInfo *term_info)
{
    g_return_val_if_fail (term_info != NULL, NULL);

    return term_info->name;
}

/**
 * chafa_term_info_set_name:
 * @term_info: A #ChafaTermInfo
 * @name: The new name for this #ChafaTermInfo, or %NULL
 *
 * Assigns a new name to @term_info. The name should be a short lowercase
 * ASCII string that uniquely identifies the terminal or program described
 * by @term_info.
 *
 * Since: 1.16
 **/
void
chafa_term_info_set_name (ChafaTermInfo *term_info,
                          const gchar *name)
{
    gchar *name_copy;

    g_return_if_fail (term_info != NULL);

    name_copy = name ? g_strdup (name) : NULL;
    g_free (term_info->name);
    term_info->name = name_copy;
}

/**
 * chafa_term_info_is_canvas_mode_supported:
 * @term_info: A #ChafaTermInfo
 * @canvas_mode: A #ChafaCanvasMode
 *
 * Checks if @term_info has terminal sequences that support @canvas_mode.
 *
 * Returns: %TRUE if @canvas_mode is supported, %FALSE otherwise.
 *
 * Since: 1.16
 **/
gboolean
chafa_term_info_is_canvas_mode_supported (ChafaTermInfo *term_info,
                                          ChafaCanvasMode canvas_mode)
{
    gboolean result = FALSE;

    g_return_val_if_fail (term_info != NULL, FALSE);

    switch (canvas_mode)
    {
        case CHAFA_CANVAS_MODE_TRUECOLOR:
            result = chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FGBG_DIRECT)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FG_DIRECT)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_BG_DIRECT);
            break;
        case CHAFA_CANVAS_MODE_INDEXED_256:
        case CHAFA_CANVAS_MODE_INDEXED_240:
            result = chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FGBG_256)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FG_256)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_BG_256);
            break;
        case CHAFA_CANVAS_MODE_INDEXED_16:
            result = chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FGBG_16)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FG_16)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_BG_16);
            break;
        case CHAFA_CANVAS_MODE_INDEXED_16_8:
            result = chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FGBG_8)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FG_8)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_BG_8)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_ENABLE_BOLD)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_RESET_ATTRIBUTES);
            break;
        case CHAFA_CANVAS_MODE_INDEXED_8:
            result = chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FGBG_8)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_FG_8)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_SET_COLOR_BG_8);
            break;
        case CHAFA_CANVAS_MODE_FGBG_BGFG:
            result = chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_INVERT_COLORS)
                && chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_RESET_ATTRIBUTES);
            break;
        case CHAFA_CANVAS_MODE_FGBG:
            result = TRUE;
            break;
        case CHAFA_CANVAS_MODE_MAX:
            g_assert_not_reached ();
    }

    return result ? TRUE : FALSE;
}

/**
 * chafa_term_info_get_best_canvas_mode:
 * @term_info: A #ChafaTermInfo
 *
 * Gets the optimal #ChafaCanvasMode supported by @term_info.
 *
 * Returns: A #ChafaCanvasMode
 *
 * Since: 1.16
 **/
ChafaCanvasMode
chafa_term_info_get_best_canvas_mode (ChafaTermInfo *term_info)
{
    g_return_val_if_fail (term_info != NULL, CHAFA_CANVAS_MODE_FGBG);

    if (chafa_term_info_is_canvas_mode_supported (term_info, CHAFA_CANVAS_MODE_TRUECOLOR))
        return CHAFA_CANVAS_MODE_TRUECOLOR;
    if (chafa_term_info_is_canvas_mode_supported (term_info, CHAFA_CANVAS_MODE_INDEXED_240))
        return CHAFA_CANVAS_MODE_INDEXED_240;
    if (chafa_term_info_is_canvas_mode_supported (term_info, CHAFA_CANVAS_MODE_INDEXED_16))
        return CHAFA_CANVAS_MODE_INDEXED_16;
    if (chafa_term_info_is_canvas_mode_supported (term_info, CHAFA_CANVAS_MODE_INDEXED_16_8))
        return CHAFA_CANVAS_MODE_INDEXED_16_8;
    if (chafa_term_info_is_canvas_mode_supported (term_info, CHAFA_CANVAS_MODE_INDEXED_8))
        return CHAFA_CANVAS_MODE_INDEXED_8;
    if (chafa_term_info_is_canvas_mode_supported (term_info, CHAFA_CANVAS_MODE_FGBG_BGFG))
        return CHAFA_CANVAS_MODE_FGBG_BGFG;

    return CHAFA_CANVAS_MODE_FGBG;
}

/**
 * chafa_term_info_is_pixel_mode_supported:
 * @term_info: A #ChafaTermInfo
 * @pixel_mode: A #ChafaPixelMode
 *
 * Checks if @term_info has terminal sequences that support @pixel_mode.
 *
 * Returns: %TRUE if @pixel_mode is supported, %FALSE otherwise.
 *
 * Since: 1.16
 **/
gboolean
chafa_term_info_is_pixel_mode_supported (ChafaTermInfo *term_info,
                                         ChafaPixelMode pixel_mode)
{
    gboolean result = FALSE;

    g_return_val_if_fail (term_info != NULL, FALSE);

    switch (pixel_mode)
    {
        case CHAFA_PIXEL_MODE_KITTY:
            result =
                chafa_term_info_get_passthrough_type (term_info) == CHAFA_PASSTHROUGH_NONE
                ? chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_BEGIN_KITTY_IMMEDIATE_IMAGE_V1)
                : chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_BEGIN_KITTY_IMMEDIATE_VIRT_IMAGE_V1);
            break;
        case CHAFA_PIXEL_MODE_SIXELS:
            result = chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_BEGIN_SIXELS);
            break;
        case CHAFA_PIXEL_MODE_ITERM2:
            result = chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_BEGIN_ITERM2_IMAGE);
            break;
        case CHAFA_PIXEL_MODE_SYMBOLS:
            result = TRUE;
            break;
        case CHAFA_PIXEL_MODE_MAX:
            g_assert_not_reached ();
    }

    return result ? TRUE : FALSE;
}

/**
 * chafa_term_info_get_best_pixel_mode:
 * @term_info: A #ChafaTermInfo
 *
 * Gets the optimal #ChafaPixelMode supported by @term_info.
 *
 * Returns: A #ChafaPixelMode
 *
 * Since: 1.16
 **/
ChafaPixelMode
chafa_term_info_get_best_pixel_mode (ChafaTermInfo *term_info)
{
    g_return_val_if_fail (term_info != NULL, CHAFA_PIXEL_MODE_SYMBOLS);

    if (chafa_term_info_is_pixel_mode_supported (term_info, CHAFA_PIXEL_MODE_KITTY))
        return CHAFA_PIXEL_MODE_KITTY;
    if (chafa_term_info_is_pixel_mode_supported (term_info, CHAFA_PIXEL_MODE_SIXELS))
        return CHAFA_PIXEL_MODE_SIXELS;
    if (chafa_term_info_is_pixel_mode_supported (term_info, CHAFA_PIXEL_MODE_ITERM2))
        return CHAFA_PIXEL_MODE_ITERM2;

    return CHAFA_PIXEL_MODE_SYMBOLS;
}

/**
 * chafa_term_info_get_passthrough_type:
 * @term_info: A #ChafaTermInfo
 *
 * Gets the passthrough mode supported by @term_info.
 *
 * Returns: A #ChafaPassthrough
 *
 * Since: 1.16
 **/
ChafaPassthrough
chafa_term_info_get_passthrough_type (ChafaTermInfo *term_info)
{
    g_return_val_if_fail (term_info != NULL, CHAFA_PASSTHROUGH_NONE);

    if (chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_BEGIN_TMUX_PASSTHROUGH))
        return CHAFA_PASSTHROUGH_TMUX;
    if (chafa_term_info_have_seq (term_info, CHAFA_TERM_SEQ_BEGIN_SCREEN_PASSTHROUGH))
        return CHAFA_PASSTHROUGH_SCREEN;

    return CHAFA_PASSTHROUGH_NONE;
}

/**
 * chafa_term_info_get_is_pixel_passthrough_needed:
 * @term_info: A #ChafaTermInfo
 * @pixel_mode: A #ChafaPixelMode
 *
 * Gets if passthrough should be used to convey images in @pixel_mode.
 * If needed, the passthrough type can be gotten from chafa_term_info_get_passthrough_type().
 *
 * Returns: %TRUE if passthrough is needed, %FALSE otherwise.
 *
 * Since: 1.16
 **/
gboolean
chafa_term_info_get_is_pixel_passthrough_needed (ChafaTermInfo *term_info,
                                                 ChafaPixelMode pixel_mode)
{
    g_return_val_if_fail (term_info != NULL, FALSE);
    g_return_val_if_fail (pixel_mode < CHAFA_PIXEL_MODE_MAX, FALSE);

    return term_info->pixel_passthrough_needed [pixel_mode] ? TRUE : FALSE;
}

/**
 * chafa_term_info_set_is_pixel_passthrough_needed:
 * @term_info: A #ChafaTermInfo
 * @pixel_mode: A #ChafaPixelMode
 * @pixel_passthrough_needed: A boolean indicating whether passthrough is needed
 *
 * Specifies if passthrough should be used to convey images in @pixel_mode.
 *
 * Since: 1.16
 **/
void
chafa_term_info_set_is_pixel_passthrough_needed (ChafaTermInfo *term_info,
                                                 ChafaPixelMode pixel_mode,
                                                 gboolean pixel_passthrough_needed)
{
    g_return_if_fail (term_info != NULL);
    g_return_if_fail (pixel_mode < CHAFA_PIXEL_MODE_MAX);

    term_info->pixel_passthrough_needed [pixel_mode] = pixel_passthrough_needed ? 1 : 0;
}

/**
 * chafa_term_info_get_quirks:
 * @term_info: A #ChafaTermInfo
 *
 * Gets the quirks associated with @term_info.
 *
 * Returns: The #ChafaTermQuirks flags
 *
 * Since: 1.16
 **/
ChafaTermQuirks
chafa_term_info_get_quirks (ChafaTermInfo *term_info)
{
    g_return_val_if_fail (term_info != NULL, 0);

    return term_info->quirks;
}

/**
 * chafa_term_info_set_quirks:
 * @term_info: A #ChafaTermInfo
 * @quirks: A set of #ChafaTermQuirks flags
 *
 * Assigns a set of quirks to @term_info.
 *
 * Since: 1.16
 **/
void
chafa_term_info_set_quirks (ChafaTermInfo *term_info, ChafaTermQuirks quirks)
{
    g_return_if_fail (term_info != NULL);

    term_info->quirks = quirks;
}

/**
 * chafa_term_info_get_safe_symbol_tags:
 * @term_info: A #ChafaTermInfo
 *
 * Gets the #ChafaSymbolTags that are likely safe to use with @term_info. The
 * #ChafaSymbolTags are a bitwise OR of flags from the enum.
 *
 * Returns: The #ChafaSymbolTags
 *
 * Since: 1.16
 **/
ChafaSymbolTags
chafa_term_info_get_safe_symbol_tags (ChafaTermInfo *term_info)
{
    g_return_val_if_fail (term_info != NULL, CHAFA_SYMBOL_TAG_NONE);

    return term_info->safe_symbol_tags;
}

/**
 * chafa_term_info_set_safe_symbol_tags:
 * @term_info: A #ChafaTermInfo
 * @tags: A set of #ChafaSymbolTags
 *
 * Sets the #ChafaSymbolTags that are likely safe to use with @term_info. The
 * @tags are a bitwise OR of flags from the enum.
 *
 * Since: 1.16
 **/
void
chafa_term_info_set_safe_symbol_tags (ChafaTermInfo *term_info, ChafaSymbolTags tags)
{
    g_return_if_fail (term_info != NULL);

    term_info->safe_symbol_tags = tags;
}

/**
 * chafa_term_info_have_seq:
 * @term_info: A #ChafaTermInfo.
 * @seq: A #ChafaTermSeq to query for.
 *
 * Checks if @term_info can emit @seq.
 *
 * Returns: %TRUE if @seq can be emitted, %FALSE otherwise
 *
 * Since: 1.6
 **/
gboolean
chafa_term_info_have_seq (const ChafaTermInfo *term_info, ChafaTermSeq seq)
{
    g_return_val_if_fail (term_info != NULL, FALSE);
    g_return_val_if_fail (seq >= 0 && seq < CHAFA_TERM_SEQ_MAX, FALSE);

    return term_info->unparsed_str [seq] ? TRUE : FALSE;
}

/**
 * chafa_term_info_get_seq:
 * @term_info: A #ChafaTermInfo.
 * @seq: A #ChafaTermSeq to query for.
 *
 * Gets the string equivalent of @seq stored in @term_info.
 *
 * Returns: An unformatted string sequence, or %NULL if not set.
 *
 * Since: 1.6
 **/
const gchar *
chafa_term_info_get_seq (ChafaTermInfo *term_info, ChafaTermSeq seq)
{
    g_return_val_if_fail (term_info != NULL, NULL);
    g_return_val_if_fail (seq >= 0 && seq < CHAFA_TERM_SEQ_MAX, NULL);

    return term_info->unparsed_str [seq];
}

/**
 * chafa_term_info_set_seq:
 * @term_info: A #ChafaTermInfo.
 * @seq: A #ChafaTermSeq to query for.
 * @str: A control sequence string, or %NULL to clear.
 * @error: A return location for error details, or %NULL.
 *
 * Sets the control sequence string equivalent of @seq stored in @term_info to @str.
 *
 * The string may contain argument indexes to be substituted with integers on
 * formatting. The indexes are preceded by a percentage character and start at 1,
 * i.e. \%1, \%2, \%3, etc.
 *
 * The string's length after formatting must not exceed %CHAFA_TERM_SEQ_LENGTH_MAX
 * bytes. Each argument can add up to four digits, or three for those specified as
 * 8-bit integers. If the string could potentially exceed this length when
 * formatted, chafa_term_info_set_seq() will return %FALSE.
 *
 * If parsing fails or @str is too long, any previously existing sequence
 * will be left untouched.
 *
 * Passing %NULL for @str clears the corresponding control sequence.
 *
 * Returns: %TRUE if parsing succeeded, %FALSE otherwise
 *
 * Since: 1.6
 **/
gboolean
chafa_term_info_set_seq (ChafaTermInfo *term_info, ChafaTermSeq seq, const gchar *str,
                         GError **error)
{
    gchar seq_str [CHAFA_TERM_SEQ_LENGTH_MAX];
    SeqArgInfo seq_args [CHAFA_TERM_SEQ_ARGS_MAX];
    gboolean result = FALSE;

    g_return_val_if_fail (term_info != NULL, FALSE);
    g_return_val_if_fail (seq >= 0 && seq < CHAFA_TERM_SEQ_MAX, FALSE);

    if (!str)
    {
        term_info->seq_str [seq] [0] = '\0';
        term_info->seq_args [seq] [0].pre_len = 0;
        term_info->seq_args [seq] [0].arg_index = ARG_INDEX_SENTINEL;

        g_free (term_info->unparsed_str [seq]);
        term_info->unparsed_str [seq] = NULL;
        result = TRUE;
    }
    else
    {
        result = parse_seq_args (&seq_str [0], &seq_args [0], str,
                                 seq_meta [seq].n_args,
                                 seq_meta [seq].type_size == 1 ? 3 : 4,
                                 error);

        if (result == TRUE)
        {
            memcpy (&term_info->seq_str [seq] [0], &seq_str [0],
                    CHAFA_TERM_SEQ_LENGTH_MAX);
            memcpy (&term_info->seq_args [seq] [0], &seq_args [0],
                    CHAFA_TERM_SEQ_ARGS_MAX * sizeof (SeqArgInfo));

            g_free (term_info->unparsed_str [seq]);
            term_info->unparsed_str [seq] = g_strdup (str);
        }
    }

    return result;
}

/**
 * chafa_term_info_emit_seq:
 * @term_info: A #ChafaTermInfo
 * @seq: A #ChafaTermSeq to emit
 * @...: A list of int arguments to insert in @seq, terminated by -1
 *
 * Formats the terminal sequence @seq, inserting positional arguments. The seq's
 * number of arguments must be supplied exactly.
 *
 * The argument list must be terminated by -1, or undefined behavior will result.
 *
 * If the wrong number of arguments is supplied, or an argument is out of range,
 * this function will return %NULL. Otherwise, it returns a zero-terminated string
 * that must be freed with g_free().
 *
 * If you want compile-time validation of arguments, consider using one of the
 * specific chafa_term_info_emit_*() functions. They are also faster, but require
 * you to allocate at least CHAFA_TERM_SEQ_LENGTH_MAX bytes up front.
 *
 * Returns: A newly allocated, zero-terminated formatted string, or %NULL on error
 *
 * Since: 1.14
 **/
gchar *
chafa_term_info_emit_seq (ChafaTermInfo *term_info, ChafaTermSeq seq, ...)
{
    va_list args;
    gchar *result;

    g_return_val_if_fail (term_info != NULL, NULL);
    g_return_val_if_fail (seq >= 0 && seq < CHAFA_TERM_SEQ_MAX, NULL);

    va_start (args, seq);
    result = chafa_term_info_emit_seq_valist (term_info, seq, &args);
    va_end (args);

    return result;
}

/**
 * chafa_term_info_emit_seq_valist:
 * @term_info: A #ChafaTermInfo
 * @seq: A #ChafaTermSeq to emit
 * @args: A #va_list of int arguments to insert in @seq
 *
 * Behaves like chafa_term_info_emit_seq(), but takes the arguments as a
 * #va_list.
 *
 * Returns: A newly allocated, zero-terminated formatted string, or %NULL on error
 *
 * Since: 1.16
 **/
gchar *
chafa_term_info_emit_seq_valist (ChafaTermInfo *term_info, ChafaTermSeq seq, va_list *args)
{
    guint args_uint [CHAFA_TERM_SEQ_ARGS_MAX];
    guint16 args_u16 [CHAFA_TERM_SEQ_ARGS_MAX];
    guint8 args_u8 [CHAFA_TERM_SEQ_ARGS_MAX];
    gchar buf [CHAFA_TERM_SEQ_LENGTH_MAX];
    guint n_args = 0;
    gchar *p;
    gchar *result = NULL;

    g_return_val_if_fail (term_info != NULL, NULL);
    g_return_val_if_fail (seq >= 0 && seq < CHAFA_TERM_SEQ_MAX, NULL);

    for (;;)
    {
        gint arg = va_arg (*args, gint);
        if (arg < 0)
            break;

        if (n_args == CHAFA_TERM_SEQ_ARGS_MAX || n_args == seq_meta [seq].n_args)
            goto out;

        if (seq_meta [seq].type_size == 1)
        {
            if (arg > 0xff)
                goto out;
            args_u8 [n_args] = arg;
        }
        else if (seq_meta [seq].type_size == 2)
        {
            if (arg > 0xffff)
                goto out;
            args_u16 [n_args] = arg;
        }
        else
        {
            args_uint [n_args] = arg;
        }

        n_args++;
    }
    
    if (seq_meta [seq].seq_type == SEQ_TYPE_FIXARGS && n_args != seq_meta [seq].n_args)
        goto out;

    /* 16-bit args are always hex for now. It has no other uses. */

    if (seq_meta [seq].seq_type == SEQ_TYPE_VARARGS)
        p = emit_seq_varargs_uint (term_info, buf, seq, args_uint, n_args);
    else if (seq_meta [seq].n_args == 0)
        p = emit_seq_0_args_uint (term_info, buf, seq);
    else if (seq_meta [seq].type_size == 1)
        p = emit_seq_guint8 (term_info, buf, seq, args_u8, n_args);
    else if (seq_meta [seq].type_size == 2)
        p = emit_seq_guint16_hex (term_info, buf, seq, args_u16, n_args);
    else
        p = emit_seq_guint (term_info, buf, seq, args_uint, n_args);

    if (p == buf)
        goto out;

    result = g_strndup (buf, p - buf);

out:
    return result;
}

/**
 * chafa_term_info_parse_seq:
 * @term_info: A #ChafaTermInfo
 * @seq: A #ChafaTermSeq to attempt to parse
 * @input: Pointer to pointer to input data
 * @input_len: Pointer to maximum input data length
 * @args_out: Pointer to parsed argument values
 *
 * Attempts to parse a terminal sequence from an input data array. If successful,
 * #CHAFA_PARSE_SUCCESS will be returned, the @input pointer will be advanced and
 * the parsed length will be subtracted from @input_len.
 *
 * Thus function can be used with seqs where the number of arguments is known
 * in advance, but since it doesn't return the number of arguments parsed, it can't
 * be used with variable-argument seqs.
 *
 * It is recommended to use chafa_term_info_parse_seq_varargs() in all cases
 * instead.
 *
 * Returns: A #ChafaParseResult indicating success, failure or insufficient input data
 *
 * Since: 1.14
 * Deprecated: 1.16: Use chafa_term_info_parse_seq_varargs() instead.
 **/
ChafaParseResult
chafa_term_info_parse_seq (ChafaTermInfo *term_info, ChafaTermSeq seq,
                           gchar **input, gint *input_len,
                           guint *args_out)
{
    guint dummy_args_out [CHAFA_TERM_SEQ_ARGS_MAX];
    gint n_args_out;
    ChafaParseResult result;

    g_return_val_if_fail (term_info != NULL, CHAFA_PARSE_FAILURE);
    g_return_val_if_fail (seq >= 0 && seq < CHAFA_TERM_SEQ_MAX, CHAFA_PARSE_FAILURE);
    g_return_val_if_fail (input != NULL, CHAFA_PARSE_FAILURE);
    g_return_val_if_fail (*input != NULL, CHAFA_PARSE_FAILURE);
    g_return_val_if_fail (input_len != NULL, CHAFA_PARSE_FAILURE);

    if (!chafa_term_info_have_seq (term_info, seq))
        return CHAFA_PARSE_FAILURE;

    if (!args_out)
        args_out = dummy_args_out;

    result = try_parse_seq (term_info, seq, input, input_len, dummy_args_out, &n_args_out);

    if (result == CHAFA_PARSE_SUCCESS && args_out)
    {
        /* CHAFA_TERM_SEQ_ARGS_MAX was increased from 8 to 24 in version 1.16.0.
         * For ABI backwards compatibility, respect the legacy value in this legacy
         * function. */
        n_args_out = MIN (n_args_out, 8);
        memcpy (args_out, dummy_args_out, n_args_out * sizeof (guint));
    }

    return result;
}

/**
 * chafa_term_info_parse_seq_varargs:
 * @term_info: A #ChafaTermInfo
 * @seq: A #ChafaTermSeq to attempt to parse
 * @input: Pointer to pointer to input data
 * @input_len: Pointer to maximum input data length
 * @args_out: Pointer to parsed argument values
 * @n_args_out: Pointer to storage for argument count
 *
 * Attempts to parse a terminal sequence from an input data array. If successful,
 * #CHAFA_PARSE_SUCCESS will be returned, the @input pointer will be advanced and
 * the parsed length will be subtracted from @input_len.
 *
 * Any numeric parsed arguments are returned as an array starting at @args_out,
 * which must have room for up to #CHAFA_TERM_SEQ_ARGS_MAX elements.
 *
 * The number of parsed arguments is returned in @n_args_out. This is useful for
 * seqs with a variable number of arguments, like #CHAFA_TERM_SEQ_PRIMARY_DEVICE_ATTRIBUTES.
 *
 * Either or both of @args_out and @n_args_out can be %NULL, in which case
 * nothing is returned for that parameter.
 *
 * Returns: A #ChafaParseResult indicating success, failure or insufficient input data
 *
 * Since: 1.16
 **/
ChafaParseResult
chafa_term_info_parse_seq_varargs (ChafaTermInfo *term_info, ChafaTermSeq seq,
                                   gchar **input, gint *input_len,
                                   guint *args_out, gint *n_args_out)
{
    guint dummy_args_out [CHAFA_TERM_SEQ_ARGS_MAX];
    gint dummy_n_args_out;

    g_return_val_if_fail (term_info != NULL, CHAFA_PARSE_FAILURE);
    g_return_val_if_fail (seq >= 0 && seq < CHAFA_TERM_SEQ_MAX, CHAFA_PARSE_FAILURE);
    g_return_val_if_fail (input != NULL, CHAFA_PARSE_FAILURE);
    g_return_val_if_fail (*input != NULL, CHAFA_PARSE_FAILURE);
    g_return_val_if_fail (input_len != NULL, CHAFA_PARSE_FAILURE);

    if (!chafa_term_info_have_seq (term_info, seq))
        return CHAFA_PARSE_FAILURE;

    if (!args_out)
        args_out = dummy_args_out;
    if (!n_args_out)
        n_args_out = &dummy_n_args_out;

    return try_parse_seq (term_info, seq, input, input_len, args_out, n_args_out);
}

/**
 * chafa_term_info_get_inherit_seq:
 * @term_info: A #ChafaTermInfo
 * @seq: A #ChafaTermSeq
 *
 * Gets whether @seq can be inherited from the outer #ChafaTermInfo
 * when chaining with chafa_term_info_chain().
 *
 * Returns: Whether @seq gets inherited when chaining
 *
 * Since: 1.16
 **/
gboolean
chafa_term_info_get_inherit_seq (ChafaTermInfo *term_info, ChafaTermSeq seq)
{
    g_return_val_if_fail (term_info != NULL, FALSE);
    g_return_val_if_fail (seq < CHAFA_TERM_SEQ_MAX, FALSE);

    return term_info->inherit_seq [seq];
}

/**
 * chafa_term_info_set_inherit_seq:
 * @term_info: A #ChafaTermInfo
 * @seq: A #ChafaTermSeq
 * @inherit: A boolean indicating whether @seq should be inherited
 *
 * Sets whether @seq can be inherited from the outer #ChafaTermInfo
 * when chaining with chafa_term_info_chain().
 *
 * Since: 1.16
 **/
void
chafa_term_info_set_inherit_seq (ChafaTermInfo *term_info, ChafaTermSeq seq,
                                 gboolean inherit)
{
    g_return_if_fail (term_info != NULL);
    g_return_if_fail (seq < CHAFA_TERM_SEQ_MAX);

    term_info->inherit_seq [seq] = inherit;
}

/**
 * chafa_term_info_supplement:
 * @term_info: A #ChafaTermInfo to supplement
 * @source: A #ChafaTermInfo to copy from
 *
 * Supplements missing sequences in @term_info with ones copied
 * from @source.
 *
 * Since: 1.6
 **/
void
chafa_term_info_supplement (ChafaTermInfo *term_info, ChafaTermInfo *source)
{
    gint i;

    g_return_if_fail (term_info != NULL);
    g_return_if_fail (source != NULL);

    for (i = 0; i < CHAFA_TERM_SEQ_MAX; i++)
    {
        if (!term_info->unparsed_str [i] && source->unparsed_str [i])
        {
            term_info->unparsed_str [i] = g_strdup (source->unparsed_str [i]);
            memcpy (&term_info->seq_str [i] [0], &source->seq_str [i] [0],
                    CHAFA_TERM_SEQ_LENGTH_MAX);
            memcpy (&term_info->seq_args [i] [0], &source->seq_args [i] [0],
                    CHAFA_TERM_SEQ_ARGS_MAX * sizeof (SeqArgInfo));
        }
    }
}

/**
 * chafa_term_info_chain:
 * @outer: The #ChafaTermInfo for the outer terminal
 * @inner: The #ChafaTermInfo for the inner terminal
 *
 * Terminal emulators and applications are often nested, with the inner
 * application's capabilities limiting, extending or modifying the outer's.
 *
 * Examples are terminal multiplexers like Screen and tmux, or terminal
 * emulators running inside editors like Emacs and vi.
 *
 * This merges the @outer and @inner sequences into a single #ChafaTermInfo
 * according to the following rules.
 *
 * For sequences marked as inherited in @inner:
 *
 * - If either @inner or @outer sequence is %NULL, pick the @outer sequence.
 * - Otherwise, pick @inner sequence.
 *
 * For sequences not marked as inherited, always use the @inner sequence.
 *
 * This allows for using the inner term's sequences while clearing them if the
 * outer term does not support the sequence at all. This is useful for muxers
 * (e.g. fbterm supports 256 colors, but with private seqs; we want to use the
 * inner mux' corresponding seqs).
 *
 * The merged #ChafaTermInfo is a new instance, with the initial reference
 * owned by the caller.
 *
 * This function can be used repeatedly to create chains that're arbitrarily
 * long, but is unlikely to be useful beyond three levels (terminal emulator,
 * multiplexer, application).
 *
 * Returns: The merged #ChafaTermInfo
 *
 * Since: 1.16
 **/
ChafaTermInfo *
chafa_term_info_chain (ChafaTermInfo *outer, ChafaTermInfo *inner)
{
    ChafaTermInfo *chained;
    const gchar *outer_name, *inner_name;
    gchar *new_name;
    gint i;

    chained = chafa_term_info_copy (outer);

    for (i = 0; i < CHAFA_TERM_SEQ_MAX; i++)
    {
        if (chafa_term_info_get_inherit_seq (inner, i))
        {
            if (chained->unparsed_str [i] && inner->unparsed_str [i])
                copy_seq (inner, chained, i);
        }
        else
        {
            copy_seq (inner, chained, i);
        }
    }

    for (i = 0; i < CHAFA_PIXEL_MODE_MAX; i++)
        chained->pixel_passthrough_needed [i]
            = inner->pixel_passthrough_needed [i] | outer->pixel_passthrough_needed [i];

    chained->safe_symbol_tags |= inner->safe_symbol_tags & outer->safe_symbol_tags;
    chained->quirks = inner->quirks | outer->quirks;

    outer_name = chafa_term_info_get_name (outer);
    inner_name = chafa_term_info_get_name (inner);

    new_name = g_strjoin (":",
                          inner_name ? inner_name : "unknown",
                          outer_name ? outer_name : "unknown",
                          NULL);
    chafa_term_info_set_name (chained, new_name);
    g_free (new_name);

    return chained;
}

#define DEFINE_EMIT_SEQ_0_none_char(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest) \
{ return emit_seq_0_args_uint (term_info, dest, CHAFA_TERM_SEQ_##seq_name); }

#define DEFINE_EMIT_SEQ_1_none_guint(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint arg) \
{ return emit_seq_1_args_uint (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg); }

#define DEFINE_EMIT_SEQ_1_none_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg) \
{ return emit_seq_1_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg); }

#define DEFINE_EMIT_SEQ_1_8fg_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg) \
{ return emit_seq_1_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg + 30); }

#define DEFINE_EMIT_SEQ_1_8bg_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg) \
{ return emit_seq_1_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg + 40); }

#define DEFINE_EMIT_SEQ_1_aix16fg_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg) \
{ return emit_seq_1_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg + (arg < 8 ? 30 : (90 - 8))); }

#define DEFINE_EMIT_SEQ_1_aix16bg_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg) \
{ return emit_seq_1_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg + (arg < 8 ? 40 : (100 - 8))); }

#define DEFINE_EMIT_SEQ_2_none_guint(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint arg0, guint arg1) \
{ return emit_seq_2_args_uint (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0, arg1); }

#define DEFINE_EMIT_SEQ_2_pos_guint(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint arg0, guint arg1) \
{ return emit_seq_2_args_uint (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0 + 1, arg1 + 1); }

#define DEFINE_EMIT_SEQ_2_none_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg0, guint8 arg1) \
{ return emit_seq_2_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0, arg1); }

#define DEFINE_EMIT_SEQ_2_8fgbg_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg0, guint8 arg1) \
{ return emit_seq_2_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0 + 30, arg1 + 40); }

#define DEFINE_EMIT_SEQ_2_aix16fgbg_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg0, guint8 arg1) \
{ return emit_seq_2_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0 + (arg0 < 8 ? 30 : (90 - 8)), arg1 + (arg1 < 8 ? 40 : (100 - 8))); }

#define DEFINE_EMIT_SEQ_3_none_guint(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint arg0, guint arg1, guint arg2) \
{ return emit_seq_3_args_uint (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0, arg1, arg2); }

#define DEFINE_EMIT_SEQ_5_none_guint(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint arg0, guint arg1, guint arg2, guint arg3, guint arg4) \
{ return emit_seq_5_args_uint (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0, arg1, arg2, arg3, arg4); }

#define DEFINE_EMIT_SEQ_6_none_guint(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint arg0, guint arg1, guint arg2, guint arg3, guint arg4, guint arg5) \
{ return emit_seq_6_args_uint (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0, arg1, arg2, arg3, arg4, arg5); }

#define DEFINE_EMIT_SEQ_3_none_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg0, guint8 arg1, guint8 arg2) \
{ return emit_seq_3_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0, arg1, arg2); }

#define DEFINE_EMIT_SEQ_6_none_guint8(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint8 arg0, guint8 arg1, guint8 arg2, guint8 arg3, guint8 arg4, guint8 arg5) \
{ return emit_seq_6_args_uint8 (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0, arg1, arg2, arg3, arg4, arg5); }

#define DEFINE_EMIT_SEQ_3_u16hex_guint16(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint16 arg0, guint16 arg1, guint16 arg2) \
{ return emit_seq_3_args_uint16_hex (term_info, dest, CHAFA_TERM_SEQ_##seq_name, arg0, arg1, arg2); }

#define DEFINE_EMIT_SEQ_VARARGS_guint(func_name, seq_name) \
gchar *chafa_term_info_emit_##func_name(const ChafaTermInfo *term_info, gchar *dest, guint *args, gint n_args) \
{ return emit_seq_varargs_uint (term_info, dest, CHAFA_TERM_SEQ_##seq_name, args, n_args); }

#define CHAFA_TERM_SEQ_DEF(name, NAME, n_args, arg_proc, arg_type, ...)  \
    DEFINE_EMIT_SEQ_##n_args##_##arg_proc##_##arg_type(name, NAME)
#define CHAFA_TERM_SEQ_DEF_VARARGS(name, NAME, arg_type) \
    DEFINE_EMIT_SEQ_VARARGS_##arg_type(name, NAME)
#include "chafa-term-seq-def.h"
#undef CHAFA_TERM_SEQ_DEF
#undef CHAFA_TERM_SEQ_DEF_VARARGS
