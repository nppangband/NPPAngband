/*
 * File: z-msg.c
 * Purpose: Message handling
 *
 * Copyright (c) 2007 Elly, Andrew Sidwell
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 */
#include "z-virt.h"
#include "z-term.h"
#include "z-msg.h"

typedef struct _message_t
{
	char *str;
	struct _message_t *newer;
	struct _message_t *older;
	MessageType type;
	u16b count;
} message_t;

typedef struct _msgcolor_t
{
	MessageType type;
	byte color;
	struct _msgcolor_t *next;
} msgcolor_t;

typedef struct _msgqueue_t
{
	message_t *head;
	message_t *tail;
	msgcolor_t *colors;
	u32b count;
	u32b max;
} msgqueue_t;

static msgqueue_t *messages = NULL;

/* Functions operating on the entire list */

/*
 * Initialize the message structures
 */
errr messages_init(void)
{
	int i;

	messages = ZNEW(msgqueue_t);
	messages->max = 2048;

	/*
	 * Start off all message types as white
	 * Some message-specific colors are later specified in message.prf.
	 * */
	for (i = 0; i < MSG_MAX; i++)
	{
		message_color_define(i, TERM_WHITE);
	}

	return 0;
}


/*
 * Free message structures and data
 */
void messages_free(void)
{
	msgcolor_t *c = messages->colors;
	msgcolor_t *nextc;
	message_t *m = messages->head;
	message_t *nextm;

	while (m)
	{
		nextm = m->older;
		FREE(m->str);
		FREE(m);
		m = nextm;
	}

	while (c)
	{
		nextc = c->next;
		FREE(c);
		c = nextc;
	}

	FREE(messages);
}

/*
 * Return the number of messages
 */
u16b messages_num(void)
{
	return messages->count;
}

/* Functions for individual messages */

/**
 * Save a new message into the memory buffer, with text `str` and type `type`.
 *
 * The new message may not be saved if it is identical to the one saved before
 * it, in which case the "count" of the message will be increased instead.
 * This count can be fetched using the message_count() function.
 */
void message_add(const char *str, const MessageType type)
{
	message_t *m;

	if (messages->head &&
	    messages->head->type == type &&
	    !strcmp(messages->head->str, str))
	{
		messages->head->count++;
		return;
	}

	m = ZNEW(message_t);
	m->str = string_make(str);
	m->type = type;
	m->count = 1;
	m->older = messages->head;

	if (messages->head)
		messages->head->newer = m;

	messages->head = m;
	messages->count++;

	if (!messages->tail)
		messages->tail = m;

	if (messages->count > messages->max)
	{
		message_t *old_tail = messages->tail;

		messages->tail = old_tail->newer;
		messages->tail->older = NULL;
		FREE(old_tail->str);
		FREE(old_tail);
		messages->count--;
	}
}


/*
 * Get a given message
 */
static message_t *message_get(u16b age)
{
	message_t *m = messages->head;

	while (m && age--)
		m = m->older;

	return m;
}


/**
 * Returns the text of the message of age `age`.  The age of the most recently
 * saved message is 0, the one before that is of age 1, etc.
 *
 * Returns the empty string if the no messages of the age specified are
 * available.
 */
const char *message_str(u16b age)
{
	message_t *m = message_get(age);
	return (m ? m->str : "");
}


/**
 * Returns the number of times the message of age `age` was saved. The age of
 * the most recently saved message is 0, the one before that is of age 1, etc.
 *
 * In other words, if message_add() was called five times, one after the other,
 * with the message "The orc sets your hair on fire.", then the text will only
 * have one age (age = 0), but will have a count of 5.
 */
u16b message_count(u16b age)
{
	message_t *m = message_get(age);
	return (m ? m->count : 0);
}


/**
 * Returns the type of the message of age `age`.  The age of the most recently
 * saved message is 0, the one before that is of age 1, etc.
 */
MessageType message_type(u16b age)
{
	message_t *m = message_get(age);
	return (m ? m->type : MSG_GENERIC);
}


/**
 * Returns the display colour of the message memorised `age` messages ago.
 * (i.e. age = 0 represents the last memorised message, age = 1 is the one
 * before that, etc).
 */
byte message_color(u16b age)
{
	message_t *m = message_get(age);
	return (m ? message_type_color(m->type) : TERM_WHITE);
}


/* Message-color functions */


/**
 * Defines the color `color` for the message type `type`.
 */
errr message_color_define(MessageType type, byte color)
{
	msgcolor_t *mc = messages->colors;

	if (!mc)
	{
		messages->colors = ZNEW(msgcolor_t);
		messages->colors->type = type;
		messages->colors->color = color;
		return 0;
	}

	while (mc->next)
	{
		if (mc->type == type)
		{
			mc->color = color;
			return 0;
		}
		mc = mc->next;
	}

	mc->next = ZNEW(msgcolor_t);
	mc->next->type = type;
	mc->next->color = color;

	return 0;
}


/**
 * Returns the colour for the message type `type`.
 */
byte message_type_color(MessageType type)
{
	msgcolor_t *mc;
	byte color = TERM_WHITE;

	if (messages)
	{
		mc = messages->colors;

		while (mc && mc->type != type)
			mc = mc->next;

		if (mc && (mc->color != TERM_DARK))
			color = mc->color;
	}

	return color;
}
