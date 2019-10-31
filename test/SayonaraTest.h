#ifndef SAYONARA_TEST_H
#define SAYONARA_TEST_H

#include <QTest>
#include <QDebug>
#include <QObject>

namespace Test
{
	class Base : public QObject
	{
		Q_OBJECT

	private:
		QString mTmpPath;

	public:
		Base(const QString& test_name);
		virtual ~Base() override;

		QString temp_path() const;
		QString temp_path(const QString& append) const;
	};
}

#endif // SAYONARA_TEST_H
