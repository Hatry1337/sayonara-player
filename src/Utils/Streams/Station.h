#ifndef ABSTRACTUTILSTREAM_H
#define ABSTRACTUTILSTREAM_H

#include "Utils/Pimpl.h"

class QString;

namespace Cover
{
	class Location;
}

class Station
{
	public:
		Station();
		virtual ~Station();
		Station(const Station& other);

		Station& station(const Station& other);

		virtual QString url() const = 0;
		virtual QString name() const = 0;
};

class Stream :
	public Station
{
	PIMPL(Stream)

	public:
		Stream();
		Stream(const QString& name, const QString& url);
		Stream(const Stream& other);
		~Stream() override;

		Stream& operator=(const Stream& stream);

		QString name() const override;
		void setName(const QString& name);

		QString url() const override;
		void setUrl(const QString& url);
};

class Podcast :
	public Station
{
	PIMPL(Podcast)

	public:
		Podcast();
		Podcast(const QString& name, const QString& url, bool reversed = false);
		Podcast(const Podcast& other);

		~Podcast() override;

		QString name() const override;
		void setName(const QString& name);

		QString url() const override;
		void setUrl(const QString& url);

		bool reversed() const;
		void setReversed(bool b);

		Podcast& operator=(const Podcast& podcast);
};

using StationPtr = std::shared_ptr<Station>;

#endif // ABSTRACTUTILSTREAM_H
