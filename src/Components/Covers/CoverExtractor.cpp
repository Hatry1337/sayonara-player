#include "CoverExtractor.h"
#include "CoverLocation.h"

#include <QString>
#include <QPixmap>

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Tagging/TaggingCover.h"

#include <mutex>

static std::mutex mutex_io;

namespace FileUtils=::Util::File;

struct Cover::Extractor::Private
{
	QPixmap pixmap;
	Cover::Location cl;

	Private(const Cover::Location& cl) :
		cl(cl)
	{}
};

Cover::Extractor::Extractor(const Location& cl, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(cl);

}

Cover::Extractor::~Extractor() {}

QPixmap Cover::Extractor::pixmap()
{
	return m->pixmap;
}

void Cover::Extractor::start()
{
	m->pixmap = QPixmap();

	{ // check for audio file target
		LOCK_GUARD(mutex_io)
		QString audio_file_target = m->cl.audio_file_target();
		if(FileUtils::exists(audio_file_target))
		{
			m->pixmap = QPixmap(m->cl.audio_file_target());
		}
	}

	// check sayonara path
	if(m->pixmap.isNull())
	{
		LOCK_GUARD(mutex_io)
		QString cover_path = m->cl.cover_path();
		if(FileUtils::exists(cover_path))
		{
			m->pixmap = QPixmap(cover_path);
		}
	}

	// check for audio file source
	if(m->pixmap.isNull())
	{
		LOCK_GUARD(mutex_io)
		QString audio_file_source = m->cl.audio_file_source();
		if(FileUtils::exists(audio_file_source))
		{
			m->pixmap = Tagging::Covers::extract_cover(audio_file_source);
		}
	}

	// check for path in library dir
	if(m->pixmap.isNull())
	{
		LOCK_GUARD(mutex_io)
		QString local_path = m->cl.local_path();
		if(FileUtils::exists(local_path))
		{
			m->pixmap = QPixmap(local_path);
		}
	}

	emit sig_finished();
}
