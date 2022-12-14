#ifndef DIRECTORYFILESCANNER_H
#define DIRECTORYFILESCANNER_H

#include "Utils/Pimpl.h"

#include <QObject>

class MetaDataList;
class QStringList;
namespace Directory
{
	class MetaDataScanner :
		public QObject
	{
		Q_OBJECT
		PIMPL(MetaDataScanner)

		signals:
			void sigFinished();
			void sigCurrentProcessedPathChanged(const QString& path);

		public:
			explicit MetaDataScanner(const QStringList& files, bool recursive, QObject* parent=nullptr);
			~MetaDataScanner() override;

			MetaDataList metadata() const;
			QStringList files() const;

		public slots:
			void start();
	};
}


#endif // DIRECTORYFILESCANNER_H
