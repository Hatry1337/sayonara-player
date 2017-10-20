#include <QTest>
#include "Utils/Tagging/Tagging.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "AbstractTaggingTest.h"

class AlbumArtistTest : public AbstractTaggingTest
{
	Q_OBJECT

private:
	void run_test(const QString& filename) override;

private slots:
	void id3_test();
	void xiph_test();
};


void AlbumArtistTest::run_test(const QString& filename)
{
	QString album_artist = QString::fromUtf8("Motörhead фыва");
	MetaData md(filename);
    Tagging::Util::getMetaDataOfFile(md);

	md.set_album_artist(album_artist);
    Tagging::Util::setMetaDataOfFile(md);

	MetaData md2(filename);
    Tagging::Util::getMetaDataOfFile(md2);

	QString md_album_artist = md.album_artist();
	QString md2_album_artist = md2.album_artist();

	QVERIFY(md_album_artist.compare(album_artist) == 0);
	QVERIFY(md_album_artist.compare(md2_album_artist) == 0);
}

void AlbumArtistTest::id3_test()
{
	AbstractTaggingTest::id3_test();
}

void AlbumArtistTest::xiph_test()
{
	AbstractTaggingTest::xiph_test();
}

QTEST_MAIN(AlbumArtistTest)

#include "AlbumArtistTest.moc"
