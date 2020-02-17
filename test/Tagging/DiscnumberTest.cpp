#include <QTest>
#include "AbstractTaggingTest.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"

class DiscnumberTest : public AbstractTaggingTest
{
	Q_OBJECT

public:
	DiscnumberTest() :
		AbstractTaggingTest("DiscnumberTest")
	{}

private:
	void run_test(const QString& filename) override;

private slots:
	void id3_test();
	void xiph_test();
};


void DiscnumberTest::run_test(const QString& filename)
{
	QString album_artist = QString::fromUtf8("Motörhead фыва");
	MetaData md(filename);
	MetaData md2(filename);

	Tagging::Utils::getMetaDataOfFile(md);
	QVERIFY(md.discnumber() == 5);

	md.setDiscnumber(1);
	md.setDiscCount(2);
	Tagging::Utils::setMetaDataOfFile(md);
	QVERIFY(md.discnumber() == 1);
	QVERIFY(md.discCount() == 2);

	Tagging::Utils::getMetaDataOfFile(md2);
	qDebug() << "Expect 1, get " << md2.discnumber();
	QVERIFY(md2.discnumber() == 1);

	qDebug() << "Expect 2, get " << md2.discCount();
	QVERIFY(md2.discCount() == 2);


	md.setDiscnumber(8);
	md.setDiscCount(9);
	Tagging::Utils::setMetaDataOfFile(md);
	QVERIFY(md.discnumber() == 8);
	QVERIFY(md.discCount() == 9);

	Tagging::Utils::getMetaDataOfFile(md2);
	qDebug() << "Expect 8, get " << md2.discnumber();
	QVERIFY(md2.discnumber() == 8);

	qDebug() << "Expect 9, get " << md2.discCount();
	QVERIFY(md2.discCount() == 9);

	md.setDiscnumber(10);
	md.setDiscCount(12);
	Tagging::Utils::setMetaDataOfFile(md);

	Tagging::Utils::getMetaDataOfFile(md2);

	qDebug() << "Expect 10, get " << md2.discnumber();
	QVERIFY(md2.discnumber() == 10);

	qDebug() << "Expect 12, get " << md2.discCount();
	QVERIFY(md2.discCount() == 12);
}

void DiscnumberTest::id3_test()
{
	AbstractTaggingTest::id3_test();
}

void DiscnumberTest::xiph_test()
{
	AbstractTaggingTest::xiph_test();
}

QTEST_GUILESS_MAIN(DiscnumberTest)

#include "DiscnumberTest.moc"
