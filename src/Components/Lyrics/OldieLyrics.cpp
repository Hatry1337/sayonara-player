#include "OldieLyrics.h"

QString Lyrics::OldieLyrics::name() const
{
	return "OldieLyrics.com";
}

QString Lyrics::OldieLyrics::address() const
{
	return "http://www.oldielyrics.com/lyrics";
}

QMap<QString, QString> Lyrics::OldieLyrics::replacements() const
{
	return QMap<QString, QString>
	{
		{" ", "_"},
		{"(", "_"},
		{")", "_"},
		{".", "_"},
		{"&", "_"},
		{"'", ""},
		{"__", "_"}
	};
}

QString Lyrics::OldieLyrics::call_policy() const
{
	return "<SERVER>/<ARTIST>/<TITLE>.html";
}

QMap<QString, QString> Lyrics::OldieLyrics::start_end_tag() const
{
	return QMap<QString, QString>
	{
		{"<div itemprop=\"text\">", "</div>"}
	};
}

bool Lyrics::OldieLyrics::is_start_tag_included() const
{
	return true;
}

bool Lyrics::OldieLyrics::is_end_tag_included() const
{
	return false;
}

bool Lyrics::OldieLyrics::is_numeric() const
{
	return false;
}

bool Lyrics::OldieLyrics::is_lowercase() const
{
	return true;
}

QString Lyrics::OldieLyrics::error_string() const
{
	return QString("error 404");
}