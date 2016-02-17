#ifndef ASXPARSER_H
#define ASXPARSER_H

#include "AbstractPlaylistParser.h"
#include <QDomNode>

/**
 * @brief The ASXParser class
 * @ingroup PlaylistParser
 */
class ASXParser : public AbstractPlaylistParser
{
public:
	ASXParser(const QString& filename);

private:
	virtual void parse() override;
	QString parse_ref_node(QDomNode node);
};

#endif // ASXPARSER_H
