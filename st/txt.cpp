/* txt.cpp -- Sawteeth "txt" atom stream, reader/writer.
 *
 * The 2000-era Sawteeth sources are not part of this repository; this file
 * implements the stream format that data/Spruce-ComicFakery.NNN was written
 * with, following the original engine (Sawteeth, MIT licence, copyright 2001
 * Jonas Norberg).
 *
 * Text form: values are decimal numbers separated by whitespace, "//" starts a
 * comment that runs to the end of the line, and strings are written after a
 * newline and terminated with a NUL byte.  That is exactly the layout of the
 * shipped song file:
 *
 *     SWTT
 *     // spspal & len
 *     882
 *     ...
 *     Comic Bakery\0\nBassChannel\0\n...
 *
 * Binary form (IsBin) writes the same atoms as raw little-endian bytes; with
 * IsRLE a zero byte is followed by a repeat count.
 */

#include "txt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ helpers */

static void skip_space_and_comments(FILE *f)
{
	int c;

	for (;;) {
		c = fgetc(f);
		if (c == EOF)
			return;

		if (c == '/') {
			int next = fgetc(f);
			if (next == '/') {
				while ((c = fgetc(f)) != EOF && c != '\n')
					;
				continue;
			}
			if (next != EOF)
				ungetc(next, f);
			ungetc('/', f);
			return;
		}

		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			continue;

		ungetc(c, f);
		return;
	}
}

/* Reads one whitespace delimited token; false at end of file. */
static bool read_token(FILE *f, char *buf, int bufsize)
{
	int c;
	int n = 0;

	skip_space_and_comments(f);

	c = fgetc(f);
	if (c == EOF)
		return false;

	while (c != EOF && c != ' ' && c != '\t' && c != '\r' && c != '\n') {
		if (n < bufsize - 1)
			buf[n++] = (char)c;
		c = fgetc(f);
	}
	if (c != EOF)
		ungetc(c, f);

	buf[n] = 0;
	return true;
}

/* Reads a NUL terminated string; leading whitespace is skipped and trailing
 * whitespace is trimmed (the original writer prefixes every name with "\n"). */
static bool read_nul_string(FILE *f, char *buf, int bufsize)
{
	int c;
	int n = 0;

	skip_space_and_comments(f);

	for (;;) {
		c = fgetc(f);
		if (c == EOF || c == 0)
			break;
		if (n < bufsize - 1)
			buf[n++] = (char)c;
	}

	while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'
			|| buf[n - 1] == ' ' || buf[n - 1] == '\t'))
		n--;

	buf[n] = 0;
	return n > 0 || c != EOF;
}

/* ---------------------------------------------------------------------- txt */

txt::txt(char *filename, bool create)
{
	f = NULL;
	open_for_reading = !create;
	bin = false;
	rle = false;
	error = B_ERROR;
	r_zero_mode = false;
	w_zero_mode = false;
	r_zero_count = 0;
	w_zero_count = 0;
	buf[0] = 0;

	if (filename == NULL)
		return;

	f = fopen(filename, create ? "wb" : "rb");
	if (f != NULL)
		error = B_OK;
}

txt::~txt()
{
	if (f != NULL)
		fclose(f);
}

status_t txt::InitCheck()
{
	return error;
}

void txt::IsBin(bool b)
{
	if (f != NULL)
		fflush(f);
	bin = b;
	r_zero_mode = false;
	r_zero_count = 0;
}

void txt::IsRLE(bool r)
{
	rle = r;
	r_zero_mode = false;
	r_zero_count = 0;
}

/* ------------------------------------------------------------------- reading */

int txt::AtomRead()
{
	int c;

	if (r_zero_mode) {
		if (r_zero_count > 0) {
			r_zero_count--;
			return 0;
		}
		r_zero_mode = false;
		return 0;
	}

	c = fgetc(f);
	if (c == EOF) {
		error = B_ERROR;
		return 0;
	}

	if (rle && c == 0) {
		int count = fgetc(f);
		if (count == EOF) {
			error = B_ERROR;
			return 0;
		}
		r_zero_mode = true;
		r_zero_count = count;
		return 0;
	}

	return c;
}

int txt::NextVal(int numbytes)
{
	if (!bin) {
		if (!read_token(f, buf, BUFSIZE))
			return 0;
		return (int)strtol(buf, NULL, 10);
	}

	{
		uint32 value = 0;
		int c;

		if (numbytes < 1)
			return 0;

		/* little endian, one byte at a time so that the 4 character magic
		 * can be read without depending on host byte order
		 */
		for (c = 0; c < numbytes; c++)
			value |= ((uint32)AtomRead()) << (c << 3);

		return (int)value;
	}
}

float txt::NextFloat()
{
	if (!bin) {
		if (!read_token(f, buf, BUFSIZE))
			return 0.0;
		return (float)strtod(buf, NULL);
	}

	{
		uint32 value = (uint32)NextVal(4);
		float result;

		memcpy(&result, &value, sizeof(float));
		return result;
	}
}

char *txt::NextString()
{
	if (bin) {
		/* binary strings are stored the same way as text ones */
		read_nul_string(f, buf, BUFSIZE);
		return buf;
	}

	read_nul_string(f, buf, BUFSIZE);
	return buf;
}

/* ------------------------------------------------------------------- writing */

void txt::AtomWrite(int val)
{
	if (rle && val == 0) {
		w_zero_count++;
		if (w_zero_count >= 255) {
			fputc(0, f);
			fputc(255, f);
			w_zero_count = 0;
		}
		return;
	}

	if (w_zero_count > 0) {
		fputc(0, f);
		fputc(w_zero_count, f);
		w_zero_count = 0;
	}

	fputc(val & 0xff, f);
}

void txt::Val(int val, int numbytes)
{
	if (f == NULL)
		return;

	if (!bin) {
		fprintf(f, "%d\n", val);
		return;
	}

	if (numbytes < 1)
		return;

	AtomWrite(val & 0xff);
	if (numbytes > 1)
		AtomWrite((val >> 8) & 0xff);
	if (numbytes > 2)
		AtomWrite((val >> 16) & 0xff);
	if (numbytes > 3)
		AtomWrite((val >> 24) & 0xff);
}

void txt::String(char *text)
{
	if (f == NULL)
		return;

	if (!bin && w_zero_count > 0) {
		/* nothing to flush, text mode has no zero runs */
		w_zero_count = 0;
	}

	fputc('\n', f);
	if (text != NULL)
		fputs(text, f);
	fputc(0, f);
}

void txt::Comment(char *text)
{
	if (f == NULL || bin)
		return;

	fprintf(f, "// %s\n", text != NULL ? text : "");
}

void txt::Comment(int val)
{
	if (f == NULL || bin)
		return;

	fprintf(f, "// %d\n", val);
}

/* jntok() was the original tokenizer; kept for source compatibility. */
void txt::jntok(char *b, char *m)
{
	if (b == NULL || m == NULL)
		return;
	read_token(f, b, BUFSIZE);
	(void)m;
}
